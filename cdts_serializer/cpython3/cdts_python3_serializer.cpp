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
#include <runtime_manager/cpython3/py_metaffi_handle.h>
#include <runtime/xllr_capi_loader.h>
#include <runtime/xcall.h>
#include <cdts_serializer/cpython3/runtime_id.h>
#include <sstream>
#include <cstring>
#include <limits>
#include <utils/logger.hpp>
#include <utils/safe_func.h>


# define throw_py_err(err) \
	METAFFI_ERROR(LOG, "{}", err); \
	throw std::runtime_error(err)

namespace metaffi::utils
{

static auto LOG = metaffi::get_logger("cpython3.serializer");

// ============================================================================
// CORE INFRASTRUCTURE
// ============================================================================

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
				throw_py_err(oss.str());
			}
			break;
			
		case metaffi_int16_type:
			if(value < -32768 || value > 32767) {
				oss << "Value " << value << " out of range for int16 [-32768, 32767]";
				throw_py_err(oss.str());
			}
			break;
			
		case metaffi_int32_type:
			if(value < -2147483648LL || value > 2147483647LL) {
				oss << "Value " << value << " out of range for int32 [-2147483648, 2147483647]";
				throw_py_err(oss.str());
			}
			break;
			
		case metaffi_int64_type:
			// int64 can hold any long long value
			break;
			
		case metaffi_uint8_type:
			if(value < 0 || value > 255) {
				oss << "Value " << value << " out of range for uint8 [0, 255]";
				throw_py_err(oss.str());
			}
			break;
			
		case metaffi_uint16_type:
			if(value < 0 || value > 65535) {
				oss << "Value " << value << " out of range for uint16 [0, 65535]";
				throw_py_err(oss.str());
			}
			break;
			
		case metaffi_uint32_type:
			if(value < 0 || value > 4294967295LL) {
				oss << "Value " << value << " out of range for uint32 [0, 4294967295]";
				throw_py_err(oss.str());
			}
			break;
			
		case metaffi_uint64_type:
			if(value < 0) {
				oss << "Value " << value << " out of range for uint64 [0, 18446744073709551615]";
				throw_py_err(oss.str());
			}
			break;
			
		default:
			oss << "Invalid integer type: " << target_type;
			throw_py_err(oss.str());
	}
}

