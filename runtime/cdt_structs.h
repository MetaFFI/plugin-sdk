#pragma once
#include "metaffi_primitives.h"

// It is important that the structs will hold a pointer to the primitives and NOT the primitives themselves.
// this is to avoid needless copy and the ability to pass by reference.

#define cdts_cache_size 50

#define cdt_struct(type) \
struct cdt_##type##_array\
{\
	union{                   \
		type* vals; \
        struct cdt_##type##_array* arr; \
    };\
    metaffi_size length; \
    metaffi_size dimension;\
};

cdt_struct(metaffi_float32);
cdt_struct(metaffi_float64);
cdt_struct(metaffi_int8);
cdt_struct(metaffi_int16);
cdt_struct(metaffi_int32);
cdt_struct(metaffi_int64);
cdt_struct(metaffi_uint8);
cdt_struct(metaffi_uint16);
cdt_struct(metaffi_uint32);
cdt_struct(metaffi_uint64);
cdt_struct(metaffi_bool);
cdt_struct(metaffi_string8);
cdt_struct(metaffi_string16);
cdt_struct(metaffi_string32);

struct cdt_metaffi_handle
{
	metaffi_handle val;
	uint64_t runtime_id;
	void* release;
};
struct cdt_metaffi_handle_array
{
	union{
		struct cdt_metaffi_handle* vals;
		struct cdt_metaffi_handle_array* arr;
	};
	
	metaffi_size   length;
	metaffi_size    dimension;
};


struct cdt_metaffi_callable
{
	metaffi_callable val;
	metaffi_type* parameters_types;
	metaffi_int8 params_types_length;
	metaffi_type* retval_types;
	metaffi_int8 retval_types_length;
};

union cdt_types
{
	metaffi_float32 metaffi_float32_val;
	struct cdt_metaffi_float32_array metaffi_float32_array_val;
	
	metaffi_float64 metaffi_float64_val;
	struct cdt_metaffi_float64_array metaffi_float64_array_val;
	
	metaffi_int8 metaffi_int8_val;
	struct cdt_metaffi_int8_array metaffi_int8_array_val;
	
	metaffi_int16 metaffi_int16_val;
	struct cdt_metaffi_int16_array metaffi_int16_array_val;
	
	metaffi_int32 metaffi_int32_val;
	struct cdt_metaffi_int32_array metaffi_int32_array_val;
	
	metaffi_int64 metaffi_int64_val;
	struct cdt_metaffi_int64_array metaffi_int64_array_val;
	
	metaffi_uint8 metaffi_uint8_val;
	struct cdt_metaffi_uint8_array metaffi_uint8_array_val;
	
	metaffi_uint16 metaffi_uint16_val;
	struct cdt_metaffi_uint16_array metaffi_uint16_array_val;
	
	metaffi_uint32 metaffi_uint32_val;
	struct cdt_metaffi_uint32_array metaffi_uint32_array_val;
	
	metaffi_uint64 metaffi_uint64_val;
	struct cdt_metaffi_uint64_array metaffi_uint64_array_val;
	
	metaffi_bool metaffi_bool_val;
	struct cdt_metaffi_bool_array metaffi_bool_array_val;

	metaffi_string8 metaffi_string8_val;
	struct cdt_metaffi_string8_array metaffi_string8_array_val;
	
	metaffi_string16 metaffi_string16_val;
	struct cdt_metaffi_string16_array metaffi_string16_array_val;
	
	metaffi_string32 metaffi_string32_val;
	struct cdt_metaffi_string32_array metaffi_string32_array_val;
	
	struct cdt_metaffi_handle metaffi_handle_val;
	struct cdt_metaffi_handle_array metaffi_handle_array_val;

	struct cdt_metaffi_callable metaffi_callable_val;
};

typedef struct cdt
{
	metaffi_type type;
	metaffi_bool free_required;
	union cdt_types cdt_val;
}cdt;

typedef struct cdts
{
	cdt* pcdt;
	uint64_t len;
	char is_free;
}cdts;