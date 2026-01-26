#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_object.h"
#include "py_utils.h"
#include "runtime_manager.h"

// Forward declarations for type checking
class py_bool;
class py_int;
class py_float;
class py_str;
class py_bytes;
class py_list;
class py_tuple;

py_object::py_object(cpython3_runtime_manager& runtime)
	: m_runtime(runtime), instance(nullptr)
{
}

py_object::py_object(cpython3_runtime_manager& runtime, PyObject* obj)
	: m_runtime(runtime), instance(obj)
{
	// Assumes GIL is held by caller
	if(instance != nullptr && !Py_IsNone(instance))
	{
		Py_XINCREF(instance);
	}
}

py_object::py_object(py_object&& other) noexcept
	: m_runtime(other.m_runtime), instance(other.instance)
{
	other.instance = nullptr;
}

py_object& py_object::operator=(const py_object& other)
{
	if(instance == other.instance)
	{
		return *this;
	}

	auto gil = m_runtime.acquire_gil();

	if(instance != nullptr && !Py_IsNone(instance))
	{
		Py_XDECREF(instance);
	}

	instance = other.instance;
	if(instance != nullptr && !Py_IsNone(instance))
	{
		Py_XINCREF(instance);
	}
	return *this;
}

py_object::operator PyObject*() const
{
	return instance;
}

std::string py_object::get_type() const
{
	auto gil = m_runtime.acquire_gil();
	return get_object_type(instance);
}

void py_object::inc_ref()
{
	auto gil = m_runtime.acquire_gil();
	if(instance != nullptr && !Py_IsNone(instance))
	{
		Py_XINCREF(instance);
	}
}

void py_object::dec_ref()
{
	auto gil = m_runtime.acquire_gil();
	if(instance != nullptr && !Py_IsNone(instance))
	{
		Py_XDECREF(instance);
	}
}

py_object::~py_object()
{
	if(instance != nullptr && !Py_IsNone(instance))
	{
		// Acquire GIL for destructor
		PyGILState_STATE gstate = pPyGILState_Ensure();
		Py_XDECREF(instance);
		pPyGILState_Release(gstate);
	}
}

PyObject* py_object::get_attribute(const char* name) const
{
	auto gil = m_runtime.acquire_gil();
	PyObject* attr = pPyObject_GetAttrString(instance, name);
	if(!attr)
	{
		std::string err = check_python_error();
		throw std::runtime_error("Failed to get attribute '" + std::string(name) + "': " + err);
	}
	return attr;
}

void py_object::set_attribute(const char* name, PyObject* val)
{
	auto gil = m_runtime.acquire_gil();
	if(pPyObject_SetAttrString(instance, name, val) < 0)
	{
		std::string err = check_python_error();
		throw std::runtime_error("Failed to set attribute '" + std::string(name) + "': " + err);
	}
}

PyObject* py_object::detach()
{
	PyObject* res = instance;
	instance = nullptr;
	return res;
}

bool py_object::is_none() const
{
	return instance == nullptr || Py_IsNone(instance);
}

std::string py_object::get_object_type(PyObject* obj)
{
	// Assumes GIL is held by caller
	if(obj == nullptr)
	{
		throw std::runtime_error("Null PyObject passed to get_object_type");
	}

	PyObject* res_type = pPyObject_Type(obj);
	if(res_type == nullptr)
	{
		std::string err = check_python_error();
		throw std::runtime_error("Failed to get object type: " + err);
	}

	PyObject* res_type_name = pPyObject_GetAttrString(res_type, "__name__");
	Py_XDECREF(res_type);
	if(res_type_name == nullptr)
	{
		std::string err = check_python_error();
		throw std::runtime_error("Failed to get type name: " + err);
	}

	const char* res_type_name_str = pPyUnicode_AsUTF8(res_type_name);
	if(res_type_name_str == nullptr)
	{
		Py_XDECREF(res_type_name);
		std::string err = check_python_error();
		throw std::runtime_error("Failed to convert type name to UTF-8: " + err);
	}

	std::string res(res_type_name_str);
	Py_XDECREF(res_type_name);

	return res;
}

metaffi_type py_object::get_metaffi_type(PyObject* obj)
{
	// Assumes GIL is held by caller
	if(pPyUnicode_Check(obj))
	{
		return metaffi_string8_type;
	}
	else if(pPyBool_Check(obj))  // Must check before PyLong_Check (bool is subclass of int)
	{
		return metaffi_bool_type;
	}
	else if(pPyLong_Check(obj))
	{
		return metaffi_int64_type;
	}
	else if(pPyFloat_Check(obj))
	{
		return metaffi_float64_type;
	}
	else if(pPyList_Check(obj) || pPyTuple_Check(obj))
	{
		return metaffi_array_type;
	}
	else if(pPyBytes_Check(obj))
	{
		return metaffi_uint8_array_type;
	}
	else if(Py_IsNone(obj))
	{
		return metaffi_null_type;
	}
	else
	{
		return metaffi_handle_type;
	}
}
