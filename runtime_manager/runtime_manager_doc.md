# MetaFFI Runtime Manager Documentation

**Version:** 1.0  
**Purpose:** Complete specification for implementing MetaFFI runtime_manager C++ libraries

---

## Table of Contents

1. [Introduction and Purpose](#1-introduction-and-purpose)
2. [Architecture and Class Structure](#2-architecture-and-class-structure)
3. [RuntimeManager Class Interface](#3-runtimemanager-class-interface)
4. [Module Class Interface](#4-module-class-interface)
5. [Entity Classes Interface](#5-entity-classes-interface)
6. [Caching Strategy](#6-caching-strategy)
7. [Thread Safety Requirements](#7-thread-safety-requirements)
8. [Error Handling](#8-error-handling)
9. [Memory Management](#9-memory-management)
10. [Implementation Notes](#10-implementation-notes)
11. [Integration with Runtime Plugins](#11-integration-with-runtime-plugins)

---

## 1. Introduction and Purpose

### 1.1 What is a Runtime Manager?

A **MetaFFI Runtime Manager** is a C++ library that provides object-oriented management of a language runtime environment. It encapsulates the lifecycle of a runtime, manages loaded modules, and provides access to entities (functions, methods, variables, fields) within those modules.

### 1.2 Key Characteristics

- **Concrete C++ Library**: The runtime_manager is a concrete C++ class library, not a base class or interface
- **Runtime-Specific**: Each runtime (cpython3, jvm, dotnet, go) implements its own runtime_manager library
- **Used by Runtime Plugins**: Existing runtime plugins will use the runtime_manager library as a dependency
- **Object-Oriented Design**: Uses RuntimeManager, Module, and Entity classes with clear ownership relationships

### 1.3 Implementation Flexibility

**Important**: While the runtime_manager class interface is defined in C++, the implementation may require cross-language calls:

- The C++ class interface is the contract that all implementations must follow
- Implementation may call into other languages when necessary
- Example: JVM runtime_manager might need to call Java code via JNI
- Example: Python runtime_manager might call Python C API functions
- Pure C++ implementation is preferable but not required when cross-language calls are necessary

### 1.4 Role in MetaFFI Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    MetaFFI Runtime Flow                     │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Runtime Plugin (existing)                                   │
│         │                                                    │
│         ↓                                                    │
│  ┌──────────────────────┐                                 │
│  │  RuntimeManager       │  ← NEW: C++ Library             │
│  │  (C++ class)          │                                 │
│  └──────────────────────┘                                 │
│         │                                                    │
│         ├─→ Module (C++ class)                              │
│         │        │                                           │
│         │        └─→ Entity (runtime-specific C++ class)   │
│         │                                                    │
│         └─→ Runtime Environment (Python/Java/Go/etc.)      │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Architecture and Class Structure

### 2.1 Class Hierarchy

The runtime_manager library consists of three main class types:

1. **RuntimeManager** - Main entry point, manages runtime lifecycle
2. **Module** - Represents a loaded module/code unit
3. **Entity** - Runtime-specific classes representing callables, variables, fields

### 2.2 RuntimeManager Class

**Type**: Concrete C++ class  
**Location**: Part of runtime_manager library  
**Purpose**: Manages the runtime environment lifecycle and caches loaded modules

**Key Responsibilities**:
- Initialize and shutdown the runtime environment
- Load and cache modules
- Provide thread-safe access to modules
- Manage module lifecycle

### 2.3 Module Class

**Type**: Concrete C++ class  
**Location**: Part of runtime_manager library  
**Purpose**: Represents a loaded module that can contain entities

**Key Responsibilities**:
- Represent a loaded module/code unit (e.g., Python module, Java class, Go package)
- Find and load entities within the module
- Cache loaded entities
- Provide thread-safe entity access

**Ownership**: Module instances are owned and managed by RuntimeManager

### 2.4 Entity Classes

**Type**: Runtime-specific concrete C++ classes  
**Location**: Part of runtime_manager library (runtime-specific implementations)  
**Purpose**: Represent callable entities (functions, methods, constructors) and variable entities (globals, fields)

**Key Characteristics**:
- **No base Entity class** - each runtime defines its own entity classes
- Entity types are determined by `sdk/idl_entities/entity_path_specs.json` for each runtime
- Each entity class is a concrete class specific to that runtime

**Entity Types by Runtime**:

**Python (cpython3)**:
- `PythonFunction` - Module-level functions
- `PythonMethod` - Class instance/static methods
- `PythonConstructor` - Class constructors (`__init__`)
- `PythonGlobalGetter` - Global variable getters
- `PythonGlobalSetter` - Global variable setters
- `PythonFieldGetter` - Class field getters
- `PythonFieldSetter` - Class field setters

**Java (jvm)**:
- `JavaMethod` - Class methods (instance and static)
- `JavaConstructor` - Class constructors
- `JavaFieldGetter` - Class field getters
- `JavaFieldSetter` - Class field setters

**Go**:
- `GoFunction` - Package-level functions and methods
- `GoGlobalGetter` - Global variable getters
- `GoGlobalSetter` - Global variable setters
- `GoFieldGetter` - Struct field getters
- `GoFieldSetter` - Struct field setters

**Ownership**: Entity instances are owned and managed by Module

---

## 3. RuntimeManager Class Interface

### 3.1 Constructor

```cpp
RuntimeManager();
```

Creates a RuntimeManager instance. The runtime is not loaded until `load_runtime()` is called.

### 3.2 Destructor

```cpp
~RuntimeManager();
```

Automatically calls `release_runtime()` if the runtime is still loaded. All modules and entities are cleaned up.

### 3.3 load_runtime

```cpp
void load_runtime();
```

**Purpose**: Initialize the runtime environment.

**Behavior**:
- Initializes the runtime environment (e.g., starts Python interpreter, loads JVM, etc.)
- Must be idempotent - calling multiple times should have no effect if already loaded
- Thread-safe - can be called concurrently
- On error, throws `std::exception` with a runtime-specific message

**Example**:
```cpp
RuntimeManager manager;
try {
    manager.load_runtime();
} catch (const std::exception& e) {
    std::cerr << "Failed to load runtime: " << e.what() << std::endl;
}
```

### 3.4 release_runtime

```cpp
void release_runtime();
```

**Purpose**: Shutdown and cleanup the runtime environment.

**Behavior**:
- Releases all loaded modules and their entities
- Shuts down the runtime environment
- Must be idempotent - calling multiple times should have no effect if not loaded
- Thread-safe - can be called concurrently
- On error, throws `std::exception` with a runtime-specific message

### 3.5 load_module

```cpp
std::shared_ptr<Module> load_module(const std::string& module_path);
```

**Purpose**: Load a module and return a Module instance.

**Parameters**:
- `module_path`: Path to the module (e.g., file path, package name, class path)

**Returns**: `std::shared_ptr<Module>` to the loaded module.

**Behavior**:
- Loads the module if not already loaded (caching)
- Returns cached module if already loaded
- Thread-safe - can be called concurrently
- Module path format is runtime-specific:
  - Python: File path (e.g., `/path/to/module.py`) or module name
  - Java: Class path or JAR file path
  - Go: Package path or file path

**Caching**: Modules are cached by `module_path`. Subsequent calls with the same path return the cached instance.

**Example**:
```cpp
try {
    auto module = manager.load_module("/path/to/mymodule.py");
} catch (const std::exception& e) {
    std::cerr << "Failed to load module: " << e.what() << std::endl;
}
```

### 3.6 is_runtime_loaded

```cpp
bool is_runtime_loaded() const;
```

**Purpose**: Check if the runtime is currently loaded.

**Returns**: `true` if runtime is loaded, `false` otherwise.

**Thread Safety**: Thread-safe read operation.

---

## 4. Module Class Interface

### 4.1 get_module_path

```cpp
const std::string& get_module_path() const;
```

**Purpose**: Get the module path that was used to load this module.

**Returns**: Reference to the module path string.

### 4.2 load_entity

```cpp
std::shared_ptr<Entity> load_entity(
    const std::string& entity_path,
    const std::vector<native_type>& params_types,
    const std::vector<native_type>& retval_types
);
```

**Purpose**: Find and load an entity within the module.

**Parameters**:
- `entity_path`: Entity path string (format defined in `sdk/idl_entities/entity_path_specs.json`)
- `params_types`: Vector of runtime-native type descriptors (runtime-specific)
- `retval_types`: Vector of runtime-native type descriptors (runtime-specific)

**Returns**: `std::shared_ptr<Entity>` to the loaded entity.

**Behavior**:
- Parses `entity_path` to determine entity type and location
- Loads the entity from the module
- Returns cached entity if already loaded with same type signature
- Thread-safe - can be called concurrently
- Entity path format is runtime-specific (see `sdk/idl_entities/entity_path_specs.json`)

**Entity Path Examples**:

**Python Function**:
```
callable=my_function
```

**Python Method**:
```
callable=MyClass.my_method,instance_required
```

**Java Method**:
```
class=com.example.MyClass,callable=myMethod,instance_required
```

**Go Function**:
```
callable=MyFunction
```

**Caching**: Entities are cached by `entity_path` + type signature. Subsequent calls with the same entity_path and types return the cached instance.

**Example**:
```cpp
using native_type = /* runtime-specific type descriptor (e.g., PyObject*, jclass) */;
std::vector<native_type> params = { /* ... */ };
std::vector<native_type> retvals = { /* ... */ };
auto entity = module->load_entity("callable=my_function", params, retvals);
```

### 4.3 unload

```cpp
void unload();
```

**Purpose**: Release module resources and unload all entities.

**Behavior**:
- Releases all entities loaded from this module
- Clears entity cache
- Module can be reloaded after unload
- Optional - RuntimeManager will automatically unload on release_runtime()
- Thread-safe

---

## 5. Entity Classes Interface

### 5.1 Overview

Entity classes are runtime-specific concrete classes. There is **no base Entity class**. Each runtime defines its own entity classes based on the entity types required by that runtime (as defined in `sdk/idl_entities/entity_path_specs.json`).

### 5.2 Callable Entity Interface

Callable entities represent functions, methods, and constructors that can be invoked.

**Required Methods**:

#### call (native types)

Runtime managers do **not** serialize/deserialize CDTS. They accept and return runtime-native types.

**Python (cpython3)**:
```cpp
PyObject* call(const std::vector<PyObject*>& args);
PyObject* call(PyObject* args_tuple);
```

**Java (JVM)**:
```cpp
jvalue call(const std::vector<jvalue>& args);
jvalue call(jobject instance, const std::vector<jvalue>& args);
```

**Purpose**: Execute the callable with runtime-native arguments.

**Behavior**:
- Calls the underlying function/method/constructor in the runtime
- Returns a runtime-native value
- Thread-safe - can be called concurrently

**Ownership Notes**:
- Python: returned `PyObject*` is a new reference (caller must `Py_DECREF`)
- Java: returned `jobject` (in `jvalue.l`) is a local reference (caller manages local refs)

#### get_params_types

```cpp
const std::vector<native_type>& get_params_types() const;
```

**Purpose**: Get parameter type information.

**Returns**: Reference to vector of parameter type information.

#### get_retval_types

```cpp
const std::vector<native_type>& get_retval_types() const;
```

**Purpose**: Get return value type information.

**Returns**: Reference to vector of return value type information.

### 5.3 Variable Entity Interface

Variable entities represent global variables and class/struct fields that can be read and written.

**Required Methods**:

#### get (native types)

**Python (cpython3)**:
```cpp
PyObject* get();
```

**Java (JVM)**:
```cpp
jvalue get(jobject instance);
```

**Purpose**: Read variable value.

**Behavior**:
- Reads the variable value from the runtime
- Returns a runtime-native value
- Thread-safe - can be called concurrently

**Ownership Notes**:
- Python: returned `PyObject*` is a new reference (caller must `Py_DECREF`)
- Java: returned `jobject` (in `jvalue.l`) is a local reference (caller manages local refs)

#### set (native types)

**Python (cpython3)**:
```cpp
void set(PyObject* value);
```

**Java (JVM)**:
```cpp
void set(jobject instance, jvalue value);
```

**Purpose**: Write variable value.

**Behavior**:
- Writes the variable value in the runtime
- Thread-safe - can be called concurrently

#### get_type

```cpp
native_type get_type() const;
```

**Purpose**: Get variable type information.

**Returns**: Type information for the variable.

### 5.4 Getter/Setter Entity Pattern

Some runtimes may implement getter/setter as separate entities, while others may combine them:

- **Separate Entities**: `GlobalGetter` and `GlobalSetter` are separate classes
- **Combined Entity**: A single `GlobalVariable` class with both `get()` and `set()` methods

Both patterns are acceptable. The choice depends on the runtime's natural API.

### 5.5 Runtime-Specific Entity Classes

Each runtime must implement entity classes for all entity types defined in `sdk/idl_entities/entity_path_specs.json`:

**Python (cpython3)**:
- `PythonFunction` - Implements callable entity interface
- `PythonMethod` - Implements callable entity interface
- `PythonConstructor` - Implements callable entity interface
- `PythonGlobalGetter` - Implements variable entity interface (get only)
- `PythonGlobalSetter` - Implements variable entity interface (set only)
- `PythonFieldGetter` - Implements variable entity interface (get only)
- `PythonFieldSetter` - Implements variable entity interface (set only)

**Java (jvm)**:
- `JavaMethod` - Implements callable entity interface
- `JavaConstructor` - Implements callable entity interface
- `JavaFieldGetter` - Implements variable entity interface (get only)
- `JavaFieldSetter` - Implements variable entity interface (set only)

**Go**:
- `GoFunction` - Implements callable entity interface
- `GoGlobalGetter` - Implements variable entity interface (get only)
- `GoGlobalSetter` - Implements variable entity interface (set only)
- `GoFieldGetter` - Implements variable entity interface (get only)
- `GoFieldSetter` - Implements variable entity interface (set only)

---

## 6. Caching Strategy

### 6.1 Module Caching

**Location**: RuntimeManager  
**Key**: `module_path` (string)  
**Value**: `std::shared_ptr<Module>`

**Behavior**:
- Modules are cached by their module path
- First call to `load_module()` loads and caches the module
- Subsequent calls with the same `module_path` return the cached instance
- Cache is cleared when `release_runtime()` is called
- Thread-safe cache access

### 6.2 Entity Caching

**Location**: Module  
**Key**: `entity_path` + type signature (params_types + retval_types)  
**Value**: `std::shared_ptr<Entity>`

**Behavior**:
- Entities are cached by entity_path and type signature
- First call to `load_entity()` loads and caches the entity
- Subsequent calls with the same entity_path and types return the cached instance
- Cache is cleared when `Module::unload()` is called or when the module is destroyed
- Thread-safe cache access

### 6.3 Cache Invalidation

**Automatic**:
- Module cache cleared on `RuntimeManager::release_runtime()`
- Entity cache cleared on `Module::unload()` or module destruction

**Manual**:
- Call `Module::unload()` to clear entity cache for a specific module
- Call `RuntimeManager::release_runtime()` to clear all caches

### 6.4 Cache Key Considerations

**Module Cache Key**:
- Use exact string match for module_path
- Consider normalization (e.g., resolve relative paths, canonicalize paths)

**Entity Cache Key**:
- Combine entity_path string with type signature
- Type signature includes both parameter types and return value types
- Same entity with different type signatures should be cached separately (overloading support)

---

## 7. Thread Safety Requirements

### 7.1 RuntimeManager Thread Safety

**All public methods must be thread-safe**:
- `load_runtime()` - Can be called concurrently (idempotent)
- `release_runtime()` - Can be called concurrently (idempotent)
- `load_module()` - Can be called concurrently (caching is thread-safe)
- `is_runtime_loaded()` - Thread-safe read operation

**Implementation**:
- Use appropriate synchronization primitives (mutexes, read-write locks)
- Module cache access must be protected
- Consider using `std::shared_mutex` for read-heavy operations

### 7.2 Module Thread Safety

**All public methods must be thread-safe**:
- `get_module_path()` - Thread-safe read operation
- `load_entity()` - Can be called concurrently (caching is thread-safe)
- `unload()` - Can be called concurrently

**Implementation**:
- Entity cache access must be protected
- Consider using `std::shared_mutex` for read-heavy operations

### 7.3 Entity Thread Safety

**All public methods must be thread-safe**:
- `call()` - Can be called concurrently (for callable entities)
- `get()` - Can be called concurrently (for variable entities)
- `set()` - Can be called concurrently (for variable entities)
- Read-only methods (`get_params_types()`, `get_retval_types()`, `get_type()`) - Thread-safe

**Implementation**:
- Entity operations should be thread-safe at the runtime level
- Some runtimes may require additional synchronization (e.g., Python GIL)

### 7.4 Synchronization Guidelines

**Where Thread Safety is Needed**:
- Cache access (module cache, entity cache)
- Runtime initialization/shutdown
- Entity loading operations

**Where Thread Safety May Not Be Needed**:
- Read-only access to cached data (if properly synchronized during cache updates)
- Internal helper methods (if not exposed publicly)

**Best Practices**:
- Use read-write locks for read-heavy, write-rarely scenarios
- Use mutexes for write-heavy scenarios
- Minimize lock contention
- Consider lock-free data structures where appropriate

---

## 8. Error Handling

### 8.1 Error Reporting Pattern

All methods that can fail throw `std::exception` (typically `std::runtime_error`) with a runtime-specific message.

**Behavior**:
- On success: method returns normally
- On failure: exception is thrown with a descriptive message

### 8.2 Error Handling Examples

```cpp
try {
    manager.load_runtime();
    auto module = manager.load_module("/invalid/path");
} catch (const std::exception& e) {
    std::cerr << "Failed: " << e.what() << std::endl;
}
```

### 8.3 Error Types

Common error scenarios:
- Runtime initialization failure
- Module not found or load failure
- Entity not found in module
- Type mismatch errors
- Runtime execution errors (during entity call)
- Memory allocation failures

---

## 9. Memory Management

### 9.1 Ownership Model

**RuntimeManager**:
- Owns all Module instances (via `std::shared_ptr`)
- Modules are destroyed when RuntimeManager is destroyed or `release_runtime()` is called

**Module**:
- Owns all Entity instances (via `std::shared_ptr`)
- Entities are destroyed when Module is destroyed or `unload()` is called

**Entity**:
- May hold references to runtime objects (e.g., Python objects, Java objects)
- Must properly release runtime resources on destruction

### 9.2 Resource Cleanup

**Automatic Cleanup**:
- Destructors should release all resources
- `release_runtime()` releases all modules and entities
- `Module::unload()` releases all entities

**Manual Cleanup**:
- Call `release_runtime()` to explicitly cleanup
- Call `Module::unload()` to cleanup specific module

### 9.3 Memory Leak Prevention

**Requirements**:
- All allocated resources must be freed
- Runtime objects must be properly released (e.g., Python object reference counting, Java object release)
- Error paths must cleanup allocated resources

**Best Practices**:
- Use RAII patterns
- Use smart pointers (`std::shared_ptr`, `std::unique_ptr`)
- Use scope guards for cleanup
- Test with memory leak detection tools

---

## 10. Implementation Notes

### 10.1 Cross-Language Implementation

**Important**: The runtime_manager class interface is C++, but implementation may require cross-language calls:

**When Cross-Language Calls Are Needed**:
- Runtime doesn't have a pure C++ API
- Native runtime APIs are in another language (e.g., Java, Python)
- Performance or compatibility requirements

**Examples**:
- **JVM Runtime**: May need to call Java code via JNI to access Java classes/methods
- **Python Runtime**: May need to call Python C API functions (which is C, but may require Python interpreter)
- **Go Runtime**: May need to call Go code via CGO

**Implementation Approach**:
- C++ class interface wraps cross-language calls
- Use appropriate FFI mechanisms (JNI, Python C API, CGO, etc.)
- Hide cross-language complexity from users of the runtime_manager

### 10.2 Entity Path Parsing

Each runtime must parse entity paths according to `sdk/idl_entities/entity_path_specs.json`:

**Entity Path Format**:
- Comma-separated key=value pairs
- Example: `"callable=my_function,varargs,named_args"`
- Flags without values: `"instance_required"` (no `=value`)

**Parsing**:
- Use `metaffi::utils::entity_path_parser` (from `sdk/utils/entity_path_parser.h`)
- Validate required keys and flags per entity type
- Extract entity type from entity_path structure

### 10.3 Type Information

**Native Type Descriptors**:
- Runtime-specific type representation (e.g., `PyObject*` for CPython, `jclass` for JVM)
- Used for parameter and return value type specification
- CDTS types are **not** used inside runtime_manager

**Type Validation**:
- Validate native types when loading entities
- Match entity signature with provided native types
- Return error if types don't match

### 10.4 Native Types vs CDTS

**RuntimeManager Scope**:
- Runtime managers operate strictly on runtime-native types
- They do **not** serialize/deserialize CDTS
- CDTS serialization is handled by the `cdts_serializer` in runtime plugins

**Guidance**:
- Runtime managers should expose native-type call/get/set interfaces
- Runtime plugins are responsible for bridging CDTS to native types

---

## 11. Integration with Runtime Plugins

### 11.1 Existing Runtime Plugin Interface

The runtime_manager library will be used by existing runtime plugins. The current runtime plugin interface is defined in `sdk/runtime/runtime_plugin_api.h`:

```c
void load_runtime(char** err);
void free_runtime(char** err);
struct xcall* load_entity(const char* module_path, const char* entity_path, 
                         metaffi_type_info* params_types, int8_t params_count,
                         metaffi_type_info* retvals_types, int8_t retval_count, 
                         char** err);
void free_xcall(xcall* pff, char** err);
```

### 11.2 Mapping to RuntimeManager

**Current Implementation** (to be replaced):
- Runtime plugins directly implement `load_entity()` which loads modules and entities inline

**Future Implementation** (using runtime_manager):
- Runtime plugins create a RuntimeManager instance
- `load_runtime()` calls `RuntimeManager::load_runtime()`
- `load_entity()` uses `RuntimeManager::load_module()` then `Module::load_entity()`
- Returns `xcall` wrapper around Entity instance
 - Runtime plugin maps `metaffi_type_info` to runtime-native type descriptors via `cdts_serializer`

### 11.3 No Changes to Runtime Plugin Interface

**Important**: This task does not modify the runtime plugin interface. The runtime_manager is an internal implementation detail that runtime plugins use, but the external API (`runtime_plugin_api.h`) remains unchanged.

### 11.4 xcall Integration

**xcall Structure**:
- Defined in `sdk/runtime/xcall.h`
- Wraps function pointer and context
- Used by runtime plugin API

**Entity to xcall Mapping**:
- Entity `call()` method provides the functionality
- xcall wrapper calls Entity `call()` method
- Context holds Entity instance

---

## Summary

This documentation specifies the requirements for implementing runtime_manager C++ libraries for each MetaFFI runtime. Key points:

1. **Concrete C++ Library**: Not a base class, but a concrete library per runtime
2. **Object-Oriented Design**: RuntimeManager → Module → Entity hierarchy
3. **Caching**: Modules and entities are cached for performance
4. **Thread Safety**: All public methods must be thread-safe
5. **Error Handling**: Exceptions with descriptive messages
6. **Memory Management**: Clear ownership with smart pointers
7. **Implementation Flexibility**: May use cross-language calls when needed
8. **Integration**: Used by existing runtime plugins without changing their interface

For comprehensive testing requirements, see [runtime_manager_tests_doc.md](runtime_manager_tests_doc.md).
