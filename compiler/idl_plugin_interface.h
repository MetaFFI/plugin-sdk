#pragma once
#include <stdint.h>

struct idl_definition;
struct module_data;
struct function_data;
struct param_return_data;

/**
 * Interface Compiler Plugin Implements
 */
struct idl_plugin_interface
{
	/**
	 * Compiles IDL to executable code called from XLLR to the foreign function
	 */
	virtual idl_data* parse_idl(const char* idl_path, uint32_t output_path_length, char** out_err, uint32_t* out_err_len) = 0;
};

struct idl_definition
{
	const char* idl_filename;
	uint32_t idl_filename_length;
	
	const char* idl_extension;
	uint32_t idl_extension_length;
	
	const char* idl_full_path;
	uint32_t idl_full_path_length;
	
	const char* target_language;
	uint32_t target_language_length;
	
	module_data* modules;
	uint32_t modules_length;
};

struct module_data
{
	const char* module_name;
	uint32_t module_name_length;
	
	function_data* functions;
	uint32_t functions_length;
};

struct function_data
{
	const char* function_name;
	uint32_t function_name_length;
	
	const char* foreign_function_name;
	uint32_t foreign_function_name_length;
	
	const char* parameters_structure_name;
	uint32_t parameters_structure_name_length;
	
	const char* return_values_structure_name;
	uint32_t return_values_structure_name_length;
	
	param_return_data* parameters;
	uint32_t parameters_length;
	
	param_return_data* return_values;
	uint32_t return_values_length;
};

struct param_return_data
{
	const char* param_return_data_name;
	uint32_t param_return_data_name_length;
	
	const char* param_return_data_type;
	uint32_t param_return_data_type_length;
	
	uint8_t is_complex_type;
	uint8_t is_array;
	uint8_t is_pointer;
};