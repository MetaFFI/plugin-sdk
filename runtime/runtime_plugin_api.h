#pragma once
#ifndef __cplusplus
#include <stdarg.h>
#include <stdint.h>
#else
#include <cstdarg>
#include <cstdint>
#endif
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
struct xcall* load_entity(const char* module_path, const char* entity_path, metaffi_type_info* params_types, int8_t params_count, metaffi_type_info* retvals_types, int8_t retval_count, char** err);

 /**
  * Load callable of foreign language
  */
struct xcall* make_callable(void* make_callable_context, metaffi_type_info* params_types, int8_t params_count, metaffi_type_info* retvals_types, int8_t retval_count, char** err);


/**
 * Free module of foreign language
 */
void free_xcall(xcall* pff, char** err);

void xcall_params_ret(void* context, cdts params_ret[2], char** out_err);
void xcall_params_no_ret(void* context, cdts parameters[1], char** out_err);
void xcall_no_params_ret(void* context, cdts return_values[1], char** out_err);
void xcall_no_params_no_ret(void* context, char** out_err);

}
