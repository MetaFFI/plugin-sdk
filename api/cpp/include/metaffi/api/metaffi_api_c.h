#pragma once

#include <runtime/metaffi_primitives.h>
#include <runtime/cdt.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque handles for the C API.
 * Each handle is a heap-allocated C++ object cast to void*.
 */
typedef void* metaffi_runtime_h;
typedef void* metaffi_module_h;
typedef void* metaffi_entity_h;

/* -----------------------------------------------------------------------
 * Runtime lifecycle
 * ---------------------------------------------------------------------- */

/** Create a runtime handle (normalises "test" → "xllr.test"). Returns NULL on error. */
metaffi_runtime_h metaffi_runtime_create(const char* runtime_plugin, char** out_err);

/** Load the runtime plugin via XLLR. */
void metaffi_runtime_load_plugin(metaffi_runtime_h h, char** out_err);

/** Release the runtime plugin via XLLR. */
void metaffi_runtime_release_plugin(metaffi_runtime_h h, char** out_err);

/** Free the runtime handle (does NOT release the plugin). */
void metaffi_runtime_free(metaffi_runtime_h h);

/* -----------------------------------------------------------------------
 * Module
 * ---------------------------------------------------------------------- */

/** Load a module from the runtime. module_path may be empty (""). Returns NULL on error. */
metaffi_module_h metaffi_module_load(metaffi_runtime_h runtime,
                                     const char* module_path,
                                     char** out_err);

/** Free the module handle. */
void metaffi_module_free(metaffi_module_h h);

/* -----------------------------------------------------------------------
 * Entity
 * ---------------------------------------------------------------------- */

/**
 * Load an entity (function/method/field).
 * params_types/retvals_types may be NULL when count is 0.
 * Returns NULL on error.
 */
metaffi_entity_h metaffi_entity_load(metaffi_module_h module,
                                     const char* entity_path,
                                     const metaffi_type_info* params_types,  int8_t params_count,
                                     const metaffi_type_info* retvals_types, int8_t retvals_count,
                                     char** out_err);

/**
 * Call an entity using a 2-element CDTS array.
 *   params_ret[0] = input parameters  (caller populates before call)
 *   params_ret[1] = output retvals    (filled by this function on success)
 *
 * On return, params_ret[0] is consumed and params_ret[1] holds the results.
 */
void metaffi_entity_call(metaffi_entity_h h, struct cdts params_ret[2], char** out_err);

/** Free the entity handle. */
void metaffi_entity_free(metaffi_entity_h h, char** out_err);

#ifdef __cplusplus
}
#endif
