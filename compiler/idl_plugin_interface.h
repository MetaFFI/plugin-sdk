#pragma once
#include <stdint.h>


/**
 * Interface Compiler Plugin Implements
 */
#ifdef __cplusplus
struct idl_plugin_interface
{
	virtual void init() = 0;
	
	/**
	 * Returns the data defined in the IDL
	 */
	virtual	void parse_idl(const char* source_code, uint32_t source_length,
							 const char* file_or_path, uint32_t file_or_path_length,
							 char** out_idl_def_json, uint32_t* out_idl_def_json_length,
							 char** out_err, uint32_t* out_err_len) = 0;

};
#endif
