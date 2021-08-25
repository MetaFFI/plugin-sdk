#pragma once
#include <stdint.h>
#include <uchar.h>

typedef double metaffi_float64;
typedef float metaffi_float32;

typedef int8_t metaffi_int8;
typedef int16_t metaffi_int16;
typedef int32_t metaffi_int32;
typedef int64_t metaffi_int64;

typedef uint8_t metaffi_uint8;
typedef uint16_t metaffi_uint16;
typedef uint32_t metaffi_uint32;
typedef uint64_t metaffi_uint64;

typedef uint8_t metaffi_bool;

typedef char metaffi_char8;
typedef char16_t metaffi_char16;
typedef char32_t metaffi_char32;

typedef metaffi_char8* metaffi_string8;
typedef metaffi_char16* metaffi_string16;
typedef metaffi_char32* metaffi_string32;

typedef uint64_t metaffi_size; // sizes of array and/or strings passed by this type
typedef void* metaffi_handle;
typedef metaffi_size metaffi_type;

enum metaffi_types
{
	metaffi_float64_type = 1ULL,
	metaffi_float32_type = 2ULL,
	metaffi_int8_type = 4ULL,
	metaffi_int16_type = 8ULL,
	metaffi_int32_type = 16ULL,
	metaffi_int64_type = 32ULL,
	metaffi_uint8_type = 64ULL,
	metaffi_uint16_type = 128ULL,
	metaffi_uint32_type = 256ULL,
	metaffi_uint64_type = 512ULL,
	metaffi_bool_type = 1024ULL,
	
	metaffi_char8_type = 524288ULL,
	metaffi_char16_type = 1048576ULL,
	metaffi_char32_type = 2097152ULL,
	
	metaffi_string8_type = 4096ULL,
	metaffi_string16_type = 8192ULL,
	metaffi_string32_type = 16384ULL,
	
	metaffi_handle_type = 32768ULL,
	
	metaffi_array_type = 65536ULL,
	
	metaffi_size_type = 262144ULL,
	
	metaffi_any_type = 4194304ULL,

	metaffi_float64_array_type = metaffi_float64_type | metaffi_array_type,
	metaffi_float32_array_type = metaffi_float32_type | metaffi_array_type,
	metaffi_int8_array_type = metaffi_int8_type | metaffi_array_type,
	metaffi_int16_array_type = metaffi_int16_type | metaffi_array_type,
	metaffi_int32_array_type = metaffi_int32_type | metaffi_array_type,
	metaffi_int64_array_type = metaffi_int64_type | metaffi_array_type,
	metaffi_uint8_array_type = metaffi_uint8_type | metaffi_array_type,
	metaffi_uint16_array_type = metaffi_uint16_type | metaffi_array_type,
	metaffi_uint32_array_type = metaffi_uint32_type | metaffi_array_type,
	metaffi_uint64_array_type = metaffi_uint64_type | metaffi_array_type,
	metaffi_bool_array_type = metaffi_bool_type | metaffi_array_type,
	metaffi_string8_array_type = metaffi_string8_type | metaffi_array_type,
	metaffi_string16_array_type = metaffi_string16_type | metaffi_array_type,
	metaffi_string32_array_type = metaffi_string32_type | metaffi_array_type,
	
	metaffi_handle_array_type = metaffi_handle_type | metaffi_array_type,
	metaffi_size_array_type = metaffi_size_type | metaffi_array_type
};