void cdts_python3_serializer::pyobject_to_cdt(PyObject* obj, cdt& target, metaffi_type target_type)
{
	// GIL is assumed to be held by caller

	if(obj == nullptr)
	{
		throw_py_err("Cannot serialize null PyObject*");
	}

	// Handle metaffi_any_type - detect actual type and serialize accordingly
	// BUT: for lists/arrays, we need special handling to detect element type
	if(target_type == metaffi_any_type)
	{
		// For lists/tuples, we'll handle them specially below (they need element type detection)
		// For other types, detect and recurse
		if(!py_list::check(obj) && !py_tuple::check(obj))
		{
			metaffi_type detected_type = py_object::get_metaffi_type(obj);
			// Recursively call with detected type
			pyobject_to_cdt(obj, target, detected_type);
			return;
		}
		// For lists/tuples, fall through to list handling below which will detect element type
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
			std::string error_msg = check_python_error();
			throw_py_err("Failed to convert Python int to long long: " + error_msg);
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
				throw_py_err(oss.str());
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
			std::string error_msg = check_python_error();
			throw_py_err("Failed to convert Python float to double: " + error_msg);
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
				throw_py_err(oss.str());
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
					std::string error_msg = check_python_error();
					throw_py_err("Failed to convert Unicode to UTF-8: " + error_msg);
				}

				Py_ssize_t size;
				char* str_data;
				if(pPyBytes_AsStringAndSize(utf8_bytes, &str_data, &size) == -1)
				{
					Py_DECREF(utf8_bytes);
					std::string error_msg = check_python_error();
					throw_py_err("Failed to get UTF-8 string data: " + error_msg);
				}

				// Allocate using xllr
				char8_t* allocated_str = xllr_alloc_string8((const char8_t*)str_data, size);
				Py_DECREF(utf8_bytes);

				if(!allocated_str)
				{
					throw_py_err("xllr_alloc_string8 failed");
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
				throw_py_err(oss.str());
			}
			default:
			{
				std::ostringstream oss;
				oss << "Invalid string target type: " << target_type;
				throw_py_err(oss.str());
			}
		}
		return;
	}

	// Handle bytes (as uint8 array) - only when not explicitly requested as handle
	if(py_bytes::check(obj))
	{
		if(target_type != metaffi_handle_type)
		{
			Py_ssize_t size;
			char* data;
			if(pPyBytes_AsStringAndSize(obj, &data, &size) == -1)
			{
				std::string error_msg = check_python_error();
				throw_py_err("Failed to get bytes data: " + error_msg);
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
		// For handle, fall through to handle serialization
	}

	// Handle list/tuple - treat as array unless explicitly requested as handle.
	if(py_list::check(obj) || py_tuple::check(obj))
	{
		if(target_type != metaffi_handle_type)
		{
			// Extract element type from target_type (if it's an array type)
			metaffi_type element_type = metaffi_any_type;
			if(target_type & metaffi_array_type)
			{
				// Extract base type from array type
				element_type = static_cast<metaffi_type>(target_type & ~metaffi_array_type);
				if(element_type == 0 || element_type == metaffi_null_type)
				{
					element_type = metaffi_any_type;
				}
			}
			else if(target_type == metaffi_any_type)
			{
				// Let pylist_to_cdt_array determine common type (if any)
				element_type = metaffi_any_type;
			}
			else
			{
				// target_type should indicate the element type directly
				element_type = target_type;
			}

			pylist_to_cdt_array(obj, target, element_type);
			return;
		}
		// For handle, fall through to handle serialization
	}

	// Handle callable type - extract xcall from Python callable created by make_metaffi_callable
	if(target_type == metaffi_callable_type)
	{
		PyObject* callable_obj = obj;
		PyObject* wrapped_callable = nullptr;

		// If it's a regular Python callable, wrap it into a MetaFFI callable
		if(!pPyObject_HasAttrString(obj, "pxcall_and_context"))
		{
			if(!pPyCallable_Check(obj))
			{
				std::ostringstream oss;
				oss << "Python object is not callable (cannot convert to MetaFFI callable)";
				throw_py_err(oss.str());
			}

			PyObject* metaffi_module = pPyImport_ImportModule("metaffi");
			if(!metaffi_module || pPyErr_Occurred())
			{
				Py_XDECREF(metaffi_module);
				std::string error_msg = check_python_error();
				throw_py_err("Failed to import metaffi module: " + error_msg);
			}

			PyObject* make_callable_func = pPyObject_GetAttrString(metaffi_module, "make_metaffi_callable");
			Py_DECREF(metaffi_module);
			if(!make_callable_func || pPyErr_Occurred())
			{
				Py_XDECREF(make_callable_func);
				std::string error_msg = check_python_error();
				throw_py_err("Failed to get make_metaffi_callable: " + error_msg);
			}

			if(!pPyCallable_Check(make_callable_func))
			{
				Py_DECREF(make_callable_func);
				throw_py_err("make_metaffi_callable is not callable");
			}

			wrapped_callable = pPyObject_CallFunctionObjArgs(make_callable_func, obj, nullptr);
			Py_DECREF(make_callable_func);
			if(!wrapped_callable || pPyErr_Occurred())
			{
				Py_XDECREF(wrapped_callable);
				std::string error_msg = check_python_error();
				throw_py_err("Failed to wrap Python callable using make_metaffi_callable: " + error_msg);
			}

			callable_obj = wrapped_callable;
		}

		// Check if this is a callable created by make_metaffi_callable
		// It should have pxcall_and_context, params_metaffi_types, and retval_metaffi_types attributes
		if(!pPyObject_HasAttrString(callable_obj, "pxcall_and_context"))
		{
			Py_XDECREF(wrapped_callable);
			std::ostringstream oss;
			oss << "Python object does not have pxcall_and_context attribute (not a MetaFFI callable)";
			throw_py_err(oss.str());
		}

		// Get pxcall_and_context attribute
		PyObject* pxcall_and_context_attr = pPyObject_GetAttrString(callable_obj, "pxcall_and_context");
		if(!pxcall_and_context_attr || pPyErr_Occurred())
		{
			if(pxcall_and_context_attr)
			{
				Py_DECREF(pxcall_and_context_attr);
			}
			Py_XDECREF(wrapped_callable);
			std::string error_msg = check_python_error();
			throw_py_err("Failed to get pxcall_and_context attribute: " + error_msg);
		}

		// Get params_metaffi_types and retval_metaffi_types
		PyObject* params_types_attr = pPyObject_GetAttrString(callable_obj, "params_metaffi_types");
		PyObject* retval_types_attr = pPyObject_GetAttrString(callable_obj, "retval_metaffi_types");
		
		if(!params_types_attr || !retval_types_attr || pPyErr_Occurred())
		{
			Py_XDECREF(pxcall_and_context_attr);
			Py_XDECREF(params_types_attr);
			Py_XDECREF(retval_types_attr);
			Py_XDECREF(wrapped_callable);
			std::string error_msg = check_python_error();
			throw_py_err("Failed to get params_metaffi_types or retval_metaffi_types: " + error_msg);
		}

		// Convert pxcall_and_context to xcall* pointer
		// It's stored as an integer (address) in the attribute, which points to the xcall structure
		xcall* pxcall_ptr = nullptr;
		if(pPyLong_Check(pxcall_and_context_attr))
		{
			void* addr = reinterpret_cast<void*>(pPyLong_AsUnsignedLongLong(pxcall_and_context_attr));
			if(pPyErr_Occurred())
			{
				Py_DECREF(pxcall_and_context_attr);
				Py_DECREF(params_types_attr);
				Py_DECREF(retval_types_attr);
				Py_XDECREF(wrapped_callable);
				std::string error_msg = check_python_error();
				throw_py_err("Failed to convert pxcall_and_context to pointer: " + error_msg);
			}
			// The address points to the xcall structure (pxcall_and_context is the first member)
			pxcall_ptr = static_cast<xcall*>(addr);
		}
		else
		{
			Py_DECREF(pxcall_and_context_attr);
			Py_DECREF(params_types_attr);
			Py_DECREF(retval_types_attr);
			Py_XDECREF(wrapped_callable);
			throw_py_err("pxcall_and_context is not a valid pointer");
		}

		// Get parameter and return type counts
		Py_ssize_t params_count = pPyTuple_Size(params_types_attr);
		Py_ssize_t retval_count = pPyTuple_Size(retval_types_attr);

		// Allocate callable structure
		void* callable_mem = xllr_alloc_memory(sizeof(cdt_metaffi_callable));
		if(!callable_mem)
		{
			Py_DECREF(pxcall_and_context_attr);
			Py_DECREF(params_types_attr);
			Py_DECREF(retval_types_attr);
			throw_py_err("Failed to allocate memory for cdt_metaffi_callable");
		}

		cdt_metaffi_callable* callable = new (callable_mem) cdt_metaffi_callable();
		callable->val = static_cast<metaffi_callable>(pxcall_ptr);  // Store xcall pointer as metaffi_callable (void*)
		callable->params_types_length = static_cast<metaffi_int8>(params_count);
		callable->retval_types_length = static_cast<metaffi_int8>(retval_count);

		// Allocate and copy parameter types
		if(params_count > 0)
		{
			callable->parameters_types = static_cast<metaffi_type*>(xllr_alloc_memory(sizeof(metaffi_type) * params_count));
			if(!callable->parameters_types)
			{
				xllr_free_memory(callable_mem);
				Py_DECREF(pxcall_and_context_attr);
				Py_DECREF(params_types_attr);
				Py_DECREF(retval_types_attr);
				Py_XDECREF(wrapped_callable);
				throw_py_err("Failed to allocate memory for callable parameter types");
			}

			for(Py_ssize_t i = 0; i < params_count; i++)
			{
				PyObject* type_obj = pPyTuple_GetItem(params_types_attr, i);
				if(pPyLong_Check(type_obj))
				{
					callable->parameters_types[i] = static_cast<metaffi_type>(pPyLong_AsUnsignedLongLong(type_obj));
				}
				else
				{
					// Try to get .type attribute if it's a metaffi_type_info instance
					PyObject* type_attr = pPyObject_GetAttrString(type_obj, "type");
					if(type_attr && pPyLong_Check(type_attr))
					{
						callable->parameters_types[i] = static_cast<metaffi_type>(pPyLong_AsUnsignedLongLong(type_attr));
						Py_DECREF(type_attr);
					}
					else
					{
						Py_XDECREF(type_attr);
						xllr_free_memory(callable->parameters_types);
						xllr_free_memory(callable_mem);
						Py_DECREF(pxcall_and_context_attr);
						Py_DECREF(params_types_attr);
						Py_DECREF(retval_types_attr);
						Py_XDECREF(wrapped_callable);
						throw_py_err("Failed to extract parameter type");
					}
				}
			}
		}
		else
		{
			callable->parameters_types = nullptr;
		}

		// Allocate and copy return value types
		if(retval_count > 0)
		{
			callable->retval_types = static_cast<metaffi_type*>(xllr_alloc_memory(sizeof(metaffi_type) * retval_count));
			if(!callable->retval_types)
			{
				if(callable->parameters_types)
				{
					xllr_free_memory(callable->parameters_types);
				}
				xllr_free_memory(callable_mem);
				Py_DECREF(pxcall_and_context_attr);
				Py_DECREF(params_types_attr);
				Py_DECREF(retval_types_attr);
				Py_XDECREF(wrapped_callable);
				throw_py_err("Failed to allocate memory for callable return value types");
			}

			for(Py_ssize_t i = 0; i < retval_count; i++)
			{
				PyObject* type_obj = pPyTuple_GetItem(retval_types_attr, i);
				if(pPyLong_Check(type_obj))
				{
					callable->retval_types[i] = static_cast<metaffi_type>(pPyLong_AsUnsignedLongLong(type_obj));
				}
				else
				{
					// Try to get .type attribute if it's a metaffi_type_info instance
					PyObject* type_attr = pPyObject_GetAttrString(type_obj, "type");
					if(type_attr && pPyLong_Check(type_attr))
					{
						callable->retval_types[i] = static_cast<metaffi_type>(pPyLong_AsUnsignedLongLong(type_attr));
						Py_DECREF(type_attr);
					}
					else
					{
						Py_XDECREF(type_attr);
						xllr_free_memory(callable->retval_types);
						if(callable->parameters_types)
						{
							xllr_free_memory(callable->parameters_types);
						}
						xllr_free_memory(callable_mem);
						Py_DECREF(pxcall_and_context_attr);
						Py_DECREF(params_types_attr);
						Py_DECREF(retval_types_attr);
						Py_XDECREF(wrapped_callable);
						throw_py_err("Failed to extract return value type");
					}
				}
			}
		}
		else
		{
			callable->retval_types = nullptr;
		}

		// Clean up Python references
		Py_DECREF(pxcall_and_context_attr);
		Py_DECREF(params_types_attr);
		Py_DECREF(retval_types_attr);
		Py_XDECREF(wrapped_callable);

		// Set target CDT
		target.type = metaffi_callable_type;
		target.cdt_val.callable_val = callable;
		target.free_required = true;
		return;
	}

	// Handle everything else as handle - ignore target_type
	// If this is a MetaFFI handle wrapper, unwrap it to CDT handle with original runtime_id.
	if(py_metaffi_handle::check(obj))
	{
		PyObject* val = pPyObject_GetAttrString(obj, "handle");
		PyObject* runtime_id = pPyObject_GetAttrString(obj, "runtime_id");
		PyObject* releaser = pPyObject_GetAttrString(obj, "releaser");

		if(val == nullptr || val == pPy_None ||
		   runtime_id == nullptr || runtime_id == pPy_None ||
		   releaser == nullptr || releaser == pPy_None)
		{
			Py_XDECREF(val);
			Py_XDECREF(runtime_id);
			Py_XDECREF(releaser);
			throw_py_err("Failed to extract MetaFFIHandle attributes");
		}

		void* handle_ptr = pPyLong_AsVoidPtr(val);
		metaffi_uint64 rt_id = pPyLong_AsUnsignedLongLong(runtime_id);
		auto release = reinterpret_cast<releaser_fptr_t>(pPyLong_AsVoidPtr(releaser));

		Py_DECREF(val);
		Py_DECREF(runtime_id);
		Py_DECREF(releaser);

		if(pPyErr_Occurred())
		{
			std::string error_msg = check_python_error();
			throw_py_err("Failed to convert MetaFFIHandle attributes: " + error_msg);
		}

		void* handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
		if(!handle_mem)
		{
			throw_py_err("Failed to allocate memory for cdt_metaffi_handle");
		}

		target.type = metaffi_handle_type;
		target.cdt_val.handle_val = new (handle_mem) cdt_metaffi_handle();
		target.cdt_val.handle_val->handle = handle_ptr;
		target.cdt_val.handle_val->runtime_id = rt_id;
		target.cdt_val.handle_val->release = release;
		target.free_required = true;
		return;
	}

	// Wrap the PyObject in a handle and increment refcount
	Py_INCREF(obj);  // Keep object alive while in handle

	target.type = metaffi_handle_type;
	void* handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
	if(!handle_mem)
	{
		Py_DECREF(obj);  // Release the reference we just added
		throw_py_err("Failed to allocate memory for cdt_metaffi_handle");
	}
	target.cdt_val.handle_val = new (handle_mem) cdt_metaffi_handle();
	target.cdt_val.handle_val->handle = obj;
	target.cdt_val.handle_val->runtime_id = PYTHON3_RUNTIME_ID;
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

// ============================================================================
// CALLABLE CONVERSION HELPERS
// ============================================================================

void cdts_python3_serializer::add_dev_python_api_to_sys_path()
{
	// GIL is assumed to be held by caller

	// Get METAFFI_SOURCE_ROOT environment variable
	const char* metaffi_source_root = metaffi_getenv_alloc("METAFFI_SOURCE_ROOT");
	if(!metaffi_source_root)
	{
		return;  // Not a development machine
	}

	// Get sys.path
	PyObject* sys_module = pPyImport_ImportModule("sys");
	if(!sys_module || pPyErr_Occurred())
	{
		metaffi_free_env((void*)metaffi_source_root);
		Py_XDECREF(sys_module);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to import sys module";
		}
		throw_py_err(error);
	}

	PyObject* sys_path = pPyObject_GetAttrString(sys_module, "path");
	Py_DECREF(sys_module);
	if(!sys_path || pPyErr_Occurred())
	{
		metaffi_free_env((void*)metaffi_source_root);
		Py_XDECREF(sys_path);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to get sys.path";
		}
		throw_py_err(error);
	}

	// Build path: $METAFFI_SOURCE_ROOT/sdk/api/python3
	std::string python3_api_path = std::string(metaffi_source_root) + "/sdk/api/python3";
	metaffi_free_env((void*)metaffi_source_root);

	PyObject* path_str = pPyUnicode_FromString(python3_api_path.c_str());
	if(!path_str || pPyErr_Occurred())
	{
		Py_XDECREF(path_str);
		Py_DECREF(sys_path);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create path string";
		}
		throw_py_err(error);
	}

	// Check if path is already in sys.path
	int contains = pPySequence_Contains(sys_path, path_str);
	if(contains == 0)
	{
		// Path not in sys.path, add it
		int append_result = pPyList_Append(sys_path, path_str);
		if(append_result == -1 || pPyErr_Occurred())
		{
			Py_DECREF(path_str);
			Py_DECREF(sys_path);
			std::string error = check_python_error();
			if(error.empty())
			{
				error = "Failed to append to sys.path";
			}
			throw_py_err(error);
		}
	}

	Py_DECREF(path_str);
	Py_DECREF(sys_path);
}

