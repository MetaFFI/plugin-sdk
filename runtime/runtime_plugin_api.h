#pragma once
#include <stdarg.h>
#include <stdint.h>
#include "cdt.h"
#include "xcall.h"


extern "C"
{
/**
 * Load runtime runtime of foreign runtime
 */
void load_runtime(char** err);

/**
 * Free runtime runtime of foreign runtime
 */
void free_runtime(char** err);

/**
 * Load module of foreign language
 */
struct xcall* load_entity(const char* module_path, uint32_t module_path_len, const char* function_path, uint32_t function_path_len, metaffi_type_info* params_types, uint8_t params_count, metaffi_type_info* retvals_types, uint8_t retval_count, char** err);

 /**
  * Load callable of foreign language
  */
struct xcall* make_callable(void* make_callable_context, metaffi_type_info* params_types, uint8_t params_count, metaffi_type_info* retvals_types, uint8_t retval_count, char** err);


/**
 * Free module of foreign language
 */
void free_function(void* pff, char** err);

void xcall_params_ret(void* context, cdts params_ret[2], char** out_err);
void xcall_params_no_ret(void* context, cdts parameters[1], char** out_err);
void xcall_no_params_ret(void* context, cdts return_values[1], char** out_err);
void xcall_no_params_no_ret(void* context, char** out_err);

}
