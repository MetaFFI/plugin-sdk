#pragma once
#include <runtime/openffi_primitives.h>
#include <functional>
#include <sstream>

namespace openffi::utils
{
//--------------------------------------------------------------------
template<typename T>
class numeric_n_array_wrapper
{
private:
	T* array = nullptr;

public:
	openffi_size* dimensions = nullptr;
	openffi_size dimensions_length = 0;

public:
	numeric_n_array_wrapper()= default;
	numeric_n_array_wrapper(T* array, openffi_size* dimensions, openffi_size dimensions_length): array(array), dimensions(dimensions), dimensions_length(dimensions_length){}
	numeric_n_array_wrapper(const numeric_n_array_wrapper& other) = default;
	
	numeric_n_array_wrapper& operator = (const numeric_n_array_wrapper& other) = default;
	
	bool is_simple_array(){ return this->dimensions_length == 1; }
	openffi_size get_simple_array_length(){ return dimensions[0]; }
	
	T get_elem_at(openffi_size index[], openffi_size index_length)
	{
		T** itemptr = (T**)array;
		T res;
		for(int i=0 ; i<index_length ; i++)
		{
			if(index[i] >= dimensions[i]){
				std::stringstream ss;
				ss << "Array out of bounds. Requested index: " << index[i] << ". Array size: " << dimensions[i];
				throw std::runtime_error(ss.str());
			}
			
			if(i + 1 == index_length){ // if last item - get number
				res = ((T*)itemptr)[index[i]];
			}
			else{ // else, get next pointer
				itemptr = ((T***)itemptr)[index[i]];
			}
		}
		
		return res;
	}
};
//===========================================================
template<typename T>
class string_n_array_wrapper
{
private:
	T* array = nullptr;
    openffi_size* string_lengths = nullptr;
	openffi_size* dimensions = nullptr;
	openffi_size dimensions_count = 0;

public:
	string_n_array_wrapper() = default;
	string_n_array_wrapper(const string_n_array_wrapper& other) = default;
	string_n_array_wrapper(T* array, openffi_size* string_lengths, openffi_size* dimensions, openffi_size dimensions_length):
			array(array), string_lengths(string_lengths), dimensions(dimensions), dimensions_count(dimensions_length){}
	
	string_n_array_wrapper& operator = (const string_n_array_wrapper& other) = default;
	//--------------------------------------------------------------------
	openffi_size get_dimensions_count(){ return this->dimensions_count; }
	//--------------------------------------------------------------------
	openffi_size get_dimension_length(int dimindex)
	{
		if(dimindex >= dimensions_count){
			std::stringstream ss;
			ss << "requested dimension length of dimension " << dimindex << " while highest dimension is " << dimensions_count - 1;
			throw std::runtime_error(ss.str());
		}
		return dimensions[dimindex];
	}
	//--------------------------------------------------------------------
	void get_elem_at(openffi_size index[], openffi_size index_length, T* out_res, openffi_size* out_length)
	{
		T** itemptr = (T**)array;
		openffi_size** sizeptr = (openffi_size**)string_lengths;
		for(int i=0 ; i<index_length ; i++)
		{
			if(index[i] >= dimensions[i]){
				std::stringstream ss;
				ss << "Array out of bounds. Requested index: " << index[i] << ". Array size: " << dimensions[i];
				throw std::runtime_error(ss.str());
			}
			
			if(i + 1 == index_length){ // if last item - get string
				*out_res = ((T*)itemptr)[index[i]];
				*out_length = ((openffi_size*)sizeptr)[index[i]];
			}
			else{ // else, get next pointer
				itemptr = (T**)itemptr[index[i]];
				sizeptr = (openffi_size**)sizeptr[index[i]];
			}
		}
	}
};
	



//--------------------------------------------------------------------
struct common_data_type_parse_callbacks
{
#define on_numeric(type) \
	std::function<void(const type&)> on_##type;\
	std::function<void(const numeric_n_array_wrapper<type>&)> on_##type##_array;
	
	on_numeric(openffi_float32);
	on_numeric(openffi_float64);
	on_numeric(openffi_int8);
	on_numeric(openffi_int16);
	on_numeric(openffi_int32);
	on_numeric(openffi_int64);
	on_numeric(openffi_uint8);
	on_numeric(openffi_uint16);
	on_numeric(openffi_uint32);
	on_numeric(openffi_uint64);
	on_numeric(openffi_bool);

#define on_string(type) \
	std::function<void(const type&, const openffi_size&)> on_##type;\
	std::function<void(const string_n_array_wrapper<type>&)> on_##type##_array;
	
	on_string(openffi_string);
	on_string(openffi_string8);
	on_string(openffi_string16);
	on_string(openffi_string32);

#define constructor_param(type) \
	typeof(on_##type) on_##type, \
	typeof(on_##type##_array) on_##type##_array
	
#define constructor_init_param(type) \
	on_##type(on_##type),on_##type##_array(on_##type##_array)
	
	common_data_type_parse_callbacks
	(
		constructor_param(openffi_float32),
		constructor_param(openffi_float64),
		constructor_param(openffi_int8),
		constructor_param(openffi_int16),
		constructor_param(openffi_int32),
		constructor_param(openffi_int64),
		constructor_param(openffi_uint8),
		constructor_param(openffi_uint16),
		constructor_param(openffi_uint32),
		constructor_param(openffi_uint64),
		constructor_param(openffi_bool),
		constructor_param(openffi_string),
		constructor_param(openffi_string8),
		constructor_param(openffi_string16),
		constructor_param(openffi_string32)
		
	): constructor_init_param(openffi_float32),
	   constructor_init_param(openffi_float64),
	   constructor_init_param(openffi_int8),
	   constructor_init_param(openffi_int16),
	   constructor_init_param(openffi_int32),
	   constructor_init_param(openffi_int64),
	   constructor_init_param(openffi_uint8),
	   constructor_init_param(openffi_uint16),
	   constructor_init_param(openffi_uint32),
	   constructor_init_param(openffi_uint64),
	   constructor_init_param(openffi_bool),
	   constructor_init_param(openffi_string),
	   constructor_init_param(openffi_string8),
	   constructor_init_param(openffi_string16),
	   constructor_init_param(openffi_string32)
	{}
};
//--------------------------------------------------------------------
class common_data_type_parser
{
private:
	void** data_buffer;
	openffi_size buffer_length;
	common_data_type_parse_callbacks callbacks;
	
public:
	common_data_type_parser(void** data_buffer, openffi_size length, common_data_type_parse_callbacks callbacks);
	~common_data_type_parser() = default;
	
	void parse();
};
}