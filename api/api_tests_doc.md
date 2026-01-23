# MetaFFI API Tests Documentation

## Scope

API tests validate the public, user-facing runtime APIs (Go, JVM, Python3). They ensure that a consumer can:

- Load a runtime plugin
- Load a module
- Load and invoke foreign entities
- Receive correct values and error signals
- Release resources cleanly

## Test Locations

Cross-language API tests currently live under:

- `lang-plugin-*/api/tests`

These directories **must not be moved or modified** as part of the API implementation. They are managed separately.

Runtime-specific unit tests (if any) may live alongside the API implementation, but they are optional and should not replace the cross-language suites.

## What To Test

### Required Scenarios

- **Runtime load/unload**
  - Load runtime plugin for each supported runtime
  - Release runtime plugin and validate cleanup

- **Module load**
  - Load modules/packages with valid and invalid paths
  - Validate error handling for missing modules

- **Entity load**
  - Functions, globals, class methods/constructors
  - Error cases when entity path is invalid

- **Type coverage**
  - All MetaFFI primitive types
  - Strings and arrays (including multi-dimensional/ragged)
  - Handles and callables where supported

- **Error propagation**
  - Exceptions/panics/errors from guest runtime should propagate cleanly

## Packaging Tests (Sanity)

Each runtime API package should be installable from its distribution target:

- Go: `go get github.com/MetaFFI/sdk/api/go`
- Python3: `pip install metaffi-api`
- JVM: JAR should be consumable as a dependency (future Maven publish)

## CMake Integration

API build targets are part of the SDK build graph (`sdk/api`).
Tests under `lang-plugin-*/api/tests` are intentionally excluded from the SDK build targets for now.
