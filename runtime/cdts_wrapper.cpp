#include "cdts_wrapper.h"
#include "cdt_capi_loader.h"
#include <mutex>
#include "../../metaffi-core/XLLR/cdts_alloc.h"

// NOLINT(bugprone-macro-parentheses)

struct capi_loader
{
	capi_loader()
	{
		const char* err = load_cdt_capi();
		if(err){
			throw std::runtime_error(err);
		}
	}
};
static capi_loader _l; // load statically the CDTS CAPI-API


namespace metaffi::runtime
{
//--------------------------------------------------------------------
cdts_wrapper::cdts_wrapper(cdt* cdts, metaffi_size cdts_length, bool is_free_cdts /*= false*/):cdts(cdts),cdts_length(cdts_length), is_free_cdts(is_free_cdts)
{
}
//--------------------------------------------------------------------
cdts_wrapper::~cdts_wrapper()
{
	//if(is_free_cdts){ free(this->cdts); }
}
//--------------------------------------------------------------------
void cdts_wrapper::parse(void* values_to_set, const cdts_parse_callbacks& callbacks)
{
	for(int index=0 ; index<this->cdts_length ; index++)
	{
		metaffi_type cur_type = get_type(this->cdts, index);
		
#define if_parse_numeric_type(otype) \
	case otype##_type: \
    { \
        callbacks.on_##otype(values_to_set, index, this->cdts[index].cdt_val.otype##_val.val); \
        continue; \
    }break;                             \
	case otype##_type | metaffi_array_type: \
	{ \
        callbacks.on_##otype##_array(values_to_set, \
                                        index,         \
                                        this->cdts[index].cdt_val.otype##_array_val.vals, \
                                        this->cdts[index].cdt_val.otype##_array_val.dimensions_lengths, \
                                        this->cdts[index].cdt_val.otype##_array_val.dimensions); \
        continue;\
	}break; \
	

#define if_parse_string_type(otype) \
	case otype##_type: \
    {                                  \
        callbacks.on_##otype(values_to_set,\
                                index, \
                                this->cdts[index].cdt_val.otype##_val.val, \
                                this->cdts[index].cdt_val.otype##_val.length); \
        continue;               \
    }break;                            \
	case otype##_type | metaffi_array_type: \
	{ \
		callbacks.on_##otype##_array(values_to_set, \
									index,                           \
									this->cdts[index].cdt_val.otype##_array_val.vals, \
									this->cdts[index].cdt_val.otype##_array_val.vals_sizes, \
									this->cdts[index].cdt_val.otype##_array_val.dimensions_lengths, \
									this->cdts[index].cdt_val.otype##_array_val.dimensions); \
		continue; \
	}break; \
	
		
		switch (cur_type)
		{
			if_parse_numeric_type(metaffi_float64);
			if_parse_numeric_type(metaffi_float32);
			if_parse_numeric_type(metaffi_int8);
			if_parse_numeric_type(metaffi_int16);
			if_parse_numeric_type(metaffi_int32);
			if_parse_numeric_type(metaffi_int64);
			if_parse_numeric_type(metaffi_uint8);
			if_parse_numeric_type(metaffi_uint16);
			if_parse_numeric_type(metaffi_uint32);
			if_parse_numeric_type(metaffi_uint64);
			if_parse_numeric_type(metaffi_bool);
			if_parse_numeric_type(metaffi_handle);
			if_parse_string_type(metaffi_string8);
			if_parse_string_type(metaffi_string16);
			if_parse_string_type(metaffi_string32);
			default:
			{
				// if got here - type is not handled!
				std::stringstream ss;
				ss << "Type: " << cur_type << " at index " << index << " is not supported";
				throw std::runtime_error(ss.str());
			}
		}
	}
}
//--------------------------------------------------------------------
void cdts_wrapper::build(const metaffi_types types[], metaffi_size types_length, void* values_to_set, int starting_index, cdts_build_callbacks& callbacks) const
{
#ifdef _DEBUG
	if(types_length != this->cdts_length)
	{
		std::stringstream ss;
		ss << "given types_array length=" << types_length << " is different then CDTS length=" <<this->cdts_length;
		throw std::runtime_error(ss.str());
	}
#endif

#define if_build_numeric_type(otype) \
	case otype##_type: \
    {                                   \
        otype val; \
        callbacks.set_##otype(values_to_set, index, val, starting_index); \
        this->cdts[index].type = otype##_type;\
        this->cdts[index].cdt_val.otype##_val.val = val;\
        this->cdts[index].free_required = 0; \
        continue;                          \
    }break;                             \
	case otype##_type | metaffi_array_type: \
	{ \
		otype* array; \
		metaffi_size dimensions; \
		metaffi_size* dimensions_lengths; \
        metaffi_bool free_required; \
        callbacks.set_##otype##_array(values_to_set, index, array, dimensions_lengths, dimensions, free_required, starting_index); \
		this->cdts[index].type = otype##_array_type;\
        this->cdts[index].free_required = free_required; \
		this->cdts[index].cdt_val.otype##_array_val.vals = array;\
        this->cdts[index].cdt_val.otype##_array_val.dimensions_lengths = dimensions_lengths;\
        this->cdts[index].cdt_val.otype##_array_val.dimensions = dimensions;\
        continue; \
	}break;

#define if_build_string_type(otype) \
	case otype##_type: \
    { \
        otype val; \
        metaffi_size length; \
        metaffi_bool free_required; \
        callbacks.set_##otype(values_to_set, index, val, length, starting_index); \
        this->cdts[index].type = otype##_type;\
        this->cdts[index].free_required = 0; \
        this->cdts[index].cdt_val.otype##_val.val = val;\
        this->cdts[index].cdt_val.otype##_val.length = length;\
        continue;                   \
    }break;                            \
	case otype##_type | metaffi_array_type: \
	{ \
		otype* array; \
        metaffi_size* string_lengths; \
        metaffi_size* dimensions_lengths; \
		metaffi_size dimensions; \
        metaffi_bool free_required; \
        callbacks.set_##otype##_array(values_to_set, index, array, string_lengths, dimensions_lengths, dimensions, free_required, starting_index); \
		this->cdts[index].type = otype##_array_type;\
        this->cdts[index].free_required = free_required; \
		this->cdts[index].cdt_val.otype##_array_val.vals = array;\
        this->cdts[index].cdt_val.otype##_array_val.vals_sizes = string_lengths;\
        this->cdts[index].cdt_val.otype##_array_val.dimensions_lengths = dimensions_lengths;\
        this->cdts[index].cdt_val.otype##_array_val.dimensions = dimensions;\
        continue; \
	}break;
	
	for(int index=0 ; index<types_length ; index++)
	{
		metaffi_type cur_type = types[index];

		switch(cur_type)
		{
			if_build_numeric_type(metaffi_float64);
			if_build_numeric_type(metaffi_float32);
			if_build_numeric_type(metaffi_int8);
			if_build_numeric_type(metaffi_int16);
			if_build_numeric_type(metaffi_int32);
			if_build_numeric_type(metaffi_int64);
			if_build_numeric_type(metaffi_uint8);
			if_build_numeric_type(metaffi_uint16);
			if_build_numeric_type(metaffi_uint32);
			if_build_numeric_type(metaffi_uint64);
			if_build_numeric_type(metaffi_bool);
			if_build_numeric_type(metaffi_handle);
			if_build_string_type(metaffi_string8);
			if_build_string_type(metaffi_string16);
			if_build_string_type(metaffi_string32);
			default:
			{
				// if got here - type is not handled!
				std::stringstream ss;
				ss << "Type: " << cur_type << " at index " << index << " is not supported";
				throw std::runtime_error(ss.str());
			}
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