#include "entity.h"
#include "python_api_wrapper.h"
#include "gil_guard.h"
#include <utils/scope_guard.hpp>
#include <sstream>
#include <mutex>
#include <algorithm>
#include <stdexcept>
#include <functional>

namespace
{
	bool is_python_runtime_active()
	{
		return pPy_IsInitialized && pPy_IsInitialized() && pPyGILState_Ensure && pPyGILState_Release;
	}

	void organize_arguments(PyObject* params_tuple,
	                        PyObject*& out_params_tuple,
	                        PyObject*& out_kwargs_dict,
	                        bool is_varargs,
	                        bool is_kwargs,
	                        bool& out_params_owned,
	                        const std::function<std::string()>& error_provider)
	{
		out_params_tuple = nullptr;
		out_kwargs_dict = nullptr;
		out_params_owned = false;

		Py_ssize_t all_args_count = pPyTuple_Size(params_tuple);
		if(all_args_count < 0)
		{
			std::string error = error_provider();
			if(error.empty())
			{
				error = "Failed to get arguments tuple size";
			}
			throw std::runtime_error(error);
		}

		if(all_args_count == 0 || (!is_varargs && !is_kwargs))
		{
			out_params_tuple = params_tuple;
			return;
		}

		PyObject* varargs = nullptr;
		if(is_varargs)
		{
			PyObject* last_arg = pPyTuple_GetItem(params_tuple, all_args_count - 1);
			if(last_arg && (pPyList_Check(last_arg) || pPyTuple_Check(last_arg)))
			{
				varargs = last_arg;
			}
			else if(is_kwargs && all_args_count > 1)
			{
				PyObject* before_last = pPyTuple_GetItem(params_tuple, all_args_count - 2);
				if(before_last && (pPyList_Check(before_last) || pPyTuple_Check(before_last)))
				{
					varargs = before_last;
				}
			}
		}

		if(is_kwargs)
		{
			PyObject* last_arg = pPyTuple_GetItem(params_tuple, all_args_count - 1);
			if(last_arg && pPyDict_Check(last_arg))
			{
				out_kwargs_dict = last_arg;
			}
		}

		if(!varargs && !out_kwargs_dict)
		{
			out_params_tuple = params_tuple;
			return;
		}

		Py_ssize_t size_without_varargs_kwargs = all_args_count - (varargs ? 1 : 0) - (out_kwargs_dict ? 1 : 0);
		Py_ssize_t varargs_size = 0;
		if(varargs)
		{
			if(pPyTuple_Check(varargs))
			{
				varargs_size = pPyTuple_Size(varargs);
			}
			else if(pPyList_Check(varargs))
			{
				varargs_size = pPyList_Size(varargs);
			}
			else
			{
				throw std::runtime_error("varargs must be a list or tuple");
			}
		}

		if(varargs_size < 0)
		{
			std::string error = error_provider();
			if(error.empty())
			{
				error = "Failed to get varargs size";
			}
			throw std::runtime_error(error);
		}

		Py_ssize_t new_size = size_without_varargs_kwargs + varargs_size;
		out_params_tuple = pPyTuple_New(new_size);
		if(!out_params_tuple)
		{
			std::string error = error_provider();
			if(error.empty())
			{
				error = "Failed to allocate argument tuple";
			}
			throw std::runtime_error(error);
		}
		out_params_owned = true;

		for(Py_ssize_t i = 0; i < size_without_varargs_kwargs; i++)
		{
			PyObject* cur_arg = pPyTuple_GetItem(params_tuple, i);
			Py_XINCREF(cur_arg);
			if(pPyTuple_SetItem(out_params_tuple, i, cur_arg) != 0)
			{
				throw std::runtime_error("Failed to set argument tuple item");
			}
		}

		if(varargs)
		{
			for(Py_ssize_t i = 0; i < varargs_size; i++)
			{
				PyObject* cur_arg = pPyTuple_Check(varargs) ? pPyTuple_GetItem(varargs, i) : pPyList_GetItem(varargs, i);
				Py_XINCREF(cur_arg);
				if(pPyTuple_SetItem(out_params_tuple, size_without_varargs_kwargs + i, cur_arg) != 0)
				{
					throw std::runtime_error("Failed to set varargs tuple item");
				}
			}
		}
	}

