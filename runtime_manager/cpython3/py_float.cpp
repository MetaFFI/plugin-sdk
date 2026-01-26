#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_float.h"
#include "py_utils.h"
#include "runtime_manager.h"

py_float::py_float(cpython3_runtime_manager& rt, float val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyFloat_FromDouble(static_cast<double>(val));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_float::py_float(cpython3_runtime_manager& rt, double val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyFloat_FromDouble(val);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_float::py_float(cpython3_runtime_manager& rt, PyObject* obj) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	if(!py_float::check(obj))
	{
		throw std::runtime_error("Object is not a Python float");
	}
	instance = obj;
	Py_XINCREF(instance);
}

py_float::py_float(py_float&& other) noexcept : py_object(std::move(other))
{
}

py_float& py_float::operator=(const py_float& other)
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

py_float::operator float() const
{
	auto gil = m_runtime.acquire_gil();
	return static_cast<float>(pPyFloat_AsDouble(instance));
}

py_float::operator double() const
{
	auto gil = m_runtime.acquire_gil();
	return pPyFloat_AsDouble(instance);
}

bool py_float::check(PyObject* obj)
{
	return pPyFloat_Check(obj);
}
