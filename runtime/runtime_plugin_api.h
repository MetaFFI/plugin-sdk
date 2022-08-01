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
int64_t load_function(const char* function_path, uint32_t function_path_len, int8_t params_count, int8_t retval_count, char** err, uint32_t* err_len);

/**
 * Free module of foreign language
 */
void free_function(int64_t function_id, char** err, uint32_t* err_len);

/***
 * Call foreign function
 */
void xcall_params_ret(
		int64_t function_id,
		cdts params_ret[2],
		char** out_err, uint64_t* out_err_len
);

void xcall_no_params_ret(
		int64_t function_id,
		cdts return_values[1],
		char** out_err, uint64_t* out_err_len
);

void xcall_params_no_ret(
		int64_t function_id,
		cdts parameters[1],
		char** out_err, uint64_t* out_err_len
);

void xcall_no_params_no_ret(
		int64_t function_id,
		char** out_err, uint64_t* out_err_len
);
}