#pragma once
#include <stdint.h>

struct idl_definition;
struct module_definition;
struct function_definition;
struct param_return_definition;

/**
 * Interface Compiler Plugin Implements
 */
#ifdef __cplusplus
struct idl_plugin_interface
{
	/**
	 * Returns the data defined in the IDL
	 */
	virtual	struct void parse_idl(const char* idl_file_path, uint32_t idl_file_path_length,
									 const char* idl, uint32_t idl_length,
									 char** out_idl_def_json, uint32_t* out_idl_def_json_length,
									 char** out_err, uint32_t* out_err_len) = 0;

};
#endif
