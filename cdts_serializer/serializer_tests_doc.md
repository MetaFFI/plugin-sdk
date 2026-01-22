# CDTS Serializer Test Documentation

This document details all tests that must be implemented for CDTS serializer implementations across all languages. These tests ensure consistent behavior and full coverage of the serialization/deserialization capabilities.

## Test Categories

### 1. Primitive Types - Serialization Round-Trip

These tests verify that primitive values can be serialized and then deserialized back to their original values.

#### 1.1 Serialize and deserialize int8
- **Purpose**: Test signed 8-bit integer serialization
- **Test Value**: -42
- **Verification**: Deserialized value equals original value

#### 1.2 Serialize and deserialize int16
- **Purpose**: Test signed 16-bit integer serialization
- **Test Value**: -12345
- **Verification**: Deserialized value equals original value

#### 1.3 Serialize and deserialize int32
- **Purpose**: Test signed 32-bit integer serialization
- **Test Value**: -123456789
- **Verification**: Deserialized value equals original value

#### 1.4 Serialize and deserialize int64
- **Purpose**: Test signed 64-bit integer serialization
- **Test Value**: -1234567890123456
- **Verification**: Deserialized value equals original value

#### 1.5 Serialize and deserialize uint8
- **Purpose**: Test unsigned 8-bit integer serialization
- **Test Value**: 255
- **Verification**: Deserialized value equals original value

#### 1.6 Serialize and deserialize uint16
- **Purpose**: Test unsigned 16-bit integer serialization
- **Test Value**: 65535
- **Verification**: Deserialized value equals original value

#### 1.7 Serialize and deserialize uint32
- **Purpose**: Test unsigned 32-bit integer serialization
- **Test Value**: 4294967295
- **Verification**: Deserialized value equals original value

#### 1.8 Serialize and deserialize uint64
- **Purpose**: Test unsigned 64-bit integer serialization
- **Test Value**: 18446744073709551615
- **Verification**: Deserialized value equals original value

#### 1.9 Serialize and deserialize float32
- **Purpose**: Test 32-bit floating point serialization
- **Test Value**: 3.14159
- **Verification**: Deserialized value approximately equals original value

#### 1.10 Serialize and deserialize float64
- **Purpose**: Test 64-bit floating point serialization
- **Test Value**: 2.71828182845904523536
- **Verification**: Deserialized value approximately equals original value

#### 1.11 Serialize and deserialize bool
- **Purpose**: Test boolean serialization
- **Test Values**: true, false
- **Verification**: Both true and false deserialize correctly

### 2. String Types - Serialization Round-Trip

#### 2.1 Serialize and deserialize string8 (UTF-8)
- **Purpose**: Test UTF-8 string serialization
- **Test Value**: "Hello, MetaFFI!"
- **Verification**: Deserialized string equals original string

#### 2.2 Serialize and deserialize string16 (UTF-16)
- **Purpose**: Test UTF-16 string serialization
- **Test Value**: u"Hello UTF-16 מה שלומך" (includes non-ASCII characters)
- **Verification**: Deserialized string equals original string
- **Note**: Language-specific implementations (e.g., std::u16string in C++, char16_t* in C)

#### 2.3 Serialize and deserialize string32 (UTF-32)
- **Purpose**: Test UTF-32 string serialization
- **Test Value**: U"Hello UTF-32 🚀" (includes emoji)
- **Verification**: Deserialized string equals original string
- **Note**: Language-specific implementations (e.g., std::u32string in C++, char32_t* in C)

#### 2.4 Serialize and deserialize empty string
- **Purpose**: Test empty string edge case
- **Test Value**: ""
- **Verification**: Deserialized string is empty

#### 2.5 Serialize and deserialize const char* (C++ specific)
- **Purpose**: Test C-style string literal serialization
- **Test Value**: "C-style string"
- **Verification**: Deserialized as proper string type

### 3. Multiple Values

#### 3.1 Serialize and deserialize multiple primitives
- **Purpose**: Test serialization of multiple values in sequence
- **Test Values**: int32(42), float32(3.14), float64(2.71828), bool(true), uint64(999)
- **Verification**: All values deserialize correctly in order

#### 3.2 Mixed types serialization/deserialization
- **Purpose**: Test diverse type mixing
- **Test Values**: int8(10), string("hello"), float32(3.14), bool(true), uint64(9999), optionally utf16string
- **Verification**: All values deserialize correctly preserving types

