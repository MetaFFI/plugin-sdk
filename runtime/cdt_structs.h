#pragma once
#include "openffi_primitives.h"

// It is important that the structs will hold a pointer to the primitives and NOT the primitives themselves.
// this is to avoid needless copy and the ability to pass by reference.

#define cdt_numeric(type) \
struct cdt_##type\
{\
	type val; \
};\
struct cdt_##type##_ptr\
{\
	type* val; \
};\
struct cdt_##type##_array\
{\
	type* vals;\
    openffi_size* dimensions_lengths; \
    openffi_size dimensions;\
};

#define cdt_string(type) \
struct cdt_##type\
{\
	type val;\
	openffi_size length;\
};\
struct cdt_##type##_ptr\
{\
	type* val;\
	openffi_size* length;\
};\
struct cdt_##type##_array\
{\
	type* vals;\
    openffi_size* vals_sizes;\
    openffi_size* dimensions_lengths; \
    openffi_size dimensions;\
};

cdt_numeric(openffi_float32);
cdt_numeric(openffi_float64);
cdt_numeric(openffi_int8);
cdt_numeric(openffi_int16);
cdt_numeric(openffi_int32);
cdt_numeric(openffi_int64);
cdt_numeric(openffi_uint8);
cdt_numeric(openffi_uint16);
cdt_numeric(openffi_uint32);
cdt_numeric(openffi_uint64);
cdt_numeric(openffi_bool);
cdt_numeric(openffi_size);
cdt_numeric(openffi_type);

cdt_string(openffi_string);
cdt_string(openffi_string8);
cdt_string(openffi_string16);
cdt_string(openffi_string32);


union cdt_types
{
	struct cdt_openffi_float32 openffi_float32_val;
	struct cdt_openffi_float32_ptr openffi_float32_ptr_val;
	struct cdt_openffi_float32_array openffi_float32_array_val;
	
	struct cdt_openffi_float64_ptr openffi_float64_ptr_val;
	struct cdt_openffi_float64 openffi_float64_val;
	struct cdt_openffi_float64_array openffi_float64_array_val;
	
	struct cdt_openffi_int8 openffi_int8_val;
	struct cdt_openffi_int8_ptr openffi_int8_ptr_val;
	struct cdt_openffi_int8_array openffi_int8_array_val;
	
	struct cdt_openffi_int16 openffi_int16_val;
	struct cdt_openffi_int16_ptr openffi_int16_ptr_val;
	struct cdt_openffi_int16_array openffi_int16_array_val;
	
	struct cdt_openffi_int32 openffi_int32_val;
	struct cdt_openffi_int32_ptr openffi_int32_ptr_val;
	struct cdt_openffi_int32_array openffi_int32_array_val;
	
	struct cdt_openffi_int64 openffi_int64_val;
	struct cdt_openffi_int64_ptr openffi_int64_ptr_val;
	struct cdt_openffi_int64_array openffi_int64_array_val;
	
	struct cdt_openffi_uint8 openffi_uint8_val;
	struct cdt_openffi_uint8_ptr openffi_uint8_ptr_val;
	struct cdt_openffi_uint8_array openffi_uint8_array_val;
	
	struct cdt_openffi_uint16 openffi_uint16_val;
	struct cdt_openffi_uint16_ptr openffi_uint16_ptr_val;
	struct cdt_openffi_uint16_array openffi_uint16_array_val;
	
	struct cdt_openffi_uint32 openffi_uint32_val;
	struct cdt_openffi_uint32_ptr openffi_uint32_ptr_val;
	struct cdt_openffi_uint32_array openffi_uint32_array_val;
	
	struct cdt_openffi_uint64 openffi_uint64_val;
	struct cdt_openffi_uint64_ptr openffi_uint64_ptr_val;
	struct cdt_openffi_uint64_array openffi_uint64_array_val;
	
	struct cdt_openffi_bool openffi_bool_val;
	struct cdt_openffi_bool_ptr openffi_bool_ptr_val;
	struct cdt_openffi_bool_array openffi_bool_array_val;
	
	struct cdt_openffi_string openffi_string_val;
	struct cdt_openffi_string_ptr openffi_string_ptr_val;
	struct cdt_openffi_string_array openffi_string_array_val;
	
	struct cdt_openffi_string8 openffi_string8_val;
	struct cdt_openffi_string8_ptr openffi_string8_ptr_val;
	struct cdt_openffi_string8_array openffi_string8_array_val;
	
	struct cdt_openffi_string16 openffi_string16_val;
	struct cdt_openffi_string16_ptr openffi_string16_ptr_val;
	struct cdt_openffi_string16_array openffi_string16_array_val;
	
	struct cdt_openffi_string32 openffi_string32_val;
	struct cdt_openffi_string32_ptr openffi_string32_ptr_val;
	struct cdt_openffi_string32_array openffi_string32_array_val;
	
	struct cdt_openffi_size openffi_size_val;
	struct cdt_openffi_size_array openffi_size_array_val;
};

struct cdt
{
	openffi_type type;
	openffi_bool free_required;
	union cdt_types cdt_val;
};





