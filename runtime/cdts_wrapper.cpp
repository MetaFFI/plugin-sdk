#include "cdts_wrapper.h"
#include "cdt_capi_loader.h"
#include <mutex>

namespace metaffi::runtime
{
std::once_flag load_cdt_capi_once;

//--------------------------------------------------------------------
cdts_wrapper::cdts_wrapper(metaffi_size cdt_count)
{
	std::call_once(load_cdt_capi_once, [](){ load_cdt_capi(); });
	
	this->cdts = alloc_cdts_buffer(cdt_count);
	this->cdts_length = cdt_count;
}
//--------------------------------------------------------------------
cdts_wrapper::cdts_wrapper(cdt* cdts, metaffi_size cdts_length):cdts(cdts),cdts_length(cdts_length)
{
	std::call_once(load_cdt_capi_once, [](){ load_cdt_capi(); });
}
//--------------------------------------------------------------------
void cdts_wrapper::parse(void* values_to_set, const cdts_parse_callbacks& callbacks)
{
	for(int index=0 ; index<this->cdts_length ; index++)
	{
		metaffi_type cur_type = get_type(this->cdts, index);
		
#define if_parse_numeric_type(otype) \
	if(cur_type & otype##_type) \
	{ \
		if(cur_type & metaffi_array_type) \
		{ \
            callbacks.on_##otype##_array(values_to_set, \
                                            index,         \
                                            this->cdts[index].cdt_val.otype##_array_val.vals, \
                                            this->cdts[index].cdt_val.otype##_array_val.dimensions_lengths, \
                                            this->cdts[index].cdt_val.otype##_array_val.dimensions); \
            continue;\
		} \
		else \
        { \
			callbacks.on_##otype(values_to_set, index, this->cdts[index].cdt_val.otype##_val.val); \
			continue; \
		}\
	}

#define if_parse_string_type(otype) \
	if(cur_type & otype##_type) \
	{ \
		if(cur_type & metaffi_array_type) \
		{ \
			callbacks.on_##otype##_array(values_to_set, \
										index,                           \
										this->cdts[index].cdt_val.otype##_array_val.vals, \
										this->cdts[index].cdt_val.otype##_array_val.vals_sizes, \
										this->cdts[index].cdt_val.otype##_array_val.dimensions_lengths, \
										this->cdts[index].cdt_val.otype##_array_val.dimensions); \
			continue; \
		} \
		else\
        {\
            callbacks.on_##otype(values_to_set,\
					            index, \
					            this->cdts[index].cdt_val.otype##_val.val, \
					            this->cdts[index].cdt_val.otype##_val.length); \
            continue; \
        }\
	}

		
		if_parse_numeric_type(metaffi_float64)
		else if_parse_numeric_type(metaffi_float32)
		else if_parse_numeric_type(metaffi_int8)
		else if_parse_numeric_type(metaffi_int16)
		else if_parse_numeric_type(metaffi_int32)
		else if_parse_numeric_type(metaffi_int64)
		else if_parse_numeric_type(metaffi_uint8)
		else if_parse_numeric_type(metaffi_uint16)
		else if_parse_numeric_type(metaffi_uint32)
		else if_parse_numeric_type(metaffi_uint64)
		else if_parse_numeric_type(metaffi_bool)
		else if_parse_numeric_type(metaffi_handle)
		else if_parse_string_type(metaffi_string8)
		else if_parse_string_type(metaffi_string16)
		else if_parse_string_type(metaffi_string32)
		else
		{
			// if got here - type is not handled!
			std::stringstream ss;
			ss << "Type: " << cur_type << " is not supported";
			throw std::runtime_error(ss.str());
		}
	}
}
//--------------------------------------------------------------------
void cdts_wrapper::build(const metaffi_types types[], metaffi_size types_length, void* values_to_set, cdts_build_callbacks& callbacks) const
{
	if(types_length != this->cdts_length)
	{
		std::stringstream ss;
		ss << "given types_array length=" << types_length << " is different then CDTS length=" <<this->cdts_length;
		throw std::runtime_error(ss.str());
	}

#define if_build_numeric_type(otype) \
	if(cur_type & otype##_type) \
	{ \
		if(cur_type & metaffi_array_type) \
		{ \
			otype* array; \
			metaffi_size dimensions; \
			metaffi_size* dimensions_lengths; \
            metaffi_bool free_required; \
            callbacks.set_##otype##_array(values_to_set, index, array, dimensions_lengths, dimensions, free_required); \
			this->cdts[index].type = otype##_array_type;\
            this->cdts[index].free_required = free_required; \
			this->cdts[index].cdt_val.otype##_array_val.vals = array;\
            this->cdts[index].cdt_val.otype##_array_val.dimensions_lengths = dimensions_lengths;\
            this->cdts[index].cdt_val.otype##_array_val.dimensions = dimensions;\
            continue; \
		} \
		else \
        { \
			otype val; \
            callbacks.set_##otype(values_to_set, index, val); \
			this->cdts[index].type = otype##_type;\
            this->cdts[index].cdt_val.otype##_val.val = val;\
			this->cdts[index].free_required = 0; \
			continue; \
		}\
	}

#define if_build_string_type(otype) \
	if(cur_type & otype##_type) \
	{ \
		if(cur_type & metaffi_array_type) \
		{ \
			otype* array; \
            metaffi_size* string_lengths; \
            metaffi_size* dimensions_lengths; \
			metaffi_size dimensions; \
            metaffi_bool free_required; \
            callbacks.set_##otype##_array(values_to_set, index, array, string_lengths, dimensions_lengths, dimensions, free_required); \
			this->cdts[index].type = otype##_array_type;\
            this->cdts[index].free_required = free_required; \
			this->cdts[index].cdt_val.otype##_array_val.vals = array;\
            this->cdts[index].cdt_val.otype##_array_val.vals_sizes = string_lengths;\
            this->cdts[index].cdt_val.otype##_array_val.dimensions_lengths = dimensions_lengths;\
            this->cdts[index].cdt_val.otype##_array_val.dimensions = dimensions;\
            continue; \
		} \
		else\
        {\
			otype val; \
            metaffi_size length; \
            metaffi_bool free_required; \
            callbacks.set_##otype(values_to_set, index, val, length); \
			this->cdts[index].type = otype##_type;\
            this->cdts[index].free_required = 0; \
			this->cdts[index].cdt_val.otype##_val.val = val;\
            this->cdts[index].cdt_val.otype##_val.length = length;\
            continue; \
        }\
	}
			
	
	for(int index=0 ; index<types_length ; index++)
	{
		metaffi_type cur_type = types[index];

		if_build_numeric_type(metaffi_float64)
		else if_build_numeric_type(metaffi_float32)
		else if_build_numeric_type(metaffi_int8)
		else if_build_numeric_type(metaffi_int16)
		else if_build_numeric_type(metaffi_int32)
		else if_build_numeric_type(metaffi_int64)
		else if_build_numeric_type(metaffi_uint8)
		else if_build_numeric_type(metaffi_uint16)
		else if_build_numeric_type(metaffi_uint32)
		else if_build_numeric_type(metaffi_uint64)
		else if_build_numeric_type(metaffi_bool)
		else if_build_numeric_type(metaffi_handle)
		else if_build_string_type(metaffi_string8)
		else if_build_string_type(metaffi_string16)
		else if_build_string_type(metaffi_string32)
		else
		{
			// if got here - type is not handled!
			std::stringstream ss;
			ss << "Type: " << cur_type << " is not supported";
			throw std::runtime_error(ss.str());
		}
	}
}
//--------------------------------------------------------------------
cdt* cdts_wrapper::operator[](int index) const
{
	if(index >= this->cdts_length)
	{
		throw std::runtime_error("operator[] requested index is out of bounds");
	}
	
	return &this->cdts[index];
}
//--------------------------------------------------------------------
cdt* cdts_wrapper::get_cdts() const
{
	return this->cdts;
}
//--------------------------------------------------------------------
metaffi_size cdts_wrapper::get_cdts_length() const
{
	return this->cdts_length;
}
//--------------------------------------------------------------------
}