PyObject* cdts_python3_serializer::create_callable_params_types_tuple(const cdt_metaffi_callable* callable)
{
	// GIL is assumed to be held by caller
	PyObject* params_types_tuple = pPyTuple_New(callable->params_types_length);
	if(!params_types_tuple || pPyErr_Occurred())
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create params_types tuple";
		}
		throw_py_err(error);
	}

	if(callable->parameters_types)
	{
		for(int8_t i = 0; i < callable->params_types_length; i++)
		{
			PyObject* type_obj = pPyLong_FromUnsignedLongLong(callable->parameters_types[i]);
			if(!type_obj || pPyErr_Occurred())
			{
				Py_DECREF(params_types_tuple);
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create param type";
				}
				throw_py_err(error);
			}
			pPyTuple_SetItem(params_types_tuple, i, type_obj);  // Steals reference
		}
	}
	else
	{
		// If parameters_types is NULL, create a tuple with metaffi_any_type for each parameter
		for(int8_t i = 0; i < callable->params_types_length; i++)
		{
			PyObject* type_obj = pPyLong_FromUnsignedLongLong(metaffi_any_type);
			if(!type_obj || pPyErr_Occurred())
			{
				Py_DECREF(params_types_tuple);
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create param type";
				}
				throw_py_err(error);
			}
			pPyTuple_SetItem(params_types_tuple, i, type_obj);  // Steals reference
		}
	}

	return params_types_tuple;  // New reference
}

