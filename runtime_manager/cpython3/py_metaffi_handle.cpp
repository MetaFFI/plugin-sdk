#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_metaffi_handle.h"
#include "py_utils.h"
#include "runtime_manager.h"
#include <sstream>

// Python3 runtime ID (should match the value used in metaffi)
constexpr uint64_t PYTHON3_RUNTIME_ID = 311;

py_object py_metaffi_handle::extract_pyobject_from_handle(cpython3_runtime_manager& rt, const cdt_metaffi_handle& cdt_handle)
{
	// Assumes GIL is held by caller
	if(cdt_handle.handle == nullptr)
	{
		return py_object(rt, pPy_None);
	}

	if(cdt_handle.runtime_id == PYTHON3_RUNTIME_ID)
	{
		// It's a Python object - just wrap it
		Py_XINCREF(static_cast<PyObject*>(cdt_handle.handle));
		return py_object(rt, static_cast<PyObject*>(cdt_handle.handle));
	}
	else
	{
		// It's from another runtime - create a MetaFFIHandle wrapper
		PyObject* sys_mod_dict = pPyImport_GetModuleDict();
		PyObject* metaffi_handle_mod = pPyMapping_GetItemString(sys_mod_dict, "metaffi");
		
		if(!metaffi_handle_mod)
		{
			pPyErr_Clear();
			rt.import_metaffi_package();
			std::string err_during_import = check_python_error();
			if(!err_during_import.empty())
			{
				throw std::runtime_error("Error importing metaffi package: " + err_during_import);
			}

			metaffi_handle_mod = pPyMapping_GetItemString(sys_mod_dict, "metaffi");
			if(!metaffi_handle_mod)
			{
				throw std::runtime_error("Failed to get metaffi module after import");
			}
		}

		PyObject* instance = pPyObject_CallMethod(metaffi_handle_mod, "MetaFFIHandle", "KKK", 
			cdt_handle.handle, cdt_handle.runtime_id, cdt_handle.release);

		if(instance == nullptr)
		{
			std::string err = check_python_error();
			throw std::runtime_error("Failed to create pythonic MetaFFIHandle object: " + err);
		}

		return py_object(rt, instance);
	}
}

py_metaffi_handle::py_metaffi_handle(cpython3_runtime_manager& rt, PyObject* obj) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	if(!check(obj))
	{
		throw std::runtime_error("Object is not a MetaFFI handle");
	}

	instance = obj;
	Py_INCREF(instance);
}

py_metaffi_handle::py_metaffi_handle(py_metaffi_handle&& other) noexcept : py_object(std::move(other))
{
}

py_metaffi_handle& py_metaffi_handle::operator=(const py_metaffi_handle& other)
{
	if(this->instance == other.instance)
	{
		return *this;
	}

	auto gil = m_runtime.acquire_gil();
	Py_XDECREF(instance);
	instance = other.instance;
	Py_XINCREF(instance);
	return *this;
}

cdt_metaffi_handle* py_metaffi_handle::as_cdt_metaffi_handle() const
{
	auto gil = m_runtime.acquire_gil();
	
	if(instance == pPy_None)
	{
		return nullptr;
	}

	if(!check(instance))
	{
		throw std::runtime_error("Object is not a MetaFFI handle");
	}

	PyObject* val = pPyObject_GetAttrString(instance, "handle");
	PyObject* runtime_id = pPyObject_GetAttrString(instance, "runtime_id");
	PyObject* releaser = pPyObject_GetAttrString(instance, "releaser");

	if(val == nullptr || val == pPy_None || 
	   runtime_id == nullptr || runtime_id == pPy_None || 
	   releaser == nullptr || releaser == pPy_None)
	{
		std::stringstream ss;
		ss << "Failed to get MetaFFI handle attributes: handle=" << val 
		   << ", runtime_id=" << runtime_id << ", releaser=" << releaser;
		
		Py_XDECREF(val);
		Py_XDECREF(runtime_id);
		Py_XDECREF(releaser);
		
		throw std::runtime_error(ss.str());
	}

	cdt_metaffi_handle* cdt_handle = new cdt_metaffi_handle{
		static_cast<metaffi_handle>(pPyLong_AsVoidPtr(val)),
		pPyLong_AsUnsignedLongLong(runtime_id),
		reinterpret_cast<releaser_fptr_t>(pPyLong_AsVoidPtr(releaser))
	};

	Py_XDECREF(val);
	Py_XDECREF(runtime_id);
	Py_XDECREF(releaser);

	return cdt_handle;
}

bool py_metaffi_handle::check(PyObject* obj)
{
	return py_object::get_object_type(obj) == "MetaFFIHandle";
}
