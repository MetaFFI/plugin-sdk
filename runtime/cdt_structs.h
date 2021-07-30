#pragma once
#include "metaffi_primitives.h"

// It is important that the structs will hold a pointer to the primitives and NOT the primitives themselves.
// this is to avoid needless copy and the ability to pass by reference.

#define cdt_numeric(type) \
struct cdt_##type\
{\
	type val; \
};\
struct cdt_##type##_array\
{\
	type* vals;\
    metaffi_size* dimensions_lengths; \
    metaffi_size dimensions;\
};

#define cdt_string(type) \
struct cdt_##type\
{\
	type val;\
	metaffi_size length;\
};\
struct cdt_##type##_array\
{\
	type* vals;\
    metaffi_size* vals_sizes;\
    metaffi_size* dimensions_lengths; \
    metaffi_size dimensions;\
};

cdt_numeric(metaffi_float32);
cdt_numeric(metaffi_float64);
cdt_numeric(metaffi_int8);
cdt_numeric(metaffi_int16);
cdt_numeric(metaffi_int32);
cdt_numeric(metaffi_int64);
cdt_numeric(metaffi_uint8);
cdt_numeric(metaffi_uint16);
cdt_numeric(metaffi_uint32);
cdt_numeric(metaffi_uint64);
cdt_numeric(metaffi_bool);
cdt_numeric(metaffi_size);
cdt_numeric(metaffi_type);

cdt_numeric(metaffi_handle);

cdt_string(metaffi_string8);
cdt_string(metaffi_string16);
cdt_string(metaffi_string32);



union cdt_types
{
	struct cdt_metaffi_float32 metaffi_float32_val;
	struct cdt_metaffi_float32_array metaffi_float32_array_val;
	
	struct cdt_metaffi_float64 metaffi_float64_val;
	struct cdt_metaffi_float64_array metaffi_float64_array_val;
	
	struct cdt_metaffi_int8 metaffi_int8_val;
	struct cdt_metaffi_int8_array metaffi_int8_array_val;
	
	struct cdt_metaffi_int16 metaffi_int16_val;
	struct cdt_metaffi_int16_array metaffi_int16_array_val;
	
	struct cdt_metaffi_int32 metaffi_int32_val;
	struct cdt_metaffi_int32_array metaffi_int32_array_val;
	
	struct cdt_metaffi_int64 metaffi_int64_val;
	struct cdt_metaffi_int64_array metaffi_int64_array_val;
	
	struct cdt_metaffi_uint8 metaffi_uint8_val;
	struct cdt_metaffi_uint8_array metaffi_uint8_array_val;
	
	struct cdt_metaffi_uint16 metaffi_uint16_val;
	struct cdt_metaffi_uint16_array metaffi_uint16_array_val;
	
	struct cdt_metaffi_uint32 metaffi_uint32_val;
	struct cdt_metaffi_uint32_array metaffi_uint32_array_val;
	
	struct cdt_metaffi_uint64 metaffi_uint64_val;
	struct cdt_metaffi_uint64_array metaffi_uint64_array_val;
	
	struct cdt_metaffi_bool metaffi_bool_val;
	struct cdt_metaffi_bool_array metaffi_bool_array_val;

	struct cdt_metaffi_string8 metaffi_string8_val;
	struct cdt_metaffi_string8_array metaffi_string8_array_val;
	
	struct cdt_metaffi_string16 metaffi_string16_val;
	struct cdt_metaffi_string16_array metaffi_string16_array_val;
	
	struct cdt_metaffi_string32 metaffi_string32_val;
	struct cdt_metaffi_string32_array metaffi_string32_array_val;
	
	struct cdt_metaffi_size metaffi_size_val;
	struct cdt_metaffi_size_array metaffi_size_array_val;
	
	struct cdt_metaffi_handle metaffi_handle_val;
	struct cdt_metaffi_handle_array metaffi_handle_array_val;
};

struct cdt
{
	metaffi_type type;
	metaffi_bool free_required;
	union cdt_types cdt_val;
};
