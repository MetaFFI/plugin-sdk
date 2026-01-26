#include "cdts_python3_serializer.h"
#include <runtime_manager/cpython3/runtime_manager.h>
#include <runtime_manager/cpython3/py_utils.h>
#include <runtime_manager/cpython3/py_int.h>
#include <runtime_manager/cpython3/py_float.h>
#include <runtime_manager/cpython3/py_bool.h>
#include <runtime_manager/cpython3/py_str.h>
#include <runtime_manager/cpython3/py_bytes.h>
#include <runtime_manager/cpython3/py_list.h>
#include <runtime_manager/cpython3/py_tuple.h>
#include <runtime_manager/cpython3/py_object.h>
#include <runtime/xllr_capi_loader.h>
#include <sstream>
#include <cstring>

namespace metaffi::utils
{

// ============================================================================
// CORE INFRASTRUCTURE
// ============================================================================

// Helper function to extract Python error message (uses dynamically loaded API)
static std::string get_python_error_message()
{
	if(!pPyErr_Occurred())
	{
		return "Unknown Python error";
	}

	PyObject *ptype, *pvalue, *ptraceback;
	pPyErr_Fetch(&ptype, &pvalue, &ptraceback);

	std::ostringstream oss;
	
	if(pvalue)
	{
		PyObject* str_obj = pPyObject_Str(pvalue);
		if(str_obj)
		{
			const char* error_str = pPyUnicode_AsUTF8(str_obj);
			if(error_str)
			{
				oss << error_str;
			}
			Py_DECREF(str_obj);
		}
	}

	if(ptype)
	{
		PyObject* type_str = pPyObject_Str(ptype);
		if(type_str)
		{
			const char* type_name = pPyUnicode_AsUTF8(type_str);
			if(type_name && oss.str().empty())
			{
				oss << type_name;
			}
			Py_DECREF(type_str);
		}
	}

	// Clean up
	Py_XDECREF(ptype);
	Py_XDECREF(pvalue);
	Py_XDECREF(ptraceback);

	std::string result = oss.str();
	return result.empty() ? "Unknown Python error" : result;
}

cdts_python3_serializer::cdts_python3_serializer(cpython3_runtime_manager& runtime, cdts& pcdts)
	: m_runtime(runtime), data(pcdts), current_index(0)
{
	// Constructor - runtime and CDTS references stored, index initialized to 0
}

void cdts_python3_serializer::reset()
{
	current_index = 0;
}

metaffi_size cdts_python3_serializer::get_index() const
{
	return current_index;
}

void cdts_python3_serializer::set_index(metaffi_size index)
{
	check_bounds(index);
	current_index = index;
}

metaffi_size cdts_python3_serializer::size() const
{
	return data.length;
}

bool cdts_python3_serializer::has_more() const
{
	return current_index < data.length;
}

void cdts_python3_serializer::check_bounds(metaffi_size index) const
{
	if(index >= data.length)
	{
		std::ostringstream oss;
		oss << "Index " << index << " out of bounds (CDTS size: " << data.length << ")";
		throw std::out_of_range(oss.str());
	}
}

metaffi_type cdts_python3_serializer::peek_type() const
{
	check_bounds(current_index);
	return data[current_index].type;
}

bool cdts_python3_serializer::is_null() const
{
	check_bounds(current_index);
	return data[current_index].type == metaffi_null_type;
}

// ============================================================================
// SERIALIZATION WITH EXPLICIT TYPE
// ============================================================================

cdts_python3_serializer& cdts_python3_serializer::add(PyObject* obj, metaffi_type target_type)
{
	auto gil = m_runtime.acquire_gil();

	check_bounds(current_index);

	pyobject_to_cdt(obj, data[current_index], target_type);
	current_index++;

	return *this;
}

// ============================================================================
// VALIDATION HELPERS
// ============================================================================

void cdts_python3_serializer::validate_int_range(long long value, metaffi_type target_type)
{
	std::ostringstream oss;
	
	switch(target_type)
	{
		case metaffi_int8_type:
			if(value < -128 || value > 127) {
				oss << "Value " << value << " out of range for int8 [-128, 127]";
				throw std::runtime_error(oss.str());
			}
			break;
			
		case metaffi_int16_type:
			if(value < -32768 || value > 32767) {
				oss << "Value " << value << " out of range for int16 [-32768, 32767]";
				throw std::runtime_error(oss.str());
			}
			break;
			
		case metaffi_int32_type:
			if(value < -2147483648LL || value > 2147483647LL) {
				oss << "Value " << value << " out of range for int32 [-2147483648, 2147483647]";
				throw std::runtime_error(oss.str());
			}
			break;
			
		case metaffi_int64_type:
			// int64 can hold any long long value
			break;
			
		case metaffi_uint8_type:
			if(value < 0 || value > 255) {
				oss << "Value " << value << " out of range for uint8 [0, 255]";
				throw std::runtime_error(oss.str());
			}
			break;
			
		case metaffi_uint16_type:
			if(value < 0 || value > 65535) {
				oss << "Value " << value << " out of range for uint16 [0, 65535]";
				throw std::runtime_error(oss.str());
			}
			break;
			
		case metaffi_uint32_type:
			if(value < 0 || value > 4294967295LL) {
				oss << "Value " << value << " out of range for uint32 [0, 4294967295]";
				throw std::runtime_error(oss.str());
			}
			break;
			
		case metaffi_uint64_type:
			if(value < 0) {
				oss << "Value " << value << " out of range for uint64 [0, 18446744073709551615]";
				throw std::runtime_error(oss.str());
			}
			break;
			
		default:
			oss << "Invalid integer type: " << target_type;
			throw std::runtime_error(oss.str());
	}
}

void cdts_python3_serializer::pyobject_to_cdt(PyObject* obj, cdt& target, metaffi_type target_type)
{
	// GIL is assumed to be held by caller

	if(obj == nullptr)
	{
		throw std::runtime_error("Cannot serialize null PyObject*");
	}

	// Handle None (null) - ignore target_type
	if(obj == pPy_None)
	{
		target.type = metaffi_null_type;
		target.free_required = false;
		return;
	}

	// Handle bool (must check before int, as bool is subclass of int) - ignore target_type
	if(py_bool::check(obj))
	{
		target.type = metaffi_bool_type;
		target.cdt_val.bool_val = (obj == pPy_True) ? 1 : 0;
		target.free_required = false;
		return;
	}

	// Handle int with explicit type
	if(py_int::check(obj))
	{
		long long value = pPyLong_AsLongLong(obj);
		if(value == -1 && pPyErr_Occurred())
		{
			std::string error_msg = get_python_error_message();
			throw std::runtime_error("Failed to convert Python int to long long: " + error_msg);
		}

		// Validate range for target type
		validate_int_range(value, target_type);

		// Set value based on target type
		target.type = target_type;
		target.free_required = false;
		
		switch(target_type)
		{
			case metaffi_int8_type:
				target.cdt_val.int8_val = static_cast<metaffi_int8>(value);
				break;
			case metaffi_int16_type:
				target.cdt_val.int16_val = static_cast<metaffi_int16>(value);
				break;
			case metaffi_int32_type:
				target.cdt_val.int32_val = static_cast<metaffi_int32>(value);
				break;
			case metaffi_int64_type:
				target.cdt_val.int64_val = static_cast<metaffi_int64>(value);
				break;
			case metaffi_uint8_type:
				target.cdt_val.uint8_val = static_cast<metaffi_uint8>(value);
				break;
			case metaffi_uint16_type:
				target.cdt_val.uint16_val = static_cast<metaffi_uint16>(value);
				break;
			case metaffi_uint32_type:
				target.cdt_val.uint32_val = static_cast<metaffi_uint32>(value);
				break;
			case metaffi_uint64_type:
				target.cdt_val.uint64_val = static_cast<metaffi_uint64>(value);
				break;
			default:
			{
				std::ostringstream oss;
				oss << "Invalid integer target type: " << target_type;
				throw std::runtime_error(oss.str());
			}
		}
		return;
	}

	// Handle float with explicit type
	if(py_float::check(obj))
	{
		double value = pPyFloat_AsDouble(obj);
		if(value == -1.0 && pPyErr_Occurred())
		{
			std::string error_msg = get_python_error_message();
			throw std::runtime_error("Failed to convert Python float to double: " + error_msg);
		}

		// Set value based on target type
		target.type = target_type;
		target.free_required = false;
		
		switch(target_type)
		{
			case metaffi_float32_type:
				target.cdt_val.float32_val = static_cast<metaffi_float32>(value);
				break;
			case metaffi_float64_type:
				target.cdt_val.float64_val = value;
				break;
			default:
			{
				std::ostringstream oss;
				oss << "Invalid float target type: " << target_type << " (must be float32 or float64)";
				throw std::runtime_error(oss.str());
			}
		}
		return;
	}

	// Handle string - use target_type to determine encoding
	if(py_str::check(obj))
	{
		switch(target_type)
		{
			case metaffi_string8_type:
			{
				// Convert to UTF-8
				PyObject* utf8_bytes = pPyUnicode_AsUTF8String(obj);
				if(!utf8_bytes)
				{
					std::string error_msg = get_python_error_message();
					throw std::runtime_error("Failed to convert Unicode to UTF-8: " + error_msg);
				}

				Py_ssize_t size;
				char* str_data;
				if(pPyBytes_AsStringAndSize(utf8_bytes, &str_data, &size) == -1)
				{
					Py_DECREF(utf8_bytes);
					std::string error_msg = get_python_error_message();
					throw std::runtime_error("Failed to get UTF-8 string data: " + error_msg);
				}

				// Allocate using xllr
				char8_t* allocated_str = xllr_alloc_string8((const char8_t*)str_data, size);
				Py_DECREF(utf8_bytes);

				if(!allocated_str)
				{
					throw std::runtime_error("xllr_alloc_string8 failed");
				}

				target.type = metaffi_string8_type;
				target.cdt_val.string8_val = allocated_str;
				target.free_required = true;
				break;
			}
			case metaffi_string16_type:
			case metaffi_string32_type:
			{
				std::ostringstream oss;
				oss << "String encoding " << (target_type == metaffi_string16_type ? "UTF-16" : "UTF-32") 
				    << " not yet implemented for Python3 serializer";
				throw std::runtime_error(oss.str());
			}
			default:
			{
				std::ostringstream oss;
				oss << "Invalid string target type: " << target_type;
				throw std::runtime_error(oss.str());
			}
		}
		return;
	}

	// Handle bytes (as uint8 array) - ignore target_type
	if(py_bytes::check(obj))
	{
		Py_ssize_t size;
		char* data;
		if(pPyBytes_AsStringAndSize(obj, &data, &size) == -1)
		{
			std::string error_msg = get_python_error_message();
			throw std::runtime_error("Failed to get bytes data: " + error_msg);
		}

		// Create uint8 array
		target.set_new_array(size, 1, static_cast<metaffi_types>(metaffi_uint8_type));
		cdts& arr = static_cast<cdts&>(target);

		// Copy bytes to array
		for(Py_ssize_t i = 0; i < size; i++)
		{
			arr[i].type = metaffi_uint8_type;
			arr[i].cdt_val.uint8_val = static_cast<metaffi_uint8>(data[i]);
			arr[i].free_required = false;
		}

		return;
	}

	// Handle list/tuple - need element type from target_type
	if(py_list::check(obj) || py_tuple::check(obj))
	{
		// Extract element type from target_type (if it's an array type)
		metaffi_type element_type = metaffi_any_type;
		if(target_type & metaffi_array_type)
		{
			// Extract base type from array type
			element_type = static_cast<metaffi_type>(target_type & ~metaffi_array_type);
		}
		else
		{
			// target_type should indicate the element type directly
			element_type = target_type;
		}
		
		pylist_to_cdt_array(obj, target, element_type);
		return;
	}

	// Handle everything else as handle - ignore target_type
	// Wrap the PyObject in a handle and increment refcount
	Py_INCREF(obj);  // Keep object alive while in handle

	target.type = metaffi_handle_type;
	target.cdt_val.handle_val = new cdt_metaffi_handle();
	target.cdt_val.handle_val->handle = obj;
	target.cdt_val.handle_val->runtime_id = 0;  // Python runtime ID (0 for now)
	target.cdt_val.handle_val->release = cpython3_runtime_manager::py_object_releaser;
	target.free_required = true;
}

PyObject* cdts_python3_serializer::extract_pyobject()
{
	auto gil = m_runtime.acquire_gil();

	check_bounds(current_index);

	PyObject* result = cdt_to_pyobject(data[current_index]);
	current_index++;

	return result;  // New reference
}

PyObject* cdts_python3_serializer::cdt_to_pyobject(const cdt& source)
{
	// GIL is assumed to be held by caller

	switch(source.type)
	{
		case metaffi_null_type:
		{
			Py_INCREF(pPy_None);
			return pPy_None;
		}

		case metaffi_bool_type:
		{
			PyObject* result = source.cdt_val.bool_val ? pPy_True : pPy_False;
			Py_INCREF(result);
			return result;
		}

		case metaffi_int8_type:
		{
			return pPyLong_FromLongLong(source.cdt_val.int8_val);
		}

		case metaffi_int16_type:
		{
			return pPyLong_FromLongLong(source.cdt_val.int16_val);
		}

		case metaffi_int32_type:
		{
			return pPyLong_FromLongLong(source.cdt_val.int32_val);
		}

		case metaffi_int64_type:
		{
			return pPyLong_FromLongLong(source.cdt_val.int64_val);
		}

		case metaffi_uint8_type:
		{
			return pPyLong_FromUnsignedLongLong(source.cdt_val.uint8_val);
		}

		case metaffi_uint16_type:
		{
			return pPyLong_FromUnsignedLongLong(source.cdt_val.uint16_val);
		}

		case metaffi_uint32_type:
		{
			return pPyLong_FromUnsignedLongLong(source.cdt_val.uint32_val);
		}

		case metaffi_uint64_type:
		{
			return pPyLong_FromUnsignedLongLong(source.cdt_val.uint64_val);
		}

		case metaffi_float32_type:
		{
			return pPyFloat_FromDouble(source.cdt_val.float32_val);
		}

		case metaffi_float64_type:
		{
			return pPyFloat_FromDouble(source.cdt_val.float64_val);
		}

		case metaffi_string8_type:
		{
			if(!source.cdt_val.string8_val)
			{
				// Empty string
				return pPyUnicode_FromString("");
			}
			return pPyUnicode_FromString((const char*)source.cdt_val.string8_val);
		}

		case metaffi_string16_type:
		{
			if(!source.cdt_val.string16_val)
			{
				// Empty string
				return pPyUnicode_FromString("");
			}

			// Count UTF-16 code units (find null terminator)
			size_t length = 0;
			while(source.cdt_val.string16_val[length] != 0)
			{
				length++;
			}

			return pPyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND,
			                                 source.cdt_val.string16_val,
			                                 length);
		}

		case metaffi_string32_type:
		{
			if(!source.cdt_val.string32_val)
			{
				// Empty string
				return pPyUnicode_FromString("");
			}

			// Count UTF-32 code units (find null terminator)
			size_t length = 0;
			while(source.cdt_val.string32_val[length] != 0)
			{
				length++;
			}

			return pPyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND,
			                                 source.cdt_val.string32_val,
			                                 length);
		}