	void incref_types(const std::vector<PyObject*>& types)
	{
		for(auto* type_obj : types)
		{
			Py_XINCREF(type_obj);
		}
	}

	void decref_types(std::vector<PyObject*>& types, bool allow_python_calls)
	{
		if(!allow_python_calls)
		{
			types.clear();
			return;
		}

		for(auto* type_obj : types)
		{
			Py_XDECREF(type_obj);
		}
		types.clear();
	}
}

// CallableEntity implementation

CallableEntity::CallableEntity(PyObject* py_callable,
                               const std::vector<PyObject*>& params_types,
                               const std::vector<PyObject*>& retval_types,
                               bool varargs, bool named_args)
	: m_pyCallable(py_callable), m_paramsTypes(params_types), m_retvalTypes(retval_types),
	  m_isVarargs(varargs), m_isNamedArgs(named_args)
{
	if(m_pyCallable)
	{
		Py_INCREF(m_pyCallable);
	}
	incref_types(m_paramsTypes);
	incref_types(m_retvalTypes);
}

CallableEntity::~CallableEntity()
{
	std::lock_guard<std::mutex> lock(m_mutex);
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
	}
	if(m_pyCallable)
	{
		if(can_call_python)
		{
			Py_DECREF(m_pyCallable);
		}
		m_pyCallable = nullptr;
	}
	decref_types(m_paramsTypes, can_call_python);
	decref_types(m_retvalTypes, can_call_python);
}

CallableEntity::CallableEntity(const CallableEntity& other)
	: m_pyCallable(nullptr), m_paramsTypes(other.m_paramsTypes), m_retvalTypes(other.m_retvalTypes),
	  m_isVarargs(other.m_isVarargs), m_isNamedArgs(other.m_isNamedArgs)
{
	std::lock_guard<std::mutex> lock(other.m_mutex);
	if(other.m_pyCallable)
	{
		m_pyCallable = other.m_pyCallable;
		Py_INCREF(m_pyCallable);
	}
	incref_types(m_paramsTypes);
	incref_types(m_retvalTypes);
}

CallableEntity::CallableEntity(CallableEntity&& other) noexcept
	: m_pyCallable(other.m_pyCallable), m_paramsTypes(std::move(other.m_paramsTypes)),
	  m_retvalTypes(std::move(other.m_retvalTypes)), m_isVarargs(other.m_isVarargs),
	  m_isNamedArgs(other.m_isNamedArgs)
{
	other.m_pyCallable = nullptr;
}

CallableEntity& CallableEntity::operator=(const CallableEntity& other)
{
	if(this != &other)
	{
		std::unique_lock<std::mutex> lock1(m_mutex, std::defer_lock);
		std::unique_lock<std::mutex> lock2(other.m_mutex, std::defer_lock);
		std::lock(lock1, lock2);
		
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
		}
		if(m_pyCallable)
		{
			if(can_call_python)
			{
				Py_DECREF(m_pyCallable);
			}
		}
		decref_types(m_paramsTypes, can_call_python);
		decref_types(m_retvalTypes, can_call_python);
		
		m_pyCallable = other.m_pyCallable;
		if(m_pyCallable)
		{
			Py_INCREF(m_pyCallable);
		}
		
		m_paramsTypes = other.m_paramsTypes;
		m_retvalTypes = other.m_retvalTypes;
		incref_types(m_paramsTypes);
		incref_types(m_retvalTypes);
		m_isVarargs = other.m_isVarargs;
		m_isNamedArgs = other.m_isNamedArgs;
	}
	return *this;
}

CallableEntity& CallableEntity::operator=(CallableEntity&& other) noexcept
{
	if(this != &other)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		
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
		}
		if(m_pyCallable)
		{
			if(can_call_python)
			{
				Py_DECREF(m_pyCallable);
			}
		}
		decref_types(m_paramsTypes, can_call_python);
		decref_types(m_retvalTypes, can_call_python);
		
		m_pyCallable = other.m_pyCallable;
		other.m_pyCallable = nullptr;
		
		m_paramsTypes = std::move(other.m_paramsTypes);
		m_retvalTypes = std::move(other.m_retvalTypes);
		m_isVarargs = other.m_isVarargs;
		m_isNamedArgs = other.m_isNamedArgs;
	}
	return *this;
}

