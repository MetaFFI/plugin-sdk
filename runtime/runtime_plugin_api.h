#pragma once
#include <stdarg.h>
#include <stdint.h>

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
int64_t load_function(const char* function_path, uint32_t function_path_len, char** err, uint32_t* err_len);

/**
 * Free module of foreign language
 */
void free_function(int64_t function_id, char** err, uint32_t* err_len);

/***
 * Call foreign function
 */
void call(
		int64_t function_id,
		void** parameters, uint64_t parameters_len,
		void** return_values, uint64_t return_values_len,
		char** out_err, uint64_t *out_err_len
);
}