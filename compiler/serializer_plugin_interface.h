#pragma once
#include <stdint.h>

/**
 * Interface Compiler Plugin Implements
 */
#ifdef __cplusplus
struct serializer_plugin_interface
{

	/**
	 * Generates serialization code
	 */
	virtual	void compile_serialization(const char* idl_name, uint32_t idl_name_length,
	                                   const char* idl, uint32_t idl_length,
	                                   char** out_serialization_code, uint32_t* out_serialization_code_length,
	                                   char** out_err, uint32_t* out_err_len) = 0;
};
#endif