const std::vector<PyObject*>& CallableEntity::get_params_types() const
{
	return m_paramsTypes;
}

const std::vector<PyObject*>& CallableEntity::get_retval_types() const
{
	return m_retvalTypes;
}

PyObject* CallableEntity::call(const std::vector<PyObject*>& args)
{
	PyObject* args_tuple = pPyTuple_New((Py_ssize_t)args.size());
	if(!args_tuple)
	{
		throw std::runtime_error("Failed to allocate Python argument tuple");
	}

	for(Py_ssize_t i = 0; i < (Py_ssize_t)args.size(); i++)
	{
		PyObject* arg = args[i];
		Py_XINCREF(arg);
		if(pPyTuple_SetItem(args_tuple, i, arg) != 0)
		{
			Py_DECREF(args_tuple);
			throw std::runtime_error("Failed to set Python argument tuple item");
		}
	}

	PyObject* result = nullptr;
	try
	{
		result = call(args_tuple);
	}
	catch(...)
	{
		Py_DECREF(args_tuple);
		throw;
	}

	Py_DECREF(args_tuple);
	return result;
}

PyObject* CallableEntity::call(PyObject* args_tuple)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if(!m_pyCallable)
	{
		throw std::runtime_error("Python callable is null");
	}

	if(args_tuple && !pPyTuple_Check(args_tuple))
	{
		throw std::runtime_error("Arguments object must be a tuple");
	}

	gil_guard guard;

	if(!pPyCallable_Check(m_pyCallable))
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Python object is not callable";
		}
		throw std::runtime_error(error);
	}

	PyObject* local_args = args_tuple;
	if(!local_args)
	{
		local_args = pPyTuple_New(0);
		if(!local_args)
		{
			std::string error = check_python_error();
			if(error.empty())
			{
				error = "Failed to allocate empty arguments tuple";
			}
			throw std::runtime_error(error);
		}
	}

	PyObject* out_params = nullptr;
	PyObject* out_kwargs = nullptr;
	bool params_owned = false;
	try
	{
		organize_arguments(local_args, out_params, out_kwargs, m_isVarargs, m_isNamedArgs, params_owned,
		                   [this](){ return check_python_error(); });
	}
	catch(...)
	{
		if(!args_tuple)
		{
			Py_DECREF(local_args);
		}
		throw;
	}

	PyObject* result = nullptr;
	if(out_params || out_kwargs)
	{
		result = pPyObject_Call(m_pyCallable, out_params, out_kwargs);
	}
	else
	{
		result = pPyObject_CallObject(m_pyCallable, nullptr);
	}

	if(params_owned && out_params)
	{
		Py_DECREF(out_params);
	}
	if(!args_tuple)
	{
		Py_DECREF(local_args);
	}

	if(!result)
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to call Python callable";
		}
		throw std::runtime_error(error);
	}

	return result;
}

std::string CallableEntity::check_python_error() const
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

// VariableEntity implementation

VariableEntity::VariableEntity(PyObject* attribute_holder, const std::string& attribute_name,
                               const std::vector<PyObject*>& params_types,
                               const std::vector<PyObject*>& retval_types)
	: m_attributeHolder(attribute_holder),
	  m_attributeName(attribute_name),
	  m_paramsTypes(params_types),
	  m_retvalTypes(retval_types)
{
	if(m_attributeHolder)
	{
		Py_INCREF(m_attributeHolder);
	}
	incref_types(m_paramsTypes);
	incref_types(m_retvalTypes);
}

VariableEntity::~VariableEntity()
{
	std::lock_guard<std::mutex> lock(m_mutex);
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
	}
	if(m_attributeHolder)
	{
		if(can_call_python)
		{
			Py_DECREF(m_attributeHolder);
		}
		m_attributeHolder = nullptr;
	}
	decref_types(m_paramsTypes, can_call_python);
	decref_types(m_retvalTypes, can_call_python);
}

VariableEntity::VariableEntity(const VariableEntity& other)
	: m_attributeHolder(nullptr),
	  m_attributeName(other.m_attributeName),
	  m_paramsTypes(other.m_paramsTypes),
	  m_retvalTypes(other.m_retvalTypes)
{
	std::lock_guard<std::mutex> lock(other.m_mutex);
	if(other.m_attributeHolder)
	{
		m_attributeHolder = other.m_attributeHolder;
		Py_INCREF(m_attributeHolder);
	}
	incref_types(m_paramsTypes);
	incref_types(m_retvalTypes);
}

