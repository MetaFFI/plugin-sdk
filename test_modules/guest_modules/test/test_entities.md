# Test Plugin Entities

This document describes all entities exposed by the test plugin `xllr.test`.

## Overview

The test plugin is designed for integration testing of the MetaFFI SDK APIs. It provides:
- Comprehensive coverage of all primitive types
- All four xcall variants (params+ret, params-only, ret-only, neither)
- Array support (1D, 2D, 3D, ragged)
- Handle (opaque object) support
- Callable (callback) support
- Error handling scenarios
- Dynamic "any" type support

## Entity Path Format

All entities are accessed via `test::<entity_name>` paths.

## Logging Behavior

- **STDOUT**: All normal operations are logged with `[xllr.test]` prefix
- **STDERR**: All errors are logged with `[xllr.test] ERROR:` prefix

---

## Primitives - No Params, No Return

| Entity | Signature | Description |
|--------|-----------|-------------|
| `test::no_op` | `() -> void` | Does nothing, just logs |
| `test::print_hello` | `() -> void` | Prints "Hello from test plugin!" to STDOUT |

---

## Primitives - Return Values Only

| Entity | Signature | Returns |
|--------|-----------|---------|
| `test::return_int8` | `() -> int8` | `42` |
| `test::return_int16` | `() -> int16` | `1000` |
| `test::return_int32` | `() -> int32` | `100000` |
| `test::return_int64` | `() -> int64` | `9223372036854775807` |
| `test::return_uint8` | `() -> uint8` | `255` |
| `test::return_uint16` | `() -> uint16` | `65535` |
| `test::return_uint32` | `() -> uint32` | `4294967295` |
| `test::return_uint64` | `() -> uint64` | `18446744073709551615` |
| `test::return_float32` | `() -> float32` | `3.14159` |
| `test::return_float64` | `() -> float64` | `3.141592653589793` |
| `test::return_bool_true` | `() -> bool` | `true` |
| `test::return_bool_false` | `() -> bool` | `false` |
| `test::return_string8` | `() -> string8` | `"Hello from test plugin"` |
| `test::return_null` | `() -> null` | `null` |

---

## Primitives - Accept Values Only

| Entity | Signature | Behavior |
|--------|-----------|----------|
| `test::accept_int8` | `(int8) -> void` | Logs received value to STDOUT |
| `test::accept_int16` | `(int16) -> void` | Logs received value to STDOUT |
| `test::accept_int32` | `(int32) -> void` | Logs received value to STDOUT |
| `test::accept_int64` | `(int64) -> void` | Logs received value to STDOUT |
| `test::accept_float32` | `(float32) -> void` | Logs received value to STDOUT |
| `test::accept_float64` | `(float64) -> void` | Logs received value to STDOUT |
| `test::accept_bool` | `(bool) -> void` | Logs received value to STDOUT |
| `test::accept_string8` | `(string8) -> void` | Logs received value to STDOUT |

---

## Echo Functions

| Entity | Signature | Behavior |
|--------|-----------|----------|
| `test::echo_int64` | `(int64) -> int64` | Returns input unchanged |
| `test::echo_float64` | `(float64) -> float64` | Returns input unchanged |
| `test::echo_string8` | `(string8) -> string8` | Returns input unchanged |
| `test::echo_bool` | `(bool) -> bool` | Returns input unchanged |

---

## Arithmetic Functions

| Entity | Signature | Behavior |
|--------|-----------|----------|
| `test::add_int64` | `(int64, int64) -> int64` | Returns `a + b` |
| `test::add_float64` | `(float64, float64) -> float64` | Returns `a + b` |
| `test::concat_strings` | `(string8, string8) -> string8` | Returns concatenation of both strings |

---

## Arrays

### Return Arrays

| Entity | Signature | Returns |
|--------|-----------|---------|
| `test::return_int64_array_1d` | `() -> int64[]` | `[1, 2, 3]` |
| `test::return_int64_array_2d` | `() -> int64[][]` | `[[1, 2], [3, 4]]` |
| `test::return_int64_array_3d` | `() -> int64[][][]` | `[[[1,2],[3,4]], [[5,6],[7,8]]]` |
| `test::return_ragged_array` | `() -> int64[][]` | `[[1, 2, 3], [4], [5, 6]]` |
| `test::return_string_array` | `() -> string8[]` | `["one", "two", "three"]` |

### Accept/Process Arrays

| Entity | Signature | Behavior |
|--------|-----------|----------|
| `test::sum_int64_array` | `(int64[]) -> int64` | Returns sum of all array elements |
| `test::echo_int64_array` | `(int64[]) -> int64[]` | Returns the input array unchanged |
| `test::join_strings` | `(string8[]) -> string8` | Joins strings with comma separator |

---

## Handles (Opaque Objects)

The test plugin manages `TestHandle` objects with `id` and `data` fields.

