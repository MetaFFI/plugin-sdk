# MetaFFI IDL Compiler Documentation

**Version:** 1.0
**Last Updated:** 2026-01-11
**Purpose:** Complete reference for implementing MetaFFI IDL compilers/extractors

---

## Table of Contents

1. [Introduction and Purpose](#1-introduction-and-purpose)
2. [MetaFFI IDL Structure](#2-metaffi-idl-structure)
3. [Entity Path Specification](#3-entity-path-specification)
4. [Architecture and Interface](#4-architecture-and-interface)
5. [IDL Compiler Requirements](#5-idl-compiler-requirements)
6. [Existing Implementations Reference](#6-existing-implementations-reference)
7. [JSON Schema Reference](#7-json-schema-reference)

**Note:** For comprehensive testing requirements and test specifications, see [compiler_tests_doc.md](compiler_tests_doc.md).

---

## 1. Introduction and Purpose

### 1.1 What is an IDL Compiler/Extractor?

A **MetaFFI IDL Compiler** (also called **IDL Extractor**) is a tool that analyzes source code or compiled runtime artifacts and generates a **MetaFFI Interface Definition Language (IDL)** description in JSON format.

The IDL compiler is responsible for:
- **Input**: Source code (.py, .go, .java) OR compiled runtime code (.class, .jar, .dll+.h, .pyc)
- **Processing**: Type analysis, introspection, and metadata extraction
- **Output**: MetaFFI IDL JSON describing functions, classes, types, and their relationships

**IMPORTANT SCOPE:** IDL compilers extract **INTERFACE definitions** (function signatures, type declarations, class structures, global variables) **NOT implementations** (function bodies, control flow logic, internal algorithms). The goal is to describe "what can be called" not "what the code does".

### 1.2 Role in MetaFFI Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                       MetaFFI Workflow                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  Source/Compiled Code                                           │
│         │                                                        │
│         ↓                                                        │
│  ┌──────────────────┐                                          │
│  │  IDL Compiler    │  ← YOU ARE HERE                          │
│  │  (Extractor)     │                                          │
│  └──────────────────┘                                          │
│         │                                                        │
│         ↓                                                        │
│  MetaFFI IDL JSON                                               │
│         │                                                        │
│         ↓                                                        │
│  ┌──────────────────┐                                          │
│  │ Compiler Plugin  │  (Generates wrapper code)                │
│  │ (Host/Guest)     │                                          │
│  └──────────────────┘                                          │
│         │                                                        │
│         ↓                                                        │
│  Executable FFI Code                                            │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

**Key Points:**
- IDL compilers **extract** interface definitions from existing code
- Compiler plugins (separate) **generate** wrapper code from IDL JSON
- IDL serves as the **universal interface contract** between languages

### 1.3 Input Types

IDL compilers can accept two primary input types:

#### Source Code
- **Advantages**: Full type annotations, comments, original names
- **Languages**: Python (.py), Go (.go), C/C++ (.h), TypeScript (.ts)
- **Method**: Abstract Syntax Tree (AST) parsing or runtime introspection

#### Compiled Runtime Code
- **Advantages**: Works without source, extracts actual runtime types
- **Languages**: Java (.class, .jar), .NET (.dll), Python (.pyc)
- **Method**: Bytecode analysis, reflection, or binary introspection

### 1.4 Output Format

All IDL compilers produce **MetaFFI IDL JSON** with this structure:

```json
{
  "idl_source": "mymodule",
  "idl_extension": ".json",
  "idl_filename_with_extension": "mymodule.json",
  "idl_full_path": "/path/to/mymodule.json",
  "metaffi_guest_lib": "mymodule_MetaFFIGuest",
  "target_language": "python",
  "modules": [
    {
      "name": "mymodule",
      "comment": "Module documentation",
      "tags": {},
      "functions": [...],
      "classes": [...],
      "globals": [...],
      "external_resources": []
    }
  ]
}
```

---

## 2. MetaFFI IDL Structure

### 2.1 Entity Hierarchy

```
IDLDefinition (root)
  ├─ IDLSource, IDLExtension, IDLFullPath (metadata)
  ├─ MetaFFIGuestLib (generated library name)
  ├─ TargetLanguage (e.g., "python", "go", "jvm")
  └─ Modules[] (array of module definitions)
       ├─ ModuleDefinition
       │    ├─ Name, Comment, Tags
       │    ├─ Functions[] (module-level functions)
       │    │    └─ FunctionDefinition
       │    │         ├─ Name, Comment, Tags, EntityPath
       │    │         ├─ Parameters[] (ArgDefinition)
       │    │         ├─ ReturnValues[] (ArgDefinition)
       │    │         └─ OverloadIndex (0 = not overloaded, 1+ = overload#)
       │    ├─ Classes[] (class definitions)
       │    │    └─ ClassDefinition
       │    │         ├─ Name, Comment, Tags, EntityPath
       │    │         ├─ Constructors[] (ConstructorDefinition)
       │    │         ├─ Releaser (ReleaseDefinition - destructor)
       │    │         ├─ Methods[] (MethodDefinition)
       │    │         │    └─ Extends FunctionDefinition +
       │    │         │         └─ InstanceRequired (bool - static or not)
       │    │         └─ Fields[] (FieldDefinition)
       │    │              ├─ Extends ArgDefinition
       │    │              ├─ Getter (optional MethodDefinition)
       │    │              └─ Setter (optional MethodDefinition)
       │    ├─ Globals[] (module-level variables)
       │    │    └─ GlobalDefinition
       │    │         ├─ Extends ArgDefinition
       │    │         ├─ Getter (optional FunctionDefinition)
       │    │         └─ Setter (optional FunctionDefinition)
       │    └─ ExternalResources[] (dependencies)
       └─ ArgDefinition (parameter/return value/field)
            ├─ Name, Type, TypeAlias
            ├─ Comment, Tags
            ├─ Dimensions (0 = scalar, >0 = array)
            └─ IsOptional (bool - for optional parameters)
```

### 2.2 Core Entities

#### IDLDefinition
Root structure containing all modules.

| Field | Type | Description |
|-------|------|-------------|
| `idl_source` | string | Filename without extension |
| `idl_extension` | string | File extension (e.g., ".json") |
| `idl_filename_with_extension` | string | Full filename |
| `idl_full_path` | string | Absolute path to IDL file |
| `metaffi_guest_lib` | string | Auto-generated guest library name (`{idl_source}_MetaFFIGuest`) |
| `target_language` | string | Target language (e.g., "go", "python", "jvm") |
| `modules` | ModuleDefinition[] | Array of module definitions |

**Key Method**: `FinalizeConstruction()` - Must be called after JSON deserialization to:
- Expand environment variables in `external_resources`
- Set parent references between nested structures
- Initialize getter/setter relationships

#### ModuleDefinition
Logical grouping of related functions, classes, and globals.

| Field | Type | Description |
|-------|------|-------------|
| `name` | string | Module name (e.g., package, namespace) |
| `comment` | string | Documentation |
| `tags` | map[string]string | Metadata key-value pairs |
| `functions` | FunctionDefinition[] | Module-level functions |
| `classes` | ClassDefinition[] | Class definitions |
| `globals` | GlobalDefinition[] | Module-level variables |
| `external_resources` | string[] | External dependencies (library paths, JARs) |

**Common Tags:**
- `"language": "python3.11"` - Language version
- `"module_type": "package"` - Module type indicator

#### FunctionDefinition
Represents a callable function or method.

| Field | Type | Description |
|-------|------|-------------|
| `name` | string | Function name |
| `comment` | string | Documentation |
| `tags` | map[string]string | Metadata |
| `entity_path` | map[string]string | Runtime routing (e.g., `{"module": "pkg", "package": "com.example"}`) |
| `parameters` | ArgDefinition[] | Input parameters |
| `return_values` | ArgDefinition[] | Return values (can be multiple) |
| `overload_index` | int32 | 0 = not overloaded, 1+ = overload number |

**Overloading**: Functions with same name but different signatures get sequential `overload_index` values (1, 2, 3...).

**Entity Path**: Maps logical names to runtime locations:
- Common keys: `"module"`, `"package"`, `"metaffi_guest_lib"`
- Example: `{"module": "math", "package": "python.stdlib"}`

#### ClassDefinition
Represents a class, struct, or interface.

| Field | Type | Description |
|-------|------|-------------|
| `name` | string | Class name |
| `comment` | string | Documentation |
| `tags` | map[string]string | Metadata |
| `entity_path` | map[string]string | Runtime routing |
| `constructors` | ConstructorDefinition[] | Constructors |
| `release` | ReleaseDefinition | Destructor/finalizer (nullable) |
| `methods` | MethodDefinition[] | Instance and static methods |
| `fields` | FieldDefinition[] | Public fields/properties |

**Automatic Releaser**: Always generated as `"Release{ClassName}"` for resource cleanup.

#### MethodDefinition
Extends FunctionDefinition for class methods.

**Additional Field:**
| Field | Type | Description |
|-------|------|-------------|
| `instance_required` | bool | `true` = instance method, `false` = static method |

When `instance_required = true`, first parameter is automatically `"this_instance": handle`.

#### ArgDefinition
Parameter, return value, or field definition.

| Field | Type | Description |
|-------|------|-------------|
| `name` | string | Argument name |
| `type` | MetaFFIType | MetaFFI type string (see section 2.3) |
| `type_alias` | string | Language-specific type name (e.g., "int", "time.Duration") |
| `comment` | string | Documentation |
| `tags` | map[string]string | Metadata |
| `dimensions` | int | 0 = scalar, 1 = 1D array, 2 = 2D array, etc. |
| `is_optional` | bool | Whether parameter is optional (default: false) |

**Type Alias Examples:**
- Go: `type: "int64"`, `type_alias: "time.Duration"`
- Java: `type: "handle"`, `type_alias: "java.util.List"`
- Python: `type: "handle"`, `type_alias: "dict"`

#### FieldDefinition
Extends ArgDefinition for class fields.

**Additional Fields:**
| Field | Type | Description |
|-------|------|-------------|
| `getter` | MethodDefinition? | Optional getter method |
| `setter` | MethodDefinition? | Optional setter method |

**Pattern**: Public fields → automatic getter/setter generation.

#### GlobalDefinition
Extends ArgDefinition for module-level variables.

**Additional Fields:**
| Field | Type | Description |
|-------|------|-------------|
| `getter` | FunctionDefinition? | Optional getter function |
| `setter` | FunctionDefinition? | Optional setter function |

**Pattern**: Module globals → accessor functions for cross-language access.

### 2.3 MetaFFI Type System

All types in MetaFFI are represented as string constants.

#### Scalar Types

| MetaFFI Type | Description | C Equivalent | Size |
|--------------|-------------|--------------|------|
| `float64` | Double precision float | `double` | 64-bit |
| `float32` | Single precision float | `float` | 32-bit |
| `int8` | Signed 8-bit integer | `int8_t` | 8-bit |
| `int16` | Signed 16-bit integer | `int16_t` | 16-bit |
| `int32` | Signed 32-bit integer | `int32_t` | 32-bit |
| `int64` | Signed 64-bit integer | `int64_t` | 64-bit |
| `uint8` | Unsigned 8-bit integer | `uint8_t` | 8-bit |
| `uint16` | Unsigned 16-bit integer | `uint16_t` | 16-bit |
| `uint32` | Unsigned 32-bit integer | `uint32_t` | 32-bit |
| `uint64` | Unsigned 64-bit integer | `uint64_t` | 64-bit |
| `bool` | Boolean | `bool` | 1-bit (stored as byte) |
| `char8` | UTF-8 character | `char` | 8-bit |
| `char16` | UTF-16 character | `char16_t` | 16-bit |
| `char32` | UTF-32 character | `char32_t` | 32-bit |
| `string8` | UTF-8 string | `char*` | Variable |
| `string16` | UTF-16 string | `char16_t*` | Variable |
| `string32` | UTF-32 string | `char32_t*` | Variable |
| `handle` | Opaque object reference | `void*` | Pointer |
| `any` | Dynamic type (any value) | `variant` | Variable |
| `size` | Platform-specific size | `size_t` | Platform |
| `null` | Null/None/nil value | `NULL` | - |

#### Array Types

All scalar types have corresponding array types by appending `_array`:

- `float64_array`, `float32_array`
- `int8_array`, `int16_array`, `int32_array`, `int64_array`
- `uint8_array`, `uint16_array`, `uint32_array`, `uint64_array`
- `bool_array`
- `char8_array`, `char16_array`, `char32_array`
- `string8_array`, `string16_array`, `string32_array`
- `handle_array`, `any_array`, `size_array`

**Multi-dimensional arrays** use the `dimensions` field:
```json
{
  "name": "matrix",
  "type": "float64_array",
  "dimensions": 2,
  "type_alias": "float[][]"
}
```

### 2.4 Key Concepts

#### Tagging
All major entities support tags (key-value metadata):

**Common Tags:**
- `"receiver_pointer": "true"` - Go pointer receivers
- `"static": "true"` - Static methods/fields
- `"const": "true"` - Constant values
- `"deprecated": "Use newFunc instead"` - Deprecation notices

#### Entity Paths
Maps logical entity names to actual runtime locations:

```json
{
  "entity_path": {
    "module": "mypackage",
    "package": "github.com/user/repo/mypackage",
    "metaffi_guest_lib": "mymodule_MetaFFIGuest"
  }
}
```

**Usage**: Runtime FFI resolution requires these paths to locate functions/classes.

#### Overloading
Functions/methods with same name but different signatures:

```json
[
  {
    "name": "add",
    "overload_index": 1,
    "parameters": [{"name": "a", "type": "int32"}, {"name": "b", "type": "int32"}]
  },
  {
    "name": "add",
    "overload_index": 2,
    "parameters": [{"name": "a", "type": "float64"}, {"name": "b", "type": "float64"}]
  }
]
```

**Entity ID**: Functions generate IDs with overload suffix: `add_overload1ID`, `add_overload2ID`

#### Comments
All entities support appending multi-line comments:

```go
func (a *ArgDefinition) AppendComment(comment string) {
    if a.Comment != "" {
        a.Comment += "\n"
    }
    a.Comment += comment
}
```

---

## 3. Entity Path Specification

### 3.1 What is Entity Path?

**entity_path** is a critical component of the MetaFFI IDL that provides **language-specific routing information** for the runtime plugin to locate and call entities (functions, methods, fields) at runtime.

**Key Characteristics:**
- **Language-specific**: Each language runtime defines what keys it needs
- **Flexible structure**: Key-value map allowing extensibility
- **Runtime-critical**: Used by runtime plugins to resolve and invoke entities
- **Serialized format**: Stored as comma-separated pairs: `"key1=val1,key2=val2"`

**Purpose:**
When a MetaFFI runtime plugin needs to call a foreign function, it uses the entity_path to:
1. Locate the entity in the loaded module
2. Determine calling conventions (instance required, varargs, etc.)
3. Apply language-specific loading strategies

### 3.2 Centralized Specification

MetaFFI maintains a **centralized entity_path specification** at:
```
<MetaFFI_Root>/entity_path_specs.json
```

This file defines the entity_path structure for all supported languages, serving as:
- ✅ **Single source of truth** for IDL compiler developers
- ✅ **Reference documentation** for runtime plugin developers
- ✅ **Validation schema** for testing and debugging

**Usage:**
- **IDL Compilers**: Read this spec to generate correct entity_path structures
- **Runtime Plugins**: Use hardcoded logic (spec documents what runtime expects)
- **Documentation**: Auto-generated from this specification

### 3.3 Entity Path Structure by Language

#### Python3

**Functions:**
```json
{
  "callable": "function_name",
  "varargs": true,       // Optional: if *args present
  "named_args": true     // Optional: if **kwargs present
}
```

**Methods:**
```json
{
  "callable": "ClassName.method_name",
  "instance_required": true,   // false for @staticmethod/@classmethod
  "varargs": true,
  "named_args": true
}
```

**Constructors:**
```json
{
  "callable": "ClassName.__init__",
  "varargs": true,
  "named_args": true
}
```

**Global Variables (Getter/Setter):**
```json
// Getter
{
  "attribute": "global_var_name",
  "getter": true
}

// Setter
{
  "attribute": "global_var_name",
  "setter": true
}
```

**Class Fields (Getter/Setter):**
```json
// Getter
{
  "attribute": "ClassName.field_name",
  "instance_required": true,
  "getter": true
}

// Setter
{
  "attribute": "ClassName.field_name",
  "instance_required": true,
  "setter": true
}
```

**Python Runtime Behavior** (`lang-plugin-python311/runtime/python_api.cpp:769`):
- Parses `callable` → loads function/method via dot-notation
- Checks `instance_required` → determines if first parameter is instance
- Checks `varargs`/`named_args` → adjusts calling convention
- Parses `attribute` → loads variable/field via getattr/setattr
- Checks `getter`/`setter` → determines read or write operation

#### Go

**Functions/Methods:**
```json
{
  "callable": "FunctionName"  // Or "StructName.MethodName"
}
```

**Global Variables (Getter/Setter):**
```json
// Getter
{
  "global": "GlobalVarName",
  "getter": true
}

// Setter
{
  "global": "GlobalVarName",
  "setter": true
}
```

**Struct Fields (Getter/Setter):**
```json
// Getter
{
  "field": "StructName.FieldName",
  "getter": true
}

// Setter
{
  "field": "StructName.FieldName",
  "setter": true
}
```

**Go Runtime Behavior** (`lang-plugin-go/runtime/go_api.cpp:53`):
- Constructs entry point name: `EntryPoint_{callable}` (dots replaced with underscores)
- For globals: `EntryPoint_Get{global}` or `EntryPoint_Set{global}`
- For fields: `EntryPoint_{StructName}_{FieldName}_Get/Set`

#### JVM

**Methods:**
```json
{
  "class": "com.example.MyClass",
  "callable": "methodName",  // Or "<init>" for constructor
  "instance_required": true   // false for static methods
}
```

**Fields (Getter/Setter):**
```json
// Getter
{
  "class": "com.example.MyClass",
  "field": "fieldName",
  "getter": true,
  "instance_required": true
}

// Setter
{
  "class": "com.example.MyClass",
  "field": "fieldName",
  "setter": true,
  "instance_required": true
}
```

**JVM Runtime Behavior** (`lang-plugin-jvm/runtime/api.cpp:300`):
- Loads class via `class` key using JNI class loader
- Resolves method via `callable` key using JNI method lookup
- Resolves field via `field` key using JNI field lookup
- Uses `instance_required` to determine static vs instance access

### 3.4 Implementation Guidelines for IDL Compilers

When implementing an IDL compiler for a new language:

1. **Review Runtime Plugin**: Examine the runtime plugin code to understand what entity_path keys it expects
2. **Document in Spec**: Add your language's entity_path structure to `entity_path_specs.json`
3. **Generate Correctly**: Ensure your IDL compiler produces entity_path matching the spec
4. **Test with Runtime**: Validate that generated entity_path works with the runtime plugin

**Example (Python IDL Compiler)**:
```python
from sdk.idl_compiler.python3.entity_path import EntityPathGenerator

generator = EntityPathGenerator()

# For a function
entity_path = generator.create_function_entity_path(
    name="my_function",
    has_varargs=True,
    has_named_args=False
)
# Result: {"callable": "my_function", "varargs": true}
```

### 3.5 Common Patterns

**Pattern 1: Dotted Paths for Nested Entities**
- Python: `"callable": "ClassName.method_name"`
- Go: `"callable": "StructName.MethodName"`

**Pattern 2: Flags for Behavioral Metadata**
- Use boolean flags (not strings): `"instance_required": true`
- Common flags: `varargs`, `named_args`, `getter`, `setter`, `instance_required`

**Pattern 3: Separate Keys for Different Entity Types**
- Functions/Methods: `callable`
- Variables/Fields: `attribute` (Python) or `global`/`field` (Go)
- Classes: `class` (JVM)

**Pattern 4: Getter/Setter Pairs**
- Variables and fields typically generate two entity_paths:
  - One with `"getter": true`
  - One with `"setter": true`

---

## 4. Architecture and Interface

### 4.1 Plugin-Based Architecture

IDL compilers integrate into MetaFFI via a **dynamic plugin system**:

```
metaffi CLI (C++)
    ↓ (loads plugin via Boost.DLL)
┌─────────────────────────────┐
│ metaffi.idl.<extension>.dll │  ← Your IDL compiler plugin
└─────────────────────────────┘
    ↓ (implements C interface)
parse_idl(source_code, file_path, out_json, out_err)
```

**Plugin Discovery:**
- Recursive search in `$METAFFI_HOME` for DLLs matching pattern
- Naming convention: `metaffi.idl.<extension>.<dll|so|dylib>`
- Example: `metaffi.idl.py.dll`, `metaffi.idl.class.so`

**Note on Integration Flexibility:** While the sections below describe the current C++ + Boost.DLL integration mechanism, this is not the only possible approach. Future MetaFFI plugins (e.g., language-specific runtimes) may provide alternative integration layers to bridge different programming language ecosystems. The C++ interface serves as the reference implementation.

### 4.2 Current Integration: C++ via Boost.DLL

**Plugin Loading** (`plugin_loader.hpp`):

```cpp
std::shared_ptr<boost::dll::shared_library> load_plugin(
    const std::string& plugin_filename_without_extension)
{
    const char* mhome_env = std::getenv("METAFFI_HOME");

    // Recursively find: metaffi.idl.<extension>.<dll|so|dylib>
    auto plugin = find_files_recursively(
        metaffi_home,
        plugin_filename_without_extension,
        boost::dll::shared_library::suffix()
    );

    plugin_dll->load(plugin_full_path,
        boost::dll::load_mode::rtld_now | rtld_global);

    return plugin_dll;
}
```

**Plugin Caching**: Loaded once, cached globally by name.

### 4.3 Plugin Naming Convention

| Extension | Plugin Name | Example |
|-----------|-------------|---------|
| `.py` | `metaffi.idl.py` | Python source files |
| `.class` | `metaffi.idl.class` | Java bytecode |
| `.jar` | `metaffi.idl.jar` | Java archives |
| `.go` | `metaffi.idl.go` | Go source files |
| `.json` | Built-in | MetaFFI IDL JSON (passthrough) |

**Auto-detection**: If user doesn't specify plugin, CLI auto-detects by file extension.

### 4.4 C Interface Contract

**Header**: `sdk/idl_compiler/idl_plugin_interface.h`

```cpp
struct idl_plugin_interface {
    virtual void init() = 0;

    virtual void parse_idl(
        const char* source_code,          // Source code (empty if path-only)
        uint32_t source_code_length,
        const char* idl_path,             // File/directory path
        uint32_t idl_path_length,
        char** out_idl_json,              // OUTPUT: Allocated JSON
        uint32_t* out_idl_json_length,
        char** out_err,                   // OUTPUT: Error message
        uint32_t* out_err_length
    ) = 0;
};
```

**Parameters:**

| Parameter | Type | Purpose |
|-----------|------|---------|
| `source_code` | `const char*` | Source code string (or empty for file-only) |
| `source_code_length` | `uint32_t` | Length of source code |
| `idl_path` | `const char*` | File path, directory, or module path |
| `idl_path_length` | `uint32_t` | Length of path |
| `out_idl_json` | `char**` | OUTPUT: Allocated MetaFFI IDL JSON |
| `out_idl_json_length` | `uint32_t*` | OUTPUT: JSON length |
| `out_err` | `char**` | OUTPUT: Error message (if failed) |
| `out_err_length` | `uint32_t*` | OUTPUT: Error length |

**Memory Management:**
- **Plugin allocates** `out_idl_json` and `out_err` using `malloc`/`calloc`
- **C++ caller frees** the memory after use
- Errors are fail-fast: return error string immediately on failure

### 3.5 Parameter Passing

**Two Input Modes:**

1. **Source Code Mode**: `source_code` populated, `idl_path` is filename
   ```cpp
   parse_idl("def add(a, b): return a+b", 28, "math.py", 7, ...)
   ```

2. **Path Mode**: `source_code` empty, `idl_path` is file/directory/module
   ```cpp
   parse_idl("", 0, "/path/to/mymodule.py", 20, ...)
   ```

**Multi-file/Directory Support**: IDL compiler should handle:
- Single file: `/path/to/file.py`
- Directory: `/path/to/package/` (recursively parse all files)
- Module path: `"github.com/user/repo/pkg"` (resolve from GOPATH/module cache)

### 3.6 Error Handling

**Fail-Fast Strategy**: All errors propagated via `out_err` pointer.

**C++ Side** (`idl_extractor.cpp`):
```cpp
std::string idl_extractor::extract(const std::string& idl_path) {
    char* out_idl_json = nullptr;
    uint32_t out_idl_json_len = 0;
    char* out_err = nullptr;
    uint32_t out_err_len = 0;

    pidl_plugin->parse_idl("", expanded_idl_path,
        &out_idl_json, &out_idl_json_len,
        &out_err, &out_err_len);

    if (out_err) {
        std::string err_msg(out_err, out_err_len);
        free(out_err);
        throw std::runtime_error(err_msg);
    }

    std::string idl_json(out_idl_json, out_idl_json_len);
    free(out_idl_json);
    return idl_json;
}
```

**Go Plugin Side** (example):
```go
//export parse_idl
func parse_idl(source_code *C.char, source_code_length C.uint,
               idl_path *C.char, idl_path_length C.uint,
               out_idl_json **C.char, out_idl_json_length *C.uint,
               out_err **C.char, out_err_length *C.uint) {

    defer func() {
        if err := recover(); err != nil {
            msg := fmt.Sprintf("IDL compilation failed: %v", err)
            *out_err = C.CString(msg)
            *out_err_length = C.uint(len(msg))
        }
    }()

    // Actual parsing logic...

    if parsing_failed {
        *out_err = C.CString("Parse error: ...")
        *out_err_length = C.uint(len(error_msg))
        return
    }

    *out_idl_json = C.CString(json_result)
    *out_idl_json_length = C.uint(len(json_result))
}
```

**Error Context**: Include file path, line number, and specific error details.

### 3.7 CLI Usage Flow

**Command**: `metaffi --idl <path> --idl-plugin <extension> --output <dir>`

**Execution** (`cli_executor.cpp`):

```cpp
bool cli_executor::compile() {
    // 1. Extract IDL (auto-detect or use specified plugin)
    std::string extracted_idl = idl_extractor::extract(
        vm["idl"].as<std::string>(),
        vm["idl-plugin"].as<std::string>()
    );

    // 2. Proceed with compilation...
    compiler cmp(extracted_idl, vm["output"].as<std::string>());

    return true;
}
```

**Auto-Detection**:
```cpp
std::string extension = fs_idl_path.extension().string();

if (extension == ".json") {
    // Pass-through: already MetaFFI IDL JSON
    pidl_plugin = std::make_shared<json_idl_plugin>();
} else {
    // Load metaffi.idl.<extension>
    pidl_plugin = idl_plugin_interface_wrapper::load(extension);
}
```

---

## 5. IDL Compiler Requirements

**CRITICAL SCOPE LIMITATION:** IDL extraction focuses **ONLY** on interface signatures and declarations. Function/method **bodies are NOT parsed or analyzed**. IDL compilers extract:
- ✅ Function signatures (names, parameters, return types)
- ✅ Class structures (constructors, methods, fields)
- ✅ Type declarations and annotations
- ✅ Global variables and constants
- ✅ Documentation comments and metadata
- ❌ Function implementations (code logic)
- ❌ Control flow (if/else, loops)
- ❌ Internal algorithms or calculations

The goal is **interface definition** (what can be called), not **implementation analysis** (what the code does).

### 5.1 Input Handling

#### 4.1.1 Source Code Parsing

**Requirements:**
- Parse language-specific syntax (AST parsing or introspection)
- Extract type annotations, signatures, comments
- Handle multi-file modules/packages
- Preserve original names and documentation

**Approaches:**

| Language | Approach | Library/Tool |
|----------|----------|--------------|
| Python | Runtime introspection | `inspect` module |
| Go | AST parsing | Custom `go-parser` |
| Java | N/A (use bytecode) | - |
| C/C++ | Header parsing | Clang libTooling |
| TypeScript | AST parsing | TypeScript Compiler API |

**Python Example** (`py_extractor.py`):
```python
import inspect
import importlib.util

# Load module dynamically
spec = importlib.util.spec_from_file_location("module", file_path)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

# Extract functions
for name, obj in inspect.getmembers(module, inspect.isfunction):
    sig = inspect.signature(obj)
    # Extract parameters, return type from signature...
```

**Go Example** (`GoIDLCompiler.go`):
```go
import "github.com/GreenFuze/go-parser"

// Parse Go source file
parser := goparser.NewParser()
file, err := parser.ParseFile(filePath)

// Extract functions
for _, decl := range file.Declarations {
    if funcDecl, ok := decl.(*goparser.FunctionDeclaration); ok {
        // Extract name, parameters, return types...
    }
}
```

#### 4.1.2 Compiled Code Introspection

**Requirements:**
- Extract type metadata from bytecode/binary
- Handle obfuscation gracefully (fallback to `any` type)
- Support archives (JAR, ZIP) and directories
- No source code required

**Approaches:**

| Format | Approach | Library/Tool |
|--------|----------|--------------|
| Java .class | Bytecode analysis | ASM library |
| Java .jar | Archive extraction + bytecode | ASM + java.util.zip |
| .NET .dll | Reflection | System.Reflection |
| Python .pyc | Disassembly | `dis` module (limited) |

**Java Bytecode Example** (`BytecodeExtractor.java`):
```java
import org.objectweb.asm.*;

ClassReader reader = new ClassReader(new FileInputStream(classFile));
ClassVisitor visitor = new ClassVisitor(ASM9) {
    @Override
    public MethodVisitor visitMethod(int access, String name,
                                      String descriptor, ...) {
        // Extract method signature
        Type methodType = Type.getMethodType(descriptor);
        Type[] paramTypes = methodType.getArgumentTypes();
        Type returnType = methodType.getReturnType();

        // Map to MetaFFI types...
        return super.visitMethod(access, name, descriptor, ...);
    }
};
reader.accept(visitor, 0);
```

**JAR Archive Example** (`JarExtractor.java`):
```java
JarFile jar = new JarFile(jarPath);
Enumeration<JarEntry> entries = jar.entries();

while (entries.hasMoreElements()) {
    JarEntry entry = entries.nextElement();

    if (entry.getName().endsWith(".class") &&
        !entry.getName().contains("$")) {  // Skip inner classes

        InputStream stream = jar.getInputStream(entry);
        // Use BytecodeExtractor on stream...
    }
}
```

#### 4.1.3 Multi-File and Package Support

**Requirements:**
- Recursively process directories
- Resolve module/package hierarchies
- Handle imports/dependencies
- Group related entities into modules

**Directory Processing Pattern**:
```python
def process_directory(directory_path):
    module_def = ModuleDefinition(os.path.basename(directory_path))

    for root, dirs, files in os.walk(directory_path):
        for file in files:
            if file.endswith('.py'):
                # Extract from each file
                functions, classes = extract_from_file(os.path.join(root, file))
                module_def.add_functions(functions)
                module_def.add_classes(classes)

    return module_def
```

**Module Path Resolution** (Go example):
```go
// Resolve GOPATH module: "github.com/user/repo/pkg"
gopath := os.Getenv("GOPATH")
modulePath := filepath.Join(gopath, "pkg", "mod", moduleName)

// Or resolve GOROOT standard library
goroot := os.Getenv("GOROOT")
stdlibPath := filepath.Join(goroot, "src", moduleName)
```

#### 4.1.4 External Dependency Tracking

**Requirements:**
- Capture library dependencies (JARs, shared libraries)
- Expand environment variables in paths
- Support multiple dependency formats

**Example**:
```json
{
  "external_resources": [
    "$JAVA_HOME/lib/rt.jar",
    "/usr/lib/libcrypto.so",
    "${GOPATH}/pkg/mod/github.com/user/dep@v1.0.0"
  ]
}
```

**Environment Expansion** (done by `FinalizeConstruction()`):
```go
for i, ex := range m.ExternalResources {
    m.ExternalResources[i] = os.ExpandEnv(ex)
}
```

### 5.2 Type Extraction

#### 4.2.1 Primitive Type Mapping

**Requirements:**
- Map language primitives to MetaFFI types
- Handle platform-specific sizes (int, size_t)
- Preserve signedness (int vs uint)

**Mapping Tables:**

**Python → MetaFFI**:
| Python | MetaFFI Type | Notes |
|--------|--------------|-------|
| `int` | `int32` | Default for annotated ints |
| `float` | `float64` | IEEE 754 double |
| `str` | `string8` | UTF-8 strings |
| `bool` | `bool` | True/False |
| `bytes` | `uint8_array` | Byte arrays |
| `None` | `null` | Null value |
| (unannotated) | `any` | Fallback |

**Java → MetaFFI**:
| Java | MetaFFI Type | Notes |
|------|--------------|-------|
| `byte` | `int8` | Signed 8-bit |
| `short` | `int16` | Signed 16-bit |
| `int` | `int32` | Signed 32-bit |
| `long` | `int64` | Signed 64-bit |
| `float` | `float32` | IEEE 754 single |
| `double` | `float64` | IEEE 754 double |
| `boolean` | `bool` | true/false |
| `char` | `char16` | UTF-16 character |
| `String` | `string8` | UTF-8 strings |
| `Object` | `handle` | Opaque reference |

**Go → MetaFFI**:
| Go | MetaFFI Type | Notes |
|----|--------------|-------|
| `int` | `int64` | Platform-dependent (use int64) |
| `int8`, `int16`, `int32`, `int64` | `int8`, `int16`, `int32`, `int64` | Direct mapping |
| `uint`, `uint8`, `uint16`, `uint32`, `uint64` | `uint64`, `uint8`, `uint16`, `uint32`, `uint64` | Unsigned types |
| `float32`, `float64` | `float32`, `float64` | Direct mapping |
| `bool` | `bool` | Direct mapping |
| `string` | `string8` | UTF-8 strings |
| `rune` | `char32` | UTF-32 character (int32) |
| `byte` | `uint8` | Alias for uint8 |
| `interface{}` | `any` | Dynamic type |

**C/C++ → MetaFFI**:
| C/C++ | MetaFFI Type | Notes |
|-------|--------------|-------|
| `char` | `char8` | Character |
| `signed char` | `int8` | Signed 8-bit |
| `unsigned char` | `uint8` | Unsigned 8-bit |
| `short` | `int16` | Signed 16-bit |
| `unsigned short` | `uint16` | Unsigned 16-bit |
| `int` | `int32` | Signed 32-bit |
| `unsigned int` | `uint32` | Unsigned 32-bit |
| `long long` | `int64` | Signed 64-bit |
| `unsigned long long` | `uint64` | Unsigned 64-bit |
| `float` | `float32` | IEEE 754 single |
| `double` | `float64` | IEEE 754 double |
| `bool` | `bool` | C++ bool |
| `char*` | `string8` | C-string |
| `void*` | `handle` | Opaque pointer |
| `size_t` | `size` | Platform size |

#### 4.2.2 Complex Type Handling

**Arrays:**
```json
{
  "name": "values",
  "type": "float64_array",
  "dimensions": 1,
  "type_alias": "double[]"
}
```

**Multi-dimensional Arrays:**
```json
{
  "name": "matrix",
  "type": "int32_array",
  "dimensions": 2,
  "type_alias": "int[][]"
}
```

**Generic Types (Java):**
```json
{
  "name": "items",
  "type": "handle",
  "type_alias": "List<String>",
  "comment": "Generic list mapped to handle"
}
```

**Dictionary/Map Types:**
```json
{
  "name": "config",
  "type": "handle",
  "type_alias": "dict",
  "comment": "Python dict as opaque handle"
}
```

**Function Pointers/Callbacks:**
```json
{
  "name": "callback",
  "type": "callable",
  "type_alias": "func(int) int",
  "comment": "Callable function type"
}
```

#### 4.2.3 Type Aliasing

**Purpose**: Preserve language-specific type names while mapping to MetaFFI types.

**Example** (Go time.Duration):
```json
{
  "name": "timeout",
  "type": "int64",
  "type_alias": "time.Duration",
  "comment": "Duration in nanoseconds"
}
```

**Example** (Python custom class):
```json
{
  "name": "user",
  "type": "handle",
  "type_alias": "User",
  "comment": "Custom User class instance"
}
```

**Benefits**:
- Improves generated wrapper code readability
- Enables type-specific optimizations
- Preserves semantic meaning

#### 4.2.4 Dimension Tracking

**Scalar** (`dimensions: 0`):
```json
{"name": "count", "type": "int32", "dimensions": 0}
```

**1D Array** (`dimensions: 1`):
```json
{"name": "values", "type": "float64_array", "dimensions": 1}
```

**2D Array** (`dimensions: 2`):
```json
{"name": "matrix", "type": "int32_array", "dimensions": 2}
```

**Ragged Arrays**: MetaFFI supports non-rectangular arrays via nested handles.

### 5.3 Entity Extraction

#### 4.3.1 Functions

**Required Information:**
- Name
- Parameters (name, type, optional)
- Return values (can be multiple)
- Overloading detection
- Entity path (module/package location)

**Example Python Extraction**:
```python
def extract_function(func):
    sig = inspect.signature(func)
    func_def = FunctionDefinition(func.__name__)

    # Extract parameters
    for param_name, param in sig.parameters.items():
        param_type = map_python_type(param.annotation)
        is_optional = param.default != inspect.Parameter.empty

        arg = ArgDefinition(param_name, param_type)
        arg.is_optional = is_optional
        func_def.add_parameter(arg)

    # Extract return type
    if sig.return_annotation != inspect.Signature.empty:
        return_type = map_python_type(sig.return_annotation)
        func_def.add_return_value(ArgDefinition("return", return_type))

    return func_def
```

**Overloading Detection**:
```python
def detect_overloads(functions):
    overload_map = {}

    for func in functions:
        if func.name in overload_map:
            overload_map[func.name].append(func)
        else:
            overload_map[func.name] = [func]

    # Assign overload indices
    for name, func_list in overload_map.items():
        if len(func_list) > 1:
            for i, func in enumerate(func_list, start=1):
                func.overload_index = i
```

#### 4.3.2 Classes

**Required Information:**
- Name
- Constructors (can be multiple, overloaded)
- Methods (instance and static)
- Fields/Properties
- Destructor/Releaser

**Example Java Extraction** (ASM):
```java
class ClassExtractorVisitor extends ClassVisitor {
    ClassInfo classInfo;

    @Override
    public void visit(int version, int access, String name, ...) {
        classInfo = new ClassInfo(name.replace('/', '.'));
    }

    @Override
    public MethodVisitor visitMethod(int access, String name, String desc, ...) {
        if (name.equals("<init>")) {
            // Constructor
            ConstructorInfo ctor = new ConstructorInfo();
            extractParameters(desc, ctor);
            classInfo.addConstructor(ctor);

        } else if (name.equals("<clinit>")) {
            // Static initializer - skip

        } else {
            // Regular method
            MethodInfo method = new MethodInfo(name);
            method.isStatic = (access & Opcodes.ACC_STATIC) != 0;
            extractParameters(desc, method);
            extractReturnType(desc, method);
            classInfo.addMethod(method);
        }

        return super.visitMethod(access, name, desc, ...);
    }

    @Override
    public FieldVisitor visitField(int access, String name, String desc, ...) {
        if ((access & Opcodes.ACC_PUBLIC) != 0) {
            FieldInfo field = new FieldInfo(name);
            field.type = mapJavaType(Type.getType(desc));
            classInfo.addField(field);
        }
        return super.visitField(access, name, desc, ...);
    }
}
```

**Automatic Releaser Generation**:
```go
func NewClassDefinition(name string) *ClassDefinition {
    c := &ClassDefinition{
        Name:    name,
        Methods: make([]*MethodDefinition, 0),
    }

    // Auto-generate releaser
    c.Releaser = NewReleaserDefinition(c, "Release" + c.Name)

    return c
}
```

#### 4.3.3 Global Variables

**Requirements:**
- Extract module-level variables
- Generate getter/setter functions for cross-language access
- Handle constants appropriately

**Python Example**:
```python
def extract_globals(module):
    globals_list = []

    for name, obj in inspect.getmembers(module):
        if not name.startswith('_') and not inspect.isfunction(obj) and not inspect.isclass(obj):
            # Module-level variable
            global_def = GlobalDefinition(name, infer_type(obj))

            # Generate getter
            global_def.getter = FunctionDefinition(f"get_{name}")
            global_def.getter.add_return_value(ArgDefinition(name, global_def.type))

            # Generate setter (if not constant)
            if not name.isupper():  # Uppercase = constant convention
                global_def.setter = FunctionDefinition(f"set_{name}")
                global_def.setter.add_parameter(ArgDefinition("value", global_def.type))

            globals_list.append(global_def)

    return globals_list
```

#### 4.3.4 Visibility Filtering

**Requirements:**
- Only extract public entities
- Skip private/internal implementation details

**Python** (underscore prefix):
```python
if not name.startswith('_'):
    # Public entity
    extract(obj)
```

**Java** (access modifiers):
```java
if ((access & Opcodes.ACC_PUBLIC) != 0) {
    // Public method/field
    extract(member);
}
```

**Go** (capitalization):
```go
if unicode.IsUpper(rune(name[0])) {
    // Exported (public) entity
    extract(entity)
}
```

**C/C++** (header-based):
```cpp
// Only extract from .h files (public interface)
// Skip static functions (internal linkage)
if (!isStatic(func)) {
    extract(func);
}
```

### 5.4 Metadata Capture

#### 4.4.1 Documentation Comments

**Requirements:**
- Extract docstrings, Javadoc, GoDoc, etc.
- Preserve formatting (markdown-friendly)
- Strip decorative characters

**Python Docstring**:
```python
def extract_docstring(func):
    if func.__doc__:
        return inspect.cleandoc(func.__doc__)  # Remove indentation
    return ""
```

**Java Javadoc** (ASM doesn't parse comments - requires source parsing):
```java
// Use JavaParser or similar tool
CompilationUnit cu = StaticJavaParser.parse(sourceFile);
MethodDeclaration method = cu.findFirst(MethodDeclaration.class, ...).get();
Optional<Javadoc> javadoc = method.getJavadoc();

if (javadoc.isPresent()) {
    String comment = javadoc.get().toText();
    methodInfo.setComment(comment);
}
```

**Go GoDoc**:
```go
// GoDoc is the comment block immediately preceding the declaration
if funcDecl.Comment != nil {
    comment := funcDecl.Comment.Text()
    funcDef.AppendComment(comment)
}
```

#### 4.4.2 Entity Paths

**Requirements:**
- Map logical names to runtime locations
- Support nested packages/modules
- Enable FFI resolution

**Common Entity Path Keys:**
- `"module"` - Module/package name
- `"package"` - Full package path
- `"metaffi_guest_lib"` - Generated guest library name

**Example** (Python):
```python
func_def.entity_path = {
    "module": module.__name__,
    "package": module.__package__ or "",
    "metaffi_guest_lib": f"{idl_source}_MetaFFIGuest"
}
```

**Example** (Java):
```java
methodInfo.entityPath.put("module", packageName);
methodInfo.entityPath.put("package", fullClassName);
methodInfo.entityPath.put("metaffi_guest_lib", guestLibName);
```

**Example** (Go):
```go
funcDef.SetEntityPath("module", pkg.Name)
funcDef.SetEntityPath("package", pkg.ImportPath)
funcDef.SetEntityPath("metaffi_guest_lib", guestLibName)
```

#### 4.4.3 Tags

**Requirements:**
- Capture language-specific metadata
- Enable compiler-specific behavior
- Support custom annotations

**Common Tags:**
- `"receiver_pointer": "true"` - Go pointer receivers
- `"static": "true"` - Static methods
- `"const": "true"` - Constant values
- `"deprecated": "<message>"` - Deprecation notices
- `"varargs": "true"` - Variadic parameters

**Example** (Go):
```go
// Detect pointer receiver
if method.Receiver != nil && method.Receiver.IsPointer {
    methodDef.SetTag("receiver_pointer", "true")
}

// Detect variadic
if method.IsVariadic {
    methodDef.SetTag("varargs", "true")
}
```

**Example** (Java):
```java
// Detect @Deprecated annotation
if (method.isAnnotationPresent(Deprecated.class)) {
    methodInfo.setTag("deprecated", "Use alternative method");
}

// Detect static
if (Modifier.isStatic(method.getModifiers())) {
    methodInfo.setTag("static", "true");
}
```

#### 4.4.4 External Resource Dependencies

**Requirements:**
- Track library dependencies
- Capture JAR files, shared libraries
- Use environment variables for portability

**Example** (Java):
```java
// JAR dependencies
if (jarPath != null) {
    module.addExternalResource(jarPath);
}

// Required runtime libraries
module.addExternalResource("$JAVA_HOME/lib/rt.jar");
```

**Example** (Python):
```python
# Detect import dependencies
for import_name in module.__dict__.get('__all__', []):
    if is_external_module(import_name):
        module_def.add_external_resource(f"$PYTHON_HOME/lib/{import_name}")
```

**Example** (Go):
```go
// Detect cgo dependencies
for _, comment := range file.Comments {
    if strings.HasPrefix(comment.Text(), "/*\n#cgo LDFLAGS:") {
        libs := extractLibraries(comment.Text())
        for _, lib := range libs {
            module.AddExternalResource(lib)
        }
    }
}
```

---

## 6. Existing Implementations Reference

### 6.1 Python311 IDL Compiler

**Location**: `lang-plugin-python311/idl/`

**Language**: Python + C++ wrapper

**Input**: Python source files (.py)

**Approach**: Runtime introspection using `inspect` module

**Key Files:**
- `py_extractor.py` - Core extraction logic
- `py_idl_generator.py` - JSON generation
- `python_idl_plugin.cpp` - C++ plugin wrapper

**Architecture:**

```
Python Source → importlib → Loaded Module → inspect → Extracted Info → JSON IDL
```

**Strengths:**
- Simple to understand
- Captures runtime type behavior
- Handles dynamic typing gracefully

**Weaknesses:**
- Requires Python runtime
- Must execute code (security risk)
- Type information loss without annotations

**Type Mapping** (`py_extractor.py`):
```python
def map_python_type_to_metaffi(python_type):
    if python_type == int:
        return "int32"
    elif python_type == float:
        return "float64"
    elif python_type == str:
        return "string8"
    elif python_type == bool:
        return "bool"
    elif python_type == None:
        return "null"
    else:
        return "handle"  # Fallback for complex types
```

**Extraction Flow:**
1. Load .py file as module with `importlib.util.spec_from_file_location`
2. Use `inspect.getmembers()` to extract functions, classes, globals
3. For each function: `inspect.signature()` → parameters + return type
4. For each class: Extract `__init__`, methods, fields
5. Generate getter/setter functions for globals
6. Convert to JSON with `py_idl_generator`

### 6.2 JVM IDL Compiler

**Location**: `lang-plugin-jvm/idl/`

**Language**: Java + C++ wrapper (JNI)

**Input**: Java bytecode (.class files) OR JAR archives (.jar)

**Approach**: Bytecode analysis using ASM library

**Key Files:**
- `JavaExtractor.java` - Factory/router
- `BytecodeExtractor.java` - .class bytecode analysis
- `JarExtractor.java` - .jar archive handling
- `jvm_idl_plugin.cpp` - C++ plugin wrapper via JNI

**Architecture:**

```
.class/.jar → ASM ClassReader → ClassVisitor → Extracted Info → JSON IDL
```

**Strengths:**
- Most robust (compiled code = complete type info)
- No source required
- Handles both .class and .jar
- Best separation of concerns (Factory pattern)

**Weaknesses:**
- Complex setup (JNI, JVM initialization)
- Large dependency (ASM, Gson libraries)
- Bytecode info loss (parameter names)

**Bytecode Parsing** (`BytecodeExtractor.java`):
```java
public class BytecodeExtractor {
    public JavaInfo extract(InputStream classStream) {
        ClassReader reader = new ClassReader(classStream);
        ClassExtractorVisitor visitor = new ClassExtractorVisitor();

        reader.accept(visitor, 0);

        return visitor.getJavaInfo();
    }
}

class ClassExtractorVisitor extends ClassVisitor {
    @Override
    public MethodVisitor visitMethod(int access, String name, String desc, ...) {
        // Extract method using ASM Type API
        Type methodType = Type.getMethodType(desc);

        Type[] paramTypes = methodType.getArgumentTypes();
        for (int i = 0; i < paramTypes.length; i++) {
            String paramName = "p" + i;  // Generated name
            MetaFFIType type = mapJavaTypeToMetaFFI(paramTypes[i]);
            method.addParameter(new ArgInfo(paramName, type));
        }

        Type returnType = methodType.getReturnType();
        method.setReturnType(mapJavaTypeToMetaFFI(returnType));

        return super.visitMethod(access, name, desc, ...);
    }
}
```

**Type Mapping**:
```java
private MetaFFIType mapJavaTypeToMetaFFI(Type type) {
    switch (type.getSort()) {
        case Type.BYTE:    return MetaFFIType.INT8;
        case Type.SHORT:   return MetaFFIType.INT16;
        case Type.INT:     return MetaFFIType.INT32;
        case Type.LONG:    return MetaFFIType.INT64;
        case Type.FLOAT:   return MetaFFIType.FLOAT32;
        case Type.DOUBLE:  return MetaFFIType.FLOAT64;
        case Type.BOOLEAN: return MetaFFIType.BOOL;
        case Type.CHAR:    return MetaFFIType.CHAR16;
        case Type.ARRAY:
            MetaFFIType elementType = mapJavaTypeToMetaFFI(type.getElementType());
            return elementType + "_array";
        case Type.OBJECT:
            if (type.getClassName().equals("java.lang.String")) {
                return MetaFFIType.STRING8;
            }
            return MetaFFIType.HANDLE;  // All objects as handles
        default:
            return MetaFFIType.HANDLE;
    }
}
```

**JAR Handling** (`JarExtractor.java`):
```java
public JavaInfo extractFromJar(String jarPath, String[] packageFilters) {
    JarFile jar = new JarFile(jarPath);
    JavaInfo combined = new JavaInfo();

    for (JarEntry entry : Collections.list(jar.entries())) {
        String name = entry.getName();

        // Skip inner classes and non-matching packages
        if (!name.endsWith(".class") || name.contains("$")) continue;
        if (!matchesPackageFilter(name, packageFilters)) continue;

        InputStream stream = jar.getInputStream(entry);
        JavaInfo classInfo = bytecodeExtractor.extract(stream);

        combined.merge(classInfo);
    }

    return combined;
}
```

### 6.3 Go IDL Compiler

**Location**: `lang-plugin-go/idl/`

**Language**: Pure Go (cgo for plugin interface)

**Input**: Go source files (.go)

**Approach**: AST parsing with custom go-parser

**Key Files:**
- `GoIDLCompiler.go` - Main compiler (dual-mode)
- `ClassesExtractor.go` - Struct/interface extraction
- `FunctionsExtractor.go` - Function extraction
- `GlobalsExtractor.go` - Package variable extraction
- `Extractors.go` - Type mapping

**Architecture:**

```
Go Source → go-parser → AST → Extractors → JSON IDL
```

**Strengths:**
- Pure source parsing (no runtime)
- Comprehensive type system support
- Good modular design
- Path resolution (GOPATH/GOROOT/modules)

**Weaknesses:**
- Global state complexity (classes/Imports maps)
- Requires careful extraction ordering
- Custom parser (less mature than standard)

**Dual Input Mode** (`GoIDLCompiler.go`):
```go
type GoIDLCompiler struct {
    Source       string    // Direct source code
    SourcePath   string    // File/directory/module path
    ModuleName   string    // Module import path
    TargetLang   string    // Target language
}

func (c *GoIDLCompiler) Compile() (*IDL.IDLDefinition, error) {
    if c.Source != "" {
        // Source mode: parse provided code
        return c.parseSource(c.Source)
    } else if c.SourcePath != "" {
        // Path mode: resolve and parse
        return c.parseFromPath(c.SourcePath)
    }
    return nil, errors.New("no source or path provided")
}
```

**Path Resolution**:
```go
func (c *GoIDLCompiler) parseFromPath(path string) (*IDL.IDLDefinition, error) {
    if strings.HasSuffix(path, ".go") {
        // Single file
        return c.parseFile(path)

    } else if isDirectory(path) {
        // Directory of .go files
        return c.parseDir(path)

    } else {
        // Module path (resolve from GOPATH/GOROOT)
        resolvedPath := c.resolveModulePath(path)
        return c.parseDir(resolvedPath)
    }
}

func (c *GoIDLCompiler) resolveModulePath(modulePath string) string {
    // Try GOPATH/pkg/mod first
    gopath := os.Getenv("GOPATH")
    modPath := filepath.Join(gopath, "pkg", "mod", modulePath)
    if exists(modPath) {
        return modPath
    }

    // Try GOROOT/src for stdlib
    goroot := os.Getenv("GOROOT")
    srcPath := filepath.Join(goroot, "src", modulePath)
    if exists(srcPath) {
        return srcPath
    }

    return ""
}
```

**Type Mapping** (`Extractors.go`):
```go
var primitiveTypes = map[string]IDL.MetaFFIType{
    "bool":       IDL.BOOL,
    "int":        IDL.INT64,  // Platform-dependent → use int64
    "int8":       IDL.INT8,
    "int16":      IDL.INT16,
    "int32":      IDL.INT32,
    "int64":      IDL.INT64,
    "uint":       IDL.UINT64,
    "uint8":      IDL.UINT8,
    "uint16":     IDL.UINT16,
    "uint32":     IDL.UINT32,
    "uint64":     IDL.UINT64,
    "float32":    IDL.FLOAT32,
    "float64":    IDL.FLOAT64,
    "string":     IDL.STRING8,
    "byte":       IDL.UINT8,  // Alias for uint8
    "rune":       IDL.CHAR32, // Alias for int32
}

func mapGoTypeToMetaFFI(goType string) (IDL.MetaFFIType, int) {
    // Check for array
    if strings.HasPrefix(goType, "[]") {
        elementType, _ := mapGoTypeToMetaFFI(strings.TrimPrefix(goType, "[]"))
        return elementType, 1  // 1D array
    }

    // Check for multi-dimensional array
    dimensions := strings.Count(goType, "[]")
    if dimensions > 0 {
        cleanType := strings.ReplaceAll(goType, "[]", "")
        elementType, _ := mapGoTypeToMetaFFI(cleanType)
        return elementType, dimensions
    }

    // Check primitive types
    if metaffiType, found := primitiveTypes[goType]; found {
        return metaffiType, 0
    }

    // Default to handle for custom types
    return IDL.HANDLE, 0
}
```

**Struct Extraction** (`ClassesExtractor.go`):
```go
func extractStruct(structDecl *goparser.StructDeclaration) *IDL.ClassDefinition {
    classDef := IDL.NewClassDefinition(structDecl.Name)

    // Extract fields
    for _, field := range structDecl.Fields {
        if unicode.IsUpper(rune(field.Name[0])) {  // Exported
            fieldType, dims := mapGoTypeToMetaFFI(field.Type)
            fieldDef := IDL.NewArgDefinition(field.Name, fieldType)
            fieldDef.Dimensions = dims

            // Generate getter/setter
            fieldDef.Getter = createFieldGetter(classDef, fieldDef)
            fieldDef.Setter = createFieldSetter(classDef, fieldDef)

            classDef.AddField(fieldDef)
        }
    }

    return classDef
}
```

### 6.4 Comparative Analysis

| Aspect | Python | Java | Go |
|--------|--------|------|-----|
| **Implementation Language** | Python | Java | Go |
| **Wrapper Language** | C++ | C++ (JNI) | Go (cgo) |
| **Input Type** | Source only | Bytecode + JAR | Source only |
| **Parsing Approach** | Runtime introspection | ASM bytecode | AST parsing |
| **Type Information** | Annotation-dependent | Complete (bytecode) | Complete (source) |
| **External Dependencies** | `inspect` (stdlib) | ASM, Gson | go-parser |
| **Complexity** | Low | High | Medium |
| **Robustness** | Medium (dynamic types) | High | High |
| **Performance** | Medium (runtime load) | Medium (JVM startup) | Fast |
| **Code Organization** | Monolithic | Modular (Factory) | Modular (global state) |

**Common Patterns:**
1. **Three-Part Pipeline**: Extract → Map Types → Generate JSON
2. **Public/Private Filtering**: All implementations filter non-public entities
3. **Type Aliasing**: All preserve original type names
4. **Getter/Setter Generation**: All generate accessors for fields/globals
5. **Overloading Detection**: Name-based grouping + sequential indexing

**Anti-Patterns to Avoid:**
1. **Python**: Tight coupling in single extractor class
2. **Java**: Overly complex JNI setup (could simplify)
3. **Go**: Global state management (classes map shared across extractors)

---

## 7. JSON Schema Reference

### 7.1 Schema Location

**File**: `sdk/idl_entities/go/IDL/schema.json`

**Schema Draft**: JSON Schema 2020-12

**Schema ID**: `https://metaffi.com/schemas/idl-definition.json`

### 7.2 How to Validate

**Using Python**:
```python
import json
import jsonschema

# Load schema
with open("sdk/idl_entities/go/IDL/schema.json") as f:
    schema = json.load(f)

# Load IDL JSON
with open("my_idl.json") as f:
    idl = json.load(f)

# Validate
try:
    jsonschema.validate(instance=idl, schema=schema)
    print("✓ Validation successful")
except jsonschema.ValidationError as e:
    print(f"✗ Validation failed: {e.message}")
    print(f"  Path: {e.json_path}")
```

**Using Go**:
```go
import (
    "encoding/json"
    "github.com/xeipuuv/gojsonschema"
)

func validateIDL(idlJSON string) error {
    schemaLoader := gojsonschema.NewReferenceLoader("file://./sdk/idl_entities/go/IDL/schema.json")
    documentLoader := gojsonschema.NewStringLoader(idlJSON)

    result, err := gojsonschema.Validate(schemaLoader, documentLoader)
    if err != nil {
        return err
    }

    if !result.Valid() {
        for _, desc := range result.Errors() {
            fmt.Printf("- %s\n", desc)
        }
        return errors.New("validation failed")
    }

    return nil
}
```

### 7.3 Schema Definitions

**Root Schema** (`#/$defs/`):

| Definition | Description |
|------------|-------------|
| `metaffi_type` | Enum of all MetaFFI type strings |
| `tags` | Key-value metadata map |
| `entity_path` | Runtime routing map |
| `arg_definition` | Parameter/return value/field |
| `function_definition` | Function/method base |
| `method_definition` | Class method (extends function) |
| `constructor_definition` | Class constructor |
| `release_definition` | Class destructor |
| `field_definition` | Class field (extends arg) |
| `global_definition` | Global variable (extends arg) |
| `class_definition` | Class/struct |
| `module` | Module containing functions/classes |

**Schema Inheritance** (using `allOf`):

```json
{
  "method_definition": {
    "allOf": [
      {"$ref": "#/$defs/function_definition"},
      {
        "properties": {
          "instance_required": {"type": "boolean"}
        }
      }
    ]
  }
}
```

### 7.4 Schema Evolution Guidelines

**When Adding New Fields:**

1. Add to Go struct with JSON tag:
   ```go
   type ArgDefinition struct {
       // ... existing fields
       NewField string `json:"new_field"`
   }
   ```

2. Update schema.json:
   ```json
   {
     "arg_definition": {
       "properties": {
         "new_field": {
           "type": "string",
           "description": "New field description"
         }
       }
     }
   }
   ```

3. Update `required` array if field is mandatory

4. Update all existing IDL compilers to populate new field

**When Removing Fields:**

1. Mark as deprecated in schema (don't remove):
   ```json
   {
     "old_field": {
       "type": "string",
       "deprecated": true,
       "description": "[DEPRECATED] Use new_field instead"
     }
   }
   ```

2. Update Go struct (keep for backwards compat):
   ```go
   type ArgDefinition struct {
       OldField string `json:"old_field,omitempty"`  // omitempty for optional
   }
   ```

3. After 2 major versions, remove from schema and struct

**Version Control:**

- Update schema `$id` with version: `https://metaffi.com/schemas/idl-definition.v2.json`
- Maintain backwards compatibility for at least 2 major versions
- Document breaking changes in CHANGELOG.md

---

## Appendix A: Glossary

| Term | Definition |
|------|------------|
| **IDL** | Interface Definition Language - describes function/class interfaces |
| **IDL Compiler** | Tool that extracts IDL from source/compiled code |
| **IDL Extractor** | Synonym for IDL Compiler |
| **MetaFFI Type** | Universal type system type (int32, float64, handle, etc.) |
| **Entity Path** | Runtime routing information (module, package, metaffi_guest_lib) |
| **Type Alias** | Language-specific type name preserved in IDL |
| **Overload Index** | Sequential number for overloaded functions (1, 2, 3...) |
| **Handle** | Opaque reference to language-specific object |
| **XLLR** | Cross-Language Low-Level Runtime - unified memory allocator |
| **Guest Library** | Foreign code being wrapped by MetaFFI |
| **Host Library** | Calling code using MetaFFI to access guest |
| **AST** | Abstract Syntax Tree - parsed code structure |
| **Bytecode** | Compiled intermediate representation (Java .class) |

---

## Appendix B: Quick Reference

### Command-Line Usage

```bash
# Extract IDL from Python source
metaffi --idl mymodule.py --output ./generated

# Extract IDL from Java JAR
metaffi --idl mylib.jar --idl-plugin jar --output ./generated

# Specify plugin explicitly
metaffi --idl myfile.ext --idl-plugin ext --output ./generated
```

### Environment Variables

| Variable | Purpose |
|----------|---------|
| `METAFFI_HOME` | Root directory for MetaFFI installation |
| `GOPATH` | Go module cache (for Go IDL compiler) |
| `GOROOT` | Go standard library (for Go IDL compiler) |
| `JAVA_HOME` | JDK location (for Java IDL compiler) |

### File Locations

| Path | Description |
|------|-------------|
| `sdk/idl_compiler/` | This documentation |
| `sdk/idl_entities/go/IDL/` | IDL struct definitions (Go) |
| `sdk/idl_entities/go/IDL/schema.json` | JSON schema |
| `lang-plugin-*/idl/` | Existing IDL compiler implementations |
| `metaffi-core/CLI/idl_extractor.cpp` | C++ plugin loader |

---

## Appendix C: Example IDL JSON

**Complete IDL Example**:

```json
{
  "idl_source": "example",
  "idl_extension": ".json",
  "idl_filename_with_extension": "example.json",
  "idl_full_path": "/path/to/example.json",
  "metaffi_guest_lib": "example_MetaFFIGuest",
  "target_language": "python",
  "modules": [
    {
      "name": "example",
      "comment": "Example module demonstrating MetaFFI IDL",
      "tags": {
        "version": "1.0"
      },
      "functions": [
        {
          "name": "add",
          "comment": "Adds two integers",
          "tags": {},
          "entity_path": {
            "module": "example",
            "package": "example"
          },
          "parameters": [
            {
              "name": "a",
              "type": "int32",
              "type_alias": "int",
              "comment": "First operand",
              "tags": {},
              "dimensions": 0,
              "is_optional": false
            },
            {
              "name": "b",
              "type": "int32",
              "type_alias": "int",
              "comment": "Second operand",
              "tags": {},
              "dimensions": 0,
              "is_optional": false
            }
          ],
          "return_values": [
            {
              "name": "result",
              "type": "int32",
              "type_alias": "int",
              "comment": "Sum of a and b",
              "tags": {},
              "dimensions": 0,
              "is_optional": false
            }
          ],
          "overload_index": 0
        }
      ],
      "classes": [
        {
          "name": "Calculator",
          "comment": "Simple calculator class",
          "tags": {},
          "entity_path": {
            "module": "example",
            "package": "example"
          },
          "constructors": [
            {
              "name": "Calculator",
              "comment": "Creates a new calculator",
              "tags": {},
              "entity_path": {
                "module": "example",
                "package": "example"
              },
              "parameters": [],
              "return_values": [
                {
                  "name": "new_instance",
                  "type": "handle",
                  "type_alias": "Calculator",
                  "comment": "New Calculator instance",
                  "tags": {},
                  "dimensions": 0,
                  "is_optional": false
                }
              ],
              "overload_index": 0
            }
          ],
          "release": {
            "name": "ReleaseCalculator",
            "comment": "Releases calculator resources",
            "tags": {},
            "entity_path": {
              "module": "example",
              "package": "example"
            },
            "parameters": [
              {
                "name": "this_instance",
                "type": "handle",
                "type_alias": "Calculator",
                "comment": "Calculator instance",
                "tags": {},
                "dimensions": 0,
                "is_optional": false
              }
            ],
            "return_values": [],
            "overload_index": 0,
            "instance_required": true
          },
          "methods": [
            {
              "name": "multiply",
              "comment": "Multiplies two numbers",
              "tags": {},
              "entity_path": {
                "module": "example",
                "package": "example"
              },
              "parameters": [
                {
                  "name": "this_instance",
                  "type": "handle",
                  "type_alias": "Calculator",
                  "comment": "",
                  "tags": {},
                  "dimensions": 0,
                  "is_optional": false
                },
                {
                  "name": "a",
                  "type": "float64",
                  "type_alias": "float",
                  "comment": "First operand",
                  "tags": {},
                  "dimensions": 0,
                  "is_optional": false
                },
                {
                  "name": "b",
                  "type": "float64",
                  "type_alias": "float",
                  "comment": "Second operand",
                  "tags": {},
                  "dimensions": 0,
                  "is_optional": false
                }
              ],
              "return_values": [
                {
                  "name": "result",
                  "type": "float64",
                  "type_alias": "float",
                  "comment": "Product of a and b",
                  "tags": {},
                  "dimensions": 0,
                  "is_optional": false
                }
              ],
              "overload_index": 0,
              "instance_required": true
            }
          ],
          "fields": []
        }
      ],
      "globals": [
        {
          "name": "PI",
          "type": "float64",
          "type_alias": "float",
          "comment": "Mathematical constant pi",
          "tags": {
            "const": "true"
          },
          "dimensions": 0,
          "is_optional": false,
          "getter": {
            "name": "get_PI",
            "comment": "Returns the value of PI",
            "tags": {},
            "entity_path": {
              "module": "example",
              "package": "example"
            },
            "parameters": [],
            "return_values": [
              {
                "name": "PI",
                "type": "float64",
                "type_alias": "float",
                "comment": "",
                "tags": {},
                "dimensions": 0,
                "is_optional": false
              }
            ],
            "overload_index": 0
          },
          "setter": null
        }
      ],
      "external_resources": []
    }
  ]
}
```

---

**End of Documentation**

For questions or clarifications, refer to:
- **Source Code**: `sdk/idl_entities/go/IDL/*.go`
- **Existing Implementations**: `lang-plugin-*/idl/`
- **MetaFFI Project**: https://github.com/MetaFFI
