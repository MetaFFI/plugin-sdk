#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "cdt_structs.h"

#ifdef __cplusplus
extern "C"{
#endif

/************************************************
*   Allocations
*************************************************/

struct cdt* alloc_cdts_buffer(openffi_size cdt_count);

// Declarations
#define alloc_numeric_on_heap_decl_fptr(type) \
type* alloc_##type##_on_heap(type val);  \
type* alloc_##type##_array_on_heap(openffi_size length);

#define alloc_string_on_heap_decl_fptr(type)\
type alloc_##type##_on_heap(type str, openffi_size str_size); \
type* alloc_##type##_array_on_heap(openffi_size length);

alloc_numeric_on_heap_decl_fptr(openffi_float64);
alloc_numeric_on_heap_decl_fptr(openffi_float32);
alloc_numeric_on_heap_decl_fptr(openffi_int64);
alloc_numeric_on_heap_decl_fptr(openffi_int32);
alloc_numeric_on_heap_decl_fptr(openffi_int16);
alloc_numeric_on_heap_decl_fptr(openffi_int8);
alloc_numeric_on_heap_decl_fptr(openffi_uint64);
alloc_numeric_on_heap_decl_fptr(openffi_uint32);
alloc_numeric_on_heap_decl_fptr(openffi_uint16);
alloc_numeric_on_heap_decl_fptr(openffi_uint8);
alloc_numeric_on_heap_decl_fptr(openffi_size);
alloc_numeric_on_heap_decl_fptr(openffi_bool);
alloc_numeric_on_heap_decl_fptr(openffi_type);
alloc_numeric_on_heap_decl_fptr(openffi_handle);

alloc_string_on_heap_decl_fptr(openffi_string8);
alloc_string_on_heap_decl_fptr(openffi_string16);
alloc_string_on_heap_decl_fptr(openffi_string32);

//====================================================================

/************************************************
*   Getters
*************************************************/


openffi_type get_type(struct cdt* data_array, int index);
struct cdt* get_cdt(struct cdt* data_array, int index);

//====================================================================

/************************************************
*   Array Elements Getters
*************************************************/

#define get_numeric_element_decl_fptr(type) \
type get_##type##_element(type* arr, int index);

#define get_string_element_decl_fptr(type) \
type get_##type##_element(type* arr, int index, const openffi_size *sizes, openffi_size *out_size);

get_numeric_element_decl_fptr(openffi_float64);
get_numeric_element_decl_fptr(openffi_float32);
get_numeric_element_decl_fptr(openffi_int64);
get_numeric_element_decl_fptr(openffi_int32);
get_numeric_element_decl_fptr(openffi_int16);
get_numeric_element_decl_fptr(openffi_int8);
get_numeric_element_decl_fptr(openffi_uint64);
get_numeric_element_decl_fptr(openffi_uint32);
get_numeric_element_decl_fptr(openffi_uint16);
get_numeric_element_decl_fptr(openffi_uint8);
get_numeric_element_decl_fptr(openffi_bool);
get_numeric_element_decl_fptr(openffi_size);
get_numeric_element_decl_fptr(openffi_handle);
get_string_element_decl_fptr(openffi_string8);
get_string_element_decl_fptr(openffi_string16);
get_string_element_decl_fptr(openffi_string32);


/************************************************
*   Array Elements Setters
*************************************************/

#define set_numeric_element_decl_fptr(type) \
void set_##type##_element(type* arr, int index, type val);

#define set_string_element_decl_fptr(type) \
void set_##type##_element(type* arr, openffi_size* sizes_array, int index, type str, openffi_size str_size);

set_numeric_element_decl_fptr(openffi_float64);
set_numeric_element_decl_fptr(openffi_float32);
set_numeric_element_decl_fptr(openffi_int64);
set_numeric_element_decl_fptr(openffi_int32);
set_numeric_element_decl_fptr(openffi_int16);
set_numeric_element_decl_fptr(openffi_int8);
set_numeric_element_decl_fptr(openffi_uint64);
set_numeric_element_decl_fptr(openffi_uint32);
set_numeric_element_decl_fptr(openffi_uint16);
set_numeric_element_decl_fptr(openffi_uint8);
set_numeric_element_decl_fptr(openffi_bool);
set_numeric_element_decl_fptr(openffi_size);
set_numeric_element_decl_fptr(openffi_handle);
set_string_element_decl_fptr(openffi_string8);
set_string_element_decl_fptr(openffi_string16);
set_string_element_decl_fptr(openffi_string32);


/************************************************
*   XLLR functions
*************************************************/
void xllr_call(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len,
				int64_t function_id,
				struct cdt* parameters, uint64_t parameters_length,
                struct cdt* return_values, uint64_t return_values_length,
				char** out_err, uint64_t* out_err_len
);

int64_t xllr_load_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
							 const char* function_path, uint32_t function_path_len,
							 int64_t function_id_opt,
							 char** out_err, uint32_t* out_err_len);

void xllr_free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);

/************************************************
*   Misc
*************************************************/

#ifdef _WIN32 //// --- START WINDOWS ---
#include <Windows.h>
void get_last_error_string(DWORD err, char** out_err_str)
#endif

void* load_library(const char* name, char** out_err);
const char* free_library(void* lib);
void* load_symbol(void* handle, const char* name, char** out_err);
const char* load_xllr_api();
const char* load_xllr();
const char* free_xllr();

/************************************************
*   Load cdt functions dynamically
*************************************************/
const char* load_cdt_capi();


#ifdef __cplusplus
}
#endif