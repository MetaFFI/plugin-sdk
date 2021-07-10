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

typedef char openffi_char8;
typedef char16_t openffi_char16;
typedef char32_t openffi_char32;

typedef openffi_char8* openffi_string8;
typedef openffi_char16* openffi_string16;
typedef openffi_char32* openffi_string32;

typedef uint64_t openffi_size; // sizes of array and/or strings passed by this type
typedef void* openffi_handle;
typedef openffi_size openffi_type;

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
	
	openffi_char8_type = 524288ULL,
	openffi_char16_type = 1048576ULL,
	openffi_char32_type = 2097152ULL,
	
	openffi_string8_type = 4096ULL,
	openffi_string16_type = 8192ULL,
	openffi_string32_type = 16384ULL,
	
	openffi_handle_type = 32768ULL,
	
	openffi_array_type = 65536ULL,
	
	openffi_size_type = 262144ULL,

	openffi_float64_array_type = openffi_float64_type | openffi_array_type,
	openffi_float32_array_type = openffi_float32_type | openffi_array_type,
	openffi_int8_array_type = openffi_int8_type | openffi_array_type,
	openffi_int16_array_type = openffi_int16_type | openffi_array_type,
	openffi_int32_array_type = openffi_int32_type | openffi_array_type,
	openffi_int64_array_type = openffi_int64_type | openffi_array_type,
	openffi_uint8_array_type = openffi_uint8_type | openffi_array_type,
	openffi_uint16_array_type = openffi_uint16_type | openffi_array_type,
	openffi_uint32_array_type = openffi_uint32_type | openffi_array_type,
	openffi_uint64_array_type = openffi_uint64_type | openffi_array_type,
	openffi_bool_array_type = openffi_bool_type | openffi_array_type,
	openffi_string8_array_type = openffi_string8_type | openffi_array_type,
	openffi_string16_array_type = openffi_string16_type | openffi_array_type,
	openffi_string32_array_type = openffi_string32_type | openffi_array_type,
	
	openffi_handle_array_type = openffi_handle_type | openffi_array_type,
	openffi_size_array_type = openffi_size_type | openffi_array_type
};
