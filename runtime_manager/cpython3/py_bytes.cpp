#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_bytes.h"
#include "py_utils.h"
#include "runtime_manager.h"
#include <cstring>

py_bytes::py_bytes(cpython3_runtime_manager& rt, const char* val, Py_ssize_t size) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyBytes_FromStringAndSize(val, size);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_bytes::py_bytes(cpython3_runtime_manager& rt, const uint8_t* val, Py_ssize_t size) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyBytes_FromStringAndSize(reinterpret_cast<const char*>(val), size);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_bytes::py_bytes(cpython3_runtime_manager& rt, PyObject* obj) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	if(!py_bytes::check(obj))
	{
		throw std::runtime_error("Object is not a Python bytes");
	}
	instance = obj;
	Py_XINCREF(instance);
}

py_bytes::py_bytes(py_bytes&& other) noexcept : py_object(std::move(other))
{
}

py_bytes& py_bytes::operator=(const py_bytes& other)
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

py_bytes::operator const uint8_t*() const
{
	auto gil = m_runtime.acquire_gil();
	return reinterpret_cast<const uint8_t*>(pPyBytes_AsString(instance));
}

py_bytes::operator uint8_t*() const
{
	auto gil = m_runtime.acquire_gil();
	Py_ssize_t sz = pPyBytes_Size(instance);
	auto buf = new uint8_t[sz];
	std::memcpy(buf, pPyBytes_AsString(instance), sz);
	return buf;
}

bool py_bytes::check(PyObject* obj)
{
	return pPyBytes_Check(obj);
}

Py_ssize_t py_bytes::size() const
{
	auto gil = m_runtime.acquire_gil();
	return pPyBytes_Size(instance);
}

uint8_t py_bytes::operator[](int i) const
{
	auto gil = m_runtime.acquire_gil();
	return static_cast<uint8_t>(pPyBytes_AsString(instance)[i]);
}
