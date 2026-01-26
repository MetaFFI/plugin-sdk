#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_int.h"
#include "py_utils.h"
#include "runtime_manager.h"

py_int::py_int(cpython3_runtime_manager& rt, int8_t val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyLong_FromLongLong(val);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_int::py_int(cpython3_runtime_manager& rt, int16_t val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyLong_FromLongLong(val);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_int::py_int(cpython3_runtime_manager& rt, int32_t val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyLong_FromLongLong(val);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_int::py_int(cpython3_runtime_manager& rt, int64_t val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyLong_FromLongLong(val);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_int::py_int(cpython3_runtime_manager& rt, uint8_t val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyLong_FromUnsignedLongLong(val);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_int::py_int(cpython3_runtime_manager& rt, uint16_t val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyLong_FromUnsignedLongLong(val);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_int::py_int(cpython3_runtime_manager& rt, uint32_t val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyLong_FromUnsignedLongLong(val);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_int::py_int(cpython3_runtime_manager& rt, uint64_t val) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyLong_FromUnsignedLongLong(val);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_int::py_int(cpython3_runtime_manager& rt, PyObject* obj) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	if(!py_int::check(obj))
	{
		throw std::runtime_error("Object is not a Python int");
	}
	instance = obj;
	Py_XINCREF(instance);
}

py_int::py_int(cpython3_runtime_manager& rt, void* ptr) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyLong_FromVoidPtr(ptr);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_int::py_int(py_int&& other) noexcept : py_object(std::move(other))
{
}

py_int& py_int::operator=(const py_int& other)
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

py_int::operator int64_t() const
{
	auto gil = m_runtime.acquire_gil();
	return pPyLong_AsLongLong(instance);
}

py_int::operator uint64_t() const
{
	auto gil = m_runtime.acquire_gil();
	return pPyLong_AsUnsignedLongLong(instance);
}

py_int::operator int8_t() const
{
	auto gil = m_runtime.acquire_gil();
	return static_cast<int8_t>(pPyLong_AsLongLong(instance));
}

py_int::operator int16_t() const
{
	auto gil = m_runtime.acquire_gil();
	return static_cast<int16_t>(pPyLong_AsLongLong(instance));
}

py_int::operator int32_t() const
{
	auto gil = m_runtime.acquire_gil();
	return static_cast<int32_t>(pPyLong_AsLongLong(instance));
}

py_int::operator uint8_t() const
{
	auto gil = m_runtime.acquire_gil();
	return static_cast<uint8_t>(pPyLong_AsUnsignedLongLong(instance));
}

py_int::operator uint16_t() const
{
	auto gil = m_runtime.acquire_gil();
	return static_cast<uint16_t>(pPyLong_AsUnsignedLongLong(instance));
}

py_int::operator uint32_t() const
{
	auto gil = m_runtime.acquire_gil();
	return static_cast<uint32_t>(pPyLong_AsUnsignedLongLong(instance));
}

bool py_int::check(PyObject* obj)
{
	return pPyLong_Check(obj);
}