| Entity | Signature | Description |
|--------|-----------|-------------|
| `test::create_handle` | `() -> handle` | Creates a new TestHandle with auto-incremented id and data="test_data" |
| `test::get_handle_data` | `(handle) -> string8` | Returns the `data` field of the handle |
| `test::set_handle_data` | `(handle, string8) -> void` | Sets the `data` field of the handle |
| `test::release_handle` | `(handle) -> void` | Logs release request (actual release via handle's release callback) |

---

## Callables (Callbacks)

| Entity | Signature | Description |
|--------|-----------|-------------|
| `test::call_callback_add` | `(callable) -> int64` | Calls the callback with `(3, 4)`, expects callback to return sum (7) |
| `test::call_callback_string` | `(callable) -> string8` | Calls the callback with `"test"`, returns callback result |
| `test::return_adder_callback` | `() -> callable` | Returns a callback that adds two int64 values |

---

## Error Handling

| Entity | Signature | Description |
|--------|-----------|-------------|
| `test::throw_error` | `() -> void` | Always returns error: "Test error thrown intentionally" |
| `test::throw_with_message` | `(string8) -> void` | Returns error with the provided message |
| `test::error_if_negative` | `(int64) -> void` | Returns error if input < 0, otherwise succeeds |

---

## Any Type (Dynamic Type at Runtime)

### `test::accept_any`

**Signature:** `(any) -> any`

This entity accepts a single parameter with `metaffi_any_type`. At runtime, the actual type in the CDTS determines behavior. It returns the same type as the input, but with a different value. The return type is also `metaffi_any_type`, with the actual return type matching the input type at runtime.

**Supported Actual Types:**

| Actual Type | Input Value | Returns (same type, different value) |
|-------------|-------------|--------------------------------------|
| `int64` | `42` | `142` (input + 100) |
| `float64` | `3.14` | `6.28` (input * 2.0) |
| `string8` | `"hello"` | `"echoed: hello"` |
| `int32[]` | `[1, 2, 3]` | `[4, 5, 6]` |
| `int64[]` | `[1, 2, 3]` | `[10, 20, 30]` |

**Usage Instructions for Callers:**

1. Load entity with:
   - `params_types = [{type: metaffi_any_type}]`
   - `retval_types = [{type: metaffi_string8_type}]`

2. Call the entity 4 times, each time populating `CDTS[0]` with a different actual type:

   **Call 1 (int64):**
   ```cpp
   params[0].type = metaffi_int64_type;
   params[0].cdt_val.int64_val = 42;
   ```

   **Call 2 (float64):**
   ```cpp
   params[0].type = metaffi_float64_type;
   params[0].cdt_val.float64_val = 3.14;
   ```

   **Call 3 (string8):**
   ```cpp
   params[0].type = metaffi_string8_type;
   params[0].cdt_val.string8_val = (metaffi_string8)"hello";
   ```

   **Call 4 (int32[]):**
   ```cpp
   params[0].set_new_array(3, 1, metaffi_int32_type);
   (*params[0].cdt_val.array_val)[0] = cdt((metaffi_int32)1);
   (*params[0].cdt_val.array_val)[1] = cdt((metaffi_int32)2);
   (*params[0].cdt_val.array_val)[2] = cdt((metaffi_int32)3);
   ```

3. Each call returns a string describing what was received.

4. If an unexpected type is received, the entity logs an error to STDERR and sets the error output parameter.

---

## Multiple Return Values

| Entity | Signature | Returns |
|--------|-----------|---------|
| `test::return_two_values` | `() -> (int64, string8)` | `(42, "answer")` |
| `test::return_three_values` | `() -> (int64, float64, bool)` | `(1, 2.5, true)` |
| `test::swap_values` | `(int64, string8) -> (string8, int64)` | Swaps the two input values |

---

## Type Constants Reference

For `metaffi_type_info.type` field:

| Type | Value |
|------|-------|
| `metaffi_int8_type` | `4` |
| `metaffi_int16_type` | `8` |
| `metaffi_int32_type` | `16` |
| `metaffi_int64_type` | `32` |
| `metaffi_uint8_type` | `64` |
| `metaffi_uint16_type` | `128` |
| `metaffi_uint32_type` | `256` |
| `metaffi_uint64_type` | `512` |
| `metaffi_float32_type` | `2` |
| `metaffi_float64_type` | `1` |
| `metaffi_bool_type` | `1024` |
| `metaffi_string8_type` | `4096` |
| `metaffi_handle_type` | `32768` |
| `metaffi_array_type` | `65536` |
| `metaffi_any_type` | `4194304` |
| `metaffi_null_type` | `8388608` |
| `metaffi_callable_type` | `16777216` |

Array types are created by OR-ing with `metaffi_array_type`:
- `metaffi_int64_array_type` = `metaffi_int64_type | metaffi_array_type` = `32 | 65536` = `65568`
