#include "module.h"
#include "runtime_manager.h"
#include "entity.h"
#include "python_api_wrapper.h"
#include "gil_guard.h"
#include <utils/entity_path_parser.h>
#include <utils/scope_guard.hpp>
#include <filesystem>
#include <boost/algorithm/string.hpp>
#include <mutex>
#include <stdexcept>

namespace
{
	bool is_python_runtime_active()
	{
		return pPy_IsInitialized && pPy_IsInitialized() && pPyGILState_Ensure && pPyGILState_Release;
	}
}

Module::Module(cpython3_runtime_manager* runtime_manager, const std::string& module_path)
	: m_runtimeManager(runtime_manager), m_modulePath(module_path), m_pyModule(nullptr)
{
	load_python_module();
}

Module::~Module()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(m_pyModule)
	{
		const bool can_call_python = is_python_runtime_active();
		PyGILState_STATE gil_state{};
		auto gil_release = metaffi::utils::scope_guard([&]() {
			if(can_call_python)
			{
				pPyGILState_Release(gil_state);
			}
		});
		if(can_call_python)
		{
			gil_state = pPyGILState_Ensure();
			Py_DECREF(m_pyModule);
		}
		m_pyModule = nullptr;
	}
}

Module::Module(const Module& other)
	: m_runtimeManager(other.m_runtimeManager), m_modulePath(other.m_modulePath), m_pyModule(nullptr)
{
	std::lock_guard<std::mutex> lock(other.m_mutex);
	if(other.m_pyModule)
	{
		m_pyModule = other.m_pyModule;
		Py_INCREF(m_pyModule);
	}
}

Module::Module(Module&& other) noexcept
	: m_runtimeManager(other.m_runtimeManager), m_modulePath(std::move(other.m_modulePath)), m_pyModule(other.m_pyModule)
{
	other.m_pyModule = nullptr;
}

Module& Module::operator=(const Module& other)
{
	if(this != &other)
	{
		std::unique_lock<std::mutex> lock1(m_mutex, std::defer_lock);
		std::unique_lock<std::mutex> lock2(other.m_mutex, std::defer_lock);
		std::lock(lock1, lock2);
		
		if(m_pyModule)
		{
			Py_DECREF(m_pyModule);
		}
		
		m_runtimeManager = other.m_runtimeManager;
		m_modulePath = other.m_modulePath;
		m_pyModule = other.m_pyModule;
		if(m_pyModule)
		{
			Py_INCREF(m_pyModule);
		}
	}
	return *this;
}

Module& Module::operator=(Module&& other) noexcept
{
	if(this != &other)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		
		if(m_pyModule)
		{
			Py_DECREF(m_pyModule);
		}
		
		m_runtimeManager = other.m_runtimeManager;
		m_modulePath = std::move(other.m_modulePath);
		m_pyModule = other.m_pyModule;
		other.m_pyModule = nullptr;
	}
	return *this;
}

const std::string& Module::get_module_path() const
{
	return m_modulePath;
}

