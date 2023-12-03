#pragma once
#include <stdarg.h>
#include <stdint.h>
#include <runtime/cdt_structs.h>

extern "C"
{
/**
 * Load runtime runtime of foreign runtime
 */
void load_runtime(char** err, uint32_t* err_len);

/**
 * Free runtime runtime of foreign runtime
 */
void free_runtime(char** err, uint32_t* err_len);

/**
 * Load module of foreign language
 */
void** load_function(const char* module_path, uint32_t module_path_len, const char* function_path, uint32_t function_path_len, metaffi_types_with_alias_ptr params_types, metaffi_types_with_alias_ptr retvals_types, uint8_t params_count, uint8_t retval_count, char** err, uint32_t* err_len);

/**
 * Free module of foreign language
 */
void free_function(void* pff, char** err, uint32_t* err_len);

void xcall_params_ret(void* context, cdts params_ret[2], char** out_err, uint64_t* out_err_len);
void xcall_params_no_ret(void* context, cdts parameters[1], char** out_err, uint64_t* out_err_len);
void xcall_no_params_ret(void* context, cdts return_values[1], char** out_err, uint64_t* out_err_len);
void xcall_no_params_no_ret(void* context, char** out_err, uint64_t* out_err_len);

}
