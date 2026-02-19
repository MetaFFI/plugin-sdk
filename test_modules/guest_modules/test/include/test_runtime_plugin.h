#pragma once

// Export macro for Windows DLL
#ifdef _WIN32
	#ifdef TEST_PLUGIN_EXPORTS
		#define TEST_PLUGIN_API __declspec(dllexport)
	#else
		#define TEST_PLUGIN_API __declspec(dllimport)
	#endif
#else
	#define TEST_PLUGIN_API
#endif

#include "cdt.h"
#include "xcall.h"
#include "metaffi_primitives.h"

// Define metaffi_any_array_type if not defined (missing from SDK)
#ifndef metaffi_any_array_type
#define metaffi_any_array_type (metaffi_any_type | metaffi_array_type)
#endif

extern "C"
{

/**
 * Load the test runtime.
 * Prints to STDOUT and initializes internal state.
 * Must be called before any other API function.
 */
TEST_PLUGIN_API void load_runtime(char** err);

/**
 * Free the test runtime.
 * Prints to STDOUT.
 * Returns error if load_runtime was not called first.
 */
TEST_PLUGIN_API void free_runtime(char** err);

/**
 * Load a test entity and return an xcall to invoke it.
 *
 * @param module_path Module path (logged but not used for test entities)
 * @param entity_path Entity path (e.g., "test::echo_int64")
 * @param params_types Array of parameter type info
 * @param params_count Number of parameters
 * @param retvals_types Array of return value type info
 * @param retval_count Number of return values
 * @param err Output error message if failed
 * @return xcall pointer or nullptr on error
 */
TEST_PLUGIN_API struct xcall* load_entity(
	const char* module_path,
	const char* entity_path,
	metaffi_type_info* params_types,
	int8_t params_count,
	metaffi_type_info* retvals_types,
	int8_t retval_count,
	char** err
);

/**
 * Create a callable wrapper around the provided context.
 *
 * @param make_callable_context Context to wrap (typically an xcall pointer)
 * @param params_types Array of parameter type info
 * @param params_count Number of parameters
 * @param retvals_types Array of return value type info
 * @param retval_count Number of return values
 * @param err Output error message if failed
 * @return xcall pointer or nullptr on error
 */
TEST_PLUGIN_API struct xcall* make_callable(
	void* make_callable_context,
	metaffi_type_info* params_types,
	int8_t params_count,
	metaffi_type_info* retvals_types,
	int8_t retval_count,
	char** err
);

/**
 * Free an xcall previously returned by load_entity or make_callable.
 */
TEST_PLUGIN_API void free_xcall(xcall* pff, char** err);

/**
 * XCall variant handlers - invoked by the SDK.
 */
TEST_PLUGIN_API void xcall_params_ret(void* context, cdts params_ret[2], char** out_err);
TEST_PLUGIN_API void xcall_params_no_ret(void* context, cdts params_ret[2], char** out_err);
TEST_PLUGIN_API void xcall_no_params_ret(void* context, cdts params_ret[2], char** out_err);
TEST_PLUGIN_API void xcall_no_params_no_ret(void* context, char** out_err);

} // extern "C"
