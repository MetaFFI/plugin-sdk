#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_str.h"
#include "py_utils.h"
#include "runtime_manager.h"
#include <cstring>

py_str::py_str(cpython3_runtime_manager& rt) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyUnicode_FromString("");
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(cpython3_runtime_manager& rt, PyObject* obj) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	if(!py_str::check(obj))
	{
		throw std::runtime_error("Object is not a Python str");
	}
	instance = obj;
	Py_XINCREF(instance);
}

py_str::py_str(py_str&& other) noexcept : py_object(std::move(other))
{
}

py_str& py_str::operator=(const py_str& other)
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

Py_ssize_t py_str::length() const
{
	auto gil = m_runtime.acquire_gil();
	return pPyUnicode_GetLength(instance);
}

py_str::py_str(cpython3_runtime_manager& rt, const char8_t* s) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyUnicode_FromString(reinterpret_cast<const char*>(s));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(cpython3_runtime_manager& rt, const char* s) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyUnicode_FromString(s);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(cpython3_runtime_manager& rt, const char32_t* s) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	uint64_t len = std::char_traits<char32_t>::length(s);

	instance = pPyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, s, static_cast<Py_ssize_t>(len));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(cpython3_runtime_manager& rt, const char16_t* s) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	uint64_t len = std::char_traits<char16_t>::length(s);

	instance = pPyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, s, static_cast<Py_ssize_t>(len));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

metaffi_string8 py_str::to_utf8() const
{
	auto gil = m_runtime.acquire_gil();
	PyObject* utf8 = pPyUnicode_AsUTF8String(instance);
	if(!utf8)
	{
		throw std::runtime_error(check_python_error());
	}

	char* s;
	Py_ssize_t len;
	pPyBytes_AsStringAndSize(utf8, &s, &len);

	// Allocate memory for the metaffi_string8
	metaffi_string8 result = new char8_t[len + 1];

	// Copy the string to the allocated memory
	std::memcpy(result, s, len);
	result[len] = '\0'; // Null-terminate the string

	Py_DECREF(utf8);
	return result;
}

metaffi_string16 py_str::to_utf16() const
{
	auto gil = m_runtime.acquire_gil();
	PyObject* bytes = pPyUnicode_AsUTF16String(instance);
	if(!bytes)
	{
		throw std::runtime_error(check_python_error());
	}

	char* s = pPyBytes_AsString(bytes);
	Py_ssize_t size = pPyBytes_Size(bytes);

	// Allocate memory for the metaffi_string16
	metaffi_string16 result = new char16_t[(size / 2) + 1];

	// Copy the string to the allocated memory (skip BOM)
	std::memcpy(result, s + 2, size - 2);  // Skip 2-byte BOM
	result[(size - 2) / 2] = '\0'; // Null-terminate the string

	Py_DECREF(bytes);
	return result;
}

metaffi_string32 py_str::to_utf32() const
{
	auto gil = m_runtime.acquire_gil();
	PyObject* bytes = pPyUnicode_AsUTF32String(instance);
	if(!bytes)
	{
		throw std::runtime_error(check_python_error());
	}

	char* s = pPyBytes_AsString(bytes);
	Py_ssize_t size = pPyBytes_Size(bytes);

	// Allocate memory for the metaffi_string32
	metaffi_string32 result = new char32_t[(size / 4) + 1];

	// Copy the string to the allocated memory (skip BOM)
	std::memcpy(result, s + 4, size - 4);  // Skip 4-byte BOM
	result[(size - 4) / 4] = '\0'; // Null-terminate the string

	Py_DECREF(bytes);
	return result;
}

py_str::operator std::u8string() const
{
	metaffi_string8 utf8 = to_utf8();
	std::u8string result(utf8);
	delete[] utf8;
	return result;
}

py_str::operator std::u16string() const
{
	metaffi_string16 utf16 = to_utf16();
	std::u16string result(utf16);
	delete[] utf16;
	return result;
}

py_str::operator std::u32string() const
{
	metaffi_string32 utf32 = to_utf32();
	std::u32string result(utf32);
	delete[] utf32;
	return result;
}

py_str::operator std::string() const
{
	auto gil = m_runtime.acquire_gil();
	const char* utf8 = pPyUnicode_AsUTF8(instance);
	if(!utf8)
	{
		throw std::runtime_error(check_python_error());
	}
	return std::string(utf8);
}

bool py_str::check(PyObject* obj)
{
	return pPyUnicode_Check(obj);
}

py_str::py_str(cpython3_runtime_manager& rt, const metaffi_char8 c) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyUnicode_FromStringAndSize(reinterpret_cast<const char*>(c.c), metaffi_char8::num_of_bytes(c.c));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(cpython3_runtime_manager& rt, const metaffi_char16 c) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, reinterpret_cast<const char*>(c.c), metaffi_char16::num_of_bytes(c.c));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(cpython3_runtime_manager& rt, const metaffi_char32 c) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, reinterpret_cast<const char*>(c.c), sizeof(char32_t));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}
