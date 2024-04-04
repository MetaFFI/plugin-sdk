#include "cdts_traverse_construct.h"
#include <queue>
#include <sstream>

namespace metaffi::runtime
{

//--------------------------------------------------------------------

void traverse_cdt(const cdt& item, const metaffi::runtime::traverse_cdts_callbacks& callbacks)
{
	traverse_cdt(item, callbacks, {});
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
			callbacks.on_handle(current_index.data(), current_index.size(), item.cdt_val.handle_val, callbacks.context);
		}break;
		
		case metaffi_callable_type:
		{
			callbacks.on_callable(current_index.data(), current_index.size(), item.cdt_val.callable_val, callbacks.context);
		}break;
		
		case metaffi_null_type:
		{
			callbacks.on_null(current_index.data(), current_index.size(), callbacks.context);
		}break;
		
		case metaffi_array_type:
		{
			callbacks.on_array(current_index.data(), current_index.size(), item.cdt_val.array_val, item.cdt_val.array_val.fixed_dimensions, common_type, callbacks.context);
			
			traverse_cdts(item.cdt_val.array_val, callbacks, current_index);
			
			// append to the queue the current array and its indices
//			for(int i = 0; i < item.cdt_val.array_val.length; i++)
//			{
//				std::vector<metaffi_size> index = current_index;
//				index.emplace_back(i);
//				queue.emplace(index, &(item.cdt_val.array_val.arr[i]));
//			}
		
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

void construct_cdt(cdt& item, const metaffi::runtime::construct_cdts_callbacks& callbacks, const std::vector<metaffi_size>& current_index)
{
	metaffi_type_info ti = callbacks.get_type_info(current_index.data(), current_index.size(), callbacks.context);
	if(ti.type == metaffi_any_type)
	{
		throw std::runtime_error("get_type_info must return a concrete type, not dynamic type like metaffi_any_type");
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
			item.cdt_val.string8_val = callbacks.get_string8(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = true;
		}break;
		
		case metaffi_char16_type:
		{
			item.cdt_val.char16_val = callbacks.get_char16(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_string16_type:
		{
			item.cdt_val.string16_val = callbacks.get_string16(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = true;
		}break;
		
		case metaffi_char32_type:
		{
			item.cdt_val.char32_val = callbacks.get_char32(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = false;
		}break;
		
		case metaffi_string32_type:
		{
			item.cdt_val.string32_val = callbacks.get_string32(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = true;
		}break;
		
		case metaffi_handle_type:
		{
			item.cdt_val.handle_val = callbacks.get_handle(current_index.data(), current_index.size(), callbacks.context);
			item.free_required = true;
		}break;
		
		case metaffi_null_type:
		{;
			item.free_required = false;
		}break;
		
		case metaffi_array_type:
		{
			item.cdt_val.array_val.fixed_dimensions = ti.fixed_dimensions;
			item.cdt_val.array_val.length = callbacks.get_array(current_index.data(), current_index.size(), &(item.cdt_val.array_val.fixed_dimensions), &common_type, callbacks.context);
			item.type = common_type | metaffi_array_type;
			item.cdt_val.array_val.arr = new cdt[item.cdt_val.array_val.length]{};
			for(int i=0 ; i<item.cdt_val.array_val.length ; i++)
			{
				std::vector<metaffi_size> new_index = current_index;
				new_index.emplace_back(i);
				construct_cdt(item.cdt_val.array_val.arr[i], callbacks, new_index);
			}
			
			item.free_required = true;
			
			
			// append to the queue the current array and its indices
//			for(int i=0 ; i<item.cdt_val.array_val.length ; i++)
//			{
//				std::vector<metaffi_size> index = currentIndex;
//				index.emplace_back(i);
//				queue.emplace(index, &(item.cdt_val.array_val.arr[i]));
//			}
		}break;
		
		case metaffi_callable_type:
		{
			item.cdt_val.callable_val = callbacks.get_callable(current_index.data(), current_index.size() , callbacks.context);
			item.free_required = true;
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
//--------------------------------------------------------------------
void construct_cdts(cdts& arr, const metaffi::runtime::construct_cdts_callbacks& callbacks, const std::vector<metaffi_size>& starting_index)
{
	std::queue<std::pair<std::vector<metaffi_size>, cdt&>> queue;

	if(arr.length == 0)
	{
		arr.length = callbacks.get_root_elements_count(callbacks.context);
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