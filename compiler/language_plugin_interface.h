#pragma once

/*
 * C-compatible declarations for the compiler plugin interface.
 * The Go plugin (sdk/compiler/go/plugin) uses this header for CGO.
 * The actual interface is defined in compiler_plugin_interface.h (C++).
 * Plugins export: init_plugin(), compile_to_guest(), compile_from_host().
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Placeholder so CGO has a valid include; symbols are exported from Go via //export */

#ifdef __cplusplus
}
#endif
