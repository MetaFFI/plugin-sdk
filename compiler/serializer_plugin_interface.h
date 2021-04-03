#pragma once
#include <stdint.h>

/**
 * Interface Compiler Plugin Implements
 */
#ifdef __cplusplus
struct serializer_plugin_interface
{
	virtual void init() = 0;

	/**
	 * Generates serialization code
	 */
	virtual	void compile_serialization(const char* idl, uint32_t idl_length,
                                       const char* language, uint32_t language_length,
	                                   char** out_serialization_code_json, uint32_t* out_serialization_code_json_length,
	                                   char** out_err, uint32_t* out_err_len) = 0;
};
#endif
