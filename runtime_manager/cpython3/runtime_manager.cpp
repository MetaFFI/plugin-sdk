#include "runtime_manager.h"
#include "module.h"
#include "python_api_wrapper.h"
#include <runtime/cdt.h>
#include <utils/entity_path_parser.h>
#include <filesystem>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include <boost/filesystem.hpp>

namespace
{
std::mutex g_python_runtime_mutex;

#ifndef NDEBUG
#define RUNTIME_LOG(msg) \
	do { std::cerr << __FILE__ << ":" << __LINE__ << " - " << msg << std::endl; } while(0)
#else
#define RUNTIME_LOG(msg) do { } while(0)
#endif
}

// ============================================================================
// GIL MANAGEMENT
// ============================================================================

cpython3_runtime_manager::scoped_gil::scoped_gil()
	: m_state(pPyGILState_Ensure())
{
}

cpython3_runtime_manager::scoped_gil::~scoped_gil()
{
	pPyGILState_Release(m_state);
}

cpython3_runtime_manager::scoped_gil cpython3_runtime_manager::acquire_gil() const
{
	if(!m_is_runtime_loaded)
	{
		throw std::runtime_error("Cannot acquire GIL: Python runtime is not loaded");
	}
	return scoped_gil();
}

void cpython3_runtime_manager::py_object_releaser(cdt_metaffi_handle* handle)
{
	if(handle && handle->handle)
	{
		PyGILState_STATE gstate = pPyGILState_Ensure();
		Py_DECREF(static_cast<PyObject*>(handle->handle));
		pPyGILState_Release(gstate);
	}
}

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR / FACTORY
// ============================================================================

cpython3_runtime_manager::cpython3_runtime_manager(const std::string& python_version)
	: m_python_version(python_version), m_is_runtime_loaded(false), m_is_embedded(false)
{
}

std::shared_ptr<cpython3_runtime_manager> cpython3_runtime_manager::create(const std::string& python_version)
{
	auto manager = std::shared_ptr<cpython3_runtime_manager>(new cpython3_runtime_manager(python_version));
	manager->load_runtime();
	return manager;
}

cpython3_runtime_manager::~cpython3_runtime_manager()
{
	if(m_is_runtime_loaded)
	{
		try
		{
			release_runtime();
		}
		catch(const std::exception&)
		{
			// Do not throw from destructor.
		}
	}
}

std::shared_ptr<cpython3_runtime_manager> cpython3_runtime_manager::load_loaded_cpython3()
{
	// Try to load Python API from an already-loaded library
	// Returns false if Python is not loaded in the process
	std::string detected_version;
	if(!load_python3_api_from_loaded_library(detected_version))
	{
		return nullptr;
	}
	RUNTIME_LOG(std::string("Python already loaded, detected version: ") + detected_version);
	
	auto manager = std::shared_ptr<cpython3_runtime_manager>(new cpython3_runtime_manager(detected_version));
	
	// Python is already initialized (it must be, since it's already loaded by ctypes or similar)
	// Verify this is the case
	if(!pPy_IsInitialized())
	{
		throw std::runtime_error("Python library is loaded but interpreter is not initialized");
	}
	
	// We did not embed Python - it was already running
	manager->m_is_embedded = false;
	
	// Initialize our environment within the existing Python runtime
	// Use PyGILState_Ensure/Release for thread-safe GIL access
	auto gil = pPyGILState_Ensure();
	try
	{
#ifndef _WIN32
		load_python3_variables_from_interpreter();
#endif
		manager->initialize_environment();
	}
	catch(const std::exception& e)
	{
		(void)e;
		pPyGILState_Release(gil);
		throw;
	}
	pPyGILState_Release(gil);
	
	manager->m_is_runtime_loaded = true;
	
	return manager;
}

std::vector<std::string> cpython3_runtime_manager::detect_installed_python3()
{
	return ::detect_installed_python3();
}

