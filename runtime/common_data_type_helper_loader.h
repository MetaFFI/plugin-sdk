#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "openffi_primitives.h"

#ifdef __cplusplus
extern "C"{
#endif

// == Defines ==
#define get_arg_type_str_decl_fptr(type) \
int get_arg_##type(void** data_array, int index, type* out_res, openffi_size* length)

#define get_arg_type_decl_fptr(type) \
int get_arg_##type(void** data_array, int index, type* out_res)

#define get_arg_type_str_array_decl_fptr(type) \
int get_arg_##type##_array(void** data_array, int index, type** array, openffi_size** sizes_array, openffi_size** dimensions, openffi_size* dimensions_length)

#define get_arg_type_array_decl_fptr(type) \
int get_arg_##type##_array(void** data_array, int index, type** out_res, openffi_size** dimensions, openffi_size* dimensions_length)

#define set_arg_openffi_str_decl_fptr(type)\
int set_arg_##type(void** data_array, int index, openffi_string val, openffi_size* string_length)

#define set_arg_type_decl_fptr(type) \
int set_arg_##type(void** data_array, int index, type* val)

#define set_arg_openffi_str_array_decl_fptr(type)\
int set_arg_##type##_array(void** data_array, int index, type* array, openffi_size* string_sizes, openffi_size* dimensions, openffi_size* dimensions_length)

#define set_arg_type_array_decl_fptr(type) \
int set_arg_##type##_array(void** data_array, int index, type* array, openffi_size* dimensions, openffi_size* dimensions_length)

#define get_numeric_element_decl_fptr(type) type get_##type##_element(type* arr, int index)

#define set_numeric_element_decl_fptr(type) void set_##type##_element(type* arr, int index, type val)

#define alloc_numeric_on_heap_decl_fptr(type) type* alloc_##type##_on_heap(type val)

#define alloc_str_on_heap_decl_fptr(type) type alloc_##type##_on_heap(type val, openffi_size str_size)

// === arg helpers pointer to functions & wrapper functions ===

//int8_t (*pis_arg_overflow) (uint64_t*, int);
int8_t is_arg_overflow(uint64_t* size_left, int size);

//int (*pget_type)(void** data_array, int index, uint64_t* out_type);
int get_type(void** data_array, int index, uint64_t* out_type);

//void** (*palloc_args_buffer)(int size);
void** alloc_args_buffer(int size);

get_arg_type_str_decl_fptr(openffi_string);
get_arg_type_str_decl_fptr(openffi_string8);
get_arg_type_str_decl_fptr(openffi_string16);
get_arg_type_str_decl_fptr(openffi_string32);

//int (*pget_arg_pointer_type) (void**, int, void**);
int get_arg_pointer_type(void** data_array, int index, void** out_res);

get_arg_type_decl_fptr(openffi_float64);
get_arg_type_decl_fptr(openffi_float32);
get_arg_type_decl_fptr(openffi_int64);
get_arg_type_decl_fptr(openffi_int32);
get_arg_type_decl_fptr(openffi_int16);
get_arg_type_decl_fptr(openffi_int8);
get_arg_type_decl_fptr(openffi_uint64);
get_arg_type_decl_fptr(openffi_uint32);
get_arg_type_decl_fptr(openffi_uint16);
get_arg_type_decl_fptr(openffi_uint8);
get_arg_type_decl_fptr(openffi_size);
get_arg_type_decl_fptr(openffi_bool);

get_arg_type_str_array_decl_fptr(openffi_string);
get_arg_type_str_array_decl_fptr(openffi_string8);
get_arg_type_str_array_decl_fptr(openffi_string16);
get_arg_type_str_array_decl_fptr(openffi_string32);

get_arg_type_array_decl_fptr(openffi_float64);
get_arg_type_array_decl_fptr(openffi_float32);
get_arg_type_array_decl_fptr(openffi_int64);
get_arg_type_array_decl_fptr(openffi_int32);
get_arg_type_array_decl_fptr(openffi_int16);
get_arg_type_array_decl_fptr(openffi_int8);
get_arg_type_array_decl_fptr(openffi_uint64);
get_arg_type_array_decl_fptr(openffi_uint32);
get_arg_type_array_decl_fptr(openffi_uint16);
get_arg_type_array_decl_fptr(openffi_uint8);
get_arg_type_array_decl_fptr(openffi_size);
get_arg_type_array_decl_fptr(openffi_bool);

//int (*pset_arg)(void**, int, void*);
int set_arg(void** data_array, int index, void* val);

set_arg_openffi_str_decl_fptr(openffi_string);
set_arg_openffi_str_decl_fptr(openffi_string8);
set_arg_openffi_str_decl_fptr(openffi_string16);
set_arg_openffi_str_decl_fptr(openffi_string32);

set_arg_type_decl_fptr(openffi_float64);
set_arg_type_decl_fptr(openffi_float32);
set_arg_type_decl_fptr(openffi_int64);
set_arg_type_decl_fptr(openffi_int32);
set_arg_type_decl_fptr(openffi_int16);
set_arg_type_decl_fptr(openffi_int8);
set_arg_type_decl_fptr(openffi_uint64);
set_arg_type_decl_fptr(openffi_uint32);
set_arg_type_decl_fptr(openffi_uint16);
set_arg_type_decl_fptr(openffi_uint8);
set_arg_type_decl_fptr(openffi_size);
set_arg_type_decl_fptr(openffi_bool);

int set_arg_array(void** data_array, int index, void** array, openffi_size** dimensions, openffi_size* dimensions_length);

set_arg_openffi_str_array_decl_fptr(openffi_string);
set_arg_openffi_str_array_decl_fptr(openffi_string8);
set_arg_openffi_str_array_decl_fptr(openffi_string16);
set_arg_openffi_str_array_decl_fptr(openffi_string32);

set_arg_type_array_decl_fptr(openffi_float64);
set_arg_type_array_decl_fptr(openffi_float32);
set_arg_type_array_decl_fptr(openffi_int64);
set_arg_type_array_decl_fptr(openffi_int32);
set_arg_type_array_decl_fptr(openffi_int16);
set_arg_type_array_decl_fptr(openffi_int8);
set_arg_type_array_decl_fptr(openffi_uint64);
set_arg_type_array_decl_fptr(openffi_uint32);
set_arg_type_array_decl_fptr(openffi_uint16);
set_arg_type_array_decl_fptr(openffi_uint8);
set_arg_type_array_decl_fptr(openffi_size);
set_arg_type_array_decl_fptr(openffi_bool);

//const char* (*pget_openffi_string_element)(int index, openffi_string *str, const openffi_size *sizes, openffi_size *out_size);
const char* get_openffi_string_element(int index, openffi_string *str, const openffi_size *sizes, openffi_size *out_size);

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
get_numeric_element_decl_fptr(openffi_size);

//void (*pset_openffi_string_element)(int index, openffi_string *str_array, openffi_size *sizes_array, openffi_string str, openffi_size str_size);
void set_openffi_string_element(int index, openffi_string *str_array, openffi_size *sizes_array, openffi_string str, openffi_size str_size);

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
set_numeric_element_decl_fptr(openffi_size);

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

alloc_str_on_heap_decl_fptr(openffi_string);
alloc_str_on_heap_decl_fptr(openffi_string8);
alloc_str_on_heap_decl_fptr(openffi_string16);
alloc_str_on_heap_decl_fptr(openffi_string32);

// === XLLR function pointers ===
extern void (*pxllr_call)(const char*, uint32_t, int64_t, void**, uint64_t, void**, uint64_t, char**, uint64_t*);
void xllr_call(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len,
				int64_t function_id,
				void** parameters, uint64_t parameters_length,
				void** return_values, uint64_t return_values_length,
				char** out_err, uint64_t* out_err_len
);

extern int64_t (*pxllr_load_function)(const char*, uint32_t, const char*, uint32_t, int64_t, char**, uint32_t*);
int64_t xllr_load_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
							 const char* function_path, uint32_t function_path_len,
							 int64_t function_id_opt,
							 char** out_err, uint32_t* out_err_len);

extern void (*pxllr_free_runtime_plugin)(const char*, uint32_t, char**, uint32_t*);
void xllr_free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);

// === Functions ===
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

// load args helpers functions
// if failed, returned error message, otherwise return NULL
const char* load_args_helpers();

#ifdef __cplusplus
}
#endif