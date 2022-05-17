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
	virtual int64_t load_function(const char* function_path, uint32_t function_path_len, char** err, uint32_t* err_len) = 0;

	/**
	 * Free module of foreign language
	 */ 
	virtual void free_function(int64_t function_id, char** err, uint32_t* err_len) = 0;

	/***
	 * Call foreign function
	 */
	virtual void xcall(
			//function id to call
			int64_t function_id,
			
			// parameters
			cdt* parameters, uint64_t parameters_len,
			
			// return values
			cdt* return_values, uint64_t return_values_len,
			
			// out error
			char** out_err, uint64_t* out_err_len
	) = 0;
};
