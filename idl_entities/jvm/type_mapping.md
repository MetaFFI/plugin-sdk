# MetaFFI <-> JVM Type Mapping

This document serves as the ground truth for converting MetaFFI types to/from Java types.

## MetaFFI -> Java Type Mapping (Deserialization)

| MetaFFI Type | Java Type | JNI Type | Notes |
|--------------|-----------|----------|-------|
| `int8` | `byte` | `jbyte` | Signed 8-bit |
| `int16` | `short` | `jshort` | Signed 16-bit |
| `int32` | `int` | `jint` | Signed 32-bit |
| `int64` | `long` | `jlong` | Signed 64-bit |
| `uint8` | `short` | `jshort` | Larger type to preserve unsigned range (0-255) |
| `uint16` | `int` | `jint` | Larger type to preserve unsigned range (0-65535) |
| `uint32` | `long` | `jlong` | Larger type to preserve unsigned range |
| `uint64` | `BigInteger` | `jobject` | BigInteger for full unsigned 64-bit range |
| `float32` | `float` | `jfloat` | |
| `float64` | `double` | `jdouble` | |
| `bool` | `boolean` | `jboolean` | |
| `char8` | `char` | `jchar` | Convert UTF-8 to UTF-16 |
| `char16` | `char` | `jchar` | Native (Java char is UTF-16) |
| `char32` | `char` | `jchar` | FAIL-FAST if > 0xFFFF (BMP only) |
| `string8` | `String` | `jstring` | Convert UTF-8 to UTF-16 via `NewStringUTF()` |
| `string16` | `String` | `jstring` | Direct via `NewString()` |
| `string32` | `String` | `jstring` | Convert UTF-32 to UTF-16 (surrogate pairs) |
| `handle` | `Object` | `jobject` | Check `runtime_id` for unwrapping |
| `callable` | `metaffi.Caller` | `jobject` | Custom class with xcall pointer + type arrays |
| `any` | `Object` | `jobject` | Runtime type detection |
| `null` | `null` | n/a | Null reference |
| `*_array` (1D) | `T[]` | `jarray` | Primitive or object array |
| `*_array` (2D+) | `T[][]...` | `jobjectArray` | Nested arrays |

## Java -> MetaFFI Type Mapping (Serialization)

| Java Type | MetaFFI Type | Notes |
|-----------|--------------|-------|
| `byte` | `int8` | |
| `short` | `int16` | |
| `int` | `int32` | |
| `long` | `int64` | |
| `float` | `float32` | |
| `double` | `float64` | |
| `boolean` | `bool` | |
| `char` | `char16` | Native Java char |
| `String` | `string8` | Default to UTF-8 |
| `void` | *(0 return values)* | NOT `null` - void means no return values |
| `null` | `null` | Any reference type can be null |
| `BigInteger` | `uint64` | Special handling for unsigned 64-bit |
| `metaffi.Caller` | `callable` | |
| `Object` (unknown) | `handle` | With type_alias = class name |
| `T[]` | `T_array` (dims=1) | |
| `T[][]` | `T_array` (dims=2) | |

## Array Handling

The JVM CDTS serializer handles multi-dimensional arrays by nesting:

- **1D arrays**: `int64_array` (dims=1) -> `long[]`
- **2D arrays**: `int64_array` (dims=2) -> `long[][]`
- **3D arrays**: `int64_array` (dims=3) -> `long[][][]`
- **Special case**: `uint8_array` (dims=1) -> `byte[]` (for binary data, interpreted as unsigned)

**Ragged arrays:** Supported. Each sub-array length is determined independently from CDTS data.

**Mixed object arrays:** Use `Object[]` with runtime type detection per element. Maps to `handle_array` in MetaFFI.

## void vs null

These are distinct concepts:

- **void**: Method returns nothing (0 return values in CDTS). JNI signature: `V`
- **null**: A reference value that is null. Any reference type (Object, String, arrays) can be null.

