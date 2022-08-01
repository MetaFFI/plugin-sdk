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
int64_t load_function(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len, const char* function_path, uint32_t function_path_len, int64_t function_id, int8_t params_count, int8_t retval_count, char** err, uint32_t* err_len);

/**
 * Free module of foreign language
 */
void free_function(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len, int64_t function_id, char** err, uint32_t* err_len);

/***
 * Call foreign entity
 */
void xcall_params_ret(
	int64_t function_id,                                                // [in] function id to call
	cdts params_ret[2],                                                 // [in/out] parameters array
	char** out_err, uint64_t* out_err_len                               // [out] error
);

/***
 * Call foreign entity
 */
void xcall_no_params_ret(
		int64_t function_id,                                                // [in] function id to call
		cdts return_values[1],                                              // [in] return values array
		char** out_err, uint64_t* out_err_len                               // [out] error
);

/***
 * Call foreign entity
 */
void xcall_params_no_ret(
		int64_t function_id,                                                // [in] function id to call
		cdts parameters[1],                                                 // [in] parameters array
		char** out_err, uint64_t* out_err_len                               // [out] error
);

/***
 * Call foreign entity
 */
void xcall_no_params_no_ret(
		int64_t function_id,                                                // [in] function id to call
		char** out_err, uint64_t* out_err_len                               // [out] error
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