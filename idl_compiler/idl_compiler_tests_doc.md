# IDL Compiler Test Documentation

This document details all tests that must be implemented for IDL compiler implementations across all languages. These tests ensure consistent behavior, full coverage, and correctness of interface extraction and IDL generation.

---

## Table of Contents

1. [Overview](#overview)
2. [Test Categories](#test-categories)
3. [Type Mapping Tests](#1-type-mapping-tests)
4. [Entity Path Generation Tests](#2-entity-path-generation-tests)
5. [Function Extraction Tests](#3-function-extraction-tests)
6. [Class Extraction Tests](#4-class-extraction-tests)
7. [Global Variable Extraction Tests](#5-global-variable-extraction-tests)
8. [Edge Case Tests](#6-edge-case-tests)
9. [Module/Package Loading Tests](#7-modulepackage-loading-tests)
10. [Schema Validation Tests](#8-schema-validation-tests)
11. [Integration Tests](#9-integration-tests)
12. [Error Handling Tests](#10-error-handling-tests)
13. [Implementation Notes](#implementation-notes)
14. [Test Coverage Checklist](#test-coverage-checklist)

---

## Overview

### Purpose

IDL compilers extract **interface definitions** (not implementations) from source code or compiled artifacts and generate MetaFFI IDL JSON. Tests verify:
- Correct type mapping to MetaFFI types
- Proper entity_path generation (language-specific routing)
- Complete extraction of functions, classes, methods, fields, globals
- Valid JSON output conforming to schema.json
- Handling of language-specific features (varargs, properties, static methods, etc.)

### Scope

Tests apply to all IDL compiler implementations:
- **Python3** (runtime introspection)
- **Go** (AST parsing)
- **JVM** (bytecode reflection)
- **Future languages** (C/C++, TypeScript, etc.)

### Test Philosophy

1. **Comprehensive Coverage**: Test all type mappings, entity types, and edge cases
2. **Schema Compliance**: All output must validate against `sdk/idl_entities/idl.schema.json`
3. **Language-Specific**: entity_path and features vary by language
4. **Regression Prevention**: Catch bugs early with exact verification criteria

---

## Test Categories

### Category Summary

| Category | Purpose | Priority | Test Count |
|----------|---------|----------|------------|
| Type Mapping | Verify correct MetaFFI type assignment | Critical | ~15 |
| Entity Path | Verify routing information correctness | Critical | ~10 |
| Function Extraction | Verify function signature extraction | Critical | ~8 |
| Class Extraction | Verify class structure extraction | High | ~12 |
| Global Extraction | Verify global variable extraction | High | ~4 |
| Edge Cases | Verify special language features | High | ~10 |
| Loading | Verify file/module/package loading | Medium | ~6 |
| Schema Validation | Verify JSON schema compliance | Critical | ~5 |
| Integration | Verify end-to-end workflows | High | ~4 |
| Error Handling | Verify graceful error handling | Medium | ~6 |
| **Total** | | | **~80 tests** |

---

## 1. Type Mapping Tests

### Purpose
Verify that language-specific types are correctly mapped to MetaFFI types.

### Critical Requirements
- **MUST use lowercase**: `int32`, NOT `INT32`
- **MUST preserve type_alias**: Original type string
- **MUST handle unknown types**: Fallback to `handle`

---

### 1.1 Map Primitive Integer Types

**Purpose**: Test signed integer type mapping

**Test Cases**:
```python
# Python
int → int32

# Go
int8 → int8
int16 → int16
int32 → int32
int64 → int64
int → int64  # platform-dependent, but use int64

# Java
byte → int8
short → int16
int → int32
long → int64
```

**Verification**:
- type = lowercase MetaFFI type
- type_alias = original language type
- dimensions = 0

---

### 1.2 Map Primitive Unsigned Integer Types

**Purpose**: Test unsigned integer type mapping (languages that support them)

**Test Cases**:
```python
# Go
uint8 → uint8
uint16 → uint16
uint32 → uint32
uint64 → uint64
uint → uint64
byte → uint8

# Python (no native unsigned types)
# If type hint specifies ctypes.c_uint32 → uint32
```

**Verification**:
- type = lowercase unsigned type
- type_alias preserved

---

### 1.3 Map Floating Point Types

**Purpose**: Test floating point type mapping

**Test Cases**:
```python
# Python
float → float64

# Go
float32 → float32
float64 → float64

# Java
float → float32
double → float64
```

**Verification**:
- type = `float32` or `float64`
- type_alias preserved

---

### 1.4 Map Boolean Type

**Purpose**: Test boolean type mapping

**Test Cases**:
```python
# Python
bool → bool

# Go
bool → bool

# Java
boolean → bool
```

**Verification**:
- type = `bool`
- type_alias = original

---

### 1.5 Map String Types

**Purpose**: Test string type mapping

**Test Cases**:
```python
# Python
str → string8 (UTF-8)

# Go
string → string8 (UTF-8)

# Java
String → string16 (UTF-16 native)
# If annotation specifies UTF-8 → string8
```

**Verification**:
- type = `string8`, `string16`, or `string32` based on language default
- type_alias = "string", "str", etc.

---

### 1.6 Map Array/Collection Types (1D)

**Purpose**: Test one-dimensional array mapping

**Test Cases**:
```python
# Python
List[int] → type=handle, type_alias="List[int]", dimensions=0
list → type=handle_array, dimensions=1
# Note: Generic types fallback to handle

# Go
[]int32 → type=int32, dimensions=1
[]string → type=string8, dimensions=1

# Java
int[] → type=int32, dimensions=1
String[] → type=string16, dimensions=1
```

**Verification**:
- Simple arrays: type = element type, dimensions = 1
- Generic collections: type = handle or handle_array

---

### 1.7 Map Multi-Dimensional Arrays

**Purpose**: Test nested array mapping

**Test Cases**:
```python
# Go
[][]int32 → type=int32, dimensions=2
[][][]float64 → type=float64, dimensions=3

# Java
int[][] → type=int32, dimensions=2
String[][][] → type=string16, dimensions=3
```

**Verification**:
- dimensions = nesting depth
- type = innermost element type

---

### 1.8 Map Complex/Generic Types

**Purpose**: Test complex type handling

**Test Cases**:
```python
# Python
Dict[str, int] → type=handle, type_alias="Dict[str, int]"
Tuple[int, str, bool] → type=handle, type_alias="Tuple[int, str, bool]"
Union[int, str] → type=any, type_alias="Union[int, str]"
Optional[int] → type=any, type_alias="Optional[int]"
Any → type=any, type_alias="Any"

# Go
map[string]int → type=handle, type_alias="map[string]int"
interface{} → type=any, type_alias="interface{}"

# Java
Map<String, Integer> → type=handle, type_alias="Map<String, Integer>"
Object → type=any, type_alias="Object"
```

**Verification**:
- Generic types → `handle`
- Union/Optional/Any → `any`
- type_alias preserves full generic signature

---

### 1.9 Map Special Types

**Purpose**: Test special/language-specific types

**Test Cases**:
```python
# Python
None/NoneType → null
bytes → uint8_array (dimensions=1)

# Go
nil → null
rune → int32
byte → uint8

# Java
void → null (for return types)
null → null
```

**Verification**:
- null type for void/None/nil
- Special types map to appropriate primitives

---

### 1.10 Map Unknown Types

**Purpose**: Test fallback for unrecognized types

**Test Cases**:
```python
# Any unrecognized custom class
CustomClass → type=handle, type_alias="CustomClass"
UnknownType → type=handle, type_alias="UnknownType"
```

**Verification**:
- type = `handle`
- type_alias = original type name
- NO exceptions thrown

---

## 2. Entity Path Generation Tests

### Purpose
Verify correct entity_path generation for runtime routing. Entity paths are **language-specific** and must match runtime plugin expectations.

### Critical Requirements
- **Read from sdk/idl_entities/entity_path_specs.json** for structure
- **Language-specific keys**: Python uses `callable`/`attribute`, Go uses different keys
- **Include routing flags**: varargs, named_args, instance_required, getter, setter

---

### 2.1 Generate Function Entity Path (Simple)

**Purpose**: Test basic function entity_path

**Test Input**:
```python
# Python
def add(a: int, b: int) -> int:
    return a + b
```

**Expected Output**:
```json
{
  "entity_path": {
    "callable": "add"
  }
}
```

**Verification**:
- `callable` key present
- Value = function name
- No extra keys for simple function

---

### 2.2 Generate Function Entity Path (Varargs)

**Purpose**: Test varargs detection

**Test Input**:
```python
# Python
def varargs_func(*args):
    pass
```

**Expected Output**:
```json
{
  "entity_path": {
    "callable": "varargs_func",
    "varargs": true
  }
}
```

**Verification**:
- `varargs` flag set to true
- Only set when *args present

---

### 2.3 Generate Function Entity Path (Named Args)

**Purpose**: Test kwargs detection

**Test Input**:
```python
# Python
def kwargs_func(**kwargs):
    pass
```

**Expected Output**:
```json
{
  "entity_path": {
    "callable": "kwargs_func",
    "named_args": true
  }
}
```

**Verification**:
- `named_args` flag set to true
- Only set when **kwargs present

---

### 2.4 Generate Function Entity Path (Both)

**Purpose**: Test both varargs and kwargs

**Test Input**:
```python
# Python
def both(a: int, *args, **kwargs):
    pass
```

**Expected Output**:
```json
{
  "entity_path": {
    "callable": "both",
    "varargs": true,
    "named_args": true
  }
}
```

**Verification**:
- Both flags present
- Regular params excluded from entity_path

---

### 2.5 Generate Method Entity Path (Instance)

**Purpose**: Test instance method entity_path

**Test Input**:
```python
# Python
class MyClass:
    def instance_method(self, x: int) -> int:
        return x
```

**Expected Output**:
```json
{
  "entity_path": {
    "callable": "MyClass.instance_method",
    "instance_required": true
  }
}
```

**Verification**:
- Dotted notation: ClassName.method_name
- `instance_required` = true
- `self` parameter NOT in parameters list

---

### 2.6 Generate Method Entity Path (Static)

**Purpose**: Test static method entity_path

**Test Input**:
```python
# Python
class MyClass:
    @staticmethod
    def static_method(x: int) -> int:
        return x
```

**Expected Output**:
```json
{
  "entity_path": {
    "callable": "MyClass.static_method",
    "instance_required": false
  }
}
```

**Verification**:
- `instance_required` = false
- No `self` parameter

---

### 2.7 Generate Constructor Entity Path

**Purpose**: Test constructor entity_path

**Test Input**:
```python
# Python
class MyClass:
    def __init__(self, value: int):
        self.value = value
```

**Expected Output**:
```json
{
  "entity_path": {
    "callable": "MyClass.__init__"
  }
}
```

**Verification**:
- Dotted notation with `__init__`
- `self` NOT in parameters

---

### 2.8 Generate Global Getter Entity Path

**Purpose**: Test global variable getter entity_path

**Test Input**:
```python
# Python
MY_CONSTANT = 42
```

**Expected Output (Getter)**:
```json
{
  "entity_path": {
    "attribute": "MY_CONSTANT",
    "getter": true
  }
}
```

**Verification**:
- Uses `attribute` key (not `callable`)
- `getter` flag = true

---

### 2.9 Generate Global Setter Entity Path

**Purpose**: Test global variable setter entity_path

**Test Input**: Same as 2.8

**Expected Output (Setter)**:
```json
{
  "entity_path": {
    "attribute": "MY_CONSTANT",
    "setter": true
  }
}
```

**Verification**:
- Uses `attribute` key
- `setter` flag = true
- Separate from getter

---

### 2.10 Generate Field Entity Path

**Purpose**: Test class field entity_path

**Test Input**:
```python
# Python
class MyClass:
    my_field: int
```

**Expected Output (Getter)**:
```json
{
  "entity_path": {
    "attribute": "MyClass.my_field",
    "instance_required": true,
    "getter": true
  }
}
```

**Verification**:
- Dotted notation for field
- `instance_required` for instance fields
- Both getter and setter generated

---

## 3. Function Extraction Tests

### Purpose
Verify correct extraction of function signatures, parameters, and return types.

---

### 3.1 Extract Simple Function

**Purpose**: Test basic function extraction

**Test Input**:
```python
# Python
def simple_func(x: int, y: str) -> bool:
    """A simple function"""
    return True
```

**Expected Output**:
```json
{
  "name": "simple_func",
  "comment": "A simple function",
  "parameters": [
    {"name": "x", "type": "int32", "type_alias": "int"},
    {"name": "y", "type": "string8", "type_alias": "str"}
  ],
  "return_values": [
    {"name": "ret_0", "type": "bool", "type_alias": "bool"}
  ],
  "overload_index": 0
}
```

**Verification**:
- name extracted
- comment from docstring
- All parameters present with correct types
- Return value extracted
- overload_index defaults to 0

---

### 3.2 Extract Function with No Parameters

**Purpose**: Test zero-parameter function

**Test Input**:
```python
# Python
def no_params() -> None:
    pass
```

**Expected Output**:
```json
{
  "name": "no_params",
  "parameters": [],
  "return_values": [
    {"name": "ret_0", "type": "null", "type_alias": "None"}
  ]
}
```

**Verification**:
- Empty parameters array
- null return type for None

---

### 3.3 Extract Function with Optional Parameters

**Purpose**: Test default value parameters

**Test Input**:
```python
# Python
def with_defaults(x: int = 10, y: str = "hello") -> None:
    pass
```

**Expected Output**:
```json
{
  "parameters": [
    {"name": "x", "type": "int32", "is_optional": true},
    {"name": "y", "type": "string8", "is_optional": true}
  ]
}
```

**Verification**:
- `is_optional` = true for params with defaults
- Params without defaults: `is_optional` absent or false

---

### 3.4 Extract Function with No Type Hints

**Purpose**: Test function without annotations

**Test Input**:
```python
# Python
def no_hints(x, y):
    return x + y
```

**Expected Output**:
```json
{
  "parameters": [
    {"name": "x", "type": "any", "type_alias": ""},
    {"name": "y", "type": "any", "type_alias": ""}
  ],
  "return_values": [
    {"name": "ret_0", "type": "any", "type_alias": ""}
  ]
}
```

**Verification**:
- type = `any` when no annotation
- type_alias empty or "Any"

---

### 3.5 Extract Function with Multiple Return Values

**Purpose**: Test tuple return type

**Test Input**:
```python
# Python
def multi_return(x: int) -> tuple[int, str]:
    return (x, str(x))
```

**Expected Output**:
```json
{
  "return_values": [
    {"name": "ret_0", "type": "handle", "type_alias": "tuple[int, str]"}
  ]
}
```

**Verification**:
- Tuple mapped to handle
- type_alias preserves tuple signature
- Single return value (tuples are single objects in Python)

**Note**: Some languages (Go) support true multiple returns - those would be separate return_values

---

### 3.6 Extract Function with Varargs

**Purpose**: Test varargs extraction

**Test Input**:
```python
# Python
def varargs(a: int, *args) -> None:
    pass
```

**Expected Output**:
```json
{
  "parameters": [
    {"name": "a", "type": "int32"}
  ],
  "entity_path": {
    "callable": "varargs",
    "varargs": true
  }
}
```

**Verification**:
- *args NOT in parameters list
- varargs flag in entity_path

---

### 3.7 Extract Function with Kwargs

**Purpose**: Test kwargs extraction

**Test Input**:
```python
# Python
def kwargs(a: int, **kwargs) -> None:
    pass
```

**Expected Output**:
```json
{
  "parameters": [
    {"name": "a", "type": "int32"}
  ],
  "entity_path": {
    "callable": "kwargs",
    "named_args": true
  }
}
```

**Verification**:
- **kwargs NOT in parameters list
- named_args flag in entity_path

---

### 3.8 Extract Private Function (Filtered)

**Purpose**: Test private function filtering

**Test Input**:
```python
# Python
def _private_func():
    pass
```

**Expected Output**:
- Function NOT included in IDL output

**Verification**:
- Functions starting with `_` are private
- Should be filtered out (not exported)

---

## 4. Class Extraction Tests

### Purpose
Verify correct extraction of classes including constructors, methods, fields, and release (destructor).

---

### 4.1 Extract Simple Class

**Purpose**: Test basic class extraction

**Test Input**:
```python
# Python
class SimpleClass:
    """A simple class"""
    pass
```

**Expected Output**:
```json
{
  "name": "SimpleClass",
  "comment": "A simple class",
  "constructors": [
    {
      "name": "__init__",
      "parameters": [],
      "return_values": [
        {"name": "ret_0", "type": "handle", "type_alias": "SimpleClass"}
      ]
    }
  ],
  "release": null,
  "methods": [],
  "fields": []
}
```

**Verification**:
- Default constructor auto-created
- release = null (no __del__)
- Empty methods/fields

---

### 4.2 Extract Class with Constructor

**Purpose**: Test explicit constructor extraction

**Test Input**:
```python
# Python
class MyClass:
    def __init__(self, value: int):
        self.value = value
```

**Expected Output**:
```json
{
  "constructors": [
    {
      "name": "__init__",
      "parameters": [
        {"name": "value", "type": "int32"}
      ],
      "return_values": [
        {"name": "ret_0", "type": "handle", "type_alias": "MyClass"}
      ]
    }
  ]
}
```

**Verification**:
- Constructor parameters extracted (no `self`)
- Return type = handle with class name alias

---

### 4.3 Extract Class with Multiple Constructors (Overloading)

**Purpose**: Test overloaded constructors

**Test Input**:
```python
# Python
class MyClass:
    def __init__(self, value: int):
        self.value = value

    @classmethod
    def from_string(cls, s: str):
        return cls(int(s))
```

**Expected Output**:
```json
{
  "constructors": [
    {
      "name": "__init__",
      "parameters": [{"name": "value", "type": "int32"}],
      "overload_index": 0
    },
    {
      "name": "from_string",
      "parameters": [{"name": "s", "type": "string8"}],
      "overload_index": 1
    }
  ]
}
```

**Verification**:
- Multiple constructors supported
- overload_index increments
- Classmethods treated as alternative constructors

---

### 4.4 Extract Class with Destructor

**Purpose**: Test destructor detection

**Test Input**:
```python
# Python
class MyClass:
    def __del__(self):
        pass
```

**Expected Output**:
```json
{
  "release": {
    "name": "release",
    "comment": "Release MyClass instance",
    "parameters": [],
    "return_values": [],
    "instance_required": true
  }
}
```

**Verification**:
- release NOT null when __del__ present
- name = "release" (standardized)
- instance_required = true

---

### 4.5 Extract Class with Instance Methods

**Purpose**: Test instance method extraction

**Test Input**:
```python
# Python
class MyClass:
    def instance_method(self, x: int) -> str:
        return str(x)
```

**Expected Output**:
```json
{
  "methods": [
    {
      "name": "instance_method",
      "parameters": [
        {"name": "x", "type": "int32"}
      ],
      "return_values": [
        {"name": "ret_0", "type": "string8"}
      ],
      "instance_required": true
    }
  ]
}
```

**Verification**:
- `self` NOT in parameters
- instance_required = true
- Method signature complete

---

### 4.6 Extract Class with Static Methods

**Purpose**: Test static method extraction

**Test Input**:
```python
# Python
class MyClass:
    @staticmethod
    def static_method(x: int) -> int:
        return x * 2
```

**Expected Output**:
```json
{
  "methods": [
    {
      "name": "static_method",
      "parameters": [{"name": "x", "type": "int32"}],
      "return_values": [{"name": "ret_0", "type": "int32"}],
      "instance_required": false
    }
  ]
}
```

**Verification**:
- instance_required = false
- No `self` or `cls` parameter

---

### 4.7 Extract Class with Class Methods

**Purpose**: Test classmethod extraction

**Test Input**:
```python
# Python
class MyClass:
    @classmethod
    def class_method(cls, x: int):
        return cls()
```

**Expected Output**:
```json
{
  "methods": [
    {
      "name": "class_method",
      "parameters": [{"name": "x", "type": "int32"}],
      "return_values": [{"name": "ret_0", "type": "handle", "type_alias": "MyClass"}],
      "instance_required": false
    }
  ]
}
```

**Verification**:
- instance_required = false
- `cls` NOT in parameters
- Return type = class handle

---

### 4.8 Extract Class with Fields

**Purpose**: Test field extraction

**Test Input**:
```python
# Python
class MyClass:
    my_field: int
    name: str
```

**Expected Output**:
```json
{
  "fields": [
    {
      "name": "my_field",
      "type": "int32",
      "getter": {
        "name": "get_my_field",
        "entity_path": {
          "attribute": "MyClass.my_field",
          "instance_required": true,
          "getter": true
        }
      },
      "setter": {
        "name": "set_my_field",
        "entity_path": {
          "attribute": "MyClass.my_field",
          "instance_required": true,
          "setter": true
        }
      }
    }
  ]
}
```

**Verification**:
- Each field has getter AND setter
- entity_path uses `attribute` key
- instance_required for instance fields

---

### 4.9 Extract Class with Properties (Read-Only)

**Purpose**: Test property extraction with getter only

**Test Input**:
```python
# Python
class MyClass:
    @property
    def readonly(self) -> int:
        return 42
```

**Expected Output**:
```json
{
  "fields": [
    {
      "name": "readonly",
      "type": "int32",
      "getter": {
        "name": "get_readonly",
        "return_values": [{"name": "ret_0", "type": "int32"}]
      },
      "setter": null
    }
  ]
}
```

**Verification**:
- getter present
- setter = null (read-only)

---

### 4.10 Extract Class with Properties (Read-Write)

**Purpose**: Test property with both getter and setter

**Test Input**:
```python
# Python
class MyClass:
    @property
    def value(self) -> int:
        return self._value

    @value.setter
    def value(self, v: int):
        self._value = v
```

**Expected Output**:
```json
{
  "fields": [
    {
      "name": "value",
      "type": "int32",
      "getter": { /* ... */ },
      "setter": { /* ... */ }
    }
  ]
}
```

**Verification**:
- Both getter and setter present
- Setter has value parameter

---

### 4.11 Extract Private Members (Filtered)

**Purpose**: Test private member filtering

**Test Input**:
```python
# Python
class MyClass:
    def _private_method(self):
        pass

    def public_method(self):
        pass
```

**Expected Output**:
```json
{
  "methods": [
    {
      "name": "public_method"
    }
  ]
}
```

**Verification**:
- _private_method NOT included
- public_method included
- Names starting with `_` filtered

---

### 4.12 Extract Nested Classes (If Supported)

**Purpose**: Test nested class extraction

**Test Input**:
```python
# Python
class OuterClass:
    class InnerClass:
        pass
```

**Expected Output**:
- Two separate classes OR
- Qualified name: OuterClass.InnerClass

**Verification**:
- Language-specific handling
- Document expected behavior

---

## 5. Global Variable Extraction Tests

### Purpose
Verify correct extraction of module-level global variables as getter/setter function pairs.

---

### 5.1 Extract Simple Global

**Purpose**: Test basic global extraction

**Test Input**:
```python
# Python
MY_CONSTANT = 42
```

**Expected Output**:
```json
{
  "functions": [
    {
      "name": "GetMY_CONSTANT",
      "entity_path": {
        "attribute": "MY_CONSTANT",
        "getter": true
      },
      "parameters": [],
      "return_values": [
        {"name": "ret_0", "type": "int32"}
      ]
    },
    {
      "name": "SetMY_CONSTANT",
      "entity_path": {
        "attribute": "MY_CONSTANT",
        "setter": true
      },
      "parameters": [
        {"name": "value", "type": "int32"}
      ],
      "return_values": []
    }
  ]
}
```

**Verification**:
- Two functions generated (getter + setter)
- Getter has no params, returns value
- Setter has value param, no return
- entity_path uses `attribute`

---

### 5.2 Extract Typed Global

**Purpose**: Test global with type annotation

**Test Input**:
```python
# Python
MY_VALUE: int = 100
```

**Expected Output**:
```json
{
  "functions": [
    {
      "name": "GetMY_VALUE",
      "return_values": [
        {"name": "ret_0", "type": "int32", "type_alias": "int"}
      ]
    },
    {
      "name": "SetMY_VALUE",
      "parameters": [
        {"name": "value", "type": "int32", "type_alias": "int"}
      ]
    }
  ]
}
```

**Verification**:
- Type correctly mapped from annotation
- type_alias preserved

---

### 5.3 Extract Untyped Global

**Purpose**: Test global without type annotation

**Test Input**:
```python
# Python
UNTYPED = "hello"
```

**Expected Output**:
```json
{
  "functions": [
    {
      "name": "GetUNTYPED",
      "return_values": [
        {"name": "ret_0", "type": "any"}
      ]
    }
  ]
}
```

**Verification**:
- type = `any` when no annotation
- Infer from value if possible, else `any`

---

### 5.4 Filter Private Globals

**Purpose**: Test private global filtering

**Test Input**:
```python
# Python
_PRIVATE = 42
PUBLIC = 100
```

**Expected Output**:
- Only GetPUBLIC/SetPUBLIC included
- _PRIVATE excluded

**Verification**:
- Globals starting with `_` filtered

---

## 6. Edge Case Tests

### Purpose
Verify handling of special language features and corner cases.

---

### 6.1 Test Property Getter Only

**Test Input**: Already covered in 4.9

---

### 6.2 Test Property Setter Only

**Purpose**: Test write-only property (rare)

**Test Input**:
```python
# Python
class MyClass:
    @property
    def write_only(self) -> None:
        raise AttributeError("write-only")

    @write_only.setter
    def write_only(self, value: int):
        self._value = value
```

**Expected Output**:
```json
{
  "fields": [
    {
      "name": "write_only",
      "getter": null,
      "setter": { /* ... */ }
    }
  ]
}
```

**Verification**:
- getter = null
- setter present

---

### 6.3 Test Overloaded Methods (Same Name)

**Purpose**: Test method overloading by signature

**Test Input**:
```python
# Languages with overloading (Java, C++)
class MyClass {
    void process(int x) {}
    void process(String s) {}
}
```

**Expected Output**:
```json
{
  "methods": [
    {
      "name": "process",
      "parameters": [{"type": "int32"}],
      "overload_index": 0
    },
    {
      "name": "process",
      "parameters": [{"type": "string16"}],
      "overload_index": 1
    }
  ]
}
```

**Verification**:
- Same name, different overload_index
- Parameter types differentiate

---

### 6.4 Test Empty Module

**Purpose**: Test module with no exports

**Test Input**:
```python
# Python - empty file or only imports
import sys
```

**Expected Output**:
```json
{
  "modules": [
    {
      "name": "empty_module",
      "functions": [],
      "classes": [],
      "globals": []
    }
  ]
}
```

**Verification**:
- Empty arrays, not null
- Module still present in output

---

### 6.5 Test Very Long Names

**Purpose**: Test name length limits

**Test Input**:
```python
# Python
def very_long_function_name_that_exceeds_typical_limits_and_continues_for_a_very_long_time():
    pass
```

**Expected Output**:
- Function extracted with full name
- NO truncation (unless language imposes limit)

**Verification**:
- Names up to 255+ chars supported
- No arbitrary truncation

---

### 6.6 Test Unicode Names (If Supported)

**Purpose**: Test non-ASCII identifiers

**Test Input**:
```python
# Python (supports Unicode)
def函数():
    pass

class Класс:
    pass
```

**Expected Output**:
```json
{
  "functions": [{"name": "函数"}],
  "classes": [{"name": "Класс"}]
}
```

**Verification**:
- Unicode names preserved
- Proper UTF-8 encoding in JSON

---

### 6.7 Test Reserved Keywords as Names

**Purpose**: Test handling of language keywords

**Test Input**:
```python
# Depends on language - generally not allowed
# but might occur in foreign language extraction
```

**Expected Behavior**:
- If language allows: extract as-is
- If not allowed: document error handling

---

### 6.8 Test Generic Type Parameters

**Purpose**: Test generic/template types

**Test Input**:
```python
# Python
def generic_func(items: List[T]) -> T:
    pass
```

**Expected Output**:
```json
{
  "parameters": [
    {"name": "items", "type": "handle", "type_alias": "List[T]"}
  ],
  "return_values": [
    {"name": "ret_0", "type": "any", "type_alias": "T"}
  ]
}
```

**Verification**:
- Generic types → handle or any
- type_alias preserves generic notation

---

### 6.9 Test Circular Dependencies

**Purpose**: Test circular imports/references

**Test Input**:
```python
# Python - a.py imports b.py, b.py imports a.py
```

**Expected Behavior**:
- Extract both modules without infinite loop
- Handle gracefully, document behavior

---

### 6.10 Test Special Methods (Dunder Methods)

**Purpose**: Test __special__ methods extraction

**Test Input**:
```python
# Python
class MyClass:
    def __str__(self) -> str:
        return "MyClass"

    def __add__(self, other):
        return self
```

**Expected Behavior**:
- __init__ → constructor
- __del__ → release
- Other dunder methods: DOCUMENT policy (extract or skip)

**Verification**:
- Consistent handling documented

---

## 7. Module/Package Loading Tests

### Purpose
Verify correct loading of different input sources.

---

### 7.1 Load From File Path

**Purpose**: Test loading .py file

**Test Input**:
```python
source_type = SourceType.FILE
source = "/path/to/module.py"
```

**Expected Behavior**:
- File loaded and executed
- Module extracted successfully

**Verification**:
- File path resolved correctly
- Module name extracted from filename

---

### 7.2 Load From Module Name

**Purpose**: Test loading importable module

**Test Input**:
```python
source_type = SourceType.MODULE
source = "os.path"
```

**Expected Behavior**:
- Module imported via importlib
- Extracted successfully

**Verification**:
- Import succeeds
- Module name = "os.path"

---

### 7.3 Load From Package (Recursive)

**Purpose**: Test recursive package loading

**Test Input**:
```python
source_type = SourceType.PACKAGE
source = "os"
```

**Expected Behavior**:
- Root package imported
- All submodules discovered and imported
- Private modules (starting with _) skipped

**Verification**:
- pkgutil.walk_packages used for discovery
- Submodules accessible after loading

---

### 7.4 Load Compiled File (.pyc)

**Purpose**: Test loading bytecode

**Test Input**:
```python
source_type = SourceType.FILE
source = "/path/to/module.pyc"
```

**Expected Behavior**:
- .pyc file loaded
- Extracted same as .py

**Verification**:
- Bytecode loading works
- Extraction identical to source

---

### 7.5 Auto-Detect Source Type

**Purpose**: Test auto-detection logic

**Test Input**:
```python
source_type = SourceType.AUTO
source = "/path/to/file.py"  # exists
```

**Expected Behavior**:
- Detect as FILE (path exists)
- Load successfully

**Verification**:
- File checked first
- Falls back to MODULE if not found

---

### 7.6 Handle Missing File

**Purpose**: Test error for non-existent file

**Test Input**:
```python
source_type = SourceType.FILE
source = "/nonexistent/path.py"
```

**Expected Behavior**:
- Raise FileNotFoundError or equivalent
- Clear error message

**Verification**:
- Exception thrown
- Message indicates missing file

---

## 8. Schema Validation Tests

### Purpose
Verify that generated JSON conforms to `sdk/idl_entities/idl.schema.json`.

---

### 8.1 Validate Against Schema

**Purpose**: Test full schema compliance

**Test Input**: Any valid IDL generation

**Test Code**:
```python
import jsonschema
import json

schema = json.load(open("sdk/idl_entities/idl.schema.json"))
idl = json.loads(generated_idl_json)

jsonschema.validate(instance=idl, schema=schema)
```

**Verification**:
- No ValidationError raised
- All required fields present
- All types match schema definitions

---

### 8.2 Verify Required Top-Level Fields

**Purpose**: Test top-level structure

**Required Fields**:
- idl_source
- idl_extension
- idl_filename_with_extension
- idl_full_path
- metaffi_guest_lib
- target_language
- modules

**Verification**:
- All present and non-empty (except where null allowed)

---

### 8.3 Verify Type Enum Values

**Purpose**: Test type values are from schema enum

**Valid Types**:
```
float64, float32, int8, int16, int32, int64,
uint8, uint16, uint32, uint64, bool,
char8, char16, char32,
string8, string16, string32,
handle, any, size, null,
float64_array, float32_array, ...
```

**Verification**:
- All type values in valid set
- Case matches (lowercase)

---

### 8.4 Verify Required Arg Fields

**Purpose**: Test arg_definition required fields

**Required**:
- name
- type
- type_alias
- comment (can be empty)
- tags (can be empty dict)
- dimensions

**Verification**:
- All present in every parameter/return/field
- Correct types (string, int, dict)

---

### 8.5 Verify Entity Path Structure

**Purpose**: Test entity_path is valid dict

**Verification**:
- entity_path is object (dict)
- Contains language-appropriate keys
- Values are strings or booleans

---

## 9. Integration Tests

### Purpose
Verify end-to-end workflows from source to validated JSON.

---

### 9.1 End-to-End: File to JSON

**Purpose**: Test complete workflow

**Steps**:
1. Create temp Python file with test code
2. Initialize extractor with file path
3. Extract module info
4. Generate IDL JSON
5. Parse JSON
6. Validate against schema

**Verification**:
- All steps succeed
- JSON valid
- Output matches expected structure

---

### 9.2 End-to-End: Module to JSON

**Purpose**: Test module extraction workflow

**Steps**:
1. Use built-in module (e.g., "sys")
2. Extract IDL
3. Validate JSON

**Verification**:
- Built-in module extracted
- Contains expected functions/classes
- Schema valid

---

### 9.3 End-to-End: Package to JSON

**Purpose**: Test package extraction workflow

**Steps**:
1. Use small package (e.g., "os")
2. Extract recursively
3. Validate JSON

**Verification**:
- Multiple modules extracted
- Submodules included
- Schema valid

---

### 9.4 Round-Trip: JSON to IDL to JSON

**Purpose**: Test JSON serialization stability

**Steps**:
1. Generate IDL JSON
2. Parse to object
3. Re-serialize to JSON
4. Compare to original

**Verification**:
- JSON identical (or semantically equivalent)
- No data loss

---

## 10. Error Handling Tests

### Purpose
Verify graceful handling of invalid inputs and error conditions.

---

### 10.1 Invalid Syntax

**Purpose**: Test syntax error handling

**Test Input**:
```python
# Python with syntax error
def broken syntax ( }:
```

**Expected Behavior**:
- SyntaxError raised (or captured)
- Clear error message

**Verification**:
- Exception includes error location
- No crash

---

### 10.2 Import Error

**Purpose**: Test missing module error

**Test Input**:
```python
source_type = SourceType.MODULE
source = "nonexistent_module"
```

**Expected Behavior**:
- ImportError/ModuleNotFoundError raised
- Clear error message

**Verification**:
- Exception indicates module not found
- No crash

---

### 10.3 Type Resolution Error

**Purpose**: Test unresolvable type

**Test Input**:
```python
# Python
def func(x: UnknownType):
    pass
```

**Expected Behavior**:
- Fallback to `handle` type
- type_alias = "UnknownType"
- NO exception (graceful degradation)

**Verification**:
- Function still extracted
- Type = handle

---

### 10.4 Circular Import

**Purpose**: Test circular dependency handling

**Test Input**: Two modules importing each other

**Expected Behavior**:
- Extract both modules
- No infinite loop
- May skip some details if unresolvable

**Verification**:
- Extraction completes
- No hang or crash

---

### 10.5 Permission Error

**Purpose**: Test file access error

**Test Input**: File without read permissions

**Expected Behavior**:
- PermissionError raised
- Clear error message

**Verification**:
- Exception indicates permission issue

---

### 10.6 Invalid UTF-8

**Purpose**: Test encoding error handling

**Test Input**: File with invalid UTF-8

**Expected Behavior**:
- UnicodeDecodeError OR
- Fallback to latin-1/replace errors

**Verification**:
- Document handling policy
- No crash

---

## Implementation Notes

### General Guidelines

1. **Use unittest framework**: All tests in single file using unittest.TestCase
2. **Test data**: Embed test code as strings or use built-in modules (sys, os)
3. **Assertions**: Use self.assertEqual, self.assertIn, self.assertTrue
4. **Setup/Teardown**: Use setUp() for common initialization
5. **Test isolation**: Each test should be independent

### Language-Specific Adaptations

#### Python3
- **Framework**: unittest
- **Introspection**: inspect module
- **Test modules**: sys, os, json (always available)
- **Temp files**: tempfile.NamedTemporaryFile for file tests
- **Entity path**: Use `callable`/`attribute` keys

#### Go
- **Framework**: go test
- **Parsing**: go/ast, go/parser packages
- **Test files**: Create .go files in testdata/
- **Entity path**: Language-specific keys

#### JVM
- **Framework**: JUnit
- **Reflection**: Java reflection API
- **Test classes**: Compile .java to .class for testing
- **Entity path**: `class`/`field` keys

### Test Organization

```python
# sdk/idl_compiler/python3/compiler_test.py

import unittest
import sys
import os
import json
import tempfile
from pathlib import Path

# Import modules to test
from sdk.idl_compiler.python3 import PythonExtractor, IDLGenerator, SourceType, TypeMapper, EntityPathGenerator

class TypeMapperTests(unittest.TestCase):
    """Test type mapping functionality"""

    def setUp(self):
        self.type_mapper = TypeMapper()

    def test_map_int_to_int32(self):
        metaffi_type, dimensions = self.type_mapper.map_type("int")
        self.assertEqual(metaffi_type, "int32")
        self.assertEqual(dimensions, 0)

    # ... more tests

class EntityPathTests(unittest.TestCase):
    """Test entity_path generation"""

    def setUp(self):
        self.entity_path_gen = EntityPathGenerator()

    def test_function_entity_path_simple(self):
        entity_path = self.entity_path_gen.create_function_entity_path("add")
        self.assertEqual(entity_path, {"callable": "add"})

    # ... more tests

class ExtractorTests(unittest.TestCase):
    """Test function/class extraction"""

    def test_extract_simple_function(self):
        # Create temp file with test code
        with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
            f.write("def add(x: int, y: int) -> int:\n    return x + y")
            temp_path = f.name

        try:
            extractor = PythonExtractor(temp_path, SourceType.FILE)
            module_info = extractor.extract()

            self.assertEqual(len(module_info.functions), 1)
            func = module_info.functions[0]
            self.assertEqual(func.name, "add")
            self.assertEqual(len(func.parameters), 2)
        finally:
            os.unlink(temp_path)

    # ... more tests

class IntegrationTests(unittest.TestCase):
    """Test end-to-end workflows"""

    def test_full_workflow_with_schema_validation(self):
        import jsonschema

        # Use built-in module
        extractor = PythonExtractor("sys", SourceType.MODULE)
        module_info = extractor.extract()

        generator = IDLGenerator("sys", module_info)
        idl_json = generator.generate_json()

        # Validate against schema
        schema_path = Path(__file__).parent.parent.parent / "compiler" / "go" / "IDL" / "schema.json"
        with open(schema_path) as f:
            schema = json.load(f)

        idl = json.loads(idl_json)
        jsonschema.validate(instance=idl, schema=schema)

        # Verify structure
        self.assertIn("idl_source", idl)
        self.assertEqual(idl["target_language"], "python3")

    # ... more tests

if __name__ == '__main__':
    unittest.main()
```

### Schema Validation

Always validate generated JSON:
```python
import jsonschema
import json

def validate_idl_json(idl_json_string):
    """Validate IDL JSON against schema"""
    schema_path = "sdk/idl_entities/idl.schema.json"
    with open(schema_path) as f:
        schema = json.load(f)

    idl = json.loads(idl_json_string)

    try:
        jsonschema.validate(instance=idl, schema=schema)
        return True
    except jsonschema.ValidationError as e:
        print(f"Validation failed: {e.message}")
        print(f"Failed at path: {e.json_path}")
        return False
```

### Memory Management

**Python**: Automatic (GC)
**Go**: Automatic (GC)
**Java**: Automatic (GC)
**C/C++**: Must use XLLR allocators for cross-runtime compatibility

---

## Test Coverage Checklist

Use this checklist when implementing a new IDL compiler:

### Type Mapping
- [ ] All primitive integer types (int8/16/32/64, uint8/16/32/64)
- [ ] Floating point types (float32, float64)
- [ ] Boolean type
- [ ] String types (language-appropriate)
- [ ] Array types (1D, 2D, 3D)
- [ ] Complex/generic types → handle
- [ ] Union/Optional types → any
- [ ] Special types (null, bytes, etc.)
- [ ] Unknown types → handle fallback
- [ ] Type aliases preserved

### Entity Path
- [ ] Simple function path
- [ ] Function with varargs
- [ ] Function with named args
- [ ] Instance method path
- [ ] Static method path
- [ ] Class method path
- [ ] Constructor path
- [ ] Global getter path
- [ ] Global setter path
- [ ] Field getter/setter paths

### Function Extraction
- [ ] Simple function
- [ ] Function with no parameters
- [ ] Function with optional parameters
- [ ] Function without type hints
- [ ] Function with multiple returns
- [ ] Function with varargs
- [ ] Function with kwargs
- [ ] Private function filtering

### Class Extraction
- [ ] Simple class (default constructor)
- [ ] Class with explicit constructor
- [ ] Multiple constructors/overloading
- [ ] Class with destructor
- [ ] Instance methods
- [ ] Static methods
- [ ] Class methods
- [ ] Class fields
- [ ] Properties (getter only)
- [ ] Properties (getter + setter)
- [ ] Private member filtering
- [x] Nested classes (if supported) - Python3 ✓

### Global Extraction
- [ ] Simple global variable
- [ ] Typed global
- [ ] Untyped global
- [ ] Private global filtering

### Edge Cases
- [ ] Read-only properties
- [ ] Write-only properties
- [ ] Method overloading
- [ ] Empty module
- [ ] Very long names
- [ ] Unicode names
- [ ] Reserved keywords
- [ ] Generic types
- [ ] Circular dependencies
- [ ] Special/dunder methods

### Loading
- [ ] Load from file path
- [ ] Load from module name
- [ ] Load from package (recursive)
- [ ] Load compiled files
- [ ] Auto-detect source type
- [ ] Handle missing file error

### Schema Validation
- [ ] Full schema validation
- [ ] Required top-level fields
- [ ] Type enum values
- [ ] Required arg fields
- [ ] Entity path structure

### Integration
- [ ] File to JSON workflow
- [ ] Module to JSON workflow
- [ ] Package to JSON workflow
- [ ] JSON round-trip

### Error Handling
- [ ] Invalid syntax
- [ ] Import error
- [ ] Type resolution error
- [ ] Circular import
- [ ] Permission error
- [ ] Invalid encoding

---

## Approximate Test Count

| Category | Count |
|----------|-------|
| Type Mapping | 15 |
| Entity Path | 10 |
| Function Extraction | 8 |
| Class Extraction | 12 |
| Global Extraction | 4 |
| Edge Cases | 10 |
| Loading | 6 |
| Schema Validation | 5 |
| Integration | 4 |
| Error Handling | 6 |
| **Total** | **~80 tests** |

---

## Future Language Implementations

When creating IDL compilers for new languages:

1. **Read this document thoroughly**
2. **Reference sdk/idl_entities/entity_path_specs.json** for entity_path structure
3. **Implement all applicable tests** (skip language-specific tests that don't apply)
4. **Add language-specific tests** for unique features
5. **Validate against schema.json**
6. **Document deviations** if any
7. **Update this document** if new test categories discovered

---

## Related Documentation

- **IDL Compiler Architecture**: `sdk/idl_compiler/idl_compiler_doc.md`
- **Entity Path Specification**: `sdk/idl_entities/entity_path_specs.json`
- **JSON Schema**: `sdk/idl_entities/idl.schema.json`
- **Serializer Tests**: `sdk/cdts_serializer/serializer_tests_doc.md` (similar pattern)

---

**End of Test Documentation**

For questions, refer to:
- **Source Code**: `sdk/idl_compiler/python3/`
- **Existing Tests**: `sdk/idl_compiler/python3/compiler_test.py`
- **MetaFFI Project**: https://github.com/MetaFFI