PyObject* cdts_python3_serializer::create_callable_retval_types_tuple(const cdt_metaffi_callable* callable)
{
	// GIL is assumed to be held by caller
	PyObject* retval_types_tuple = pPyTuple_New(callable->retval_types_length);
	if(!retval_types_tuple || pPyErr_Occurred())
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create retval_types tuple";
		}
		throw_py_err(error);
	}

	if(callable->retval_types)
	{
		for(int8_t i = 0; i < callable->retval_types_length; i++)
		{
			PyObject* type_obj = pPyLong_FromUnsignedLongLong(callable->retval_types[i]);
			if(!type_obj || pPyErr_Occurred())
			{
				Py_DECREF(retval_types_tuple);
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create retval type";
				}
				throw_py_err(error);
			}
			pPyTuple_SetItem(retval_types_tuple, i, type_obj);  // Steals reference
		}
	}
	else
	{
		// If retval_types is NULL, create a tuple with metaffi_any_type for each return value
		for(int8_t i = 0; i < callable->retval_types_length; i++)
		{
			PyObject* type_obj = pPyLong_FromUnsignedLongLong(metaffi_any_type);
			if(!type_obj || pPyErr_Occurred())
			{
				Py_DECREF(retval_types_tuple);
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create retval type";
				}
				throw_py_err(error);
			}
			pPyTuple_SetItem(retval_types_tuple, i, type_obj);  // Steals reference
		}
	}

	return retval_types_tuple;  // New reference
}

PyObject* cdts_python3_serializer::create_callable_from_lambda(void* pxcall, void* context, PyObject* params_types, PyObject* retval_types)
{
	// GIL is assumed to be held by caller

	// If this is a development machine (i.e. METAFFI_SOURCE_ROOT is set),
	// then add to sys.path $METAFFI_SOURCE_ROOT/sdk/api/python3
	add_dev_python_api_to_sys_path();

	// Import metaffi module
	PyObject* metaffi_module = pPyImport_ImportModule("metaffi");
	if(!metaffi_module || pPyErr_Occurred())
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to import metaffi module";
		}
		throw_py_err(error);
	}

	// Get create_lambda function
	PyObject* create_lambda_func = pPyObject_GetAttrString(metaffi_module, "create_lambda");
	Py_DECREF(metaffi_module);
	if(!create_lambda_func || pPyErr_Occurred())
	{
		Py_XDECREF(create_lambda_func);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to get create_lambda function";
		}
		throw_py_err(error);
	}

	if(!pPyCallable_Check(create_lambda_func))
	{
		Py_DECREF(create_lambda_func);
		throw_py_err("create_lambda is not callable");
	}

	// Create PyLong objects for pxcall and context
	PyObject* pxcall_obj = pPyLong_FromVoidPtr(pxcall);
	if(!pxcall_obj || pPyErr_Occurred())
	{
		Py_XDECREF(pxcall_obj);
		Py_DECREF(create_lambda_func);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create pxcall object";
		}
		throw_py_err(error);
	}

	PyObject* context_obj = pPyLong_FromVoidPtr(context);
	if(!context_obj || pPyErr_Occurred())
	{
		Py_XDECREF(context_obj);
		Py_DECREF(pxcall_obj);
		Py_DECREF(create_lambda_func);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create context object";
		}
		throw_py_err(error);
	}

	// Call create_lambda(pxcall, context, params_types, retval_types)
	PyObject* callable_obj = pPyObject_CallFunctionObjArgs(
		create_lambda_func,
		pxcall_obj,
		context_obj,
		params_types,
		retval_types,
		nullptr
	);
	Py_DECREF(pxcall_obj);
	Py_DECREF(context_obj);
	Py_DECREF(create_lambda_func);

	if(!callable_obj || pPyErr_Occurred())
	{
		Py_XDECREF(callable_obj);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create callable from create_lambda";
		}
		throw_py_err(error);
	}

	if(!pPyCallable_Check(callable_obj))
	{
		Py_DECREF(callable_obj);
		throw_py_err("Failed to create callable: result is not callable");
	}

	return callable_obj;  // New reference
}

