# MetaFFI → C++ Type Mapping

This document serves as the ground truth for converting MetaFFI types to C++ types.

## Type Mapping Table

| MetaFFI Type | C++ Type | Notes |
|--------------|----------|-------|
| `int8` | `int8_t` | 8-bit signed integer |
| `int16` | `int16_t` | 16-bit signed integer |
| `int32` | `int32_t`, `int` | 32-bit signed integer (platform-dependent for `int`) |
| `int64` | `int64_t`, `long long` | 64-bit signed integer |
| `uint8` | `uint8_t` | 8-bit unsigned integer |
| `uint16` | `uint16_t` | 16-bit unsigned integer |
| `uint32` | `uint32_t`, `unsigned int` | 32-bit unsigned integer (platform-dependent for `unsigned int`) |
| `uint64` | `uint64_t`, `unsigned long long` | 64-bit unsigned integer |
| `float32` | `float` | 32-bit floating point |
| `float64` | `double` | 64-bit floating point |
| `bool` | `bool` | Boolean value |
| `char8` | `char`, `char8_t`, `metaffi_char8` | UTF-8 character (see metaffi_primitives.h for struct definition) |
| `char16` | `char16_t`, `metaffi_char16` | UTF-16 character (see metaffi_primitives.h for struct definition) |
| `char32` | `char32_t`, `metaffi_char32` | UTF-32 character (see metaffi_primitives.h for struct definition) |
| `string8` | `std::string`, `const char*`, `metaffi_string8` | UTF-8 string |
| `string16` | `std::u16string`, `metaffi_string16` | UTF-16 string |
| `string32` | `std::u32string`, `metaffi_string32` | UTF-32 string |
| `null` | `nullptr`, `std::nullopt` | Null value |
| `handle` | `void*`, `T*`, `std::shared_ptr<T>`, `metaffi_handle` | Generic object reference |
| `callable` | `metaffi_callable`, `std::function` | Callback function pointer |
| `any` | `std::any`, `std::variant`, `metaffi_variant` | Runtime-determined type |
| `*_array` | `std::vector<T>`, `T*`, `T[]`, `std::array<T,N>` | Arrays/containers |
| `uint8_array` (1D) | `std::vector<uint8_t>`, `std::span<uint8_t>` | Binary data, special case |
| `size` | `size_t`, `metaffi_size` | Size type for arrays/strings |

## Handle Type Alias

For `handle` and `handle_array` types, use the `type_alias` field to store the C++ type information:
- For primitive types: Use standard type names (`"int64_t"`, `"double"`, `"std::string"`)
- For class types: Use `typeid(T).name()` or custom demangled type names (e.g., `"MyClass*"`, `"std::map<std::string,int>"`)
- For `void*` handles: May leave `type_alias` empty or specify as `"void*"`

## Special Notes

### Character Types
`metaffi_char8`, `metaffi_char16`, and `metaffi_char32` are **structs** defined in `sdk/runtime/metaffi_primitives.h`, not simple primitive types. They handle multi-byte UTF-8/16/32 characters correctly:
- `metaffi_char8`: Contains up to 4 bytes for UTF-8
- `metaffi_char16`: Contains up to 2 char16_t for UTF-16 surrogate pairs
- `metaffi_char32`: Single char32_t for UTF-32

### Platform-Dependent Types
- `int` and `unsigned int` are platform-dependent (typically 32-bit)
- For portable code, prefer fixed-width types (`int32_t`, `uint32_t`, etc.)

### Array Dimensions
Array types can have multiple dimensions specified via the `dimensions` field:
- `dimensions: 0` → Scalar value
- `dimensions: 1` → 1D array (e.g., `std::vector<int64_t>`)
- `dimensions: 2` → 2D array (e.g., `std::vector<std::vector<int64_t>>`)
- `dimensions: 3+` → Higher-dimensional arrays

### Binary Data
`uint8_array` with `dimensions: 1` is often used for binary data and maps to:
- `std::vector<uint8_t>` (most common)
- `std::span<uint8_t>` (C++20 view)
- `unsigned char*` (raw pointer)

## Implementation Reference

The actual type conversion and serialization is implemented in:
- **IDL type mapping**: `sdk/idl_entities/cpp/include/metaffi/idl/type_mapper.hpp`
- **Runtime primitives**: `sdk/runtime/metaffi_primitives.h`
- **CDTS serialization** (C++ serializer): To be implemented in `sdk/cdts_serializer/cpp/`

## Usage in IDL Files

When creating IDL files for C++, use these type strings in the `"type"` field:

```json
{
  "name": "my_function",
  "parameters": [
    {"name": "count", "type": "int64", "type_alias": "int64_t"},
    {"name": "name", "type": "string8", "type_alias": "std::string"},
    {"name": "data", "type": "uint8_array", "type_alias": "std::vector<uint8_t>", "dimensions": 1}
  ],
  "return_values": [
    {"name": "result", "type": "handle", "type_alias": "MyClass*"}
  ]
}
```

## Alignment with Runtime

These types align with the `metaffi_types` enum defined in `sdk/runtime/metaffi_primitives.h`, which uses bit-flag values (1ULL, 2ULL, 4ULL, etc.) for runtime type identification.