#### 3.3 Chaining operations
- **Purpose**: Test that serialization and deserialization can be chained
- **Test Values**: Three consecutive int32 values (1, 2, 3)
- **Verification**: Chained deserialization produces correct values

### 4. Arrays - Serialization Round-Trip

#### 4.1 Serialize and deserialize 1D array of int32
- **Purpose**: Test one-dimensional integer array
- **Test Value**: [1, 2, 3, 4, 5]
- **Verification**: Array length and all elements match

#### 4.2 Serialize and deserialize 1D array of double
- **Purpose**: Test one-dimensional floating-point array
- **Test Value**: [1.1, 2.2, 3.3]
- **Verification**: Array length and all elements approximately match

#### 4.3 Serialize and deserialize empty array
- **Purpose**: Test empty array edge case
- **Test Value**: Empty array of int32
- **Verification**: Deserialized array is empty

#### 4.4 Serialize and deserialize 2D array
- **Purpose**: Test two-dimensional array (regular)
- **Test Value**: [[1, 2, 3], [4, 5, 6]]
- **Verification**: Outer and inner dimensions match, all elements correct

#### 4.5 Serialize and deserialize 3D array
- **Purpose**: Test three-dimensional array
- **Test Value**: [[[1, 2], [3, 4]], [[5, 6], [7, 8]]]
- **Verification**: All dimensions match, all elements correct (8 total elements)

#### 4.6 Serialize and deserialize ragged 2D array
- **Purpose**: Test non-rectangular arrays
- **Test Value**: [[1], [2, 3], [4, 5, 6]]
- **Verification**: Outer dimension (3), inner dimensions (1, 2, 3), all elements correct

### 5. Special Values

#### 5.1 Serialize and deserialize null
- **Purpose**: Test null value handling
- **Test**: Serialize null, then check `is_null()` and `peek_type()` returns null type
- **Verification**: Deserializer recognizes null correctly

#### 5.2 Handle serialization
- **Purpose**: Test handle (pointer/reference) serialization
- **Test Setup**: Create handle with pointer to int(42), runtime_id(100)
- **Verification**: Handle pointer, runtime_id, and dereferenced value match

#### 5.3 Callable serialization
- **Purpose**: Test callable (function pointer) serialization
- **Test Setup**: Create callable with:
  - Function pointer: non-null value (e.g., 0x1234ABCD)
  - Parameters: [int32, string8]
  - Return values: [float64]
- **Verification**: Function pointer, parameter types, and return types match
- **Memory Note**: Arrays must be allocated with xllr_alloc for cross-runtime safety

### 6. Type Introspection

#### 6.1 Type query with peek_type
- **Purpose**: Test type inspection without consuming values
- **Test Values**: int32(42), string("hello"), float64(3.14)
- **Verification**:
  - peek_type() returns correct type before each extraction
  - Values are not consumed by peek_type()

#### 6.2 Extract ANY type (C++ specific with std::variant)
- **Purpose**: Test dynamic type extraction
- **Test Cases**:
  - int32(99) → holds int32
  - string("test string") → holds string
  - float64(2.71828) → holds float64
  - bool(true) → holds bool
  - null → holds monostate
- **Verification**: variant holds correct type and value
- **Note**: Language-specific feature, may not apply to all implementations

### 7. Utility Methods

#### 7.1 Utility methods
- **Purpose**: Test serializer state management
- **Tests**:
  - get_index() returns 0 initially
  - size() returns CDTS length
  - has_more() returns true when data remains
  - After adding 3 values, get_index() returns 3
  - reset() returns index to 0
  - set_index(2) moves to position 2
- **Verification**: All utility methods work correctly

### 8. Error Handling

#### 8.1 Error: Type mismatch on deserialization
- **Purpose**: Test that deserializing as wrong type produces error
- **Test**: Serialize int32(42), attempt to deserialize as string
- **Verification**: Throws exception or returns error code

#### 8.2 Error: Bounds violation on serialization
- **Purpose**: Test that exceeding CDTS capacity produces error
- **Test**: Create CDTS(2), serialize 3 values
- **Verification**: Third serialization throws exception or returns error code

#### 8.3 Error: Bounds violation on deserialization
- **Purpose**: Test that reading beyond CDTS end produces error
- **Test**: Create CDTS(1) with one value, attempt to read twice
- **Verification**: Second read throws exception or returns error code

