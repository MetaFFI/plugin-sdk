#include "cdts_traverse_construct.h"
#include <iostream>
#include <queue>
#include <sstream>
#include <limits.h>

namespace metaffi::runtime
{

//--------------------------------------------------------------------

void traverse_cdt(const cdt& item, const metaffi::runtime::traverse_cdts_callbacks& callbacks)
{
	traverse_cdt(item, callbacks, {});
}

void traverse_cdt(const cdt* item, const traverse_cdts_callbacks* callbacks, char** out_nul_term_err) noexcept
{
	try
	{
		traverse_cdt(*item, *callbacks);
	}
	catch(const std::exception& e)
	{
		std::string err = e.what();
		*out_nul_term_err = new char[err.size()+1];
		std::copy(err.begin(), err.end(), *out_nul_term_err);
		(*out_nul_term_err)[err.size()] = '\0';
	}
}

void traverse_cdt(const cdt& item, const metaffi::runtime::traverse_cdts_callbacks& callbacks, const std::vector<metaffi_size>& current_index)
{
	if(item.type == metaffi_any_type)
	{
		throw std::runtime_error("traversed CDT must have a concrete type, not dynamic type like metaffi_any_type");
	}
	
	metaffi_type common_type = metaffi_any_type;
	metaffi_type type_to_use = item.type;
	if(type_to_use & metaffi_array_type && type_to_use != metaffi_array_type)
	{
		common_type = type_to_use & ~metaffi_array_type;
		type_to_use = metaffi_array_type;
	}
	
	switch(type_to_use)
	{
		case metaffi_float64_type:
		{
			callbacks.on_float64(current_index.data(), current_index.size(), item.cdt_val.float64_val, callbacks.context);
		}break;
		
		case metaffi_float32_type:
		{
			callbacks.on_float32(current_index.data(), current_index.size(), item.cdt_val.float32_val, callbacks.context);
		}break;
		
		case metaffi_int8_type:
		{
			callbacks.on_int8(current_index.data(), current_index.size(), item.cdt_val.int8_val, callbacks.context);
		}break;
		
		case metaffi_uint8_type:
		{
			callbacks.on_uint8(current_index.data(), current_index.size(), item.cdt_val.uint8_val, callbacks.context);
		}break;
		
		case metaffi_int16_type:
		{
			callbacks.on_int16(current_index.data(), current_index.size(), item.cdt_val.int16_val, callbacks.context);
		}break;
		
		case metaffi_uint16_type:
		{
			callbacks.on_uint16(current_index.data(), current_index.size(), item.cdt_val.uint16_val, callbacks.context);
		}break;
		
		case metaffi_int32_type:
		{
			callbacks.on_int32(current_index.data(), current_index.size(), item.cdt_val.int32_val, callbacks.context);
		}break;
		
		case metaffi_uint32_type:
		{
			callbacks.on_uint32(current_index.data(), current_index.size(), item.cdt_val.uint32_val, callbacks.context);
		}break;
		
		case metaffi_int64_type:
		{
			callbacks.on_int64(current_index.data(), current_index.size(), item.cdt_val.int64_val, callbacks.context);
		}break;
		
		case metaffi_uint64_type:
		{
			callbacks.on_uint64(current_index.data(), current_index.size(), item.cdt_val.uint64_val, callbacks.context);
		}break;
		
		case metaffi_bool_type:
		{
			callbacks.on_bool(current_index.data(), current_index.size(), item.cdt_val.bool_val, callbacks.context);
		}break;
		
		case metaffi_char8_type:
		{
			callbacks.on_char8(current_index.data(), current_index.size(), item.cdt_val.char8_val, callbacks.context);
		}break;
		
		case metaffi_string8_type:
		{
			callbacks.on_string8(current_index.data(), current_index.size(), item.cdt_val.string8_val, callbacks.context);
		}break;
		
		case metaffi_char16_type:
		{
			callbacks.on_char16(current_index.data(), current_index.size(), item.cdt_val.char16_val, callbacks.context);
		}break;
		
		case metaffi_string16_type:
		{
			callbacks.on_string16(current_index.data(), current_index.size(), item.cdt_val.string16_val, callbacks.context);
		}break;
		
		case metaffi_char32_type:
		{
			callbacks.on_char32(current_index.data(), current_index.size(), item.cdt_val.char32_val, callbacks.context);
		}break;
		
		case metaffi_string32_type:
		{
			callbacks.on_string32(current_index.data(), current_index.size(), item.cdt_val.string32_val, callbacks.context);
		}break;
		
		case metaffi_handle_type:
		{
			if(!item.cdt_val.handle_val)
			{
				cdt_metaffi_handle null_handle{};
				callbacks.on_handle(current_index.data(), current_index.size(), null_handle, callbacks.context);
			}
			else
			{
				callbacks.on_handle(current_index.data(), current_index.size(), *item.cdt_val.handle_val, callbacks.context);
			}
		}break;
		
		case metaffi_callable_type:
		{
			if(!item.cdt_val.callable_val){
				throw std::runtime_error("Callable value is null");
			}
			
			callbacks.on_callable(current_index.data(), current_index.size(), *item.cdt_val.callable_val, callbacks.context);
		}break;
		
		case metaffi_null_type:
		{
			callbacks.on_null(current_index.data(), current_index.size(), callbacks.context);
		}break;
		
		case metaffi_array_type:
		{
			if(!item.cdt_val.array_val){
				throw std::runtime_error("Array value is null");
			}
			
			metaffi_bool continue_traverse = callbacks.on_array(current_index.data(), current_index.size(), *item.cdt_val.array_val, item.cdt_val.array_val->fixed_dimensions, common_type, callbacks.context);
			
			if(continue_traverse){
				traverse_cdts(*item.cdt_val.array_val, callbacks, current_index);
			}
		}break;
		
		default:
		{
			std::stringstream ss;
			ss << "Unknown type while traversing CDTS: " << item.type;
			throw std::runtime_error(ss.str());
		}
	}
}

void traverse_cdts(const cdts& arr, const metaffi::runtime::traverse_cdts_callbacks& callbacks)
{
	traverse_cdts(arr, callbacks, {});
}

void traverse_cdts(const cdts* arr, const traverse_cdts_callbacks* callbacks, char** out_nul_term_err) noexcept
{
	try
	{
		traverse_cdts(*arr, *callbacks);
	}
	catch(const std::exception& e)
	{
		std::string err = e.what();
		*out_nul_term_err = new char[err.size()+1];
		std::copy(err.begin(), err.end(), *out_nul_term_err);
		(*out_nul_term_err)[err.size()] = '\0';
	}
}

void traverse_cdts(const cdts& arr, const metaffi::runtime::traverse_cdts_callbacks& callbacks, const std::vector<metaffi_size>& starting_index)
{
	if(arr.length == 0){ // empty CDTS
		return;
	}
	
	std::queue<std::pair<std::vector<metaffi_size>, cdt&>> queue;
	for(metaffi_size i=0 ; i<arr.length ; i++)
	{
		std::vector<metaffi_size> index = starting_index;
		index.emplace_back(i);
		queue.emplace(index, arr[i]);
	}
	
	while(!queue.empty())
	{
		auto [currentIndex, pcdt] = queue.front();
		queue.pop();
		
		metaffi::runtime::traverse_cdt(pcdt, callbacks, currentIndex);
	}
}
//--------------------------------------------------------------------
void construct_cdt(cdt& item, const metaffi::runtime::construct_cdts_callbacks& callbacks)
{
	construct_cdt(item, callbacks, {});
}

void construct_cdt(cdt* item, const construct_cdts_callbacks* callbacks, char** out_nul_term_err) noexcept
{
	try
	{
		construct_cdt(*item, *callbacks);
	}
	catch(const std::exception& e)
	{
		std::string err = e.what();
		*out_nul_term_err = (char*)malloc( (err.size()+1) * sizeof(char) );
		std::copy(err.begin(), err.end(), *out_nul_term_err);
		(*out_nul_term_err)[err.size()] = '\0';
	}
}
//--------------------------------------------------------------------
void construct_cdt(cdt& item, const metaffi::runtime::construct_cdts_callbacks& callbacks, const std::vector<metaffi_size>& current_index, const metaffi_type_info& known_type /*=metaffi_any_type*/)
{
	
	metaffi_type_info ti;
	if(known_type.type == metaffi_any_type){
		ti = callbacks.get_type_info(current_index.data(), current_index.size(), callbacks.context);
	} else {
		ti = known_type;
	}
	
	if(ti.type == metaffi_any_type)
	{
		throw std::runtime_error("get_type_info must return a concrete type, not dynamic type like metaffi_any_type");
	}
	
	if((ti.type != metaffi_any_type) && ti.fixed_dimensions > 0){
		ti.type |= metaffi_array_type;
	}
	
	item.type = ti.type;
	
	// if type is an array with common_type, then extract the common type and reset to array type
	metaffi_type common_type = 0;
	if(ti.type & metaffi_array_type && ti.type != metaffi_array_type)
	{
		common_type = ti.type & ~metaffi_array_type;
		item.type = metaffi_array_type;
	}
	
	switch(item.type)
	{
		case metaffi_float64_type:
		{
			item.cdt_val.float64_val = callbacks.get_float64(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_float32_type:
		{
			item.cdt_val.float32_val = callbacks.get_float32(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_int8_type:
		{
			item.cdt_val.int8_val = callbacks.get_int8(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_uint8_type:
		{
			item.cdt_val.uint8_val = callbacks.get_uint8(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_int16_type:
		{
			item.cdt_val.int16_val = callbacks.get_int16(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_uint16_type:
		{
			item.cdt_val.uint16_val = callbacks.get_uint16(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_int32_type:
		{
			item.cdt_val.int32_val = callbacks.get_int32(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_uint32_type:
		{
			item.cdt_val.uint32_val = callbacks.get_uint32(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_int64_type:
		{
			item.cdt_val.int64_val = callbacks.get_int64(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_uint64_type:
		{
			item.cdt_val.uint64_val = callbacks.get_uint64(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_bool_type:
		{
			item.cdt_val.bool_val = callbacks.get_bool(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_char8_type:
		{
			item.cdt_val.char8_val = callbacks.get_char8(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_string8_type:
		{
			item.cdt_val.string8_val = callbacks.get_string8(current_index.data(), current_index.size(), &item.free_required, callbacks.context);
		}break;
		
		case metaffi_char16_type:
		{
			item.cdt_val.char16_val = callbacks.get_char16(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_string16_type:
		{
			item.cdt_val.string16_val = callbacks.get_string16(current_index.data(), current_index.size(), &item.free_required, callbacks.context);
		}break;
		
		case metaffi_char32_type:
		{
			item.cdt_val.char32_val = callbacks.get_char32(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_string32_type:
		{
			item.cdt_val.string32_val = callbacks.get_string32(current_index.data(), current_index.size(), &item.free_required, callbacks.context);
		}break;
		
		case metaffi_handle_type:
		{
			item.cdt_val.handle_val = callbacks.get_handle(current_index.data(), current_index.size(), &item.free_required, callbacks.context);
		}break;
		
		case metaffi_null_type:
		{;
			item.free_required = false;
		}break;
		
		case metaffi_array_type:
		{
			metaffi_bool is_manually_construct_array = 0;
			metaffi_bool is_fixed_dimension = 0;
			metaffi_bool is_1d_array = 0;
			metaffi_size array_length = callbacks.get_array_metadata(current_index.data(),
			                                                current_index.size(),
			                                                &is_fixed_dimension,
				                                            &is_1d_array,
			                                                &common_type,
			                                                &is_manually_construct_array,
			                                                callbacks.context);
			
			item.type = common_type == metaffi_any_type ? metaffi_array_type : (common_type | metaffi_array_type);
			item.free_required = true;
			item.cdt_val.array_val = new cdts{array_length, (metaffi_int64)INT_MIN};
			
			if(is_manually_construct_array)
			{
				// let callback construct the array
				callbacks.construct_cdt_array(current_index.data(), current_index.size(), item.cdt_val.array_val, callbacks.context);
			}
			else // iterate into array
			{
				item.cdt_val.array_val->length = array_length;
				
				// initialize found_dims - if already detected mixed dimensions - skip the dimensions calculation
				// else, set to INT_MIN to perform the calculation
				metaffi_int64 found_dims = is_fixed_dimension ? INT_MIN : MIXED_OR_UNKNOWN_DIMENSIONS;
				for(int i=0 ; i<item.cdt_val.array_val->length ; i++)
				{
					std::vector<metaffi_size> new_index = current_index;
					new_index.emplace_back(i);
					cdt& new_item = item.cdt_val.array_val->arr[i];
					construct_cdt(new_item, callbacks, new_index);
					
					if(is_1d_array && (new_item.type & metaffi_array_type))
					{
						throw std::runtime_error("Something is wrong - 1D array cannot contain another array");
					}
					else if(!is_1d_array) // not 1D array - for fixed_dimensions, all elements have the same dimension
					{
						// if already detected mixed dimensions - skip the dimensions calculation
						if(found_dims == MIXED_OR_UNKNOWN_DIMENSIONS)
						{
							continue;
						}
						
						if((new_item.type & metaffi_array_type) == 0) // found non-array type
						{
							// mixed dimensions!
							found_dims = MIXED_OR_UNKNOWN_DIMENSIONS;
						}
						
						if(found_dims == INT_MIN) // set first dimension found
						{
							found_dims = new_item.cdt_val.array_val->fixed_dimensions;
						}
						else if(found_dims != new_item.cdt_val.array_val->fixed_dimensions) // compare the dimensions, to make sure it is not mixed!
						{
							found_dims = MIXED_OR_UNKNOWN_DIMENSIONS; // mixed dimensions
						}
					}
				}
				
				// if 1d array, set fixed_dimensions to 1
				// if found dims is a positive integer - set fixed_dimensions to found_dims+1
				// if mixed dimensions - set fixed_dimensions to MIXED_OR_UNKNOWN_DIMENSIONS
				if(is_1d_array)
				{
					item.cdt_val.array_val->fixed_dimensions = 1;
				}
				else if(found_dims != MIXED_OR_UNKNOWN_DIMENSIONS)
				{
					item.cdt_val.array_val->fixed_dimensions = found_dims+1;
				}
				else
				{
					item.cdt_val.array_val->fixed_dimensions = MIXED_OR_UNKNOWN_DIMENSIONS;
				}
			}
			
		}break;
		
		case metaffi_callable_type:
		{
			if(!item.cdt_val.callable_val)
			{
				item.cdt_val.callable_val = new cdt_metaffi_callable{};
			}
			
			item.free_required = true; // default is true
			item.cdt_val.callable_val = callbacks.get_callable(current_index.data(), current_index.size(), &item.free_required, callbacks.context);
		}break;
		
		default:
		{
			std::stringstream ss;
			ss << "Unknown type while constructing CDTS: " << item.type;
			throw std::runtime_error(ss.str());
		}
	}
}
//--------------------------------------------------------------------
void construct_cdts(cdts& arr, const metaffi::runtime::construct_cdts_callbacks& callbacks)
{
	construct_cdts(arr, callbacks, {});
}

void construct_cdts(cdts* arr, const construct_cdts_callbacks* callbacks, char** out_nul_term_err) noexcept
{
	try
	{
		construct_cdts(*arr, *callbacks);
	}
	catch(const std::exception& e)
	{
		std::string err = e.what();
		*out_nul_term_err = new char[err.size()+1];
		std::copy(err.begin(), err.end(), *out_nul_term_err);
		(*out_nul_term_err)[err.size()] = '\0';
	}
}
//--------------------------------------------------------------------
void construct_cdts(cdts& arr, const metaffi::runtime::construct_cdts_callbacks& callbacks, const std::vector<metaffi_size>& starting_index, const metaffi_type_info& known_type /*=metaffi_any_type*/)
{
	std::queue<std::pair<std::vector<metaffi_size>, cdt&>> queue;

	if(arr.length == 0)
	{
		arr.length = callbacks.get_root_elements_count(callbacks.context);
		delete[] arr.arr;
		
		arr.arr = new cdt[arr.length]{};
	}
	
	for(metaffi_size i=0 ; i<arr.length ; i++)
	{
		std::vector<metaffi_size> index = starting_index;
		index.emplace_back(i);
		queue.emplace(index, arr[i]);
	}
	
	while(!queue.empty())
	{
		auto [currentIndex, pcdt] = queue.front();
		queue.pop();
		
		metaffi::runtime::construct_cdt(pcdt, callbacks, currentIndex);
	}
}
//--------------------------------------------------------------------

}