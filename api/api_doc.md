# MetaFFI API Documentation

## Purpose

Runtime APIs are the user-facing packages that let developers call MetaFFI from their own language/runtime (Go, JVM, Python3, etc.). These APIs are **not** for MetaFFI plugin developers; they are distributed to consumers (Go module, Python package, JAR) and provide a stable, friendly interface over the low-level XLLR bridge.

## Structure

Each runtime has its own API under:

- `sdk/api/go`
- `sdk/api/jvm`
- `sdk/api/python3`

Despite language differences, the APIs follow the same logical structure:

### Core Types

1. **MetaFFIRuntime**
   - Loads and releases the runtime plugin (e.g., `python3`, `jvm`, `go`).
   - Encapsulates any runtime initialization required by the bridge.

2. **MetaFFIModule**
   - Loads foreign entities from a module/package.
   - Creates callable wrappers for functions/methods/fields.

### Required Behavior

- **Load runtime plugin** via XLLR (load/free).
- **Load module** and **load entities** using the runtime bridge API.
- **Convert MetaFFI type metadata** (use `sdk/idl_entities` for `MetaFFITypeInfo`).
- **Return user-friendly callables** that map to the runtime’s native type system.
- **Fail fast** on invalid module/entity paths or type mismatches.
- **Release resources** explicitly (runtime/plugin unload, handle cleanup).

## Distribution Requirements

Each runtime API must be packaged and published using the runtime’s ecosystem:

- **Go**: module at `github.com/MetaFFI/sdk/api/go`
- **Python3**: package (PyPI) `metaffi-api`
- **JVM**: JAR (future Maven publication)

These packages are part of the SDK core and should be buildable from `sdk/api/<runtime>`.

## What Does NOT Belong Here

- Cross-language test suites (`lang-plugin-*/api/tests`) are **not** part of the API and are handled separately.
- Plugin entrypoints and compiler plugins are **not** part of the API.

## Implementation Checklist

- [ ] Runtime API directory exists under `sdk/api/<runtime>`
- [ ] Implements MetaFFIRuntime + MetaFFIModule analogs
- [ ] Uses XLLR bridge for load/call/release
- [ ] Uses `sdk/idl_entities` for MetaFFI type info
- [ ] Packaged for the runtime (module/JAR/PyPI)
- [ ] CMake target added and bubbled up to `sdk/`