Example:
```java
// Returns void - 0 return values
public void doSomething() { ... }

// Returns String - 1 return value (can be null)
public String getName() { return null; }
```

## Handle Types

Handle types represent cross-language object references:

- **Type annotation**: Mapped to `Object` for flexibility
- **Type alias**: The `type_alias` field in IDL preserves the original class name for documentation
- **Runtime behavior**: Check `runtime_id` field:
  - If `runtime_id == JVM_RUNTIME_ID` (3006477107): Unwrap to original jobject
  - Otherwise: Create a MetaFFIHandle wrapper object

## Callable Types

The `metaffi_callable` type maps to `metaffi.Caller` Java class with:
- `xcallAndContext` (long): Pointer to native xcall function and context
- `parametersTypesArray` (long[]): Array of parameter type codes
- `retvalsTypesArray` (long[]): Array of return value type codes

## Any Type Runtime Detection

When deserializing `metaffi_any_type`, the actual type is detected at runtime:

| Java Class | MetaFFI Type |
|------------|--------------|
| `java.lang.Byte` | `int8` |
| `java.lang.Short` | `int16` |
| `java.lang.Integer` | `int32` |
| `java.lang.Long` | `int64` |
| `java.lang.Float` | `float32` |
| `java.lang.Double` | `float64` |
| `java.lang.Boolean` | `bool` |
| `java.lang.Character` | `char16` |
| `java.lang.String` | `string8` |
| Other classes | `handle` (with class name as type_alias) |

## Handle Runtime ID

When working with handles:

```cpp
#include "runtime_id.h"

// JVM_RUNTIME_ID defined as 3006477107ULL
if (cdt_handle.runtime_id == JVM_RUNTIME_ID) {
    // It's a JVM object - unwrap directly
    jobject obj = static_cast<jobject>(cdt_handle.handle);
} else {
    // It's from another runtime - wrap in MetaFFIHandle
    // ... create wrapper object
}
```

## Generated Code Examples

### Simple Method
**MetaFFI IDL:**
```json
{
  "name": "add",
  "parameters": [
    {"name": "a", "type": "int32"},
    {"name": "b", "type": "int32"}
  ],
  "return_values": [
    {"name": "result", "type": "int32"}
  ]
}
```

**Generated Java:**
```java
public int add(int a, int b) {
    // Use MetaFFI runtime to make the cross-language call
    return metaffiCaller.call(a, b);
}
```

### Array Method
**MetaFFI IDL:**
```json
{
  "name": "sum_array",
  "parameters": [
    {"name": "numbers", "type": "int64_array", "dimensions": 1}
  ],
  "return_values": [
    {"name": "total", "type": "int64"}
  ]
}
```

**Generated Java:**
```java
public long sumArray(long[] numbers) {
    return metaffiCaller.call(numbers);
}
```

### 2D Array Method
**MetaFFI IDL:**
```json
{
  "name": "transpose_matrix",
  "parameters": [
    {"name": "matrix", "type": "int64_array", "dimensions": 2}
  ],
  "return_values": [
    {"name": "transposed", "type": "int64_array", "dimensions": 2}
  ]
}
```

**Generated Java:**
```java
public long[][] transposeMatrix(long[][] matrix) {
    return metaffiCaller.call(matrix);
}
```

### Handle Type
**MetaFFI IDL:**
```json
{
  "name": "process_data",
  "parameters": [
    {"name": "data", "type": "handle", "type_alias": "DataObject"},
    {"name": "config", "type": "any"}
  ],
  "return_values": [
    {"name": "result", "type": "any"}
  ]
}
```

**Generated Java:**
```java
public Object processData(Object data, Object config) {
    return metaffiCaller.call(data, config);
}
```

## Implementation Reference

The type conversion is implemented in:
- `sdk/cdts_serializer/jvm/cdts_jvm_serializer.cpp` - C++ serialization/deserialization
- `sdk/idl_entities/jvm/src/com/metaffi/idl/entities/TypeMapper.java` - IDL type mapping
