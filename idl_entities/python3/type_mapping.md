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
| `handle` | `Any` | Generic object reference (type annotation) |
| `callable` | `Any` | Callable reference (type annotation) |
| `any` | `Any` | Runtime-determined type |
| `*_array` (1D) | `List[T]` | 1D arrays → `List[base_type]` |
| `*_array` (2D) | `List[List[T]]` | 2D arrays → nested lists |
| `*_array` (3D+) | `List[List[List[...]]]` | Higher-dimensional nested lists |
| `uint8_array` (1D) | `bytes` | Special case: 1D uint8 array → bytes |
| `size` | `int` | Size type |

## Array Handling

The Python3 host compiler handles multi-dimensional arrays by nesting `List` type annotations:

- **1D arrays**: `int64_array` (dims=1) → `List[int]`
- **2D arrays**: `int64_array` (dims=2) → `List[List[int]]`
- **3D arrays**: `int64_array` (dims=3) → `List[List[List[int]]]`
- **Special case**: `uint8_array` (dims=1) → `bytes` (optimized for binary data)

Array types with `_array` suffix (e.g., `int64_array`) are automatically converted to `List[base_type]` in generated code.

## Handle Types

Handle types represent generic object references:
- **Type annotation**: Always mapped to `Any` for flexibility
- **Type alias**: The `type_alias` field in IDL is preserved but only used for documentation/debugging
- **Runtime behavior**: Actual Python type is determined at runtime by the MetaFFI runtime

## Generated Code Examples

### Simple Function
**MetaFFI IDL:**
```json
{
  "name": "add_int64",
  "parameters": [
    {"name": "a", "type": "int64", "type_alias": "int64_t"},
    {"name": "b", "type": "int64", "type_alias": "int64_t"}
  ],
  "return_values": [
    {"name": "result", "type": "int64", "type_alias": "int64_t"}
  ]
}
```

**Generated Python:**
```python
def add_int64(a: int, b: int) -> int:
    """Generated stub for add_int64"""
    # Use cached MetaFFIEntity to make the call
    result = add_int64_caller(a, b)
    return result
```

### Array Function
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

**Generated Python:**
```python
def sum_array(numbers: List[int]) -> int:
    """Generated stub for sum_array"""
    # Use cached MetaFFIEntity to make the call
    result = sum_array_caller(numbers)
    return result
```

### Multi-Dimensional Array
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

**Generated Python:**
```python
def transpose_matrix(matrix: List[List[int]]) -> List[List[int]]:
    """Generated stub for transpose_matrix"""
    # Use cached MetaFFIEntity to make the call
    result = transpose_matrix_caller(matrix)
    return result
```

### Handle and Any Types
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

**Generated Python:**
```python
def process_data(data: Any, config: Any) -> Any:
    """Generated stub for process_data"""
    # Use cached MetaFFIEntity to make the call
    result = process_data_caller(data, config)
    return result
```

## Handle Runtime ID

When deserializing a `handle`, check the `runtime_id`:
- If `runtime_id == PYTHON3_RUNTIME_ID`: Unwrap to original PyObject*
  - PYTHON3_RUNTIME_ID defined in the python3 cdts serializer
- Otherwise: Wrap in `MetaFFIHandle` Python class

## Implementation Reference

The actual type conversion is implemented in:
- `sdk/cdts_serializer/cpython3/cdts_python3_serializer.cpp` - C++ serialization
- `sdk/idl_entities/python3/type_mapper.py` - IDL type mapping
