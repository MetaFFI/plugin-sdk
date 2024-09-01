#pragma once
#include <cstdint>
#include <cstdarg>
#include <runtime/cdt.h>
#include <runtime/xcall.h>

/**
 * Interface XLLR Plugin Implements
 */
struct runtime_plugin_interface
{
	virtual ~runtime_plugin_interface()= default;

	/**
	 * Load runtime
	 */ 
	virtual void load_runtime(char** err) = 0;

	/**
	 * Free runtime
	 */ 
	virtual void free_runtime(char** err) = 0;

	/**
	 * Load entity
	 */ 
	virtual xcall* load_entity(const char* module_path, const char* entity_path, metaffi_type_info* params_types, uint8_t params_count, metaffi_type_info* retvals_types, uint8_t retval_count, char** err) = 0;

	/**
	 * Wrap callable with XCall
	 */
	virtual xcall* make_callable(void* make_callable_context, metaffi_type_info* params_types, uint8_t params_count, metaffi_type_info* retvals_types, uint8_t retval_count, char** err) = 0;

	/**
	 * Free loaded entity
	 */ 
	virtual void free_xcall(xcall* pff, char** err) = 0;
	
};