		default:
		{
			if(source.type & metaffi_array_type)
			{
				// Array type - convert to Python list
				if(!source.cdt_val.array_val)
				{
					// Empty array
					return pPyList_New(0);
				}

				return cdt_array_to_pylist(*source.cdt_val.array_val);
			}
			else if(source.type == metaffi_handle_type)
			{
				// Extract PyObject from handle
				if(!source.cdt_val.handle_val || !source.cdt_val.handle_val->handle)
				{
					// Null handle
					Py_INCREF(pPy_None);
					return pPy_None;
				}

				PyObject* obj = static_cast<PyObject*>(source.cdt_val.handle_val->handle);
				Py_INCREF(obj);  // Return new reference
				return obj;
			}
			else if(source.type == metaffi_callable_type)
			{
				// For callable, wrap it as a handle-like PyObject
				// In practice, callables are rarely deserialized directly to Python
				// They're typically kept as opaque handles for cross-language calls
				if(!source.cdt_val.callable_val)
				{
					Py_INCREF(pPy_None);
					return pPy_None;
				}

				// Return the function pointer wrapped as a Python capsule
				// This allows Python to hold the callable without calling it
				void* func_ptr = source.cdt_val.callable_val->val;
				PyObject* capsule = pPyCapsule_New(func_ptr, "metaffi.callable", nullptr);
				return capsule;  // New reference
			}
			else
			{
				std::ostringstream oss;
				oss << "Unsupported CDTS type: " << source.type;
				throw std::runtime_error(oss.str());
			}
		}
	}
}

