#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_bool.h"
#include "py_utils.h"
#include "runtime_manager.h"

py_bool::py_bool(cpython3_runtime_manager& rt, bool val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = val ? pPy_True : pPy_False;
	Py_INCREF(instance);
}

py_bool::py_bool(cpython3_runtime_manager& rt, PyObject* obj) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	if(!py_bool::check(obj))
	{
		throw std::runtime_error("Object is not a Python bool");
	}
	instance = obj;
	Py_XINCREF(instance);
}

py_bool::py_bool(py_bool&& other) noexcept : py_object(std::move(other))
{
}

py_bool& py_bool::operator=(const py_bool& other)
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

py_bool::operator bool() const
{
	auto gil = m_runtime.acquire_gil();
	return instance == pPy_True;
}

py_bool::operator metaffi_bool() const
{
	auto gil = m_runtime.acquire_gil();
	return instance == pPy_True ? 1 : 0;
}

bool py_bool::check(PyObject* obj)
{
	return pPyBool_Check(obj);
}