void cdts_python3_serializer::set_callable_attributes(PyObject* callable_obj, void* pxcall, void* context, PyObject* params_types, PyObject* retval_types)
{
	// GIL is assumed to be held by caller
	// Set params_metaffi_types and retval_metaffi_types attributes
	int result = pPyObject_SetAttrString(callable_obj, "params_metaffi_types", params_types);
	if(result == -1 || pPyErr_Occurred())
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to set params_metaffi_types attribute";
		}

		throw_py_err(error);
	}

	result = pPyObject_SetAttrString(callable_obj, "retval_metaffi_types", retval_types);
	if(result == -1 || pPyErr_Occurred())
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to set retval_metaffi_types attribute";
		}
		throw_py_err(error);
	}

	// Create pxcall_and_context array for the attribute
	PyObject* ctypes_module = pPyImport_ImportModule("ctypes");
	if(!ctypes_module || pPyErr_Occurred())
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to import ctypes module";
		}
		throw_py_err(error);
	}

	PyObject* c_void_p = pPyObject_GetAttrString(ctypes_module, "c_void_p");
	Py_DECREF(ctypes_module);
	if(!c_void_p || pPyErr_Occurred())
	{
		Py_XDECREF(c_void_p);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to get c_void_p from ctypes";
		}
		throw_py_err(error);
	}

	// Create array type: c_void_p * 2
	PyObject* two = pPyLong_FromLong(2);
	PyObject* array_type = pPyNumber_Multiply(c_void_p, two);  // c_void_p * 2
	Py_DECREF(two);

	Py_DECREF(c_void_p);
	if(!array_type || pPyErr_Occurred())
	{
		Py_XDECREF(array_type);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create array type c_void_p * 2";
		}
		throw_py_err(error);
	}

	// Create array instance with pxcall and context
	PyObject* array_args = pPyTuple_New(2);
	if(!array_args || pPyErr_Occurred())
	{
		Py_XDECREF(array_args);
		Py_DECREF(array_type);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create array_args tuple";
		}
		throw_py_err(error);
	}

	PyObject* pxcall_long = pPyLong_FromVoidPtr(pxcall);
	if(!pxcall_long || pPyErr_Occurred())
	{
		Py_XDECREF(pxcall_long);
		Py_DECREF(array_args);
		Py_DECREF(array_type);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create pxcall PyLong";
		}
		throw_py_err(error);
	}

	PyObject* context_long = pPyLong_FromVoidPtr(context);
	if(!context_long || pPyErr_Occurred())
	{
		Py_XDECREF(context_long);
		Py_DECREF(pxcall_long);
		Py_DECREF(array_args);
		Py_DECREF(array_type);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create context PyLong";
		}
		throw_py_err(error);
	}


	PyObject* pxcall_cvp  = pPyObject_CallFunctionObjArgs(c_void_p, pxcall_long, nullptr);
	PyObject* context_cvp = pPyObject_CallFunctionObjArgs(c_void_p, context_long, nullptr);

	int set_result = pPyTuple_SetItem(array_args, 0, pxcall_cvp);  // Steals reference
	if(set_result == -1 || pPyErr_Occurred())
	{
		Py_DECREF(array_args);
		Py_DECREF(array_type);
		Py_DECREF(pxcall_cvp);
		Py_DECREF(context_cvp);

		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to set pxcall in array_args tuple";
		}
		throw_py_err(error);
	}

	set_result = pPyTuple_SetItem(array_args, 1, context_cvp);  // Steals reference
	if(set_result == -1 || pPyErr_Occurred())
	{
		Py_DECREF(array_args);
		Py_DECREF(array_type);
		Py_DECREF(pxcall_cvp);
		Py_DECREF(context_cvp);

		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to set context in array_args tuple";
		}
		throw_py_err(error);
	}

	// Call array_type(array_args) to create the array instance
	PyObject* array_instance = pPyObject_CallObject(array_type, array_args);  // <-- HERE
	Py_DECREF(array_args);
	Py_DECREF(array_type);
	Py_DECREF(pxcall_cvp);
	Py_DECREF(context_cvp);

	
	if(!array_instance || pPyErr_Occurred())
	{
		Py_XDECREF(array_instance);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create ctypes array instance from array_type and array_args";
		}
		throw_py_err(error);
	}

	// Get addressof function
	PyObject* ctypes_module_for_addressof = pPyImport_ImportModule("ctypes");
	if(!ctypes_module_for_addressof || pPyErr_Occurred())
	{
		Py_XDECREF(ctypes_module_for_addressof);
		Py_DECREF(array_instance);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to import ctypes module for addressof";
		}
		throw_py_err(error);
	}

	PyObject* addressof_func = pPyObject_GetAttrString(ctypes_module_for_addressof, "addressof");
	Py_DECREF(ctypes_module_for_addressof);
	if(!addressof_func || pPyErr_Occurred())
	{
		Py_XDECREF(addressof_func);
		Py_DECREF(array_instance);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to get addressof function from ctypes";
		}
		throw_py_err(error);
	}

	PyObject* address_obj = pPyObject_CallFunctionObjArgs(addressof_func, array_instance, nullptr);
	Py_DECREF(addressof_func);
	Py_DECREF(array_instance);
	if(!address_obj || pPyErr_Occurred())
	{
		Py_XDECREF(address_obj);
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to call addressof on array_instance";
		}
		throw_py_err(error);
	}

	result = pPyObject_SetAttrString(callable_obj, "pxcall_and_context", address_obj);
	Py_DECREF(address_obj);
	if(result == -1 || pPyErr_Occurred())
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to set pxcall_and_context attribute";
		}
		throw_py_err(error);
	}
}