PyObject* cdts_python3_serializer::extract_as_tuple()
{
	auto gil = m_runtime.acquire_gil();

	metaffi_size remaining = data.length - current_index;
	PyObject* tuple = pPyTuple_New(remaining);

	if(!tuple)
	{
		throw std::runtime_error("Failed to create Python tuple");
	}

	try
	{
		for(metaffi_size i = 0; i < remaining; i++)
		{
			PyObject* item = cdt_to_pyobject(data[current_index + i]);
			pPyTuple_SetItem(tuple, i, item);  // Steals reference to item
		}

		current_index = data.length;  // Mark all extracted
		return tuple;  // New reference
	}
	catch(...)
	{
		Py_DECREF(tuple);
		throw;
	}
}

// ============================================================================
// ARRAY SUPPORT
// ============================================================================

// Helper: Detect array dimensions from nested lists
static Py_ssize_t detect_dimensions(PyObject* list)
{
	if(!py_list::check(list) && !py_tuple::check(list))
	{
		return 0;  // Not an array
	}

	Py_ssize_t len = py_list::check(list) ? pPyList_Size(list) : pPyTuple_Size(list);

	if(len == 0)
	{
		return 1;  // Empty 1D array
	}

	// Get first element to check if nested
	PyObject* first = py_list::check(list) ? pPyList_GetItem(list, 0) : pPyTuple_GetItem(list, 0);

	// Check if first element is a nested list (multi-dimensional array)
	if(py_list::check(first) || py_tuple::check(first))
	{
		// Nested array - recurse
		return 1 + detect_dimensions(first);
	}

	// Flat array
	return 1;
}

