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
	virtual	struct idl_definition* parse_idl(const char* idl_name, uint32_t idl_name_length,
									 const char* idl, uint32_t idl_length,
									 char** out_err, uint32_t* out_err_len) = 0;

};
#endif

typedef struct idl_definition
{
	const char* idl_filename;
	uint32_t idl_filename_length;
	
	const char* idl_code;
	uint32_t idl_code_length;
	
	const char* idl_extension;
	uint32_t idl_extension_length;
	
	const char* idl_full_path;
	uint32_t idl_full_path_length;
	
	const char* target_language;
	uint32_t target_language_length;
	
	struct module_definition* modules;
	uint32_t modules_length;
}idl_definition;

typedef struct module_definition
{
	const char* module_name;
	uint32_t module_name_length;
	
	struct function_definition* functions;
	uint32_t functions_length;
}module_definition;

typedef struct function_definition
{
	const char* function_name;
	uint32_t function_name_length;
	
	const char* foreign_function_name;
	uint32_t foreign_function_name_length;
	
	const char* parameters_structure_name;
	uint32_t parameters_structure_name_length;
	
	const char* return_values_structure_name;
	uint32_t return_values_structure_name_length;
	
	struct param_return_definition* parameters;
	uint32_t parameters_length;
	
	struct param_return_definition* return_values;
	uint32_t return_values_length;
}function_definition;

typedef struct param_return_definition
{
	const char* param_return_definition_name;
	uint32_t param_return_definition_name_length;
	
	const char* param_return_definition_type;
	uint32_t param_return_definition_type_length;
	
	uint8_t is_complex_type;
	uint8_t is_array;
}param_return_definition;