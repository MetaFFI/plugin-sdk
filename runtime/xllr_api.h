#pragma once
/***
 * MetaFFI XLLR API 
 */

#include <stdint.h>
#include <stdarg.h>
#include <runtime/cdt_structs.h>


#ifndef SKIP_XLLR_API_EXTERN
extern "C"
{
#endif
/**
 * Load language runtime of foreign language
 */
void load_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);

/**
 * Free language runtime of foreign language
 */
void free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);

/**
 * Load module of foreign language
 */
void** load_function(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len, const char* module_path, uint32_t module_path_len, const char* function_path, uint32_t function_path_len, metaffi_types_with_alias_ptr params_types, metaffi_types_with_alias_ptr retvals_types, uint8_t params_count, uint8_t retval_count, char** err, uint32_t* err_len);

 /**
  * Load module of foreign language
  */
void** make_callable(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len, void* make_callable_context, metaffi_types_with_alias_ptr params_types, metaffi_types_with_alias_ptr retvals_types, uint8_t params_count, uint8_t retval_count, char** err, uint32_t* err_len);

/**
 * Free module of foreign language
 */
void free_function(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len, void** pff, char** err, uint32_t* err_len);

/***
 * Call foreign entity
 */
void xcall_params_ret(
	void* pplugin_xcall_params_ret_and_context[2],            // [in] pointer to plugin's xcall_params_ret + context
	cdts params_ret[2],                                       // [in/out] parameters array
	char** out_err, uint64_t* out_err_len                     // [out] error
);

/***
 * Call foreign entity
 */
void xcall_no_params_ret(
		void* pplugin_xcall_no_params_ret_and_context[2],         // [in] pointer to plugin's xcall_no_params_ret + context
		cdts return_values[1],                                    // [in] return values array
		char** out_err, uint64_t* out_err_len                     // [out] error
);

/***
 * Call foreign entity
 */
void xcall_params_no_ret(
		void* pplugin_xcall_params_no_ret_and_context[2],         // [in] pointer to plugin's xcall_params_no_ret + context
		cdts parameters[1],                                       // [in] parameters array
		char** out_err, uint64_t* out_err_len                     // [out] error
);

/***
 * Call foreign entity
 */
void xcall_no_params_no_ret(
		void* pplugin_xcall_no_params_no_ret_and_context[2],      // [in] pointer to plugin's xcall_no_params_no_ret + context
		char** out_err, uint64_t* out_err_len                     // [out] error
);

/**
 * @brief Sets a flag in XLLR
 */
void set_runtime_flag(const char* flag_name, uint64_t flag_name_length);

/**
 * @brief Test a flag in XLLR
 */
int is_runtime_flag_set(const char* flag_name, uint64_t flag_name_length);

#ifndef SKIP_XLLR_API_EXTERN
}
#endif