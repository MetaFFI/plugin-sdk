#pragma once
#include <cstdint>
#include <cstdarg>
#include <runtime/cdt_structs.h>

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
	virtual int64_t load_function(const char* function_path, uint32_t function_path_len, int8_t params_count, int8_t retval_count, char** err, uint32_t* err_len) = 0;

	/**
	 * Free module of foreign language
	 */ 
	virtual void free_function(int64_t function_id, char** err, uint32_t* err_len) = 0;

	/***
	 * Call foreign function
	 */
	virtual void xcall_params_ret(
			//function id to call
			int64_t function_id,
			
			// parameters
			cdts params_ret[2],
			
			// out error
			char** out_err, uint64_t* out_err_len
	) = 0;
	
	/***
	 * Call foreign function
	 */
	virtual void xcall_params_no_ret(
			//function id to call
			int64_t function_id,
			
			// parameters
			cdts parameters[1],
			
			// out error
			char** out_err, uint64_t* out_err_len
	) = 0;
	
	virtual void xcall_no_params_ret(
			//function id to call
			int64_t function_id,
			
			// return values
			cdts return_values[1],
			
			// out error
			char** out_err, uint64_t* out_err_len
	) = 0;
	
	virtual void xcall_no_params_no_ret(
			//function id to call
			int64_t function_id,
			
			// out error
			char** out_err, uint64_t* out_err_len
	) = 0;
};