void cpython3_runtime_manager::load_runtime()
{
	std::scoped_lock lock(g_python_runtime_mutex, m_mutex);
	
	if(m_is_runtime_loaded)
	{
		return; // Already loaded
	}
	
	try
	{
		// Load Python API for the specified version
		RUNTIME_LOG(std::string("Using Python version: ") + m_python_version);
		RUNTIME_LOG("Loading Python API");
		load_python3_api(m_python_version);
	}
	catch(const std::exception&)
	{
		throw;
	}
	
	// Initialize Python interpreter if not already initialized
	if(!pPy_IsInitialized())
	{
		RUNTIME_LOG("Initializing Python interpreter");
		pPy_InitializeEx(0); // Do not install signal handlers
		// After Py_InitializeEx, the calling thread already holds the GIL
		// We can do our setup work directly without PyGILState_Ensure
		m_is_embedded = true;
		
		try
		{
#ifndef _WIN32
			load_python3_variables_from_interpreter();
#endif
			initialize_environment();
		}
		catch(const std::exception& e)
		{
			(void)e;
			// Release GIL before returning
			// PyEval_SaveThread releases the GIL and returns thread state
			RUNTIME_LOG("Initialization failed - releasing GIL");
			pPyEval_SaveThread();
			throw;
		}
		
		// Release GIL after initialization
		// https://stackoverflow.com/questions/75846775/embedded-python-3-10-py-finalizeex-hangs-deadlock-on-threading-shutdown/
		// PyEval_SaveThread releases the GIL - we don't need to store the thread state
		// since we're not going to restore it (Python will handle cleanup)
		RUNTIME_LOG("Initialization done - releasing GIL");
		pPyEval_SaveThread();
	}
	else
	{
		// Python already initialized by another process/thread
		// Use PyGILState_Ensure/Release for thread-safe GIL access
		RUNTIME_LOG("Python already initialized - using GILState");
		auto gil = pPyGILState_Ensure();
		try
		{
#ifndef _WIN32
			load_python3_variables_from_interpreter();
#endif
			initialize_environment();
		}
		catch(const std::exception& e)
		{
			(void)e;
			pPyGILState_Release(gil);
			throw;
		}
		pPyGILState_Release(gil);
	}
	
	m_is_runtime_loaded = true;
}

void cpython3_runtime_manager::release_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	if(!m_is_runtime_loaded)
	{
		return; // Not loaded
	}
	
	// If we didn't initialize the interpreter, don't finalize it
	if(!m_is_embedded)
	{
		m_is_runtime_loaded = false;
		return;
	}
	
	// Check if Python is still initialized before trying to interact with it
	if(!pPy_IsInitialized())
	{
		m_is_runtime_loaded = false;
		return;
	}
	
	bool gil_acquired = false;
	PyGILState_STATE gstate{};
	
	try
	{
		// Ensure we have the GIL before we interact with Python
		// Note: If Python was already initialized by another thread/process,
		// we need to ensure we have the GIL properly
		gstate = pPyGILState_Ensure();
		gil_acquired = true;
		
		// Import the threading module
		PyObject* threadingModule = pPyImport_ImportModule("threading");
		if(!threadingModule)
		{
			std::string error = check_python_error();
			if(error.empty())
			{
				error = "Failed to import Python threading module";
			}
			throw std::runtime_error(error);
		}
		
		// Get the active count of threads
		PyObject* activeCount = pPyObject_CallMethod(threadingModule, "active_count", NULL);
		if(!activeCount)
		{
			std::string error = check_python_error();
			Py_DECREF(threadingModule);
			if(error.empty())
			{
				error = "Failed to query threading.active_count";
			}
			throw std::runtime_error(error);
		}
		
		long count = pPyLong_AsLong(activeCount);
		Py_DECREF(activeCount);
		
		if(count > 1)
		{
			// More than one thread is active - don't finalize
			Py_DECREF(threadingModule);
			pPyGILState_Release(gstate);
			m_is_runtime_loaded = false;
			return;
		}
		else if(count == 1)
		{
			PyObject* currentThread = pPyObject_CallMethod(threadingModule, "current_thread", NULL);
			PyObject* currentThreadId = pPyObject_GetAttrString(currentThread, "ident");
			long currentThreadIdValue = pPyLong_AsLong(currentThreadId);
			
			PyObject* mainThreadIdObj = pPyObject_CallMethod(threadingModule, "main_thread", NULL);
			PyObject* mainThreadId = pPyObject_GetAttrString(mainThreadIdObj, "ident");
			long mainThreadIdValue = pPyLong_AsLong(mainThreadId);
			
			if(currentThreadIdValue == mainThreadIdValue)
			{
				// Only the main thread is active - safe to finalize
				Py_DECREF(currentThread);
				Py_DECREF(currentThreadId);
				Py_DECREF(mainThreadIdObj);
				Py_DECREF(mainThreadId);
				Py_DECREF(threadingModule);
				
				// Finalize Python
				int res = pPy_FinalizeEx();
				if(res < 0)
				{
					std::string error = check_python_error();
					if(error.empty())
					{
						error = "Py_FinalizeEx reported failure";
					}
					throw std::runtime_error(error);
				}
				// After Py_FinalizeEx, Python is finalized and we should not release GIL
				// The finalization process releases all thread states
				m_is_runtime_loaded = false;
				return;
			}
			else
			{
				Py_DECREF(currentThread);
				Py_DECREF(currentThreadId);
				Py_DECREF(mainThreadIdObj);
				Py_DECREF(mainThreadId);
				Py_DECREF(threadingModule);
			}
		}
		
		// Release GIL only if Python is still initialized
		// If we finalized Python above, we already returned
		if(pPy_IsInitialized())
		{
			pPyGILState_Release(gstate);
		}
		m_is_runtime_loaded = false;
	}
		catch(const std::exception&)
		{
			if(gil_acquired && pPy_IsInitialized())
			{
				pPyGILState_Release(gstate);
		}
		m_is_runtime_loaded = false;
		throw;
	}
}

