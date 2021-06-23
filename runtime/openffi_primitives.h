#pragma once
#include <stdint.h>
#include <uchar.h>

typedef double openffi_float64;
typedef float openffi_float32;

typedef int8_t openffi_int8;
typedef int16_t openffi_int16;
typedef int32_t openffi_int32;
typedef int64_t openffi_int64;

typedef uint8_t openffi_uint8;
typedef uint16_t openffi_uint16;
typedef uint32_t openffi_uint32;
typedef uint64_t openffi_uint64;

typedef uint8_t openffi_bool;

typedef char* openffi_string;
typedef char* openffi_string8;
typedef char16_t* openffi_string16;
typedef char32_t* openffi_string32;

typedef uint64_t openffi_size; // sizes of array and/or strings passed by this type
typedef void* openffi_handle;
typedef uint64_t openffi_type;

enum openffi_types
{
	openffi_float64_type = 1ULL,
	openffi_float32_type = 2ULL,
	openffi_int8_type = 4ULL,
	openffi_int16_type = 8ULL,
	openffi_int32_type = 16ULL,
	openffi_int64_type = 32ULL,
	openffi_uint8_type = 64ULL,
	openffi_uint16_type = 128ULL,
	openffi_uint32_type = 256ULL,
	openffi_uint64_type = 512ULL,
	openffi_bool_type = 1024ULL,
	openffi_string_type = 2048ULL,
	openffi_string8_type = 4096ULL,
	openffi_string16_type = 8192ULL,
	openffi_string32_type = 16384ULL,
	openffi_handle_type = 32768ULL,
	
	openffi_array_type = 65536ULL,
	openffi_pointer_type = 131072ULL,
	
	openffi_size_type = 262144ULL,
};

enum openffi_types_size
{
	openffi_type_type_size = 1ULL,
	
	openffi_float64_type_size = 1ULL,
	openffi_float32_type_size = 1ULL,
	openffi_int8_type_size = 1ULL,
	openffi_int16_type_size = 1ULL,
	openffi_int32_type_size = 1ULL,
	openffi_int64_type_size = 1ULL,
	openffi_uint8_type_size = 1ULL,
	openffi_uint16_type_size = 1ULL,
	openffi_uint32_type_size = 1ULL,
	openffi_uint64_type_size = 1ULL,
	openffi_bool_type_size = 1ULL,
	openffi_size_type_size = 1ULL,
	
	openffi_string_type_size = 2ULL,
	openffi_string8_type_size = 2ULL,
	openffi_string16_type_size = 2ULL,
	openffi_string32_type_size = 2ULL,
	
	openffi_handle_type_size = 2ULL,
	
	openffi_array_type_size = 2ULL,
};