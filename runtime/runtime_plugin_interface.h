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
	virtual void* load_function(const char* module_path, uint32_t module_path_len, const char* function_path, uint32_t function_path_len, int8_t params_count, int8_t retval_count, char** err, uint32_t* err_len) = 0;

	/**
	 * Free module of foreign language
	 */ 
	virtual void free_function(void* pff, char** err, uint32_t* err_len) = 0;

};
