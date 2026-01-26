#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_list.h"
#include "py_tuple.h"
#include "py_bytes.h"
#include "py_utils.h"
#include "runtime_manager.h"
#include <sstream>

py_list::py_list(cpython3_runtime_manager& rt, Py_ssize_t size) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	instance = pPyList_New(size);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_list::py_list(cpython3_runtime_manager& rt, PyObject* obj) : py_object(rt)
{
	auto gil = m_runtime.acquire_gil();
	if(!py_list::check(obj))
	{
		std::stringstream ss;
		ss << "Object is not a list. It is " << py_object::get_object_type(obj);
		throw std::runtime_error(ss.str());
	}
	instance = obj;
	Py_XINCREF(instance);
}

py_list::py_list(py_list& other) noexcept : py_object(other.m_runtime)
{
	auto gil = m_runtime.acquire_gil();
	instance = other.instance;
	Py_XINCREF(instance);
}

py_list::py_list(py_list&& other) noexcept : py_object(std::move(other))
{
}

py_list& py_list::operator=(const py_list& other)
{
	if(this == &other)
	{
		return *this;
	}

	auto gil = m_runtime.acquire_gil();
	Py_XDECREF(instance);
	instance = other.instance;
	Py_XINCREF(instance);
	return *this;
}

py_list& py_list::operator=(PyObject* other)
{
	if(instance == other)
	{
		return *this;
	}

	auto gil = m_runtime.acquire_gil();
	Py_XDECREF(instance);
	instance = other;
	Py_XINCREF(instance);
	return *this;
}

PyObject* py_list::operator[](int index)
{
	auto gil = m_runtime.acquire_gil();
	PyObject* item = pPyList_GetItem(instance, index);
	if(!item)
	{
		throw std::runtime_error(check_python_error());
	}
	return item;
}

Py_ssize_t py_list::length() const
{
	auto gil = m_runtime.acquire_gil();
	return pPyList_Size(instance);
}

void py_list::append(PyObject* obj)
{
	auto gil = m_runtime.acquire_gil();
	int res = pPyList_Append(instance, obj);
	if(res == -1)
	{
		throw std::runtime_error(check_python_error());
	}
}

void py_list::set_item(Py_ssize_t index, PyObject* obj)
{
	auto gil = m_runtime.acquire_gil();
	if(pPyList_SetItem(instance, index, obj) == -1)
	{
		throw std::runtime_error(check_python_error());
	}
}

bool py_list::check(PyObject* obj)
{
#ifdef _WIN32
	return pPyList_Check(obj);
#else
	return py_object::get_object_type(obj) == "list";
#endif
}

void py_list::get_metadata(PyObject* obj, bool& out_is_1d_array, bool& out_is_fixed_dimension, 
                          Py_ssize_t& out_size, metaffi_type& out_common_type)
{
	// Assumes GIL is held by caller
	out_size = pPyList_Size(obj);
	out_is_1d_array = true;
	out_is_fixed_dimension = true;
	out_common_type = 0;

#define ARRAY       1
#define NOT_ARRAY   2

	int8_t last_item = 0;
	for(Py_ssize_t i = 0; i < out_size; i++)
	{
		PyObject* item = pPyList_GetItem(obj, i);

		if(i == 0)
		{
			out_common_type = py_object::get_metaffi_type(item);
		}
		else if(out_common_type != 0 && out_common_type != py_object::get_metaffi_type(item))
		{
			out_common_type = 0; // no common type
		}

		if(py_list::check(item) || py_tuple::check(item) || py_bytes::check(item))
		{
			out_is_1d_array = false;

			if(i == 0)
			{
				last_item = ARRAY;
			}
			else if(last_item == NOT_ARRAY)
			{
				out_is_fixed_dimension = false;
				break;
			}
		}
		else
		{
			if(last_item == 0)
			{
				last_item = NOT_ARRAY;
			}
			else if(last_item == ARRAY)
			{
				out_is_fixed_dimension = false;
				break;
			}
		}
	}

	if(out_size == 0)
	{
		out_is_fixed_dimension = true;
	}

	if(!out_is_fixed_dimension)
	{
		out_is_1d_array = false;
	}
	else if(out_is_fixed_dimension && (out_size == 0 || last_item == NOT_ARRAY))
	{
		out_is_1d_array = true;
	}

#undef ARRAY
#undef NOT_ARRAY
}
