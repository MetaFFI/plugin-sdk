#pragma once
#include "cdt_structs.h"
#include <sstream>
#include <functional>
#include <utility>
#include "cdt_capi_loader.h"
#include <queue>
#include <vector>

namespace metaffi::runtime
{
	
metaffi_type_info make_type_with_options(metaffi_type type, const std::string& alias = "", int dimensions = 0);
	
	
	template<typename cdt_metaffi_type_t, typename metaffi_type_t>
	void traverse_ragged_array(const cdt_metaffi_type_t& arr, void* other_array,
	                           void(*on_array)(metaffi_size* index, metaffi_size index_size, metaffi_size array_length, void* other_array),
	                           void (*on_1d_array)(metaffi_size* index, metaffi_size index_size, metaffi_type_t* arr, metaffi_size length, void* other_array))
	{
		std::queue<std::vector<metaffi_size>> q;
		q.push(std::vector<metaffi_size>());
		
		
		while(!q.empty())
		{
			std::vector<metaffi_size> index = q.front();
			q.pop();
			
			metaffi_size* current_lengths = arr.dimensions_lengths;
			metaffi_type_t* current_vals = arr.vals;
			
			for (metaffi_size i = 0; i < index.size(); i++)
			{
				current_lengths = ((metaffi_size**)current_lengths)[index.size()];
				for(metaffi_size j : index){
					current_lengths = (metaffi_size*)(current_lengths + j);
				}
				current_vals = ((metaffi_type_t**)current_vals)[index[i]];
			}
			
			
			if(index.size() == arr.dimensions - 1)
			{
				on_1d_array(index.data(), index.size(), current_vals, *current_lengths, other_array);
			}
			else
			{
				on_array(index.data(), index.size(), *current_lengths, other_array);
			}
			
			for (metaffi_size i = 0; i < *current_lengths && index.size() < arr.dimensions - 1 ; i++)
			{
				std::vector<metaffi_size> next_index = index;
				next_index.push_back(i);
				q.push(next_index);
			}
		}
	}
	
	
	
	
	
	
	template<typename cdt_metaffi_type_t>
void build_array(cdt_metaffi_type_t& arr, metaffi_size dimensions, void* other_array,
				 metaffi_size (*get_array)(metaffi_size* index, metaffi_size index_length, void* other_array),
				 cdt_metaffi_type_t* (*get_1d_array)(metaffi_size* index, metaffi_size index_length, metaffi_size* out_1d_array_length, void* other_array))
{
	arr.dimensions = dimensions;
	arr.dimensions_lengths = new metaffi_size[dimensions];
	std::vector<metaffi_size> index(dimensions, 0);
	
	// Calculate the total size of the array and initialize dimensions_lengths
	metaffi_size total_size = 1;
	for (metaffi_size i = 0; i < dimensions; ++i) {
		arr.dimensions_lengths[i] = get_array(index.data(), i, other_array);
		total_size *= arr.dimensions_lengths[i];
	}
	
	// Allocate memory for the array
	arr.vals = new cdt_metaffi_type_t[total_size];
	
	// Fill the array with values
	for (metaffi_size i = 0; i < total_size; ++i) {
		// Calculate the index in the multi-dimensional array corresponding to i
		metaffi_size temp = i;
		for (metaffi_size j = dimensions; j-- > 0; ) {
			index[j] = temp % arr.dimensions_lengths[j];
			temp /= arr.dimensions_lengths[j];
		}
		
		// Get the 1D array at the current index
		metaffi_size length;
		cdt_metaffi_type_t* data = get_1d_array(index.data(), dimensions, &length, other_array);
		
		// Copy the values from the 1D array to the multi-dimensional array
		for (metaffi_size j = 0; j < length; ++j) {
			arr.vals[i + j] = data[j];
		}
		
		// Skip the elements that have been filled
		i += length - 1;
	}
}

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
*   CDTS wrapper class
*************************************************/
	
class cdts_wrapper
{
private:
	cdt* cdts;
	metaffi_size cdts_length;
	bool is_free_cdts;

public:
	explicit cdts_wrapper(cdt* cdts, metaffi_size cdts_length, bool is_free_cdts = false);
	