#### 8.4 Error: Peek type beyond bounds
- **Purpose**: Test that peek_type() at end of CDTS produces error
- **Test**: Create CDTS(1), serialize one value (index now at 1), call peek_type()
- **Verification**: Throws exception or returns error code

### 9. Deserialization-Only Tests

These tests simulate receiving pre-filled CDTS from MetaFFI cross-language calls. They test deserialization without prior serialization.

#### 9.1 Deserialize pre-filled CDTS with all primitive types
- **Purpose**: Test direct deserialization of all primitives
- **Test Setup**: Manually create CDTS with 11 elements:
  - int8(-10), int16(-1000), int32(-100000), int64(-10000000)
  - uint8(10), uint16(1000), uint32(100000), uint64(10000000)
  - float32(3.14), float64(2.71828), bool(true)
- **Verification**: All values deserialize correctly

#### 9.2 Deserialize pre-filled CDTS with strings
- **Purpose**: Test direct string deserialization
- **Test Setup**: Manually create CDTS with:
  - string8("Hello UTF-8")
  - string16(u"Hello UTF-16")
  - string32(U"Hello UTF-32")
- **Verification**: All strings deserialize correctly

#### 9.3 Deserialize pre-filled CDTS with 1D array
- **Purpose**: Test direct 1D array deserialization
- **Test Setup**: Manually create 1D array [10, 20, 30, 40, 50]
- **Verification**: Array length (5) and all elements correct

#### 9.4 Deserialize pre-filled CDTS with 2D array
- **Purpose**: Test direct 2D array deserialization
- **Test Setup**: Manually create 2D array [[1, 2, 3], [4, 5, 6]]
- **Verification**: 2 rows, 3 columns each, all elements correct

#### 9.5 Deserialize pre-filled CDTS with 3D array
- **Purpose**: Test direct 3D array deserialization
- **Test Setup**: Manually create 3D array [[[1, 2]], [[3, 4]]]
- **Verification**: Correct dimensions (2×1×2), all elements correct

#### 9.6 Deserialize pre-filled CDTS with ragged array
- **Purpose**: Test direct ragged array deserialization
- **Test Setup**: Manually create [[1], [2, 3], [4, 5, 6]]
- **Verification**: 3 rows with lengths 1, 2, 3, all elements correct

#### 9.7 Deserialize pre-filled CDTS with NULL value
- **Purpose**: Test direct null deserialization
- **Test Setup**: Manually create CDTS with null element
- **Verification**: is_null() returns true, peek_type() returns null type

#### 9.8 Deserialize pre-filled CDTS with handle
- **Purpose**: Test direct handle deserialization
- **Test Setup**: Manually create handle with int(99), runtime_id(123)
- **Verification**: Handle pointer, runtime_id, and dereferenced value correct

#### 9.9 Deserialize pre-filled CDTS with callable
- **Purpose**: Test direct callable deserialization
- **Test Setup**: Manually create callable with:
  - Function pointer: 0x5678EFAB
  - Parameters: [int32, bool]
  - Return values: [string8]
- **Verification**: Function pointer, parameter types, and return types match
- **Memory Note**: Must use xllr_alloc for parameter/return type arrays

#### 9.10 Type query on pre-filled CDTS before deserialization
- **Purpose**: Test type inspection on externally-provided CDTS
- **Test Setup**: Manually create CDTS with int32(42), string("test"), float64(3.14), null
- **Verification**: peek_type() works correctly before each extraction

#### 9.11 Extract ANY from pre-filled CDTS (C++ specific)
- **Purpose**: Test dynamic extraction from externally-provided CDTS
- **Test Setup**: Manually create CDTS with int32(42), string("hello"), double(3.14), bool(true)
- **Verification**: extract_any() returns correct variant types and values

### 10. Pre-existing CDTS Tests

#### 10.1 Deserialization from existing CDTS
- **Purpose**: Test deserializing from simulated MetaFFI call result
- **Test Setup**: Create CDTS with int32(123), string("test"), float64(9.99)
- **Verification**: All values deserialize correctly

## Implementation Notes