VariableEntity::VariableEntity(VariableEntity&& other) noexcept
	: m_attributeHolder(other.m_attributeHolder),
	  m_attributeName(std::move(other.m_attributeName)),
	  m_paramsTypes(std::move(other.m_paramsTypes)),
	  m_retvalTypes(std::move(other.m_retvalTypes))
{
	other.m_attributeHolder = nullptr;
}

VariableEntity& VariableEntity::operator=(const VariableEntity& other)
{
	if(this != &other)
	{
		std::unique_lock<std::mutex> lock1(m_mutex, std::defer_lock);
		std::unique_lock<std::mutex> lock2(other.m_mutex, std::defer_lock);
		std::lock(lock1, lock2);
		
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
		}
		if(m_attributeHolder)
		{
			if(can_call_python)
			{
				Py_DECREF(m_attributeHolder);
			}
		}
		decref_types(m_paramsTypes, can_call_python);
		decref_types(m_retvalTypes, can_call_python);
		
		m_attributeHolder = other.m_attributeHolder;
		if(m_attributeHolder)
		{
			Py_INCREF(m_attributeHolder);
		}
		
		m_attributeName = other.m_attributeName;
		m_paramsTypes = other.m_paramsTypes;
		m_retvalTypes = other.m_retvalTypes;
		incref_types(m_paramsTypes);
		incref_types(m_retvalTypes);
	}
	return *this;
}

VariableEntity& VariableEntity::operator=(VariableEntity&& other) noexcept
{
	if(this != &other)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		
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
		}
		if(m_attributeHolder)
		{
			if(can_call_python)
			{
				Py_DECREF(m_attributeHolder);
			}
		}
		decref_types(m_paramsTypes, can_call_python);
		decref_types(m_retvalTypes, can_call_python);
		
		m_attributeHolder = other.m_attributeHolder;
		other.m_attributeHolder = nullptr;
		
		m_attributeName = std::move(other.m_attributeName);
		m_paramsTypes = std::move(other.m_paramsTypes);
		m_retvalTypes = std::move(other.m_retvalTypes);
	}
	return *this;
}

const std::vector<PyObject*>& VariableEntity::get_params_types() const
{
	return m_paramsTypes;
}

const std::vector<PyObject*>& VariableEntity::get_retval_types() const
{
	return m_retvalTypes;
}

std::string VariableEntity::check_python_error() const
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

// PythonFunction implementation

PythonFunction::PythonFunction(PyObject* py_function,
                               const std::vector<PyObject*>& params_types,
                               const std::vector<PyObject*>& retval_types,
                               bool varargs, bool named_args)
	: CallableEntity(py_function, params_types, retval_types, varargs, named_args)
{
}

PythonFunction::~PythonFunction() = default;

// PythonMethod implementation

PythonMethod::PythonMethod(PyObject* py_method,
                          const std::vector<PyObject*>& params_types,
                          const std::vector<PyObject*>& retval_types,
                          bool varargs, bool named_args, bool instance_required)
	: CallableEntity(py_method, params_types, retval_types, varargs, named_args),
	  m_instanceRequired(instance_required)
{
}

PythonMethod::~PythonMethod() = default;

// PythonConstructor implementation

PythonConstructor::PythonConstructor(PyObject* py_constructor,
                                    const std::vector<PyObject*>& params_types,
                                    const std::vector<PyObject*>& retval_types,
                                    bool varargs, bool named_args, bool instance_required)
	: CallableEntity(py_constructor, params_types, retval_types, varargs, named_args),
	  m_instanceRequired(instance_required)
{
}

PythonConstructor::~PythonConstructor() = default;

// PythonGlobalGetter implementation

PythonGlobalGetter::PythonGlobalGetter(PyObject* module, const std::string& global_name,
                                      const std::vector<PyObject*>& retval_types)
	: VariableEntity(module, global_name, {}, retval_types)
{
}

PythonGlobalGetter::~PythonGlobalGetter() = default;

