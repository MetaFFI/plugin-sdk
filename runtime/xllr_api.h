#pragma once
/***
 * MetaFFI XLLR API 
 */
#ifdef __cplusplus
#include <cstdarg>
#include <cstdint>
#else
#include <stdarg.h>
#include <stdint.h>
#endif
#include <runtime/cdt.h>
#include <runtime/xcall.h>

#ifndef SKIP_XLLR_API_EXTERN
extern "C" {
#endif
/**
 * Load language runtime of foreign language
 */
void load_runtime_plugin(const char* runtime_plugin, char** err);

/**
 * Free language runtime of foreign language
 */
void free_runtime_plugin(const char* runtime_plugin, char** err);

/**
 * Load module of foreign language
 */
struct xcall* load_entity(const char* runtime_plugin_name, const char* module_path, const char* entity_path, metaffi_type_info* params_types, int8_t params_count, metaffi_type_info* retvals_types, int8_t retval_count, char** err);

/**
  * Load module of foreign language
  */
struct xcall* make_callable(const char* runtime_plugin_name, void* make_callable_context, metaffi_type_info* params_types, int8_t params_count, metaffi_type_info* retvals_types, int8_t retval_count, char** err);

/**
 * Free module of foreign language
 */
void free_xcall(const char* runtime_plugin_name, struct xcall* pxcall, char** err);

char* alloc_string(const char* err, uint64_t length);
char8_t* alloc_string8(const char8_t* err, uint64_t length);
char16_t* alloc_string16(const char16_t* err, uint64_t length);
char32_t* alloc_string32(const char32_t* err, uint64_t length);
void free_string(const char* err_to_free);


void* metaffi_alloc(uint64_t size);
void metaffi_free(void* ptr);

/***
 * Call foreign entity
 */
void xcall_params_ret(
        struct xcall* pplugin_xcall_params_ret_and_context,// [in] pointer to plugin's xcall_params_ret + context
        cdts params_ret[2],                                // [in/out] parameters array
        char** out_err                                     // [out] error
);

/***
 * Call foreign entity
 */
void xcall_no_params_ret(
        struct xcall* pplugin_xcall_no_params_ret_and_context,// [in] pointer to plugin's xcall_no_params_ret + context
        cdts return_values[1],                                // [in] return values array
        char** out_err                                        // [out] error
);

/***
 * Call foreign entity
 */
void xcall_params_no_ret(
        struct xcall* pplugin_xcall_params_no_ret_and_context,// [in] pointer to plugin's xcall_params_no_ret + context
        cdts parameters[1],                                   // [in] parameters array
        char** out_err                                        // [out] error
);

/***
 * Call foreign entity
 */
void xcall_no_params_no_ret(
        struct xcall* pplugin_xcall_no_params_no_ret_and_context,// [in] pointer to plugin's xcall_no_params_no_ret + context
        char** out_err                                           // [out] error
);

/**
 * @brief Sets a flag in XLLR
 */
void set_runtime_flag(const char* flag_name);

/**
 * @brief Test a flag in XLLR
 */
int is_runtime_flag_set(const char* flag_name);

#ifndef SKIP_XLLR_API_EXTERN
}
#endif