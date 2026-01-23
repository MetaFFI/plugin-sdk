# CPython3 Runtime Manager

C++ library for managing CPython 3.8-3.13 runtime, loading modules, and accessing entities (functions, methods, variables, fields).

## Overview

This library provides a LoadLibrary/GetProcAddress-like interface for CPython3:
- **LoadLibrary** = `RuntimeManager::load_module()` - Loads Python modules
- **GetProcAddress** = `Module::load_entity()` - Loads entities (functions, methods, variables) from modules
- **Call** = `Entity::call()` / `Entity::get()` / `Entity::set()` - Execute entities

## Key Features

- **Dynamic Python Loading**: Supports Python 3.8-3.13 via dynamic library loading
- **Version Detection**: `RuntimeManager::detect_installed_python3()` finds available Python versions
- **RAII**: Proper PyObject* reference counting
- **Thread-Safe**: All public methods are thread-safe
- **No Caching**: Each load creates new instances (as per requirements)
- **No CDTS**: CDTS conversion is **NOT** part of this library (implemented by runtime plugin)

## Usage Example

```cpp
#include "runtime_manager.h"
#include "module.h"
#include "entity.h"

// Detect available Python versions
auto versions = RuntimeManager::detect_installed_python3();
// versions = ["3.8", "3.9", "3.11", ...]

// Create runtime manager for specific version
RuntimeManager manager("3.11");

try {
    // Load runtime
    manager.load_runtime();

    // Load module
    auto module = manager.load_module("/path/to/mymodule.py");

    // Load entity (function)
    std::vector<PyObject*> params = { /* Python type objects */ };
    std::vector<PyObject*> retvals = { /* Python type objects */ };
    auto entity = module->load_entity("callable=my_function", params, retvals);

    // Call entity using native PyObject* args
    std::vector<PyObject*> args = { /* ... */ };
    PyObject* result = entity->call(args);
    Py_XDECREF(result);
    // ...

    // Release runtime
    manager.release_runtime();
} catch(const std::exception& e) {
    // Handle error
    return;
}
```

## Important Notes

### Native Types vs CDTS

**This library operates on native PyObject* values.** It does not serialize or deserialize CDTS. Runtime plugins should use `cdts_serializer` to bridge CDTS and PyObject* values.

This library provides:
- `py_callable_` - The Python callable object
- `params_types_` - Parameter type information
- `retval_types_` - Return value type information
- `is_varargs_`, `is_named_args_` - Function signature flags

### RAII and Reference Counting

All PyObject* references are managed with proper reference counting:
- Constructor: `Py_INCREF()`
- Destructor: `Py_DECREF()`
- Copy constructor: `Py_INCREF()` on copy
- Move constructor: Transfer ownership (no refcount change)

### Thread Safety

- All public methods use mutexes for thread safety
- GIL is acquired/released around Python API calls
- Multiple threads can safely call methods concurrently

## Entity Types

Supported entity types (from `sdk/idl_entities/entity_path_specs.json`):

- **PythonFunction** - Module-level functions
- **PythonMethod** - Class methods (instance and static)
- **PythonConstructor** - Class constructors
- **PythonGlobalGetter** - Global variable getters
- **PythonGlobalSetter** - Global variable setters
- **PythonFieldGetter** - Class field getters
- **PythonFieldSetter** - Class field setters

## Building

The library is built via CMake as part of the SDK:

```bash
cd build
cmake ..
make cpython3_runtime_manager
```

The library will be installed to `$METAFFI_HOME/sdk/runtime_manager/cpython3/`.
