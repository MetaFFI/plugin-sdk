#include "cdts_traverse_construct.h"
#include <queue>
#include <sstream>

//--------------------------------------------------------------------
void traverse_cdts(const cdts& arr, const traverse_cdts_callbacks& callbacks)
{
	if(arr.length == 0){ // empty CDTS
		return;
	}
	
	std::queue<std::pair<std::vector<metaffi_size>, cdt*>> queue;
	for(metaffi_size i=0 ; i<arr.length ; i++)
	{
		std::vector<metaffi_size> index = {i};
		queue.emplace(index, arr[i]);
	}
	
	while(!queue.empty())
	{
		auto [currentIndex, pcdt] = queue.front();
		queue.pop();
		
		if(pcdt->type == metaffi_any_type)
		{
			throw std::runtime_error("traversed CDT must have a concrete type, not dynamic type like metaffi_any_type");
		}
		
		metaffi_type common_type = metaffi_any_type;
		if(pcdt->type & metaffi_array_type && pcdt->type != metaffi_array_type)
		{
			common_type = pcdt->type & ~metaffi_array_type;
			pcdt->type = metaffi_array_type;
		}
		
		switch(pcdt->type)
		{
			case metaffi_float64_type:
			{
				callbacks.on_float64(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.float64_val, callbacks.context);
			}break;
			
			case metaffi_float32_type:
			{
				callbacks.on_float32(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.float32_val, callbacks.context);
			}break;
			
			case metaffi_int8_type:
			{
				callbacks.on_int8(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.int8_val, callbacks.context);
			}break;
			
			case metaffi_uint8_type:
			{
				callbacks.on_uint8(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.uint8_val, callbacks.context);
			}break;
			
			case metaffi_int16_type:
			{
				callbacks.on_int16(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.int16_val, callbacks.context);
			}break;
			
			case metaffi_uint16_type:
			{
				callbacks.on_uint16(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.uint16_val, callbacks.context);
			}break;
			
			case metaffi_int32_type:
			{
				callbacks.on_int32(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.int32_val, callbacks.context);
			}break;
			
			case metaffi_uint32_type:
			{
				callbacks.on_uint32(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.uint32_val, callbacks.context);
			}break;
			
			case metaffi_int64_type:
			{
				callbacks.on_int64(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.int64_val, callbacks.context);
			}break;
			
			case metaffi_uint64_type:
			{
				callbacks.on_uint64(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.uint64_val, callbacks.context);
			}break;
			
			case metaffi_bool_type:
			{
				callbacks.on_bool(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.bool_val, callbacks.context);
			}break;
			
			case metaffi_char8_type:
			{
				callbacks.on_char8(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.char8_val, callbacks.context);
			}break;
			
			case metaffi_string8_type:
			{
				callbacks.on_string8(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.string8_val, callbacks.context);
			}break;
			
			case metaffi_char16_type:
			{
				callbacks.on_char16(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.char16_val, callbacks.context);
			}break;
			
			case metaffi_string16_type:
			{
				callbacks.on_string16(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.string16_val, callbacks.context);
			}break;
			
			case metaffi_char32_type:
			{
				callbacks.on_char32(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.char32_val, callbacks.context);
			}break;
			
			case metaffi_string32_type:
			{
				callbacks.on_string32(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.string32_val, callbacks.context);
			}break;
			
			case metaffi_handle_type:
			{
				callbacks.on_handle(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.handle_val, callbacks.context);
			}break;
			
			case metaffi_callable_type:
			{
				callbacks.on_callable(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.callable_val, callbacks.context);
			}break;
			
			case metaffi_null_type:
			{
				callbacks.on_null(&currentIndex[0], currentIndex.size(), callbacks.context);
			}break;
			
			case metaffi_array_type:
			{
				callbacks.on_array(&currentIndex[0], currentIndex.size(), pcdt->cdt_val.array_val, pcdt->cdt_val.array_val.fixed_dimensions, common_type, callbacks.context);
				
				// append to the queue the current array and its indices
				for(int i = 0; i < pcdt->cdt_val.array_val.length; i++)
				{
					std::vector<metaffi_size> index = currentIndex;
					index.emplace_back(i);
					queue.emplace(index, &(pcdt->cdt_val.array_val.arr[i]));
				}
				
			}break;
			
			default:
			{
				std::stringstream ss;
				ss << "Unknown type while traversing CDTS: " << pcdt->type;
				throw std::runtime_error(ss.str());
			}
		}
	}
}
//--------------------------------------------------------------------
[[maybe_unused]] void construct_cdts(cdts& arr, const construct_cdts_callbacks& callbacks)
{
	std::queue<std::pair<std::vector<metaffi_size>, cdt*>> queue;
	arr.length = callbacks.get_root_elements_count(callbacks.context);
	arr.arr = new cdt[arr.length]{};
	
	for(metaffi_size i=0 ; i<arr.length ; i++)
	{
		std::vector<metaffi_size> index = {i};
		queue.emplace(index, arr[i]);
	}
	
	while(!queue.empty())
	{
		auto [currentIndex, pcdt] = queue.front();
		queue.pop();
		
		metaffi_type_info ti = callbacks.get_type_info(&currentIndex[0], currentIndex.size(), callbacks.context);
		if(ti.type == metaffi_any_type)
		{
			throw std::runtime_error("get_type_info must return a concrete type, not dynamic type like metaffi_any_type");
		}
		
		pcdt->type = ti.type;
		
		// if type is an array with common_type, then extract the common type and reset to array type
		metaffi_type common_type = 0;
		if(ti.type & metaffi_array_type && ti.type != metaffi_array_type)
		{
			common_type = ti.type & ~metaffi_array_type;
			pcdt->type = metaffi_array_type;
		}
		
		switch(pcdt->type)
		{
			case metaffi_float64_type:
			{
				pcdt->cdt_val.float64_val = callbacks.get_float64(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_float32_type:
			{
				pcdt->cdt_val.float32_val = callbacks.get_float32(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_int8_type:
			{
				pcdt->cdt_val.int8_val = callbacks.get_int8(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_uint8_type:
			{
				pcdt->cdt_val.uint8_val = callbacks.get_uint8(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_int16_type:
			{
				pcdt->cdt_val.int16_val = callbacks.get_int16(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_uint16_type:
			{
				pcdt->cdt_val.uint16_val = callbacks.get_uint16(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_int32_type:
			{
				pcdt->cdt_val.int32_val = callbacks.get_int32(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_uint32_type:
			{
				pcdt->cdt_val.uint32_val = callbacks.get_uint32(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_int64_type:
			{
				pcdt->cdt_val.int64_val = callbacks.get_int64(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_uint64_type:
			{
				pcdt->cdt_val.uint64_val = callbacks.get_uint64(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_bool_type:
			{
				pcdt->cdt_val.bool_val = callbacks.get_bool(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_char8_type:
			{
				pcdt->cdt_val.char8_val = callbacks.get_char8(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_string8_type:
			{
				pcdt->cdt_val.string8_val = callbacks.get_string8(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = true;
			}break;
			
			case metaffi_char16_type:
			{
				pcdt->cdt_val.char16_val = callbacks.get_char16(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_string16_type:
			{
				pcdt->cdt_val.string16_val = callbacks.get_string16(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = true;
			}break;
			
			case metaffi_char32_type:
			{
				pcdt->cdt_val.char32_val = callbacks.get_char32(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = false;
			}break;
			
			case metaffi_string32_type:
			{
				pcdt->cdt_val.string32_val = callbacks.get_string32(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = true;
			}break;
			
			case metaffi_handle_type:
			{
				pcdt->cdt_val.handle_val = callbacks.get_handle(&currentIndex[0], currentIndex.size(), callbacks.context);
				pcdt->free_required = true;
			}break;
			
			case metaffi_null_type:
			{;
				pcdt->free_required = false;
			}break;
			
			case metaffi_array_type:
			{
				pcdt->cdt_val.array_val.fixed_dimensions = -1;
				pcdt->cdt_val.array_val.length = callbacks.get_array(&currentIndex[0], currentIndex.size(), &(pcdt->cdt_val.array_val.fixed_dimensions), &common_type, callbacks.context);
				pcdt->type = common_type | metaffi_array_type;
				pcdt->cdt_val.array_val.arr = new cdt[pcdt->cdt_val.array_val.length]{};
				pcdt->free_required = true;
				
				// append to the queue the current array and its indices
				for(int i=0 ; i<pcdt->cdt_val.array_val.length ; i++)
				{
					std::vector<metaffi_size> index = currentIndex;
					index.emplace_back(i);
					queue.emplace(index, &(pcdt->cdt_val.array_val.arr[i]));
				}
			}break;
			
			case metaffi_callable_type:
			{
				pcdt->cdt_val.callable_val = callbacks.get_callable(&currentIndex[0], currentIndex.size() ,callbacks.context);
				pcdt->free_required = true;
			}break;
			
			default:
			{
				std::stringstream ss;
				ss << "Unknown type while constructing CDTS: " << pcdt->type;
				throw std::runtime_error(ss.str());
			}
		}
		
	}
}
//--------------------------------------------------------------------