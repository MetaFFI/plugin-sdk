#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include "cdt.h"
#include "metaffi_primitives.h"

// Define metaffi_any_array_type if not defined (missing from SDK)
#ifndef metaffi_any_array_type
#define metaffi_any_array_type (metaffi_any_type | metaffi_array_type)
#endif

namespace test_plugin
{

// Log prefix for normal output (stdout)
#define LOG_PREFIX "[xllr.test] "

// Log prefix for errors (stderr)
#define LOG_ERROR_PREFIX "[xllr.test] ERROR: "

// Helper to get type name string
inline const char* get_type_name(metaffi_type t)
{
	const char* str;
	metaffi_type_to_str(t, str);
	return str;
}

// Log a single cdt value to the given stream
inline void log_cdt_value(std::ostream& os, const cdt& c, int indent_level = 0)
{
	std::string indent(indent_level * 2, ' ');

	os << indent << "type=" << get_type_name(c.type);

	// Check if it's an array type
	if(c.type & metaffi_array_type)
	{
		os << " [array";
		if(c.cdt_val.array_val)
		{
			os << " length=" << c.cdt_val.array_val->length
			   << " fixed_dims=" << c.cdt_val.array_val->fixed_dimensions;
		}
		os << "]";

		if(c.cdt_val.array_val && c.cdt_val.array_val->arr)
		{
			os << std::endl;
			for(metaffi_size i = 0; i < c.cdt_val.array_val->length; ++i)
			{
				os << indent << "  [" << i << "] ";
				log_cdt_value(os, c.cdt_val.array_val->arr[i], indent_level + 2);
				if(i < c.cdt_val.array_val->length - 1)
				{
					os << std::endl;
				}
			}
		}
		return;
	}

	// Handle non-array types
	metaffi_type base_type = c.type;

	switch(base_type)
	{
		case metaffi_int8_type:
			os << " value=" << static_cast<int>(c.cdt_val.int8_val);
			break;
		case metaffi_int16_type:
			os << " value=" << c.cdt_val.int16_val;
			break;
		case metaffi_int32_type:
			os << " value=" << c.cdt_val.int32_val;
			break;
		case metaffi_int64_type:
			os << " value=" << c.cdt_val.int64_val;
			break;
		case metaffi_uint8_type:
			os << " value=" << static_cast<unsigned>(c.cdt_val.uint8_val);
			break;
		case metaffi_uint16_type:
			os << " value=" << c.cdt_val.uint16_val;
			break;
		case metaffi_uint32_type:
			os << " value=" << c.cdt_val.uint32_val;
			break;
		case metaffi_uint64_type:
			os << " value=" << c.cdt_val.uint64_val;
			break;
		case metaffi_float32_type:
			os << " value=" << c.cdt_val.float32_val;
			break;
		case metaffi_float64_type:
			os << " value=" << c.cdt_val.float64_val;
			break;
		case metaffi_bool_type:
			os << " value=" << (c.cdt_val.bool_val ? "true" : "false");
			break;
		case metaffi_char8_type:
			os << " value='" << static_cast<char>(c.cdt_val.char8_val.c[0]) << "'";
			break;
		case metaffi_string8_type:
			if(c.cdt_val.string8_val)
			{
				os << " value=\"" << reinterpret_cast<const char*>(c.cdt_val.string8_val) << "\"";
			}
			else
			{
				os << " value=null";
			}
			break;
		case metaffi_handle_type:
			if(c.cdt_val.handle_val)
			{
				os << " handle=" << c.cdt_val.handle_val->handle
				   << " runtime_id=" << c.cdt_val.handle_val->runtime_id;
			}
			else
			{
				os << " handle=null";
			}
			break;
		case metaffi_callable_type:
			if(c.cdt_val.callable_val)
			{
				os << " callable=" << c.cdt_val.callable_val->val
				   << " params=" << static_cast<int>(c.cdt_val.callable_val->params_types_length)
				   << " rets=" << static_cast<int>(c.cdt_val.callable_val->retval_types_length);
			}
			else
			{
				os << " callable=null";
			}
			break;
		case metaffi_null_type:
			os << " value=null";
			break;
		case metaffi_any_type:
			os << " value=<any>";
			break;
		default:
			os << " value=<unknown type " << base_type << ">";
			break;
	}
}

// Log a cdts structure to stdout
inline void log_cdts(const char* label, const cdts& c)
{
	std::cout << LOG_PREFIX << label << ": length=" << c.length
	          << " fixed_dims=" << c.fixed_dimensions << std::endl;

	for(metaffi_size i = 0; i < c.length; ++i)
	{
		std::cout << LOG_PREFIX << "  [" << i << "] ";
		log_cdt_value(std::cout, c.arr[i], 1);
		std::cout << std::endl;
	}
}

// Log API function entry
inline void log_api_call(const char* func_name)
{
	std::cout << LOG_PREFIX << func_name << " called" << std::endl;
}

// Log API function success
inline void log_api_success(const char* func_name)
{
	std::cout << LOG_PREFIX << func_name << " completed successfully" << std::endl;
}

// Log xcall invocation
inline void log_xcall(const char* variant, const std::string& entity_name)
{
	std::cout << LOG_PREFIX << variant << " - " << entity_name << std::endl;
}

// Log entity-specific message
inline void log_entity(const std::string& entity_name, const std::string& message)
{
	std::cout << LOG_PREFIX << entity_name << ": " << message << std::endl;
}

// Log error to stderr
inline void log_error(const std::string& message)
{
	std::cerr << LOG_ERROR_PREFIX << message << std::endl;
}

// Log error with function context to stderr
inline void log_error(const char* func_name, const std::string& message)
{
	std::cerr << LOG_ERROR_PREFIX << func_name << ": " << message << std::endl;
}

// Format an int32 array for logging
inline std::string format_int32_array(const cdts* arr)
{
	if(!arr || !arr->arr)
	{
		return "null";
	}

	std::ostringstream oss;
	oss << "[";
	for(metaffi_size i = 0; i < arr->length; ++i)
	{
		if(i > 0) oss << ", ";
		oss << arr->arr[i].cdt_val.int32_val;
	}
	oss << "]";
	return oss.str();
}

// Format an int64 array for logging
inline std::string format_int64_array(const cdts* arr)
{
	if(!arr || !arr->arr)
	{
		return "null";
	}

	std::ostringstream oss;
	oss << "[";
	for(metaffi_size i = 0; i < arr->length; ++i)
	{
		if(i > 0) oss << ", ";
		oss << arr->arr[i].cdt_val.int64_val;
	}
	oss << "]";
	return oss.str();
}

// Format a string array for logging
inline std::string format_string_array(const cdts* arr)
{
	if(!arr || !arr->arr)
	{
		return "null";
	}

	std::ostringstream oss;
	oss << "[";
	for(metaffi_size i = 0; i < arr->length; ++i)
	{
		if(i > 0) oss << ", ";
		if(arr->arr[i].cdt_val.string8_val)
		{
			oss << "\"" << reinterpret_cast<const char*>(arr->arr[i].cdt_val.string8_val) << "\"";
		}
		else
		{
			oss << "null";
		}
	}
	oss << "]";
	return oss.str();
}

} // namespace test_plugin