std::shared_ptr<Entity> Module::load_entity(
	const std::string& entity_path,
	const std::vector<PyObject*>& params_types,
	const std::vector<PyObject*>& retval_types)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	if(!m_pyModule)
	{
		throw std::runtime_error("Module is not loaded: " + m_modulePath);
	}
	
	// Acquire GIL
	gil_guard guard;
	
	metaffi::utils::entity_path_parser fp(entity_path);
		
		// Parse entity path and create appropriate Entity
		if(fp.contains("callable"))
		{
			// Function, method, or constructor
			std::string callable_path = fp["callable"];
			bool instance_required = fp.contains("instance_required");
			bool varargs = fp.contains("varargs");
			bool named_args = fp.contains("named_args");
			
			// Navigate to the callable
			std::vector<std::string> path_to_object;
			boost::split(path_to_object, callable_path, boost::is_any_of("."));
			
			PyObject* pyobj = m_pyModule;
			Py_INCREF(pyobj); // We'll hold a reference
			
			for(const std::string& step: path_to_object)
			{
				PyObject* temp = pPyObject_GetAttrString(pyobj, step.c_str());
				Py_DECREF(pyobj); // Release previous step
				if(!temp)
				{
					std::string error = check_python_error();
					if(error.empty())
					{
						error = "Failed to resolve callable path step \"" + step + "\" in \"" + callable_path + "\"";
					}
					throw std::runtime_error(error);
				}
				pyobj = temp;
			}
			
			// Verify it's callable
			if(!pPyCallable_Check(pyobj))
			{
				Py_DECREF(pyobj);
				throw std::runtime_error("Resolved Python object is not callable");
			}
			
			// Determine entity type
			std::shared_ptr<Entity> entity;
			if(callable_path.find("__init__") != std::string::npos)
			{
				// Constructor
				entity = std::make_shared<PythonConstructor>(pyobj, params_types, retval_types, varargs, named_args, instance_required);
			}
			else if(instance_required || callable_path.find(".") != std::string::npos)
			{
				// Method
				entity = std::make_shared<PythonMethod>(pyobj, params_types, retval_types, varargs, named_args, instance_required);
			}
			else
			{
				// Function
				entity = std::make_shared<PythonFunction>(pyobj, params_types, retval_types, varargs, named_args);
			}
			
			// pyobj is now owned by entity, don't DECREF
			return entity;
		}
		else if(fp.contains("attribute"))
		{
			// Global variable or field
			std::string attribute_path = fp["attribute"];
			bool instance_required = fp.contains("instance_required");
			bool is_getter = fp.contains("getter");
			bool is_setter = fp.contains("setter");
			
			if(!is_getter && !is_setter)
			{
				throw std::runtime_error("Entity path missing getter/setter specifier: " + entity_path);
			}
			
			// Navigate to attribute holder
			std::vector<std::string> path_to_object;
			boost::split(path_to_object, attribute_path, boost::is_any_of("."));
			
			PyObject* attribute_holder = m_pyModule;
			Py_INCREF(attribute_holder);
			
			std::string attribute_name;
			if(path_to_object.size() > 1)
			{
				// Navigate to parent (e.g., "MyClass.field" -> navigate to MyClass)
				for(size_t i = 0; i < path_to_object.size() - 1; i++)
				{
					PyObject* temp = pPyObject_GetAttrString(attribute_holder, path_to_object[i].c_str());
					Py_DECREF(attribute_holder);
					if(!temp)
					{
						std::string error = check_python_error();
						if(error.empty())
						{
							error = "Failed to resolve attribute path step \"" + path_to_object[i] + "\" in \"" + attribute_path + "\"";
						}
						throw std::runtime_error(error);
					}
					attribute_holder = temp;
				}
				attribute_name = path_to_object.back();
			}
			else
			{
				// Module-level attribute
				attribute_name = path_to_object[0];
			}
			
			// Create appropriate entity
			std::shared_ptr<Entity> entity;
			if(instance_required)
			{
				// Field
				if(is_getter)
				{
					entity = std::make_shared<PythonFieldGetter>(attribute_holder, attribute_name, retval_types);
				}
				else
				{
					entity = std::make_shared<PythonFieldSetter>(attribute_holder, attribute_name, params_types);
				}
			}
			else
			{
				// Global
				if(is_getter)
				{
					entity = std::make_shared<PythonGlobalGetter>(attribute_holder, attribute_name, retval_types);
				}
				else
				{
					entity = std::make_shared<PythonGlobalSetter>(attribute_holder, attribute_name, params_types);
				}
			}
			
			// attribute_holder is now owned by entity, don't DECREF
			return entity;
		}
		else
		{
			throw std::runtime_error("Entity path must contain \"callable\" or \"attribute\": " + entity_path);
		}
}

void Module::unload()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	// Module will be cleaned up in destructor
	// This is a no-op since we use RAII
}

void Module::load_python_module()
{
	gil_guard guard;
	// Normalize the path to handle Windows paths and relative paths correctly
	std::filesystem::path p = std::filesystem::absolute(std::filesystem::path(m_modulePath));

	// Add module directory to sys.path if it's a file path
	if(std::filesystem::exists(p) && std::filesystem::is_regular_file(p))
	{
		std::filesystem::path dir = p.parent_path();
		// Convert to string and normalize separators for Python (use forward slashes)
		std::string dir_str = dir.generic_string();

		PyObject* sysPath = pPySys_GetObject("path");
		PyObject* path = pPyUnicode_FromString(dir_str.c_str());
		if(path != nullptr)
		{
			if(pPySequence_Contains(sysPath, path) == 0)
			{
				pPyList_Append(sysPath, path);
			}
			Py_DECREF(path);
		}

		// Import module by file stem
		m_pyModule = pPyImport_ImportModuleLevel(p.stem().string().c_str(), pPy_None, pPy_None, pPy_None, 0);
	}
	else if(std::filesystem::exists(p) && std::filesystem::is_directory(p))
	{
		// Add parent directory to sys.path and import by directory name
		std::filesystem::path dir = p.parent_path();
		std::string dir_str = dir.generic_string();

		PyObject* sysPath = pPySys_GetObject("path");
		PyObject* path = pPyUnicode_FromString(dir_str.c_str());
		if(path != nullptr)
		{
			if(pPySequence_Contains(sysPath, path) == 0)
			{
				pPyList_Append(sysPath, path);
			}
			Py_DECREF(path);
		}

		m_pyModule = pPyImport_ImportModule(p.filename().string().c_str());
	}
	else
	{
		// Import module by name
		m_pyModule = pPyImport_ImportModule(m_modulePath.c_str());
	}
	
	if(!m_pyModule)
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to import module: " + m_modulePath;
		}
		throw std::runtime_error(error);
	}
	
	// m_pyModule now has a reference - we own it
}

std::string Module::check_python_error() const
{
	if(!pPyErr_Occurred())
	{
		return "";
	}
	
	PyObject* ptype = nullptr;
	PyObject* pvalue = nullptr;
	PyObject* ptraceback = nullptr;
	
	pPyErr_Fetch(&ptype, &pvalue, &ptraceback);
	
	std::string error_msg;
	if(pvalue)
	{
		PyObject* pstr = pPyObject_Str(pvalue);
		if(pstr)
		{
			const char* err_str = pPyUnicode_AsUTF8(pstr);
			if(err_str)
			{
				error_msg = err_str;
			}
			Py_DECREF(pstr);
		}
		Py_DECREF(pvalue);
	}
	
	if(ptype)
	{
		Py_DECREF(ptype);
	}
	if(ptraceback)
	{
		Py_DECREF(ptraceback);
	}
	
	return error_msg;
}