PyObject* cdts_python3_serializer::cdt_to_pyobject(const cdt& source)
{
	// GIL is assumed to be held by caller
	METAFFI_DEBUG(LOG, "cdt_to_pyobject: type={}", source.type);

	try
	{
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
			PyObject* result = pPyLong_FromLongLong(source.cdt_val.int8_val);
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create int8 PyLong";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_int16_type:
		{
			PyObject* result = pPyLong_FromLongLong(source.cdt_val.int16_val);
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create int16 PyLong";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_int32_type:
		{
			PyObject* result = pPyLong_FromLongLong(source.cdt_val.int32_val);
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create int32 PyLong";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_int64_type:
		{
			PyObject* result = pPyLong_FromLongLong(source.cdt_val.int64_val);
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create int64 PyLong";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_uint8_type:
		{
			PyObject* result = pPyLong_FromUnsignedLongLong(source.cdt_val.uint8_val);
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create uint8 PyLong";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_uint16_type:
		{
			PyObject* result = pPyLong_FromUnsignedLongLong(source.cdt_val.uint16_val);
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create uint16 PyLong";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_uint32_type:
		{
			PyObject* result = pPyLong_FromUnsignedLongLong(source.cdt_val.uint32_val);
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create uint32 PyLong";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_uint64_type:
		{
			PyObject* result = pPyLong_FromUnsignedLongLong(source.cdt_val.uint64_val);
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create uint64 PyLong";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_float32_type:
		{
			PyObject* result = pPyFloat_FromDouble(source.cdt_val.float32_val);
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create float32 PyFloat";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_float64_type:
		{
			PyObject* result = pPyFloat_FromDouble(source.cdt_val.float64_val);
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create float64 PyFloat";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_string8_type:
		{
			PyObject* result;
			if(!source.cdt_val.string8_val)
			{
				// Empty string
				result = pPyUnicode_FromString("");
			}
			else
			{
				result = pPyUnicode_FromString((const char*)source.cdt_val.string8_val);
			}
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create string8 PyUnicode";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_string16_type:
		{
			PyObject* result;
			if(!source.cdt_val.string16_val)
			{
				// Empty string
				result = pPyUnicode_FromString("");
			}
			else
			{
				// Count UTF-16 code units (find null terminator)
				size_t length = 0;
				while(source.cdt_val.string16_val[length] != 0)
				{
					length++;
				}

				result = pPyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND,
				                                 source.cdt_val.string16_val,
				                                 length);
			}
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create string16 PyUnicode";
				}
				throw_py_err(error);
			}
			return result;
		}

		case metaffi_string32_type:
		{
			PyObject* result;
			if(!source.cdt_val.string32_val)
			{
				// Empty string
				result = pPyUnicode_FromString("");
			}
			else
			{
				// Count UTF-32 code units (find null terminator)
				size_t length = 0;
				while(source.cdt_val.string32_val[length] != 0)
				{
					length++;
				}

				result = pPyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND,
				                                 source.cdt_val.string32_val,
				                                 length);
			}
			if(!result || pPyErr_Occurred())
			{
				std::string error = check_python_error();
				if(error.empty())
				{
					error = "Failed to create string32 PyUnicode";
				}
				throw_py_err(error);
			}
			return result;
		}

		default:
		{
			if(source.type & metaffi_array_type)
			{
				// Array type - convert to Python list
				if(!source.cdt_val.array_val)
				{
					// Empty array
					PyObject* result = pPyList_New(0);
					if(!result || pPyErr_Occurred())
					{
						std::string error = check_python_error();
						if(error.empty())
						{
							error = "Failed to create empty array PyList";
						}
						throw_py_err(error);
					}
					return result;
				}

				metaffi_type element_type = static_cast<metaffi_type>(source.type & ~metaffi_array_type);
				if(element_type == metaffi_uint8_type || element_type == metaffi_int8_type)
				{
					return cdt_array_to_pybytes(*source.cdt_val.array_val, element_type);
				}

				return cdt_array_to_pylist(*source.cdt_val.array_val);
			}
			else if(source.type == metaffi_handle_type)
			{
				if(!source.cdt_val.handle_val)
				{
					Py_INCREF(pPy_None);
					return pPy_None;
				}

				py_object obj = py_metaffi_handle::extract_pyobject_from_handle(m_runtime, *source.cdt_val.handle_val);
				PyObject* raw = obj.detach();
				if(!raw)
				{
					Py_INCREF(pPy_None);
					return pPy_None;
				}
				return raw;
			}
			else if(source.type == metaffi_callable_type)
			{
				METAFFI_DEBUG(LOG, "cdt_to_pyobject: entering metaffi_callable_type case, callable_val={}", source.cdt_val.callable_val);
				// For callable, create a Python callable object that wraps the xcall
				if(!source.cdt_val.callable_val)
				{
					Py_INCREF(pPy_None);
					return pPy_None;
				}

				cdt_metaffi_callable* callable = source.cdt_val.callable_val;
				
				METAFFI_DEBUG(LOG, "cdt_to_pyobject callable: callable={}", static_cast<void*>(callable));
				
				if(!callable)
				{
					METAFFI_DEBUG(LOG, "cdt_to_pyobject callable: callable is NULL!");
					Py_INCREF(pPy_None);
					return pPy_None;
				}
				
				METAFFI_DEBUG(LOG, "cdt_to_pyobject callable: callable->val={}", callable->val);
				
				if(!callable->val)
				{
					METAFFI_DEBUG(LOG, "cdt_to_pyobject callable: callable->val is NULL!");
					Py_INCREF(pPy_None);
					return pPy_None;
				}

				// callable->val is a metaffi_callable (void*), which points to an xcall structure
				// The xcall structure's first member is pxcall_and_context[2], so we can access it directly
				void** pxcall_and_context_array = static_cast<void**>(callable->val);
				if(!pxcall_and_context_array)
				{
					METAFFI_DEBUG(LOG, "cdt_to_pyobject callable: pxcall_and_context_array is NULL!");
					Py_INCREF(pPy_None);
					return pPy_None;
				}

				// Extract pxcall and context from the array
				void* pxcall = pxcall_and_context_array[0];
				void* context = pxcall_and_context_array[1];
				
				// Debug output
				METAFFI_DEBUG(LOG, "cdt_to_pyobject callable: pxcall={}, context={}", pxcall, context);
				METAFFI_DEBUG(LOG, "cdt_to_pyobject callable: params_count={}, retval_count={}",
					static_cast<int>(callable->params_types_length),
					static_cast<int>(callable->retval_types_length));
				
				if(!pxcall)
				{
					METAFFI_DEBUG(LOG, "cdt_to_pyobject callable: pxcall is NULL!");
					Py_INCREF(pPy_None);
					return pPy_None;
				}

				// Create params and retval type tuples using helper functions
				PyObject* params_types = create_callable_params_types_tuple(callable);
				PyObject* retval_types = create_callable_retval_types_tuple(callable);

				// Create callable from lambda
				PyObject* callable_obj = create_callable_from_lambda(pxcall, context, params_types, retval_types);

				// Set attributes
				set_callable_attributes(callable_obj, pxcall, context, params_types, retval_types);

				// Clean up type tuples (callable_obj now owns references via attributes)
				Py_DECREF(params_types);
				Py_DECREF(retval_types);

				return callable_obj;  // New reference
			}
			else
			{
				std::ostringstream oss;
				oss << "Unsupported CDTS type: " << source.type;
				throw_py_err(oss.str());
			}
		}
	}
	}
	catch(...)
	{
		throw;
	}
}

PyObject* cdts_python3_serializer::extract_as_tuple()
{
	auto gil = m_runtime.acquire_gil();

	metaffi_size remaining = data.length - current_index;
	METAFFI_DEBUG(LOG, "extract_as_tuple: remaining={}, current_index={}, data.length={}",
		remaining, current_index, data.length);
	METAFFI_DEBUG(LOG, "extract_as_tuple: data.arr={}", static_cast<void*>(data.arr));
	PyObject* tuple = pPyTuple_New(remaining);

	if(!tuple)
	{
		throw_py_err("Failed to create Python tuple");
	}

	try
	{
		for(metaffi_size i = 0; i < remaining; i++)
		{
			METAFFI_DEBUG(LOG, "extract_as_tuple: processing index {}, current_index={}", i, current_index);
			METAFFI_DEBUG(LOG, "extract_as_tuple: data[{}].type={}", (current_index + i), data[current_index + i].type);
			try
			{
				PyObject* item = cdt_to_pyobject(data[current_index + i]);
				METAFFI_DEBUG(LOG, "extract_as_tuple: cdt_to_pyobject returned {}", static_cast<void*>(item));
				int set_result = pPyTuple_SetItem(tuple, i, item);  // Steals reference to item
				if(set_result == -1 || pPyErr_Occurred())
				{
					std::string error = check_python_error();
					if(error.empty())
					{
						error = "Failed to set item in tuple at index " + std::to_string(i);
					}
					throw_py_err(error);
				}
			}
			catch(const std::exception& e)
			{
				METAFFI_DEBUG(LOG, "extract_as_tuple: EXCEPTION at index {}: {}", i, e.what());
				throw;
			}
			catch(...)
			{
				METAFFI_DEBUG(LOG, "extract_as_tuple: UNKNOWN EXCEPTION at index {}", i);
				throw;
			}
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

static void validate_integer_range_strict(long long value, metaffi_type target_type)
{
	switch(target_type)
	{
		case metaffi_int8_type:
			if(value < -128 || value > 127) { throw std::runtime_error("Value out of range for int8"); }
			break;
		case metaffi_int16_type:
			if(value < -32768 || value > 32767) { throw std::runtime_error("Value out of range for int16"); }
			break;
		case metaffi_int32_type:
			if(value < -2147483648LL || value > 2147483647LL) { throw std::runtime_error("Value out of range for int32"); }
			break;
		case metaffi_int64_type:
			break;
		case metaffi_uint8_type:
			if(value < 0 || value > 255) { throw std::runtime_error("Value out of range for uint8"); }
			break;
		case metaffi_uint16_type:
			if(value < 0 || value > 65535) { throw std::runtime_error("Value out of range for uint16"); }
			break;
		case metaffi_uint32_type:
			if(value < 0 || value > 4294967295LL) { throw std::runtime_error("Value out of range for uint32"); }
			break;
		default:
			break;
	}
}

static bool is_fast_1d_primitive_type(metaffi_type t)
{
	switch(t)
	{
		case metaffi_float64_type:
		case metaffi_float32_type:
		case metaffi_int8_type:
		case metaffi_uint8_type:
		case metaffi_int16_type:
		case metaffi_uint16_type:
		case metaffi_int32_type:
		case metaffi_uint32_type:
		case metaffi_int64_type:
		case metaffi_uint64_type:
		case metaffi_bool_type:
			return true;
		default:
			return false;
	}
}

static bool try_fill_1d_primitive_cdts_from_pysequence(PyObject* list, cdts& arr, metaffi_type resolved_type)
{
	if(!is_fast_1d_primitive_type(resolved_type))
	{
		return false;
	}

	if(arr.length == 0)
	{
		return true;
	}

	for(metaffi_size i = 0; i < arr.length; i++)
	{
		PyObject* item = py_list::check(list) ? pPyList_GetItem(list, (Py_ssize_t)i) : pPyTuple_GetItem(list, (Py_ssize_t)i);
		cdt& dst = arr[i];
		dst.type = resolved_type;
		dst.free_required = false;

		switch(resolved_type)
		{
			case metaffi_float64_type:
			{
				double v = pPyFloat_AsDouble(item);
				if(v == -1.0 && pPyErr_Occurred())
				{
					throw std::runtime_error("Failed converting element to float64");
				}
				dst.cdt_val.float64_val = (metaffi_float64)v;
				break;
			}
			case metaffi_float32_type:
			{
				double v = pPyFloat_AsDouble(item);
				if(v == -1.0 && pPyErr_Occurred())
				{
					throw std::runtime_error("Failed converting element to float32");
				}
				dst.cdt_val.float32_val = (metaffi_float32)v;
				break;
			}
			case metaffi_int8_type:
			case metaffi_int16_type:
			case metaffi_int32_type:
			case metaffi_int64_type:
			case metaffi_uint8_type:
			case metaffi_uint16_type:
			case metaffi_uint32_type:
			{
				long long v = pPyLong_AsLongLong(item);
				if(v == -1 && pPyErr_Occurred())
				{
					throw std::runtime_error("Failed converting element to integer");
				}
				validate_integer_range_strict(v, resolved_type);

				switch(resolved_type)
				{
					case metaffi_int8_type:   dst.cdt_val.int8_val = (metaffi_int8)v; break;
					case metaffi_int16_type:  dst.cdt_val.int16_val = (metaffi_int16)v; break;
					case metaffi_int32_type:  dst.cdt_val.int32_val = (metaffi_int32)v; break;
					case metaffi_int64_type:  dst.cdt_val.int64_val = (metaffi_int64)v; break;
					case metaffi_uint8_type:  dst.cdt_val.uint8_val = (metaffi_uint8)v; break;
					case metaffi_uint16_type: dst.cdt_val.uint16_val = (metaffi_uint16)v; break;
					case metaffi_uint32_type: dst.cdt_val.uint32_val = (metaffi_uint32)v; break;
					default: break;
				}
				break;
			}
			case metaffi_uint64_type:
			{
				unsigned long long v = pPyLong_AsUnsignedLongLong(item);
				if(v == (unsigned long long)-1 && pPyErr_Occurred())
				{
					throw std::runtime_error("Failed converting element to uint64");
				}
				dst.cdt_val.uint64_val = (metaffi_uint64)v;
				break;
			}
			case metaffi_bool_type:
			{
				int truth = pPyObject_IsTrue(item);
				if(truth < 0)
				{
					throw std::runtime_error("Failed converting element to bool");
				}
				dst.cdt_val.bool_val = truth ? 1 : 0;
				break;
			}
			default:
				return false;
		}
	}

	return true;
}

void cdts_python3_serializer::pylist_to_cdt_array(PyObject* list, cdt& target, metaffi_type element_type)
{
	// GIL assumed to be held

	if(!py_list::check(list) && !py_tuple::check(list))
	{
		throw_py_err("pylist_to_cdt_array: not a list or tuple");
	}

	Py_ssize_t len = py_list::check(list) ? pPyList_Size(list) : pPyTuple_Size(list);

	if(len == 0)
	{
		// Empty array - use element_type if valid, otherwise use metaffi_any_type
		metaffi_type final_type = (element_type == 0 || element_type == metaffi_null_type) ? metaffi_any_type : element_type;
		target.set_new_array(0, 1, static_cast<metaffi_types>(final_type));
		return;
	}

	// Determine common type and fixed dimensions
	bool is_1d_array = true;
	bool is_fixed_dimension = true;
	Py_ssize_t size = 0;
	metaffi_type common_type = 0;

	if(py_list::check(list))
	{
		py_list::get_metadata(list, is_1d_array, is_fixed_dimension, size, common_type);
	}
	else
	{
		py_tuple::get_metadata(list, is_1d_array, is_fixed_dimension, size, common_type);
	}

	// Resolve element type
	metaffi_type resolved_type = element_type;
	if(resolved_type == metaffi_any_type || resolved_type == 0 || resolved_type == metaffi_null_type)
	{
		resolved_type = common_type;
	}

	// If common type is unknown or nested arrays, treat as any
	if(resolved_type == 0 || resolved_type == metaffi_null_type || (resolved_type & metaffi_array_type))
	{
		resolved_type = metaffi_any_type;
	}

	// Detect array dimensions (use MIXED_OR_UNKNOWN_DIMENSIONS for ragged/mixed)
	metaffi_int64 dimensions = MIXED_OR_UNKNOWN_DIMENSIONS;
	if(is_fixed_dimension)
	{
		dimensions = detect_dimensions(list);
	}

	// Allocate CDTS array
	target.set_new_array(len, dimensions, static_cast<metaffi_types>(resolved_type));
	cdts& arr = static_cast<cdts&>(target);

	// Fast path: 1D homogeneous primitive arrays can be filled directly.
	if(is_1d_array && try_fill_1d_primitive_cdts_from_pysequence(list, arr, resolved_type))
	{
		return;
	}

	// Fill array elements
	for(Py_ssize_t i = 0; i < len; i++)
	{
		PyObject* item = py_list::check(list) ? pPyList_GetItem(list, i) : pPyTuple_GetItem(list, i);
		if(resolved_type == metaffi_any_type)
		{
			pyobject_to_cdt(item, arr[i], metaffi_any_type);
		}
		else
		{
			pyobject_to_cdt(item, arr[i], resolved_type);
		}
	}
}

PyObject* cdts_python3_serializer::cdt_array_to_pylist(const cdts& arr)
{
	// GIL assumed to be held

	PyObject* list = pPyList_New(arr.length);
	if(!list)
	{
		throw_py_err("Failed to create Python list");
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

PyObject* cdts_python3_serializer::cdt_array_to_pybytes(const cdts& arr, metaffi_type element_type)
{
	// GIL assumed to be held
	if(element_type != metaffi_uint8_type && element_type != metaffi_int8_type)
	{
		throw_py_err("cdt_array_to_pybytes: unsupported element type");
	}

	if(arr.length == 0)
	{
		PyObject* empty_bytes = pPyBytes_FromStringAndSize("", 0);
		if(!empty_bytes || pPyErr_Occurred())
		{
			std::string error = check_python_error();
			if(error.empty())
			{
				error = "Failed to create empty bytes";
			}
			throw_py_err(error);
		}
		return empty_bytes;
	}

	std::string buffer;
	buffer.resize(arr.length);

	for(metaffi_size i = 0; i < arr.length; i++)
	{
		const cdt& item = arr.arr[i];
		if(item.type != metaffi_uint8_type && item.type != metaffi_int8_type)
		{
			throw_py_err("cdt_array_to_pybytes: array element is not int8/uint8");
		}

		if(item.type == metaffi_uint8_type)
		{
			buffer[i] = static_cast<char>(item.cdt_val.uint8_val);
		}
		else
		{
			buffer[i] = static_cast<char>(static_cast<uint8_t>(item.cdt_val.int8_val));
		}
	}

	PyObject* bytes_obj = pPyBytes_FromStringAndSize(buffer.data(), static_cast<Py_ssize_t>(buffer.size()));
	if(!bytes_obj || pPyErr_Occurred())
	{
		std::string error = check_python_error();
		if(error.empty())
		{
			error = "Failed to create bytes from array";
		}
		throw_py_err(error);
	}

	return bytes_obj;
}

} // namespace metaffi::utils