	[[nodiscard]] cdt* get_cdts() const;
	[[nodiscard]] metaffi_size get_cdts_length() const;
	
	cdt* operator[](int index) const;
	
	
	void set(int index, metaffi_float64 v) const;
	void set(int index, metaffi_float32 v) const;
	
	void set(int index, metaffi_int8 v) const;
	void set(int index, metaffi_int16 v) const;
	void set(int index, metaffi_int32 v) const;
	void set(int index, metaffi_int64 v) const;
	
	void set(int index, metaffi_uint8 v) const;
	void set(int index, metaffi_uint16 v) const;
	void set(int index, metaffi_uint32 v) const;
	void set(int index, metaffi_uint64 v) const;
	
	void set(int index, bool v) const;
	
	void set(int index, const std::string& v) const; // string8
	
	void set(int index, const metaffi_float64* v, int length) const;
	void set(int index, const metaffi_float32* v, int length) const;
	
	void set(int index, const metaffi_int8* v, int length) const;
	void set(int index, const metaffi_int16* v, int length) const;
	void set(int index, const metaffi_int32* v, int length) const;
	void set(int index, const metaffi_int64* v, int length) const;
	
	void set(int index, const metaffi_uint8* v, int length) const;
	void set(int index, const metaffi_uint16* v, int length) const;
	void set(int index, const metaffi_uint32* v, int length) const;
	void set(int index, const metaffi_uint64* v, int length) const;
	
	void set(int index, const bool* v, int length) const;
	
	void set(int index, const std::vector<std::string>& v) const; // string8
	
	void set(int index, const cdt_metaffi_handle& v) const;
	void set_null_handle(int index) const;
	void set(int index, cdt_metaffi_handle* v, int length) const;

	void set(int index, metaffi_callable v, const std::vector<metaffi_type>& parameters_types, const std::vector<metaffi_type>& retvals_types) const;
	
	metaffi_float64 get_metaffi_float64(int index) const;
	metaffi_float32 get_metaffi_float32(int index) const;
	
	metaffi_int8 get_metaffi_int8(int index) const;
	metaffi_int16 get_metaffi_int16(int index) const;
	metaffi_int32 get_metaffi_int32(int index) const;
	metaffi_int64 get_metaffi_int64(int index) const;
	
	metaffi_uint8 get_metaffi_uint8(int index) const;
	metaffi_uint16 get_metaffi_uint16(int index) const;
	metaffi_uint32 get_metaffi_uint32(int index) const;
	metaffi_uint64 get_metaffi_uint64(int index) const;
	
	bool get_bool(int index) const;
	
	std::string get_string(int index) const; // string8
	
	metaffi_handle get_metaffi_handle(int index) const;
	
	std::vector<metaffi_float64> get_metaffi_float64_array(int index) const;
	std::vector<metaffi_float32> get_metaffi_float32_array(int index) const;
	
	std::vector<metaffi_int8> get_metaffi_int8_array(int index) const;
	std::vector<metaffi_int16> get_metaffi_int16_array(int index) const;
	std::vector<metaffi_int32> get_metaffi_int32_array(int index) const;
	std::vector<metaffi_int64> get_metaffi_int64_array(int index) const;
	
	std::vector<metaffi_uint8> get_metaffi_uint8_array(int index) const;
	std::vector<metaffi_uint16> get_metaffi_uint16_array(int index) const;
	std::vector<metaffi_uint32> get_metaffi_uint32_array(int index) const;
	std::vector<metaffi_uint64> get_metaffi_uint64_array(int index) const;
	
	std::vector<bool> get_bool_array(int index) const;
	
	std::vector<std::string> get_vector_string(int index) const; // string8
	
	std::vector<cdt_metaffi_handle> get_metaffi_handle_array(int index) const;
	
	
};
	
//--------------------------------------------------------------------



};

