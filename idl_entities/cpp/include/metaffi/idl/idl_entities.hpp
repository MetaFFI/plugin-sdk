#pragma once

/**
 * MetaFFI IDL Entities - C++ Header-Only Library
 *
 * Single include header for all IDL entity definitions.
 * Provides reading/writing MetaFFI IDL JSON files.
 *
 * Usage:
 *   #include <metaffi/idl/idl_entities.hpp>
 *   using namespace metaffi::idl;
 *
 * Features:
 *   - C++20 header-only library
 *   - JSON serialization/deserialization
 *   - Type-safe entity definitions
 *   - RAII resource management
 *   - Fail-fast error handling
 *
 * Example:
 *   auto idl = IDLDefinition::load_from_file("test.idl.json");
 *   idl.finalize_construction();
 *   for (const auto& module : idl.modules()) {
 *       // Process modules
 *   }
 */

// Core type system
#include "metaffi_type.hpp"

// Base entity definitions
#include "arg_definition.hpp"

// Function-based entities
#include "function_definition.hpp"
#include "method_definition.hpp"
#include "constructor_definition.hpp"
#include "release_definition.hpp"

// Field and global entities
#include "field_definition.hpp"
#include "global_definition.hpp"

// Container entities
#include "class_definition.hpp"
#include "module_definition.hpp"

// Root definition
#include "idl_definition.hpp"

// Type mapping utilities
#include "type_mapper.hpp"
