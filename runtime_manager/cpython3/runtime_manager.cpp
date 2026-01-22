#include "runtime_manager.h"
#include "module.h"
#include "python_api_wrapper.h"
#include <utils/entity_path_parser.h>
#include <filesystem>
#include <mutex>
#include <stdexcept>
#include <boost/filesystem.hpp>

cpython3_runtime_manager::cpython3_runtime_manager(const std::string& python_version)
	: m_pythonVersion(python_version), m_isRuntimeLoaded(false), m_isEmbedded(false)
{
}

cpython3_runtime_manager::~cpython3_runtime_manager()
{
	if(m_isRuntimeLoaded)
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

std::vector<std::string> cpython3_runtime_manager::detect_installed_python3()
{
	return ::detect_installed_python3();
}

void cpython3_runtime_manager::load_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	if(m_isRuntimeLoaded)
	{
		return; // Already loaded
	}
	
	try
	{
		// Load Python API for the specified version
		load_python3_api(m_pythonVersion);
	}
	catch(const std::exception&)
	{
		throw;
	}
	
	// Initialize Python interpreter if not already initialized
	if(!pPy_IsInitialized())
	{
		pPy_InitializeEx(0); // Do not install signal handlers
		// After Py_InitializeEx, the calling thread already holds the GIL
		// We can do our setup work directly without PyGILState_Ensure
		m_isEmbedded = true;
		
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
			pPyEval_SaveThread();
			throw;
		}
		
		// Release GIL after initialization
		// https://stackoverflow.com/questions/75846775/embedded-python-3-10-py-finalizeex-hangs-deadlock-on-threading-shutdown/
		// PyEval_SaveThread releases the GIL - we don't need to store the thread state
		// since we're not going to restore it (Python will handle cleanup)
		pPyEval_SaveThread();
	}
	else
	{
		// Python already initialized by another process/thread
		// Use PyGILState_Ensure/Release for thread-safe GIL access
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
	
	m_isRuntimeLoaded = true;
}

void cpython3_runtime_manager::release_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	if(!m_isRuntimeLoaded)
	{
		return; // Not loaded
	}
	
	// If we didn't initialize the interpreter, don't finalize it
	if(!m_isEmbedded)
	{
		m_isRuntimeLoaded = false;
		return;
	}
	
	// Check if Python is still initialized before trying to interact with it
	if(!pPy_IsInitialized())
	{
		m_isRuntimeLoaded = false;
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
			m_isRuntimeLoaded = false;
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
				m_isRuntimeLoaded = false;
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
		m_isRuntimeLoaded = false;
	}
		catch(const std::exception&)
		{
			if(gil_acquired && pPy_IsInitialized())
			{
				pPyGILState_Release(gstate);
		}
		m_isRuntimeLoaded = false;
		throw;
	}
}

std::shared_ptr<Module> cpython3_runtime_manager::load_module(const std::string& module_path)
{
	// Ensure runtime is loaded
	if(!m_isRuntimeLoaded)
	{
		load_runtime();
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
	return m_isRuntimeLoaded;
}

const std::string& cpython3_runtime_manager::get_python_version() const
{
	return m_pythonVersion;
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
	
	const char* metaffi_home = getenv("METAFFI_HOME");
	if(metaffi_home)
	{
		PyObject* metaffi_home_pystr = pPyUnicode_FromString(metaffi_home);
		if(pPyList_Append(sys_path, metaffi_home_pystr) == -1)
		{
			Py_DECREF(metaffi_home_pystr);
			std::string error = check_python_error();
			if(error.empty())
			{
				error = "Failed to append METAFFI_HOME to sys.path";
			}
			throw std::runtime_error(error);
		}
		Py_DECREF(metaffi_home_pystr);
	}
	// Note: METAFFI_HOME is not required for basic runtime operation
	// Only needed if metaffi package needs to be imported
	
	// Import metaffi package (helper function)
	const char* script = R"(
import sys
sys.__loading_within_xllr_python3 = True
from metaffi import *
del sys.__loading_within_xllr_python3
)";
	
	pPyRun_SimpleString(script);
	if(pPyErr_Occurred())
	{
		// Non-fatal - metaffi package may not be installed
		pPyErr_Clear();
	}
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
		PyObject* pstr = pPyObject_Repr(pvalue);
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
