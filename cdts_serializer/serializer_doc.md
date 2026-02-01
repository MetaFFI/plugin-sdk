# CDTS Serializer/Deserializer Documentation

## Purpose

CDTS (Common Data Type Serializers) are components that convert between language-native types and MetaFFI's Common Data Types (CDT). They enable seamless data exchange between different programming languages through MetaFFI's interoperability layer.

## Why Serializers Are Needed

When calling functions across language boundaries via MetaFFI, parameters and return values must be converted between:
- **Language-native types** (e.g., Python's `int`, C++'s `int32_t`, Java's `Integer`)
- **MetaFFI CDT types** (e.g., `metaffi_int32`, `metaffi_string8`, `metaffi_array`)

Serializers handle this conversion automatically, providing a developer-friendly API that hides the complexity of the CDT format.

## Critical: The C Interface Foundation

**Most Important Understanding**: Regardless of the implementation language, CDTS is **always a C struct interface**. This is the fundamental contract across all languages.

### The C Struct Definition
CDTS is defined in `sdk/runtime/cdt.h` as plain C structs:

```c
struct cdt {
    metaffi_type type;           // Type enum (int8, string, array, etc.)
    union cdt_val cdt_val;       // Value union holding actual data
    bool free_required;          // Memory ownership flag
};

struct cdts {
    struct cdt* arr;             // Pointer to array of CDT elements
    metaffi_size length;         // Number of elements
    metaffi_int8 fixed_dimensions;  // Dimension count (NOT lengths!)
    bool allocated_on_cache;     // Memory pooling flag
};
```

### Language-Specific Access Patterns

Different languages access these C structs in different ways:

**C++** (Special Case):
- Can wrap C structs in C++ classes with constructors/destructors
- Example: `cdts_cpp_serializer` wraps the C `cdts` struct
- Uses operator overloading for intuitive API: `ser << value`
- Still manipulates the underlying C struct

**Java**:
- Receives `cdts*` as a `long` pointer via JNI/JNA
- Must use JNI/JNA to read/write struct fields
- Example: `long cdtsPtr = metaffiCall(...);`
- Access via: `Memory.getLong(cdtsPtr + offset_arr);`

**Go**:
- Receives `*C.cdts` via cgo
- Must use C interop to access fields
- Example: `cdtsPtr := C.metaffi_call(...)`
- Access via: `C.get_cdts_length(cdtsPtr)`

**C#**:
- Receives `IntPtr` to `cdts` struct via P/Invoke
- Must marshal to/from unmanaged memory
- Example: `IntPtr cdtsPtr = MetaffiCall(...);`
- Access via: `Marshal.ReadInt32(cdtsPtr + offset_length);`

### Why This Matters

1. **Testing**: When writing tests, you'll manually create CDTS using the C API (or FFI to the C API)
2. **Interoperability**: All languages must produce/consume the exact same C struct layout
3. **No Assumptions**: Don't assume C++ wrappers exist - work with raw C interface
4. **Memory Management**: The C struct defines ownership semantics (free_required flag)

## Core Concept

### Serialization (Language → CDT)
Converting language-native values into CDT format:
```
Native Type     →   CDTS Serializer   →   CDT Structure
int32_t(42)     →   serializer << 42  →   cdt{type: metaffi_int32, val: 42}
"hello"         →   serializer << str →   cdt{type: metaffi_string8, val: "hello"}
[1,2,3]         →   serializer << vec →   cdts{arr: [cdt, cdt, cdt], length: 3}
```

### Deserialization (CDT → Language)
Converting CDT structures back into language-native values:
```
CDT Structure                       →   CDTS Serializer   →   Native Type
cdt{type: metaffi_int32, val: 42}  →   serializer >> x   →   int32_t x = 42
cdt{type: metaffi_string8, ...}    →   serializer >> str →   string str = "hello"
cdts{arr: [...], length: 3}        →   serializer >> vec →   vector<int> vec = [1,2,3]
```

## Language-Agnostic Requirements

Every language implementation of a CDTS serializer must handle:

### 1. Type Mapping

Map native types to MetaFFI types bidirectionally:

| Category | Native Examples | MetaFFI Type |
|----------|----------------|--------------|
| **Integers** | int8, int16, int32, int64 | `metaffi_int8` through `metaffi_int64` |
| **Unsigned Integers** | uint8, uint16, uint32, uint64 | `metaffi_uint8` through `metaffi_uint64` |
| **Floating Point** | float, double | `metaffi_float32`, `metaffi_float64` |
| **Boolean** | bool, Boolean, True/False | `metaffi_bool` |
| **Characters** | char (UTF-8/16/32) | `metaffi_char8/16/32` |
| **Strings** | string, String, str | `metaffi_string8/16/32` (UTF-8/16/32) |
| **Arrays** | vector, list, array, slice | `metaffi_array` (cdts structure) |
| **Handles** | object references, pointers | `metaffi_handle` (cdt_metaffi_handle) |
| **Callables** | functions, lambdas, closures | `metaffi_callable` (cdt_metaffi_callable) |
| **Null/None** | null, nil, None, nullptr | `metaffi_null_type` |

### 2. Memory Management

**CRITICAL: Use XLLR Alloc/Free Functions**

**All memory allocations and deallocations in C/C++ implementations MUST use the XLLR alloc/free functions** (`xllr_alloc*`/`xllr_free*`). This is crucial because different modules (especially in different operating systems) might use different C-runtimes. Allocating in one C-runtime and freeing in another leads to undefined behavior.

**Available XLLR Functions** (from `sdk/runtime/xllr_capi_loader.h`):
- `void* xllr_alloc_memory(uint64_t size)` / `void xllr_free_memory(void* ptr)` - General memory allocation
- `char8_t* xllr_alloc_string8(const char8_t* src, uint64_t length)` / `void xllr_free_string(char* ptr)` - UTF-8 strings
- `char16_t* xllr_alloc_string16(const char16_t* src, uint64_t length)` / `void xllr_free_string(char* ptr)` - UTF-16 strings
- `char32_t* xllr_alloc_string32(const char32_t* src, uint64_t length)` / `void xllr_free_string(char* ptr)` - UTF-32 strings
- `struct cdt* xllr_alloc_cdt_array(uint64_t count)` / `void xllr_free_cdt_array(struct cdt* arr)` - CDT arrays
- `struct cdts* xllr_alloc_cdts_buffer(metaffi_size params, metaffi_size rets)` / `void xllr_free_cdts_buffer(struct cdts* pcdts)` - CDTS buffers

**Important Notes**:
- **Never use** `malloc`/`free`, `new`/`delete`, or standard library allocators in serializer implementations
- **Always use** `xllr_alloc*`/`xllr_free*` functions for all dynamic memory
- These functions are accessed via `sdk/runtime/xllr_capi_loader.c/.h`
- The XLLR functions ensure memory is allocated/freed from the same C-runtime across all modules

**String Ownership**:
- **Serialization**: Copy strings into CDT with `free_required=true` (use `xllr_alloc_string8/16/32`)
- **Deserialization**: Allocate new native strings using `xllr_alloc_string*`, caller must free with `xllr_free_string`
- **Lifetime**: CDT owns its string data until destroyed (CDT core uses XLLR functions internally)

**Array Ownership**:
- **Serialization**: Create nested `cdts` structures using `xllr_alloc_cdts_buffer` or `xllr_alloc_memory`, CDT owns array memory
- **Deserialization**: Allocate new native arrays/lists using `xllr_alloc_memory`, copy elements
- **Dimensions**: Track `fixed_dimensions` field correctly

**Handle Lifetime**:
- **User-managed**: Handles store pointers, user controls object lifetime
- **Optional cleanup**: Can set `release` callback for automatic cleanup
- **Runtime tracking**: `runtime_id` identifies which runtime owns the object

### 3. Array Handling

**Fixed Dimensions**:
- `fixed_dimensions` = count of array dimensions, NOT array lengths
- Examples:
  - `[1,2,3]` → `fixed_dimensions = 1` (1D array)
  - `[[1,2],[3,4]]` → `fixed_dimensions = 2` (2D array)
  - `[[[1]]]` → `fixed_dimensions = 3` (3D array)

**Ragged Arrays**:
- Arrays with varying inner lengths: `[[1], [2,3], [4,5,6]]`
- Still have `fixed_dimensions = 2` (always 2 levels)
- Length varies per subarray

**Mixed Dimensions**:
- Arrays mixing scalars and arrays: `[1, [2,3]]`
- Set `fixed_dimensions = MIXED_OR_UNKNOWN_DIMENSIONS (-1)`
- Requires special handling during traversal

### 4. Type Safety

**Serialization** (Native → CDT):
- Use compile-time or runtime type information
- Set correct `type` field in CDT
- Handle incompatible types with clear errors

**Deserialization** (CDT → Native):
- Validate CDT `type` matches expected native type
- Throw/raise error on type mismatch
- Provide detailed error messages with context

### 5. Error Handling

**Fail-Fast Principle**:
- Detect errors immediately, don't propagate invalid state
- Throw exceptions or return error codes (language-dependent)
- Never silently ignore type mismatches or bounds violations

**Error Context**:
- Include CDT index in error messages
- Show expected vs actual types
- Indicate which operation failed (serialization/deserialization)

Example error messages:
```
"Type mismatch at index 2: expected metaffi_int32, got metaffi_string8"
"Bounds violation: attempted to access index 5 in CDTS of length 3"
"Cannot deserialize: null CDTS pointer"
```

## Implementation Patterns

**Important**: All patterns below ultimately manipulate the same C `cdt` and `cdts` structs. The differences are in the API style (operator overloading vs methods vs callbacks), but all must:
- Read/write the C struct fields correctly
- Set the `type` field to the appropriate `metaffi_type` enum value
- Manage the `free_required` flag for memory ownership
- Handle `fixed_dimensions` correctly for arrays

**API Design Principle** (Lesson from C++ Implementation):
Use **native language types** in the API, NOT MetaFFI types:
- **Good**: `ser.add(int32_t(42))` or `ser << 42` (where 42 is native int)
- **Bad**: `ser.add(metaffi_int32(42))` - users shouldn't need to know MetaFFI types

The serializer internally converts native types to the appropriate `metaffi_type` enum values when setting the CDT's `type` field.

### Pattern 1: Stream-Based API (C++)

**Characteristics**:
- Sequential insertion/extraction with operators (`<<`, `>>`)
- Index tracking within serializer object
- Chainable operations

**Example**:
```cpp
cdts params(3);
cdts_cpp_serializer ser(params);
ser << int32_t(42) << std::string("hello") << 3.14;

int32_t x; std::string s; double d;
ser.reset();
ser >> x >> s >> d;
```

**Pros**: Intuitive, familiar to C++ developers, simple for sequential access
**Cons**: Nested arrays need template recursion, may need helper methods for complex cases

### Pattern 2: Explicit Type Specification API (Python3)

**CRITICAL: Python Type Ambiguity Problem**

Python's `int` and `float` types don't specify size (int8 vs int64, float32 vs float64). This causes **information loss** during serialization:

**The Problem**:
- Python `int` could be int8, int16, int32, or int64
- Python `float` could be float32 or float64
- Caller might need int32, but serializer would default to int64
- This breaks cross-language calls where exact types matter

**The Solution**:
Python3 serializer **requires explicit type specification** for ambiguous types.

**Characteristics**:
- **Explicit type parameter required** for int/float (no defaults)
- Method-based API: `add(obj, target_type)`
- Range validation: throws error if value doesn't fit target type
- Unambiguous types (bool, string, None) still work naturally

**Example**:
```cpp
cdts data(5);
cdts_python3_serializer ser(data);

PyObject* i = PyLong_FromLong(42);
PyObject* f = PyFloat_FromDouble(3.14);

// MUST specify explicit types for int/float
ser.add(i, metaffi_int32_type)        // Explicit: int32 (not int64!)
   .add(f, metaffi_float32_type)      // Explicit: float32 (not float64!)
   .add(Py_True, metaffi_bool_type)   // Unambiguous: bool
   .add(Py_None, metaffi_null_type);  // Unambiguous: null

Py_DECREF(i); Py_DECREF(f);

// Range validation
PyObject* big = PyLong_FromLong(300);
ser.add(big, metaffi_int8_type);  // ERROR: 300 doesn't fit in int8 [-128, 127]
```

**Deserialization** (CDTS → Python):
```cpp
// Deserialization works naturally - CDTS already has type info
PyObject* obj = ser.extract_pyobject();  // Type preserved from CDTS
```

**Pros**: Prevents information loss, explicit about type requirements, validates ranges
**Cons**: More verbose than auto-detection (but necessary for correctness)

### Pattern 3: Method-Based API (Java/Go)

**Characteristics**:
- Explicit methods for each type (addInt32, addString, etc.)
- Clear API surface, no operator overloading
- Builder pattern for construction

**Example (conceptual)**:
```java
CDTSSerializer ser = new CDTSSerializer(cdts);
ser.addInt32(42);
ser.addString("hello");
ser.addFloat64(3.14);

int x = ser.getInt32();
String s = ser.getString();
double d = ser.getFloat64();
```

**Pros**: Explicit, self-documenting, works in all languages
**Cons**: More verbose, requires many method declarations

## Reference Implementations

### C++ Implementation
**Location**: `sdk/cdts_serializer/cpp/`

**Features**:
- Stream-based API with `operator<<` and `operator>>`
- Template recursion for automatic nested array support
- Variant-based `extract_any()` for unknown types
- Type query with `peek_type()`

**Key Files**:
- `cdts_cpp_serializer.h` - Class definition and API
- `cdts_cpp_serializer.cpp` - Implementation
- `cdts_cpp_serializer_test.cpp` - Comprehensive unit tests

### Python Implementation
**Location**: `lang-plugin-python311/runtime/`

**Features**:
- Callback-based using traverse/construct pattern
- `to_py_tuple()` converts CDT → Python tuple
- `to_cdts()` converts Python tuple → CDT
- Handles arbitrary nesting naturally

**Key Files**:
- `cdts_python3.h` - Class definition
- `cdts_python3.cpp` - Implementation with callbacks

## Implementation Checklist

When implementing a CDTS serializer for a new language:

- [ ] **Type Mapping**: Define mappings for all primitive types
- [ ] **Strings**: Handle UTF-8/16/32 encodings, manage memory correctly
- [ ] **Arrays**: Support multi-dimensional and ragged arrays
- [ ] **Handles**: Wrap native object references
- [ ] **Callables**: Wrap function/closure references
- [ ] **Serialization**: Native → CDT conversion
- [ ] **Deserialization**: CDT → Native conversion
- [ ] **Error Handling**: Fail-fast with detailed messages
- [ ] **Type Validation**: Runtime checks during deserialization
- [ ] **Memory Safety**: No leaks, clear ownership
- [ ] **Unit Tests**: Comprehensive coverage of all types
- [ ] **Integration Tests**: Round-trip conversion tests
- [ ] **Documentation**: API examples and usage patterns

## Testing Strategy

### Deserialization-Only Tests (CRITICAL)

**Why This is Critical**: The primary use case for CDTS serializers is receiving data from MetaFFI cross-language calls. When Language A calls Language B, Language B receives a pre-filled CDTS that it must deserialize. Testing only round-trip (serialize then deserialize) doesn't validate this real-world scenario.

**How to Test**: Manually create CDTS using the C API (or FFI to C API), then ONLY test deserialization.

**Example from C++ implementation**:
```cpp
// Manually create CDTS as if received from MetaFFI
cdts data(3);
data[0] = int32_t(42);                    // Uses C++ wrapper to set C struct
data[1].set_string((const char8_t*)"hello", true);  // Sets cdt.type and cdt_val
data[2] = double(3.14);

// ONLY deserialize (no serialization step)
cdts_cpp_serializer deser(data);
int32_t i; std::string s; double d;
deser >> i >> s >> d;

// Verify
assert(i == 42);
assert(s == "hello");
assert(d == 3.14);
```

**Conceptual Java example** (using JNI/JNA):
```java
// Manually create CDTS using C FFI
Pointer cdtsPtr = allocateCdts(3);

// Set first element (int32)
Pointer arrPtr = cdtsPtr.getPointer(OFFSET_ARR);
arrPtr.setInt(OFFSET_TYPE, METAFFI_INT32_TYPE);
arrPtr.setInt(OFFSET_CDAT_VAL, 42);

// Set second element (string)
Pointer cdt1 = arrPtr.share(SIZE_OF_CDT);
cdt1.setInt(OFFSET_TYPE, METAFFI_STRING8_TYPE);
// ... set string value

// ONLY deserialize
CDTSDeserializer deser = new CDTSDeserializer(cdtsPtr);
int i = deser.getInt32();
String s = deser.getString();
double d = deser.getFloat64();
```

**Test Coverage** (from C++ implementation with 49 total tests, 10 deserialization-only):
- All primitive types (int8-64, uint8-64, float32/64, bool) with manually created CDTS
- All string encodings (UTF-8/16/32) with manual creation using `set_string()`
- 1D arrays: manually create with `set_new_array(length, 1, type)`
- 2D arrays: manually create nested cdts structures
- 3D arrays: manually create 3-level nested structures
- Ragged arrays: varying lengths at each level
- Handles: manually set with `set_handle()`
- NULL values: manually set `cdt.type = metaffi_null_type`
- Type queries: `peek_type()` before deserializing
- ANY type extraction: `extract_any()` on pre-filled CDTS

### Unit Tests (Round-Trip)

Test each type bidirectionally (serialize then deserialize):
- All primitive types
- All string encodings
- 1D, 2D, 3D+ arrays
- Ragged arrays
- Mixed-type sequences
- Handles and callables

### Error Tests

Verify proper error handling:
- Type mismatches
- Bounds violations
- Null/invalid inputs
- Encoding errors (strings)

### Integration Tests

Test with actual MetaFFI calls:
- Cross-language function calls
- Complex data structures
- Large datasets
- Edge cases (empty arrays, null values, etc.)

## Lessons from C++ Implementation

The C++ implementation (`sdk/cdts_serializer/cpp/`) provides valuable lessons applicable to all language implementations:

### 1. Use Native Types in API, Not MetaFFI Types

**What We Learned**: The initial C++ implementation used MetaFFI types (`metaffi_int32`, `metaffi_float32`, etc.) in the API. This was confusing because users had to know about MetaFFI's internal type names.

**The Fix**: Changed to standard C++ types (`int32_t`, `float`, `double`, `std::string`). The serializer internally maps these to `metaffi_type` enum values.

**Before** (confusing):
```cpp
ser << metaffi_int32(42) << metaffi_float32(3.14);
```

**After** (clear):
```cpp
ser << int32_t(42) << float(3.14);
```

**Takeaway**: Use your language's native types in the public API. Users should think in terms of their language, not MetaFFI internals.

### 2. Type Validation is Critical

Always validate the `cdt.type` field before extracting values:

```cpp
void validate_type_at(metaffi_size index) const {
    metaffi_type expected = get_metaffi_type<T>();  // From template parameter
    metaffi_type actual = data[index].type;          // From C struct field

    if (actual != expected) {
        throw std::runtime_error(
            "Type mismatch at index " + std::to_string(index) +
            ": expected " + std::to_string(expected) +
            ", got " + std::to_string(actual)
        );
    }
}
```

**Why**: Prevents silent type errors that could corrupt data or crash.

### 3. Nested Array Handling Requires Recursion

For nested arrays (2D, 3D, etc.), you must:
1. Set `fixed_dimensions` to the dimension count (2 for 2D, 3 for 3D)
2. Recursively create nested `cdts` structures
3. Handle ragged arrays (varying inner lengths)

**C++ uses template recursion** (language-specific):
```cpp
template<typename T>
cdts_cpp_serializer& operator<<(const std::vector<T>& vec) {
    constexpr int depth = array_depth<std::vector<T>>::value;  // Compile-time

    data[current_index].set_new_array(vec.size(), depth, element_type);

    for (size_t i = 0; i < vec.size(); ++i) {
        if constexpr (is_vector<T>::value) {
            // Nested vector - recurse
            cdts_cpp_serializer nested(arr);
            nested << vec[i];  // Recursive call
        } else {
            // Base type
            arr[i] = vec[i];
        }
    }
}
```

**Takeaway**: Other languages will need their own approach (runtime type inspection, explicit depth parameter, etc.).

### 4. Variant Pattern for ANY Types

When deserializing unknown types, use a variant/union pattern:

```cpp
using cdts_any_variant = std::variant<
    int8_t, int16_t, int32_t, int64_t,
    uint8_t, uint16_t, uint32_t, uint64_t,
    float, double, bool,
    std::string, std::u16string, std::u32string,
    // ... etc
>;

cdts_any_variant extract_any() {
    switch (data[current_index].type) {
        case metaffi_int32_type: return static_cast<int32_t>(data[current_index]);
        case metaffi_string8_type: /* ... */
        // ... etc
    }
}
```

**Takeaway**: Provide both explicit type extraction and generic ANY extraction.

### 5. Memory Management: The free_required Flag

When setting strings or creating arrays:
- Set `free_required = true` if the CDT owns the memory (must free on destruction)
- Set `free_required = false` if the memory is externally managed

```cpp
// Serialization: CDT owns a copy
data[i].set_string((const char8_t*)str.c_str(), true);  // is_copy=true
data[i].free_required = true;  // CDT will free this memory

// Deserialization: Extract to native type, CDT still owns its copy
std::string s = std::string((const char*)data[i].cdt_val.string8_val);
// CDT will clean up its own memory when destroyed
```

### 6. Bounds Checking Prevents Crashes

Always check array bounds before accessing:

```cpp
void check_bounds(metaffi_size index) const {
    if (index >= data.length) {
        throw std::out_of_range(
            "Index out of bounds: " + std::to_string(index) +
            " >= " + std::to_string(data.length)
        );
    }
}
```

### 7. Comprehensive Testing Prevents Bugs

The C++ implementation has 49 tests:
- 39 round-trip tests (serialize then deserialize)
- 10 deserialization-only tests (manually created CDTS)
- 100% pass rate

This comprehensive testing caught several issues during development:
- UTF-16/32 string memory allocation bugs (fixed in C core)
- Type validation edge cases
- Array dimension handling

## Common Pitfalls

### 1. String Encoding Confusion
**Issue**: Mixing UTF-8/16/32 or assuming ASCII
**Solution**: Explicitly handle encoding, use appropriate metaffi_string* type

### 2. Fixed Dimensions Misunderstanding
**Issue**: Confusing `fixed_dimensions` (dimension count) with array length
**Solution**: Remember: `fixed_dimensions = 1` for [1,2,3], not `3`

### 3. Memory Leaks
**Issue**: Not setting `free_required` flag or not calling destructors
**Solution**: Always set `free_required=true` when copying, use RAII patterns

### 4. Type Safety Bypass
**Issue**: Casting without validation, silent failures
**Solution**: Always validate type field before casting, fail-fast on mismatch

### 5. Nested Array Complexity
**Issue**: Not handling recursion correctly, flattening nested structures
**Solution**: Use recursive traversal or construction, track depth properly

### 6. Using MetaFFI Types in API Instead of Native Types (NEW)
**Issue**: Exposing `metaffi_int32`, `metaffi_float32`, etc. in the public API makes it confusing. Users don't know these types and shouldn't need to.
**Solution**: Use native language types (`int32_t`, `float`, `double`, `String`, etc.) in the public API. Internally map to `metaffi_type` enum values when setting the CDT's `type` field.
**Example**: `ser.add(42)` not `ser.add(metaffi_int32(42))`

### 7. Not Testing Deserialization-Only Scenarios (NEW)
**Issue**: Only testing round-trip (serialize then deserialize) doesn't validate the real MetaFFI use case: receiving pre-filled CDTS from cross-language calls.
**Solution**: Add deserialization-only tests where you manually create CDTS using the C API (or FFI), then only test deserialization. This simulates receiving data from another language.
**Example**: Manually set `cdt.type`, `cdt.cdt_val`, and `cdt.free_required`, then deserialize.

### 8. Assuming C++ Wrapper Exists (NEW)
**Issue**: Assuming there's a high-level `cdts` class with methods like in C++. In Java/Go/C#, you only have raw C pointers.
**Solution**: Use FFI (JNI, cgo, P/Invoke) to directly manipulate the C struct fields. Reference `sdk/runtime/cdt.h` for the C struct layout.
**Example (Java)**: `Memory.getInt(cdtsPtr + OFFSET_LENGTH)` not `cdts.getLength()`

### 9. Using Standard Allocators Instead of XLLR Functions (CRITICAL)
**Issue**: Using `malloc`/`free`, `new`/`delete`, or standard library allocators causes undefined behavior when modules use different C-runtimes (especially on Windows).
**Solution**: **ALWAYS** use `xllr_alloc*`/`xllr_free*` functions from `sdk/runtime/xllr_capi_loader.h` for all memory allocations and deallocations.
**Example**: 
- ❌ `char* str = (char*)malloc(len + 1);` 
- ✅ `char8_t* str = xllr_alloc_string8(src, len);`
- ❌ `free(str);`
- ✅ `xllr_free_string(str);`
**Why**: Different modules (DLLs, shared libraries) may be compiled with different C-runtimes. Allocating in one runtime and freeing in another causes crashes and memory corruption.

### 10. Python Type Ambiguity Causing Information Loss (CRITICAL - Python Only)
**Issue**: Python's `int`/`float` don't specify size. Serializing Python int with auto-detection defaults to int64, but caller might need int32. This breaks cross-language calls.
**Solution**: **Python3 serializer requires explicit type specification**. Use `add(obj, metaffi_int32_type)` not auto-detection.
**Example**:
- ❌ `ser << PyLong_FromLong(42)` - would default to int64 (old API, removed)
- ✅ `ser.add(PyLong_FromLong(42), metaffi_int32_type)` - explicit int32
**Why**: Cross-language calls need exact types. If C++ caller expects int32 but Python returns int64, the type mismatch breaks interoperability.
**Note**: C/C++ don't have this issue because they use explicit types (`int32_t`, `float`, etc.).

## Performance Considerations

- **Zero-Copy Where Possible**: For const references, avoid copying large data
- **Batch Operations**: Process arrays in bulk rather than element-by-element
- **Type Caching**: Cache compile-time type information
- **Memory Pooling**: Reuse CDTS allocations when possible (use cache flag)

## Conclusion

CDTS serializers are the bridge between language-native code and MetaFFI's interoperability layer.

### Key Takeaways for Future Implementations

1. **Foundation**: CDTS is always a C struct interface (`sdk/runtime/cdt.h`). All languages work with the same C structs, just with different access methods (C++ wrappers, Java JNI, Go cgo, C# P/Invoke).

2. **API Design**: Use native language types in your API (`int`, `float`, `String`), not MetaFFI types (`metaffi_int32`). Convert internally.

3. **Testing**: Include deserialization-only tests that manually create CDTS using the C API. This validates the real use case: receiving data from cross-language calls.

4. **Type Safety**: Always validate `cdt.type` before extracting values. Fail-fast with detailed error messages.

5. **Memory**: Understand the `free_required` flag. CDT manages its own memory when this is true. **CRITICAL**: Always use `xllr_alloc*`/`xllr_free*` functions for all memory operations to avoid C-runtime mismatches.

6. **Arrays**: Set `fixed_dimensions` to dimension count (not lengths). Handle nested structures recursively.

7. **Memory Allocation**: **NEVER** use standard allocators (`malloc`/`free`, `new`/`delete`). Always use XLLR functions (`xllr_alloc_memory`, `xllr_alloc_string8/16/32`, etc.) to ensure memory is managed by the same C-runtime across all modules.

### Reference Implementations

- **C++** (`sdk/cdts_serializer/cpp/`): Stream-based API, uses native C++ types, 49 comprehensive tests, template recursion for nested arrays
- **Python** (`lang-plugin-python311/runtime/`): Callback-based, uses traverse/construct pattern

### For Future Agents/Developers

This document provides conceptual understanding and lessons learned. When implementing a serializer:
- Start by understanding the C struct interface
- Choose an API pattern that fits your language (stream-based, method-based, callback-based)
- Use native types in the API
- Write comprehensive tests (both round-trip and deserialization-only)
- Study the C++ implementation as a reference for lessons learned

### type mapping

In case there is no 1:1 mapping of the programming language type system to metaffi types, the agent and the user need to decide on the mapping. for example, python int can be metaffi_int32, metaffi_int64, metaffi_int16 etc. Similarly with strings. Java might receive String as metaffi_string8 or metaffi_string16 etc. same issue with metaffi_char8 and so on. When serializing FROM CDTS to the programming language type system, the mapping is within the serializer (under the hood), but when serializing FROM the programming language type system to CDTS, in case of ambiguity, the serializer needs to know explicitly to which metaffi type it is serializing to. This is how cpython3 and jvm serializers has implemented.
