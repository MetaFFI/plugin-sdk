#pragma once
#include "cdt_structs.h"
#include <sstream>
#include <functional>
#include <utility>

namespace metaffi::runtime
{

/************************************************
*   Wrapper for N-dimensions numeric array
*************************************************/

template<typename T>
class numeric_n_array_wrapper
{
private:
	T* array = nullptr;

public:
	metaffi_size* dimensions_lengths = nullptr;
	metaffi_size dimensions = 0;

public:
	numeric_n_array_wrapper()= default;
	numeric_n_array_wrapper(T* array, metaffi_size* dimensions_lengths, metaffi_size& dimensions): array(array), dimensions_lengths(dimensions_lengths), dimensions(dimensions){}
	numeric_n_array_wrapper(const numeric_n_array_wrapper& other) = default;
	numeric_n_array_wrapper& operator = (const numeric_n_array_wrapper& other) = default;
	
	bool is_simple_array(){ return this->dimensions == 1; }
	metaffi_size get_simple_array_length(){ return dimensions_lengths[0]; }
	
	T get_elem_at(metaffi_size index[], metaffi_size index_length) const
	{
		T** itemptr = (T**)array;
		T res;
		for(int i=0 ; i<index_length ; i++)
		{
			if(index[i] >= dimensions_lengths[i]){
				std::stringstream ss;
				ss << "Array out of bounds. Requested index: " << index[i] << ". Array size: " << dimensions_lengths[i];
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

/************************************************
*   Wrapper for N-dimensions string array
*************************************************/

template<typename T>
class string_n_array_wrapper
{
private:
	T* array = nullptr;

public:
	metaffi_size* string_lengths = nullptr;
	metaffi_size* dimensions_lengths = nullptr;
	metaffi_size dimensions = 0;

public:
	string_n_array_wrapper() = default;
	string_n_array_wrapper(const string_n_array_wrapper& other) = default;
	string_n_array_wrapper(T* array, metaffi_size* string_lengths, metaffi_size* dimensions_lengths, metaffi_size dimensions):
			array(array), string_lengths(string_lengths), dimensions_lengths(dimensions_lengths), dimensions(dimensions){}
	
	string_n_array_wrapper& operator = (const string_n_array_wrapper& other) = default;
	//--------------------------------------------------------------------
	metaffi_size get_dimensions_count(){ return this->dimensions; }
	//--------------------------------------------------------------------
	metaffi_size get_dimension_length(int dimindex)
	{
		if(dimindex >= dimensions){
			std::stringstream ss;
			ss << "requested dimension length of dimension " << dimindex << " while highest dimension is " << dimensions - 1;
			throw std::runtime_error(ss.str());
		}
		return dimensions_lengths[dimindex];
	}
	//--------------------------------------------------------------------
	void get_elem_at(metaffi_size index[], metaffi_size index_length, T* out_res, metaffi_size* out_length) const
	{
		T** itemptr = (T**)array;
		metaffi_size** sizeptr = (metaffi_size**)string_lengths;
		for(int i=0 ; i<index_length ; i++)
		{
			if(index[i] >= dimensions_lengths[i]){
				std::stringstream ss;
				ss << "Array out of bounds. Requested index: " << index[i] << ". Array size: " << dimensions_lengths[i];
				throw std::runtime_error(ss.str());
			}
			
			if(i + 1 == index_length){ // if last item - get string
				*out_res = ((T*)itemptr)[index[i]];
				*out_length = ((metaffi_size*)sizeptr)[index[i]];
			}
			else{ // else, get next pointer
				itemptr = (T**)itemptr[index[i]];
				sizeptr = (metaffi_size**)sizeptr[index[i]];
			}
		}
	}
};

/************************************************
*   Callback functions called while parsing CDTS
*************************************************/

struct cdts_parse_callbacks
{
#define on_parse_numeric(type) \
	std::function<void(void* values_to_set, int index, const type& val_to_set)> on_##type;\
	std::function<void(void* values_to_set, int index, const type* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions)> on_##type##_array;

#define on_parse_string(type) \
	std::function<void(void* values_to_set, int index, const type& val_to_set, const metaffi_size& val_length)> on_##type; \
    std::function<void(void* values_to_set, int index, const type* parray_to_set, const metaffi_size* pelements_lengths_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions)>  on_##type##_array;
	
	on_parse_numeric(metaffi_float32);
	on_parse_numeric(metaffi_float64);
	on_parse_numeric(metaffi_int8);
	on_parse_numeric(metaffi_int16);
	on_parse_numeric(metaffi_int32);
	on_parse_numeric(metaffi_int64);
	on_parse_numeric(metaffi_uint8);
	on_parse_numeric(metaffi_uint16);
	on_parse_numeric(metaffi_uint32);
	on_parse_numeric(metaffi_uint64);
	on_parse_numeric(metaffi_bool);
	on_parse_numeric(metaffi_handle);
	on_parse_string(metaffi_string8);
	on_parse_string(metaffi_string16);
	on_parse_string(metaffi_string32);

#define prase_constructor_param(type) \
	typeof(on_##type) on_##type, \
	typeof(on_##type##_array) on_##type##_array

#define parse_constructor_init_param(type) \
	on_##type(std::move(on_##type)),          \
	on_##type##_array(std::move(on_##type##_array))
	
	cdts_parse_callbacks
	(
		prase_constructor_param(metaffi_float32),
		prase_constructor_param(metaffi_float64),
		prase_constructor_param(metaffi_int8),
		prase_constructor_param(metaffi_int16),
		prase_constructor_param(metaffi_int32),
		prase_constructor_param(metaffi_int64),
		prase_constructor_param(metaffi_uint8),
		prase_constructor_param(metaffi_uint16),
		prase_constructor_param(metaffi_uint32),
		prase_constructor_param(metaffi_uint64),
		prase_constructor_param(metaffi_bool),
		prase_constructor_param(metaffi_handle),
		prase_constructor_param(metaffi_string8),
		prase_constructor_param(metaffi_string16),
		prase_constructor_param(metaffi_string32)
	): parse_constructor_init_param(metaffi_float32),
	   parse_constructor_init_param(metaffi_float64),
	   parse_constructor_init_param(metaffi_int8),
	   parse_constructor_init_param(metaffi_int16),
	   parse_constructor_init_param(metaffi_int32),
	   parse_constructor_init_param(metaffi_int64),
	   parse_constructor_init_param(metaffi_uint8),
	   parse_constructor_init_param(metaffi_uint16),
	   parse_constructor_init_param(metaffi_uint32),
	   parse_constructor_init_param(metaffi_uint64),
	   parse_constructor_init_param(metaffi_bool),
	   parse_constructor_init_param(metaffi_handle),
	   parse_constructor_init_param(metaffi_string8),
	   parse_constructor_init_param(metaffi_string16),
	   parse_constructor_init_param(metaffi_string32)
	{}
};

/************************************************
*   Callback functions called while building CDTS
*************************************************/

struct cdts_build_callbacks
{
#define set_build_numeric(type) \
	std::function<void(void* values_to_set, int index, type& val_to_set)> set_##type;\
    std::function<void(void* values_to_set, int index, type*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required)> set_##type##_array;
	
#define set_build_string(type) \
	std::function<void(void* values_to_set, int index, type& val_to_set, metaffi_size& val_length)> set_##type; \
    std::function<void(void* values_to_set, int index, type*& parray_to_set, metaffi_size*& pelements_lengths_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required)>  set_##type##_array;
		
	set_build_numeric(metaffi_float32);
	set_build_numeric(metaffi_float64);
	set_build_numeric(metaffi_int8);
	set_build_numeric(metaffi_int16);
	set_build_numeric(metaffi_int32);
	set_build_numeric(metaffi_int64);
	set_build_numeric(metaffi_uint8);
	set_build_numeric(metaffi_uint16);
	set_build_numeric(metaffi_uint32);
	set_build_numeric(metaffi_uint64);
	set_build_numeric(metaffi_bool);
	set_build_numeric(metaffi_handle);
	set_build_string(metaffi_string8);
	set_build_string(metaffi_string16);
	set_build_string(metaffi_string32);
	std::function<void(void* values_to_set, int index, cdt* cdt_elem_to_set)> set_any;
	
	
#define build_constructor_param(type) \
	const typeof(set_##type)& set_##type, \
    const typeof(set_##type##_array)& set_##type##_array

#define build_constructor_init_param(type) \
	set_##type(std::move(set_##type)),\
	set_##type##_array(std::move(set_##type##_array))
		
	cdts_build_callbacks
	(
		build_constructor_param(metaffi_float32),
		build_constructor_param(metaffi_float64),
		build_constructor_param(metaffi_int8),
		build_constructor_param(metaffi_int16),
		build_constructor_param(metaffi_int32),
		build_constructor_param(metaffi_int64),
		build_constructor_param(metaffi_uint8),
		build_constructor_param(metaffi_uint16),
		build_constructor_param(metaffi_uint32),
		build_constructor_param(metaffi_uint64),
		build_constructor_param(metaffi_bool),
		build_constructor_param(metaffi_handle),
		build_constructor_param(metaffi_string8),
		build_constructor_param(metaffi_string16),
		build_constructor_param(metaffi_string32)
	): build_constructor_init_param(metaffi_float32),
	   build_constructor_init_param(metaffi_float64),
	   build_constructor_init_param(metaffi_int8),
	   build_constructor_init_param(metaffi_int16),
	   build_constructor_init_param(metaffi_int32),
	   build_constructor_init_param(metaffi_int64),
	   build_constructor_init_param(metaffi_uint8),
	   build_constructor_init_param(metaffi_uint16),
	   build_constructor_init_param(metaffi_uint32),
	   build_constructor_init_param(metaffi_uint64),
	   build_constructor_init_param(metaffi_bool),
	   build_constructor_init_param(metaffi_handle),
	   build_constructor_init_param(metaffi_string8),
	   build_constructor_init_param(metaffi_string16),
	   build_constructor_init_param(metaffi_string32)
	{}
};

/************************************************
*   CDTS wrapper class
*************************************************/

class cdts_wrapper
{
private:
	cdt* cdts;
	metaffi_size cdts_length;
	
public:
	explicit cdts_wrapper(cdt* cdts, metaffi_size cdts_length);
	explicit cdts_wrapper(metaffi_size cdt_count);
	~cdts_wrapper() = default;
	
	[[nodiscard]] cdt* get_cdts() const;
	[[nodiscard]] metaffi_size get_cdts_length() const;
	
	cdt* operator[](int index) const;
	
	/**
	 * @brief Parses CDTS and for each CDT calls the relevant callback function
	 * @param callbacks with functions to call with CDT values
	 */
	void parse(void* values_to_set, const cdts_parse_callbacks& callbacks);
	
	/**
	 @brief For each element in CDTS, call revelant cdts_build_callback based on given types[]. types[] length must equal CDTS length.
	 * @param types
	 * @param types_length
	 * @param values_to_set is passed to the callbacks. Parameter should contain the data to convert.
	 * @param callbacks
	 */
	void build(const metaffi_types types[], metaffi_size types_length, void* values_to_set, cdts_build_callbacks& callbacks) const;

};

};