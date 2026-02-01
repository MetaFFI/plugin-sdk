# Host Test Modules - Expectations

This document describes how to implement host test modules for a runtime plugin.
**Important**: host tests live under the plugin repository, not under `sdk/test_modules/host_modules/`.

For plugins in this repo, use:
`lang-plugin-<runtime>/test/`

Host test modules verify that a runtime plugin can load a guest module, load its
entities, call them via the MetaFFI API, and correctly handle errors.

---

## 1. Preconditions

Before implementing a host module:

- Ensure a corresponding guest module exists at:
  `sdk/test_modules/guest_modules/<programming-language>/`.
- If it does not exist, implement it first according to:
  `sdk/test_modules/guest_modules/guest_modules_doc.md`.
- The guest module must be buildable and the artifacts must be placed under:
  `sdk/test_modules/guest_modules/<language>/test_bin`.

---

## 2. What is a Host Test Module?

A host test module is a C/C++ test project that:

- Uses the C++ MetaFFI API (`sdk/api/cpp`) to load the runtime plugin.
- Loads the guest module for the target runtime.
- Enumerates all guest entities (functions, globals, classes, methods, etc.).
- Calls each entity and validates the expected results.
- Validates all expected error paths (invalid entity paths, type mismatches,
  runtime exceptions, etc.).

Host modules should be placed in:
`lang-plugin-<runtime>/test/`.

Example:
- Python3 host tests live under `lang-plugin-python3/test/`.

---

## 3. Required Test Coverage

For each entity exposed by the guest module:

- Load the entity using a correct `entity_path`.
- Call it with correct parameters and validate the return value(s).
- Call it with incorrect parameters (when meaningful) and validate error handling.
- Validate any runtime exceptions are surfaced as errors.

The host tests must cover **everything exposed** by the guest module,
including:

- Functions (no-arg, returning values, arrays, errors, callbacks)
- Globals/constants (getter/setter)
- Classes/objects (constructors, instance methods, static/class methods)
- Collections and arrays (1D/2D/3D, ragged, object arrays)
- Language-specific features (e.g., varargs, kwargs, overloads)
- Handle round-trips across runtimes, including runtime_id behavior:
  - Create a native object/handle in the host runtime (e.g., `std::vector<int>*`).
  - Pass it into the guest runtime (e.g., store inside a Python dict).
  - Ensure the guest runtime treats it as a foreign handle.
  - Retrieve it back in the host runtime and verify the native pointer/object is returned.

If the guest module includes `third_parties.md` or `special_primitives.md`,
the host tests must include either:

- Direct tests for those entities, **or**
- Explicit documentation of which ones are intentionally not tested and why.

---

## 4. Using the C++ MetaFFI API

Use `sdk/api/cpp` to load the runtime and entities:

1. Create `MetaFFIRuntime` with the runtime plugin name.
2. Load the runtime plugin (`load_runtime_plugin()`).
3. Load the guest module (`load_module()`).
4. For each test:
   - Load the entity (`load_entity` / `load_entity_with_info`).
   - Call the entity using the typed C++ call interface.

Note: host tests must ensure the guest module is importable by the runtime.
For example, Python host tests should make sure the guest module path
(`test_bin`) is visible to Python (via module path or environment).

Environment variables available during CMake/build:
- `METAFFI_SOURCE_ROOT` points to the MetaFFI repository root.
- `METAFFI_HOME` points to the MetaFFI installation/output directory.
- `sdk_include_dir` points to the root of the SDK include tree.

---

## 5. Testing Framework

Prefer `doctest` for C++ host tests.

- Each entity should be tested in its own `TEST_CASE`.
- Add tests for invalid entity paths, incorrect signatures, and runtime errors.

---

## 6. Build Integration

Each host module must include a `CMakeLists.txt` that:

- Builds the host test executable(s).
- Links to required SDK libraries (MetaFFI API, runtime, utils).
- Registers tests using `add_test(...)`.

The plugin's top-level `CMakeLists.txt` should include its `test/` directory
via `add_subdirectory(test)`.
