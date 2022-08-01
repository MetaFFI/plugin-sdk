#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "cdt_structs.h"

#ifdef __cplusplus
extern "C"{
#endif

#define METAFFI_NULL_HANDLE (metaffi_handle)NULL;

/************************************************
*   Allocations
*************************************************/

// Declarations
#define alloc_numeric_on_heap_decl_fptr(type) \
type* alloc_##type##_on_heap(type val);  \
type* alloc_##type##_array_on_heap(metaffi_size length);

#define alloc_string_on_heap_decl_fptr(type)\
type alloc_##type##_on_heap(type str, metaffi_size str_size); \
type* alloc_##type##_array_on_heap(metaffi_size length);

alloc_numeric_on_heap_decl_fptr(metaffi_float64);
alloc_numeric_on_heap_decl_fptr(metaffi_float32);
alloc_numeric_on_heap_decl_fptr(metaffi_int64);
alloc_numeric_on_heap_decl_fptr(metaffi_int32);
alloc_numeric_on_heap_decl_fptr(metaffi_int16);
alloc_numeric_on_heap_decl_fptr(metaffi_int8);
alloc_numeric_on_heap_decl_fptr(metaffi_uint64);
alloc_numeric_on_heap_decl_fptr(metaffi_uint32);
alloc_numeric_on_heap_decl_fptr(metaffi_uint16);
alloc_numeric_on_heap_decl_fptr(metaffi_uint8);
alloc_numeric_on_heap_decl_fptr(metaffi_size);
alloc_numeric_on_heap_decl_fptr(metaffi_bool);
alloc_numeric_on_heap_decl_fptr(metaffi_type);
alloc_numeric_on_heap_decl_fptr(metaffi_handle);

alloc_string_on_heap_decl_fptr(metaffi_string8);
alloc_string_on_heap_decl_fptr(metaffi_string16);
alloc_string_on_heap_decl_fptr(metaffi_string32);

//====================================================================

/************************************************
*   Getters
*************************************************/


metaffi_type get_type(struct cdt* data_array, int index);
struct cdt* get_cdt(struct cdt* data_array, int index);

//====================================================================

/************************************************
*   Array Elements Getters
*************************************************/

#define get_numeric_element_decl_fptr(type) \
type get_##type##_element(type* arr, int index);

#define get_string_element_decl_fptr(type) \
type get_##type##_element(type* arr, int index, const metaffi_size *sizes, metaffi_size *out_size);

get_numeric_element_decl_fptr(metaffi_float64);
get_numeric_element_decl_fptr(metaffi_float32);
get_numeric_element_decl_fptr(metaffi_int64);
get_numeric_element_decl_fptr(metaffi_int32);
get_numeric_element_decl_fptr(metaffi_int16);
get_numeric_element_decl_fptr(metaffi_int8);
get_numeric_element_decl_fptr(metaffi_uint64);
get_numeric_element_decl_fptr(metaffi_uint32);
get_numeric_element_decl_fptr(metaffi_uint16);
get_numeric_element_decl_fptr(metaffi_uint8);
get_numeric_element_decl_fptr(metaffi_bool);
get_numeric_element_decl_fptr(metaffi_size);
get_numeric_element_decl_fptr(metaffi_handle);
get_string_element_decl_fptr(metaffi_string8);
get_string_element_decl_fptr(metaffi_string16);
get_string_element_decl_fptr(metaffi_string32);


/************************************************
*   Array Elements Setters
*************************************************/

#define set_numeric_element_decl_fptr(type) \
void set_##type##_element(type* arr, int index, type val);

#define set_string_element_decl_fptr(type) \
void set_##type##_element(type* arr, metaffi_size* sizes_array, int index, type str, metaffi_size str_size);

set_numeric_element_decl_fptr(metaffi_float64);
set_numeric_element_decl_fptr(metaffi_float32);
set_numeric_element_decl_fptr(metaffi_int64);
set_numeric_element_decl_fptr(metaffi_int32);
set_numeric_element_decl_fptr(metaffi_int16);
set_numeric_element_decl_fptr(metaffi_int8);
set_numeric_element_decl_fptr(metaffi_uint64);
set_numeric_element_decl_fptr(metaffi_uint32);
set_numeric_element_decl_fptr(metaffi_uint16);
set_numeric_element_decl_fptr(metaffi_uint8);
set_numeric_element_decl_fptr(metaffi_bool);
set_numeric_element_decl_fptr(metaffi_size);
set_numeric_element_decl_fptr(metaffi_handle);
set_string_element_decl_fptr(metaffi_string8);
set_string_element_decl_fptr(metaffi_string16);
set_string_element_decl_fptr(metaffi_string32);


/************************************************
*   XLLR functions
*************************************************/
void xllr_xcall_params_ret(int64_t function_id,
				struct cdts parameters[2],
				char** out_err, uint64_t* out_err_len
);

void xllr_xcall_no_params_ret(int64_t function_id,
                struct cdts return_values[1],
                char** out_err, uint64_t* out_err_len
);

void xllr_xcall_params_no_ret(int64_t function_id,
                struct cdts parameters[1],
                char** out_err, uint64_t* out_err_len
);

void xllr_xcall_no_params_no_ret(int64_t function_id,
                char** out_err, uint64_t* out_err_len
);

int64_t xllr_load_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
							 const char* function_path, uint32_t function_path_len,
							 int64_t function_id_opt,
                            int8_t params_count, int8_t retval_count,
							 char** out_err, uint32_t* out_err_len);

void xllr_free_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
                           int64_t function_id_opt,
                           char** out_err, uint32_t* out_err_len);

void xllr_load_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);
void xllr_free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);

void xllr_set_runtime_flag(const char* flag_name, uint64_t flag_name_len);
int xllr_is_runtime_flag_set(const char* flag_name, uint64_t flag_name_len);

struct cdts* xllr_alloc_cdts_buffer(metaffi_size params, metaffi_size rets);

/************************************************
*   Misc
*************************************************/

#ifdef _WIN32 //// --- START WINDOWS ---
#include <Windows.h>
void get_last_error_string(DWORD err, char** out_err_str, uint64_t* out_err_size);
#endif

void* load_library(const char* name, char** out_err, uint64_t* out_err_size);
const char* free_library(void* lib);
void* load_symbol(void* handle, const char* name, char** out_err, uint64_t* out_err_len);
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