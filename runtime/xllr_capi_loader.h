#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "cdt.h"
#include "xcall.h"

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

void xllr_xcall_params_ret(struct xcall* pxcall,
				struct cdts parameters[2],
				char** out_err
);

void xllr_xcall_no_params_ret(struct xcall* pxcall,
                struct cdts return_values[1],
                char** out_err
);

void xllr_xcall_params_no_ret(struct xcall* pxcall,
                struct cdts parameters[1],
                char** out_err
);

void xllr_xcall_no_params_no_ret(struct xcall* pxcall,
                char** out_err
);

struct xcall* xllr_load_entity(const char* runtime_plugin,
                          const char* module_path,
                          const char* entity_path,
                          struct metaffi_type_info* params_types, int8_t params_count,
                          struct metaffi_type_info* retvals_types, int8_t retval_count,
                          char** out_err);

struct xcall* xllr_load_callable(void* make_callable_context,
                                struct metaffi_type_info* params_types, int8_t params_count,
                                struct metaffi_type_info* retvals_types, int8_t retval_count,
                                char** out_err);

void xllr_free_xcall(const char* runtime_plugin,
                           struct xcall* pxcall,
                           char** out_err);

char* xllr_alloc_string(const char* err_message, uint64_t length);
char8_t* xllr_alloc_string8(const char8_t* err_message, uint64_t length);
char16_t* xllr_alloc_string16(const char16_t* err_message, uint64_t length);
char32_t* xllr_alloc_string32(const char32_t* err_message, uint64_t length);
void xllr_free_string(char* err_to_free);

void xllr_metaffi_free(void* ptr);
void* xllr_metaffi_alloc(uint64_t size);

void xllr_load_runtime_plugin(const char* runtime_plugin, char** err);
void xllr_free_runtime_plugin(const char* runtime_plugin, char** err);

void xllr_set_runtime_flag(const char* flag_name);
int xllr_is_runtime_flag_set(const char* flag_name);

struct cdts* xllr_alloc_cdts_buffer(metaffi_size params, metaffi_size rets);
void xllr_free_cdts_buffer(struct cdts* pcdts);

struct construct_cdts_callbacks;
struct traverse_cdts_callbacks;

void xllr_construct_cdts(struct cdts* pcdts, struct construct_cdts_callbacks* callbacks, char** out_nul_term_err);
void xllr_construct_cdt(struct cdt* pcdts, struct construct_cdts_callbacks* callbacks, char** out_nul_term_err);
void xllr_traverse_cdts(struct cdts* pcdts, struct traverse_cdts_callbacks* callbacks, char** out_nul_term_err);
void xllr_traverse_cdt(struct cdt* pcdts, struct traverse_cdts_callbacks* callbacks, char** out_nul_term_err);

/************************************************
*   Misc
*************************************************/

#ifdef _WIN32 //// --- START WINDOWS ---
#include <Windows.h>
void get_last_error_string(DWORD err, char** out_err_str);
#endif

void* load_library(const char* name, char** out_err);
const char* free_library(void* lib);
void* load_symbol(void* handle, const char* name, char** out_err);
const char* load_xllr();
const char* free_xllr();

/************************************************
*   Load cdt functions dynamically
*************************************************/
const char* load_xllr_capi();


#ifdef __cplusplus
// make sure you include utils/scope_guard.hpp
#define cdts_scope_guard(name) \
	metaffi::utils::scope_guard sg_##name([&name]() \
	{ \
		if(name) \
		{ \
			xllr_free_cdts_buffer(name); \
			name = nullptr; \
		} \
	});
#endif

#ifdef __cplusplus
}
#endif