void cdts_python3_serializer::pylist_to_cdt_array(PyObject* list, cdt& target, metaffi_type element_type)
{
	// GIL assumed to be held

	if(!py_list::check(list) && !py_tuple::check(list))
	{
		throw std::runtime_error("pylist_to_cdt_array: not a list or tuple");
	}

	Py_ssize_t len = py_list::check(list) ? pPyList_Size(list) : pPyTuple_Size(list);

	if(len == 0)
	{
		// Empty array
		target.set_new_array(0, 1, static_cast<metaffi_types>(element_type));
		return;
	}

	// Detect array dimensions
	Py_ssize_t dimensions = detect_dimensions(list);

	// Allocate CDTS array
	target.set_new_array(len, dimensions, static_cast<metaffi_types>(element_type));
	cdts& arr = static_cast<cdts&>(target);

	// Fill array elements
	for(Py_ssize_t i = 0; i < len; i++)
	{
		PyObject* item = py_list::check(list) ? pPyList_GetItem(list, i) : pPyTuple_GetItem(list, i);
		pyobject_to_cdt(item, arr[i], element_type);
	}
}

PyObject* cdts_python3_serializer::cdt_array_to_pylist(const cdts& arr)
{
	// GIL assumed to be held

	PyObject* list = pPyList_New(arr.length);
	if(!list)
	{
		throw std::runtime_error("Failed to create Python list");
	}

	try
	{
		for(metaffi_size i = 0; i < arr.length; i++)
		{
			PyObject* item = cdt_to_pyobject(arr[i]);
			pPyList_SetItem(list, i, item);  // Steals reference
		}

		return list;  // New reference
	}
	catch(...)
	{
		Py_DECREF(list);
		throw;
	}
}

} // namespace metaffi::utils
