#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "cdt.h"

#ifdef __cplusplus
extern "C"{
#endif

#define METAFFI_NULL_HANDLE (metaffi_handle)NULL;

/************************************************
*   Allocations
*************************************************/

int64_t get_cache_size();

/************************************************
*   XLLR functions
*************************************************/

void xllr_xcall_params_ret(void* pff,
				struct cdts parameters[2],
				char** out_err, uint64_t* out_err_len
);

void xllr_xcall_no_params_ret(void* pff,
                struct cdts return_values[1],
                char** out_err, uint64_t* out_err_len
);

void xllr_xcall_params_no_ret(void* pff,
                struct cdts parameters[1],
                char** out_err, uint64_t* out_err_len
);

void xllr_xcall_no_params_no_ret(void* pff,
                char** out_err, uint64_t* out_err_len
);

void** xllr_load_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
                          const char* module_path, uint32_t module_path_len,
                          const char* function_path, uint32_t function_path_len,
                          struct metaffi_type_info* params_types, struct metaffi_type_info* retvals_types,
                          uint8_t params_count, uint8_t retval_count,
                          char** out_err, uint32_t* out_err_len);

void xllr_free_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
                           void* pff_opt,
                           char** out_err, uint32_t* out_err_len);

void xllr_load_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);
void xllr_free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);

void xllr_set_runtime_flag(const char* flag_name, uint64_t flag_name_len);
int xllr_is_runtime_flag_set(const char* flag_name, uint64_t flag_name_len);

struct cdts* xllr_alloc_cdts_buffer(metaffi_size params, metaffi_size rets);

struct construct_cdts_callbacks;
struct traverse_cdts_callbacks;

void xllr_construct_cdts(struct cdts* pcdts, struct construct_cdts_callbacks* callbacks);
void xllr_construct_cdt(struct cdt* pcdts, struct construct_cdts_callbacks* callbacks);
void xllr_traverse_cdts(struct cdts* pcdts, struct traverse_cdts_callbacks* callbacks);
void xllr_traverse_cdt(struct cdt* pcdts, struct traverse_cdts_callbacks* callbacks);

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
const char* load_xllr();
const char* free_xllr();

/************************************************
*   Load cdt functions dynamically
*************************************************/
const char* load_xllr_capi();


#ifdef __cplusplus
}
#endif