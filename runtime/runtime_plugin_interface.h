#pragma once
#include <cstdint>
#include <cstdarg>
#include <runtime/cdt_structs.h>

/**
 * Interface XLLR Plugin Implements
 */
struct runtime_plugin_interface
{
	virtual ~runtime_plugin_interface()= default;

	/**
	 * Load runtime
	 */ 
	virtual void load_runtime(char** err, uint32_t* err_len) = 0;

	/**
	 * Free runtime
	 */ 
	virtual void free_runtime(char** err, uint32_t* err_len) = 0;

	/**
	 * Load entity
	 */ 
	virtual void** load_function(const char* module_path, uint32_t module_path_len, const char* function_path, uint32_t function_path_len, metaffi_type_infos_ptr params_types, metaffi_type_infos_ptr retvals_types, uint8_t params_count, uint8_t retval_count, char** err, uint32_t* err_len) = 0;

	/**
	 * Wrap callable with XCall
	 */
	virtual void** make_callable(void* make_callable_context, metaffi_type_infos_ptr params_types, metaffi_type_infos_ptr retvals_types, uint8_t params_count, uint8_t retval_count, char** err, uint32_t* err_len) = 0;

	/**
	 * Free loaded entity
	 */ 
	virtual void free_function(void* pff, char** err, uint32_t* err_len) = 0;

};
