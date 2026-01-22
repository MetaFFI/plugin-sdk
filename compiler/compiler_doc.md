# SDK Compiler Documentation

## Purpose

The SDK compiler layer generates runtime-specific code from MetaFFI IDL (JSON).
It is the canonical implementation used by all language plugins. Plugins must
only forward calls from the MetaFFI CLI to the SDK compiler.

This document describes what a host/guest compiler in the SDK must provide and
how it should be structured.

## High-Level Architecture

MetaFFI compilation is split into two layers:

1) **IDL Compilers**
   - Convert runtime-native artifacts (e.g., Java bytecode) into MetaFFI IDL.
   - Implemented under `sdk/idl_compiler/<runtime>`.

2) **IDL-Based Compilers (this document)**
   - Read MetaFFI IDL JSON and generate runtime-specific artifacts.
   - Implemented under `sdk/compiler/<runtime>`.

Both layers need IDL type mappings, so shared **IDL entities** must be central
and reused by both sides.

## IDL Entities (Shared Mapping)

To avoid multiple IDL parsing implementations, the SDK introduces:

- `sdk/idl_entities/<runtime>`

This package provides the IDL <-> runtime-native entity mapping.
All compilers must use these entities rather than re-implementing parsing logic.

## Host vs. Guest Compiler

### Host Compiler

- Generates host-side code that calls MetaFFI API.
- Produces user-friendly stubs whose bodies delegate to MetaFFI calls.
- Supported for all runtimes.

Example outputs:
- Python3: `.py` stubs
- JVM: `.jar` with compiled classes
- Go/C/C++: `.go` / `.h` or similar runtime-specific artifacts

### Guest Compiler

- Generates guest-side entrypoints when a runtime does not expose functions by
  default (e.g., Go, C/C++).
- Reads MetaFFI IDL and emits code that creates explicit exported entrypoints,
  then compiles with the guest source.
- Required only for runtimes without default entrypoint discovery.

## Responsibilities and Boundaries

### SDK Compiler (Core)

- Owns IDL parsing via `sdk/idl_entities/<runtime>`
- Owns code generation templates and logic
- Owns host/guest compiler implementations
- Provides a stable API (if possible) that plugins call

### Language Plugin Compiler (Wrapper)

- Implements `language_plugin_interface.h` entrypoint
- Forwards calls to SDK compiler packages
- Handles packaging/CLI plumbing only

## Directory Layout Requirements

Per runtime (`<runtime>`):

- `sdk/compiler/<runtime>/host/`  -> host compiler package
- `sdk/compiler/<runtime>/guest/` -> guest compiler package (if needed)
- `sdk/compiler/<runtime>/plugin/` -> helpers for plugin wrappers (optional)
- `sdk/compiler/<runtime>/common/` -> shared utils/templates

Shared templates and IDL types must be reused, not duplicated.

## Output Conventions

Compiler outputs should be placed under:

- `$METAFFI_HOME/<runtime>/...`

Host and guest outputs should match the runtime's packaging conventions.

## CMake Integration

- Compiler tests are per-runtime targets (e.g., `compiler_go_tests`).
- Those targets are bubbled up to an aggregate `metaffi_compiler_tests`.
- `metaffi_compiler_tests` is bubbled to `metaffi_sdk` and ultimately to root.
- Targets should be excluded from default build and only run when requested.

## Implementation Checklist

- [ ] Define IDL entity mapping in `sdk/idl_entities/<runtime>`.
- [ ] Update `sdk/idl_compiler/<runtime>` to use IDL entities.
- [ ] Implement host compiler package under `sdk/compiler/<runtime>/host`.
- [ ] Implement guest compiler package under `sdk/compiler/<runtime>/guest` (if required).
- [ ] Reuse templates and helpers from `sdk/compiler/<runtime>/common`.
- [ ] Ensure plugin wrapper (`lang-plugin-<runtime>/compiler`) calls SDK code.
- [ ] Provide per-runtime tests and CMake targets.
- [ ] Update docs and tests documentation.

## Notes

- `sdk/compiler/language_plugin_interface.h` remains as the interface expected
  by MetaFFI CLI. It is not runtime-specific and should not move.
- The SDK root must not be a Go module; each runtime compiler has its own Go
  module.
