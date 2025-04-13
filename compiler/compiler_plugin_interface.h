#pragma once
#include <stdint.h>
/**
 * Interface Compiler Plugin Implements
 */
#ifdef __cplusplus
struct language_plugin_interface
{

	virtual void init() = 0;
	
	/**
	 * Compiles IDL to executable code called from XLLR to the foreign function
	 */ 
	virtual	void compile_to_guest(const char* idl_def_json, uint32_t idl_def_json_length,
						   const char* output_path, uint32_t output_path_length,
						   const char* guest_options, uint32_t guest_options_length,
						   char** out_err, uint32_t* out_err_len) = 0;

	/**
	 * Compile IDL to code calling to XLLR from host code
	 */
	virtual	void compile_from_host(const char* idl_def_json, uint32_t idl_def_json_length,
							const char* output_path, uint32_t output_path_length,
				            const char* host_options, uint32_t host_options_length,
							char** out_err, uint32_t* out_err_len) = 0;

};
#endif