### Memory Management
- **Critical**: All memory allocations and deallocations in C/C++ **MUST** use `xllr_alloc*`/`xllr_free*` functions
- **Reason**: Different modules may use different C-runtimes (especially on different operating systems)
- **Risk**: Allocating in one C-runtime and freeing in another leads to undefined behavior
- **Solution**: XLLR dynamic library provides unified alloc/free functions accessible via `xllr_capi_loader.h`
- **Applies to**:
  - Strings (use `xllr_alloc_string8`, `xllr_alloc_string16`, `xllr_alloc_string32`)
  - Arrays (use `xllr_alloc_cdt_array`)
  - General memory (use `xllr_alloc_memory`)
  - Callable parameter/return type arrays (use `xllr_alloc_memory`)
- **Freeing**:
  - Strings: `xllr_free_string`
  - CDT arrays: `xllr_free_cdt_array`
  - General memory: `xllr_free_memory`

### Test Framework
- Tests use **doctest** framework
- All test files include `<doctest/doctest.h>`
- Main entry: `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`
- Test structure:
  ```cpp
  TEST_SUITE("CDTS [Language] Serializer") {
      TEST_CASE("Test name") {
          // Setup
          // Execute
          // Verify with CHECK() or REQUIRE()
          // Cleanup
      }
  }
  ```

### Language-Specific Adaptations

#### C++
- Uses `std::string`, `std::u16string`, `std::u32string` for strings
- Uses `std::vector<T>` for arrays
- Supports `std::variant` for ANY type extraction
- Operator overloading: `<<` for serialization, `>>` for deserialization
- RAII: Automatic memory management

#### C
- Uses `char*`, `char16_t*`, `char32_t*` for strings (manually allocated/freed)
- Uses raw arrays with explicit length tracking
- Function-based API: `cdts_ser_add_*`, `cdts_ser_get_*`
- Return codes: `CDTS_SER_SUCCESS`, `CDTS_SER_ERROR_*`
- Manual resource management: `cdts_ser_create`, `cdts_ser_destroy`

#### JVM
- Uses JNI types: `jint`, `jlong`, `jstring`, `jobject`, `jarray`, etc.
- Uses `jstring` for strings (UTF-16 native, supports UTF-8/UTF-16/UTF-32 conversion)
- Explicit type API for primitives: `add(jint val, metaffi_int32_type)` to prevent information loss
- Auto-detection for wrapper objects: `Integer` → `int32`, `Long` → `int64`, etc.
- Operator overloading: `<<` for serialization (jobject), typed extraction methods: `extract_int()`, `extract_string()`, etc.
- RAII: Automatic JNI local reference cleanup
- Global references: Only for handles (metaffi_handle_type)
- JVM initialization: Tests must initialize JVM using `JNI_CreateJavaVM`
- Multi-dimensional arrays: Currently not implemented (throws "not yet implemented")

### Test Coverage Checklist
Use this checklist when implementing a new serializer:

- [ ] All 8 integer types (int8/16/32/64, uint8/16/32/64)
- [ ] Both float types (float32, float64)
- [ ] Boolean type
- [ ] All string types supported by language
- [ ] Empty string edge case
- [ ] Multiple values in sequence
- [ ] Mixed type sequences
- [ ] 1D, 2D, 3D arrays
- [ ] Empty array edge case
- [ ] Ragged arrays
- [ ] Null values
- [ ] Handle serialization
- [ ] Callable serialization (with proper memory allocation)
- [ ] Type introspection (peek_type, is_null)
- [ ] Utility methods (get_index, set_index, reset, size, has_more)
- [ ] All error cases (type mismatch, bounds violations)
- [ ] All deserialization-only scenarios
- [ ] Pre-existing CDTS deserialization

### Approximate Test Count
- **Core serialization tests**: ~15 tests
- **String tests**: ~3-5 tests (language dependent)
- **Array tests**: ~6 tests
- **Special values**: ~3 tests
- **Type introspection**: ~2 tests (+ optional ANY tests for languages supporting it)
- **Utility methods**: ~1 test
- **Error handling**: ~4 tests
- **Deserialization-only**: ~11 tests
- **Total**: ~45-51 tests per language

## Future Language Implementations

When creating serializers for new languages:
1. Read this document thoroughly
2. Implement all applicable tests
3. Skip language-specific tests that don't apply (e.g., C++ `std::variant` tests for other languages)
4. Add language-specific tests if the language has unique features
5. Ensure all memory allocations use XLLR functions
6. Update this document if new test categories are discovered
