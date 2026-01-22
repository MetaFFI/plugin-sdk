# SDK Compiler Tests Documentation

## Purpose

Compiler tests verify that SDK host/guest compilers:
- Parse MetaFFI IDL correctly via shared IDL entities
- Generate correct runtime-specific outputs
- Produce buildable artifacts (where applicable)
- Handle edge cases and error paths consistently

This document defines the testing strategy for all runtimes.

## Test Categories

### 1) IDL Parsing Tests

Validate that MetaFFI IDL JSON is correctly loaded into runtime-native entities
from `sdk/idl_entities/<runtime>`.

Coverage:
- All primitive types
- Arrays (fixed, multi-dim, ragged)
- Classes, methods, constructors, fields
- Global functions/variables
- Callbacks and error pathways

### 2) Host Compiler Tests

Validate generated host stubs:
- Correct function/method signatures
- Correct entity paths and MetaFFI API calls
- Proper handling of in/out parameters
- Correct module structure and packaging

Recommended checks:
- Golden-file comparisons for generated code
- Snapshot tests for generated artifacts
- Negative tests for unsupported constructs

### 3) Guest Compiler Tests

For runtimes that require explicit entrypoints (Go/C/C++):
- Verify generated entrypoint code compiles
- Verify exported symbols exist
- Validate ABI signature compatibility

### 4) Integration Tests (CGo required)

Comprehensive tests using CGo to validate cross-boundary behavior.
These should compile the generated artifacts and validate actual execution
where feasible.

## Test Execution Requirements

- Tests are per-runtime and should be named consistently:
  - `compiler_<runtime>_tests`
- All per-runtime targets are bubbled into `metaffi_compiler_tests`.
- The aggregate is bubbled up to `metaffi_sdk` and root.
- Tests are excluded from default build and run explicitly.

## Expected Test Inputs

Provide IDL fixtures representing:
- Simple functions
- Classes with methods and fields
- Overloads (if language supports)
- Callbacks (callable parameters/returns)
- Errors/exceptions
- Arrays: 1D/2D/3D and ragged
- Mixed primitive and non-primitive types

## Suggested Outputs to Validate

- Host stubs compile (or parse) successfully
- Guest entrypoints compile and export required symbols
- Generated code references correct MetaFFI runtime/plugin name
- Error handling paths report meaningful errors

## Failure Rules

- Fail fast on IDL parse errors
- Fail on missing or malformed outputs
- Fail on ABI/entrypoint mismatches

## Notes

- Tests must not rely on runtime plugin internals; only SDK APIs.
- Tests must be runnable independently from plugin repositories.
