#include "common_data_type_parser.h"
#include "common_data_type_helper_loader.h"
#include <sstream>
#include <cstdio>

namespace openffi::utils
{
//--------------------------------------------------------------------
common_data_type_parser::common_data_type_parser(void** data_buffer,
												 openffi_size length,
												 common_data_type_parse_callbacks callbacks):
												                data_buffer(data_buffer), buffer_length(length), callbacks(std::move(callbacks))
{
	const char* err = load_args_helpers();
	if(err){
		throw std::runtime_error(err);
	}
}
#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-macro-parentheses"
//--------------------------------------------------------------------
void common_data_type_parser::parse()
{
	int index = 0;
	
	while(this->buffer_length - index > 0)
	{
		openffi_type cur_type;
		index = get_type(this->data_buffer, index, &cur_type);

		if(cur_type == 0)
		{
			std::stringstream ss;
			ss << "Invalid type at index " << index;
			throw std::runtime_error(ss.str());
		}

#define handle_numeric_type(type) \
		if(cur_type & type##_type) \
		{ \
			if(cur_type & openffi_array_type) \
			{ \
				type* array; \
				openffi_size* dimensions; \
				openffi_size dimensions_length; \
				index = get_arg_##type##_array(this->data_buffer, index, &array, &dimensions, &dimensions_length); \
				callbacks.on_##type##_array(numeric_n_array_wrapper<type>(array, dimensions, dimensions_length)); \
                continue; \
			} \
			else \
			{ \
				type item; \
				index = get_arg_##type(this->data_buffer, index, &item); \
				callbacks.on_##type(item); \
                continue; \
			} \
		}
		
		handle_numeric_type(openffi_float64);
		handle_numeric_type(openffi_float32);
		handle_numeric_type(openffi_int8);
		handle_numeric_type(openffi_int16);
		handle_numeric_type(openffi_int32);
		handle_numeric_type(openffi_int64);
		handle_numeric_type(openffi_uint8);
		handle_numeric_type(openffi_uint16);
		handle_numeric_type(openffi_uint32);
		handle_numeric_type(openffi_uint64);
		handle_numeric_type(openffi_bool);

#define handle_string_type(type) \
		if(cur_type & type##_type) \
		{ \
			if(cur_type & openffi_array_type) \
			{ \
				type* array; \
                openffi_size* string_lengths; \
				openffi_size* dimensions; \
				openffi_size dimensions_length; \
				index = get_arg_##type##_array(this->data_buffer, index, &array, &string_lengths, &dimensions, &dimensions_length); \
				callbacks.on_##type##_array(string_n_array_wrapper<type>(array, string_lengths, dimensions, dimensions_length)); \
                continue; \
			} \
			else \
			{ \
				type item; \
                openffi_size length; \
				index = get_arg_##type(this->data_buffer, index, &item, &length); \
				callbacks.on_##type(item, length); \
                continue; \
			} \
		}
		
		handle_string_type(openffi_string);
		handle_string_type(openffi_string8);
		handle_string_type(openffi_string16);
		handle_string_type(openffi_string32);
		
		// if got here - type is not handled!
		std::stringstream ss;
		ss << "Type: " << cur_type << " is not handled";
		throw std::runtime_error(ss.str());
	}
}
#pragma clang diagnostic pop
//--------------------------------------------------------------------
}