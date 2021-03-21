#pragma once
#include <stdint.h>
#include "idl_plugin_interface.h"

/**
 * Interface Compiler Plugin Implements
 */ 
struct compiler_plugin_interface
{
	/**
	 * Compiles IDL to executable code called from XLLR to the foreign function
	 */ 
	virtual void compile_to_guest(idl_definition* idl_def,
							   const char* output_path, uint32_t output_path_length,
						       const char* serialization_code, uint32_t serialization_code_length,
							   char** out_err, uint32_t* out_err_len) = 0;

	/**
	 * Compile IDL to code calling to XLLR from host code
	 */ 
	virtual void compile_from_host(idl_definition* idl_def,
								const char* output_path, uint32_t output_path_length,
							   const char* serialization_code, uint32_t serialization_code_length,
								char** out_err, uint32_t* out_err_len) = 0;
	
};

