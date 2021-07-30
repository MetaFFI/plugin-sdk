#pragma once
/***
 * MetaFFI XLLR API 
 */

#include <stdint.h>
#include <stdarg.h>
#include <runtime/cdt_structs.h>

#ifndef SKIP_XLLR_API_EXTERN
extern "C"
{
#endif
/**
 * Load language runtime of foreign language
 */
void load_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);

/**
 * Free language runtime of foreign language
 */
void free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);

/**
 * Load module of foreign language
 */
int64_t load_function(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len, const char* function_path, uint32_t function_path_len, int64_t function_id, char** err, uint32_t* err_len);

/**
 * Free module of foreign language
 */
void free_function(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len, int64_t function_id, char** err, uint32_t* err_len);

/***
 * Call foreign function
 */
void call(
	// [in] runtime plugin name
	const char* runtime_plugin_name, uint32_t runtime_plugin_name_len,
	
	// [in] function id to call
	int64_t function_id,
	
	// [in] parameters array
	cdt* parameters,
	
	// [in] number of parameters
	uint64_t parameters_len,
	
	// [in] return values array
	cdt* return_values,
	
	// [in] number of return values
	uint64_t return_values_len,
	
	// [out] error
	char** out_err, uint64_t* out_err_len
);

/**
 * @brief Sets a flag in XLLR
 */
void set_runtime_flag(const char* flag_name, uint64_t flag_name_length);

/**
 * @brief Test a flag in XLLR
 */
int is_runtime_flag_set(const char* flag_name, uint64_t flag_name_length);

#ifndef SKIP_XLLR_API_EXTERN
}
#endif