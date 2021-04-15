#pragma once
#include <stdint.h>

// TODO: Make sure out_err must always be freed by the called (if non-null)

/**
 * Interface XLLR Plugin Implements
 */
struct runtime_plugin_interface
{
	/**
	 * Load runtime runtime of foreign runtime
	 */ 
	virtual void load_runtime(char** err, uint32_t* err_len) = 0;

	/**
	 * Free runtime runtime of foreign runtime
	 */ 
	virtual void free_runtime(char** err, uint32_t* err_len) = 0;

	/**
	 * Load module of foreign language
	 */ 
	virtual int64_t load_function(const char* function_path, uint32_t function_path_len, char** err, uint32_t* err_len) = 0;

	/**
	 * Free module of foreign language
	 */ 
	virtual void free_function(int64_t function_id, char** err, uint32_t* err_len) = 0;

	/***
	 * Call foreign function
	 */
	virtual void call(
			// function id to call
			int64_t function_id,
			
			// serialized parameters
			unsigned char* in_params, uint64_t in_params_len,

			// serialized returned ref parameters
			unsigned char** out_params, uint64_t* out_params_len,
			
			// out - serialized result or error message
			unsigned char** out_ret, uint64_t* out_ret_len,
			
			// out - 0 if not an error, otherwise an error
			uint8_t* is_error
	) = 0;
};

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
		// function id to call
		int64_t function_id,
		
		// serialized parameters
		unsigned char* in_params, uint64_t in_params_len,
		
		// serialized returned ref parameters
		unsigned char** out_params, uint64_t* out_params_len,
		
		// out - serialized result or error message
		unsigned char** out_ret, uint64_t* out_ret_len,
		
		// out - 0 if not an error, otherwise an error
		uint8_t* is_error
);
}