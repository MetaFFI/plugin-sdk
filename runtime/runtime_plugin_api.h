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
void* load_function(const char* function_path, uint32_t function_path_len, int8_t params_count, int8_t retval_count, /*void** out_params,*/ char** err, uint32_t* err_len);

/**
 * Free module of foreign language
 */
void free_function(void* pff, char** err, uint32_t* err_len);
}