std::shared_ptr<Module> cpython3_runtime_manager::load_module(const std::string& module_path)
{
	if(!m_is_runtime_loaded)
	{
		throw std::runtime_error("Python runtime is not loaded");
	}
	
	std::lock_guard<std::mutex> lock(m_mutex);
	
	try
	{
		// Create new Module instance (no caching per requirements)
		return std::make_shared<Module>(this, module_path);
	}
	catch(const std::exception& e)
	{
		(void)e;
		throw;
	}
}

bool cpython3_runtime_manager::is_runtime_loaded() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_is_runtime_loaded;
}

const std::string& cpython3_runtime_manager::get_python_version() const
{
	return m_python_version;
}

void cpython3_runtime_manager::add_sys_path(const std::string& path)
{
	if(path.empty())
	{
		throw std::invalid_argument("Path is empty");
	}

	if(!m_is_runtime_loaded)
	{
		throw std::runtime_error("Python runtime is not loaded");
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	auto gil = pPyGILState_Ensure();

	PyObject* sys_path = pPySys_GetObject("path");
	if(!sys_path)
	{
		pPyGILState_Release(gil);
		throw std::runtime_error("Failed to retrieve sys.path");
	}

	PyObject* path_pystr = pPyUnicode_FromString(path.c_str());
	if(pPyList_Append(sys_path, path_pystr) == -1)
	{
		Py_DECREF(path_pystr);
		std::string error = check_python_error();
		pPyGILState_Release(gil);
		if(error.empty())
		{
			error = "Failed to append path to sys.path";
		}
		throw std::runtime_error(error);
	}
	Py_DECREF(path_pystr);

	pPyGILState_Release(gil);
}

void cpython3_runtime_manager::import_module(const std::string& module_name)
{
	if(module_name.empty())
	{
		throw std::invalid_argument("Module name is empty");
	}

	if(!m_is_runtime_loaded)
	{
		throw std::runtime_error("Python runtime is not loaded");
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	auto gil = pPyGILState_Ensure();

	PyObject* module = pPyImport_ImportModule(module_name.c_str());
	if(!module)
	{
		std::string error = check_python_error();
		pPyGILState_Release(gil);
		if(error.empty())
		{
			error = "Failed to import module";
		}
		throw std::runtime_error(error);
	}

	Py_DECREF(module);
	pPyGILState_Release(gil);
}

void cpython3_runtime_manager::initialize_environment()
{
	std::string curpath(boost::filesystem::current_path().string());

	PyObject* sys_path = pPySys_GetObject("path");
	if(!sys_path)
	{
		throw std::runtime_error("Failed to retrieve sys.path");
	}

	PyObject* curpath_pystr = pPyUnicode_FromString(curpath.c_str());
	if(pPyList_Append(sys_path, curpath_pystr) == -1)
	{
		Py_DECREF(curpath_pystr);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to append current working directory to sys.path";
		}
		throw std::runtime_error(error);
	}
	Py_DECREF(curpath_pystr);
}

void cpython3_runtime_manager::import_metaffi_package()
{
	if(!is_runtime_loaded())
	{
		throw std::runtime_error("Python runtime not loaded");
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	auto gil = pPyGILState_Ensure();

	// Check for pre-existing Python errors (fail-fast)
	if(pPyErr_Occurred())
	{
		std::string error = check_python_error();
		pPyGILState_Release(gil);
		throw std::runtime_error("Python error before importing metaffi: " + error);
	}

	// Import metaffi package with special flag to indicate loading context
	const char* script = R"(
import sys
sys.__loading_within_xllr_python3 = True
from metaffi import *
del sys.__loading_within_xllr_python3
)";

	int result = pPyRun_SimpleString(script);
	if(result != 0 || pPyErr_Occurred())
	{
		std::string error = check_python_error();
		pPyGILState_Release(gil);
		throw std::runtime_error("Failed to import metaffi package. Did you pip install metaffi-api? Error: " + error);
	}

	pPyGILState_Release(gil);
}

std::string cpython3_runtime_manager::check_python_error() const
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