PyObject* PythonGlobalGetter::get()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	if(!m_attributeHolder)
	{
		throw std::runtime_error("Attribute holder is null for global getter: " + m_attributeName);
	}
	
	gil_guard guard;
	
	PyObject* value = pPyObject_GetAttrString(m_attributeHolder, m_attributeName.c_str());
	if(!value)
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to get global variable: " + m_attributeName;
		}
		throw std::runtime_error(error);
	}

	return value;
}

void PythonGlobalGetter::set(PyObject* value)
{
	throw std::runtime_error("Setter not supported for GlobalGetter: " + m_attributeName);
}

PyObject* PythonGlobalGetter::get_type() const
{
	if(!m_retvalTypes.empty())
	{
		return m_retvalTypes[0];
	}
	return pPy_None;
}

// PythonGlobalSetter implementation

PythonGlobalSetter::PythonGlobalSetter(PyObject* module, const std::string& global_name,
                                      const std::vector<PyObject*>& params_types)
	: VariableEntity(module, global_name, params_types, {})
{
}

PythonGlobalSetter::~PythonGlobalSetter() = default;

PyObject* PythonGlobalSetter::get()
{
	throw std::runtime_error("Getter not supported for GlobalSetter: " + m_attributeName);
}

void PythonGlobalSetter::set(PyObject* value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	if(!m_attributeHolder)
	{
		throw std::runtime_error("Attribute holder is null for global setter: " + m_attributeName);
	}
	
	if(!value)
	{
		throw std::runtime_error("Value is null for global setter: " + m_attributeName);
	}

	gil_guard guard;

	if(pPyObject_SetAttrString(m_attributeHolder, m_attributeName.c_str(), value) != 0)
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to set global variable: " + m_attributeName;
		}
		throw std::runtime_error(error);
	}
}

PyObject* PythonGlobalSetter::get_type() const
{
	if(!m_paramsTypes.empty())
	{
		return m_paramsTypes[0];
	}
	return pPy_None;
}

// PythonFieldGetter implementation

PythonFieldGetter::PythonFieldGetter(PyObject* class_or_instance, const std::string& field_name,
                                    const std::vector<PyObject*>& retval_types)
	: VariableEntity(class_or_instance, field_name, {}, retval_types)
{
}

PythonFieldGetter::~PythonFieldGetter() = default;

PyObject* PythonFieldGetter::get()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	if(!m_attributeHolder)
	{
		throw std::runtime_error("Attribute holder is null for field getter: " + m_attributeName);
	}
	
	gil_guard guard;
	
	PyObject* value = pPyObject_GetAttrString(m_attributeHolder, m_attributeName.c_str());
	if(!value)
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to get field: " + m_attributeName;
		}
		throw std::runtime_error(error);
	}

	return value;
}

void PythonFieldGetter::set(PyObject* value)
{
	throw std::runtime_error("Setter not supported for FieldGetter: " + m_attributeName);
}

PyObject* PythonFieldGetter::get_type() const
{
	if(!m_retvalTypes.empty())
	{
		return m_retvalTypes[0];
	}
	return pPy_None;
}

// PythonFieldSetter implementation

PythonFieldSetter::PythonFieldSetter(PyObject* class_or_instance, const std::string& field_name,
                                     const std::vector<PyObject*>& params_types)
	: VariableEntity(class_or_instance, field_name, params_types, {})
{
}

PythonFieldSetter::~PythonFieldSetter() = default;

PyObject* PythonFieldSetter::get()
{
	throw std::runtime_error("Getter not supported for FieldSetter: " + m_attributeName);
}

void PythonFieldSetter::set(PyObject* value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	if(!m_attributeHolder)
	{
		throw std::runtime_error("Attribute holder is null for field setter: " + m_attributeName);
	}
	
	if(!value)
	{
		throw std::runtime_error("Value is null for field setter: " + m_attributeName);
	}

	gil_guard guard;

	if(pPyObject_SetAttrString(m_attributeHolder, m_attributeName.c_str(), value) != 0)
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to set field: " + m_attributeName;
		}
		throw std::runtime_error(error);
	}
}

PyObject* PythonFieldSetter::get_type() const
{
	if(!m_paramsTypes.empty())
	{
		return m_paramsTypes[0];
	}
	return pPy_None;
}
