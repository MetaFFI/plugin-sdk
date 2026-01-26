# MetaFFI → Python3 Type Mapping

This document serves as the ground truth for converting MetaFFI types to Python3 types.

**Note:** Python3 → MetaFFI serialization requires explicit type specification by the caller
(e.g., `add(py_int, metaffi_int32_type)`), since Python's `int` and `float` don't specify size.

## Type Mapping Table

| MetaFFI Type | Python3 Type | Notes |
|--------------|--------------|-------|
| `int8` | `int` | Python int handles all integer sizes |
| `int16` | `int` | |
| `int32` | `int` | |
| `int64` | `int` | |
| `uint8` | `int` | |
| `uint16` | `int` | |
| `uint32` | `int` | |
| `uint64` | `int` | |
| `float32` | `float` | Python float is 64-bit, may lose precision |
| `float64` | `float` | |
| `bool` | `bool` | |
| `char8` | `str` | Single UTF-8 character |
| `char16` | `str` | Single UTF-16 character |
| `char32` | `str` | Single UTF-32 character |
| `string8` | `str` | UTF-8 decoded |
| `string16` | `str` | UTF-16 decoded |
| `string32` | `str` | UTF-32 decoded |
| `null` | `None` | |
| `handle` | `object` | Unwrapped if Python handle, else wrapped |
| `callable` | `MetaFFICallable` | Python callable wrapper class |
| `any` | varies | Type determined at runtime |
| `*_array` | `list` | Arrays become Python lists |
| `uint8_array` (1D) | `bytes` | Special case: 1D uint8 array → bytes |

## Handle Runtime ID

When deserializing a `handle`, check the `runtime_id`:
- If `runtime_id == PYTHON3_RUNTIME_ID`: Unwrap to original PyObject*
- Otherwise: Wrap in `MetaFFIHandle` Python class

## Implementation Reference

The actual type conversion is implemented in:
- `sdk/cdts_serializer/cpython3/cdts_python3_serializer.cpp` - C++ serialization
- `sdk/idl_entities/python3/type_mapper.py` - IDL type mapping
