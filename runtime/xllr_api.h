#pragma once
/***
 * OpenFFI XLLR API 
 */

#include <stdint.h> 

extern "C"
{
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
		
		// [in] serialized parameters
		unsigned char* in_params, uint64_t in_params_len,
		
		// [out] serialized returned ref parameters
		unsigned char** out_params, uint64_t *out_params_len,
		
		// [out] serialized result or error message
		unsigned char** out_ret, uint64_t *out_ret_len,
		
		// [out] 0 if not an error, otherwise an error
		uint8_t* is_error
);
}