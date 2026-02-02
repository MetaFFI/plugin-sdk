#pragma once

#include "cdt.h"

namespace test_plugin
{

//----------------------------------------------------------------------
// NO_PARAMS_NO_RET handlers
//----------------------------------------------------------------------
void handler_no_op(cdts* data, char** out_err);
void handler_print_hello(cdts* data, char** out_err);

//----------------------------------------------------------------------
// NO_PARAMS_RET handlers - Return primitives
//----------------------------------------------------------------------
void handler_return_int8(cdts* data, char** out_err);
void handler_return_int16(cdts* data, char** out_err);
void handler_return_int32(cdts* data, char** out_err);
void handler_return_int64(cdts* data, char** out_err);
void handler_return_uint8(cdts* data, char** out_err);
void handler_return_uint16(cdts* data, char** out_err);
void handler_return_uint32(cdts* data, char** out_err);
void handler_return_uint64(cdts* data, char** out_err);
void handler_return_float32(cdts* data, char** out_err);
void handler_return_float64(cdts* data, char** out_err);
void handler_return_bool_true(cdts* data, char** out_err);
void handler_return_bool_false(cdts* data, char** out_err);
void handler_return_string8(cdts* data, char** out_err);
void handler_return_null(cdts* data, char** out_err);

//----------------------------------------------------------------------
// PARAMS_NO_RET handlers - Accept primitives
//----------------------------------------------------------------------
void handler_accept_int8(cdts* data, char** out_err);
void handler_accept_int16(cdts* data, char** out_err);
void handler_accept_int32(cdts* data, char** out_err);
void handler_accept_int64(cdts* data, char** out_err);
void handler_accept_float32(cdts* data, char** out_err);
void handler_accept_float64(cdts* data, char** out_err);
void handler_accept_bool(cdts* data, char** out_err);
void handler_accept_string8(cdts* data, char** out_err);

//----------------------------------------------------------------------
// PARAMS_RET handlers - Echo functions
//----------------------------------------------------------------------
void handler_echo_int64(cdts* data, char** out_err);
void handler_echo_float64(cdts* data, char** out_err);
void handler_echo_string8(cdts* data, char** out_err);
void handler_echo_bool(cdts* data, char** out_err);

//----------------------------------------------------------------------
// PARAMS_RET handlers - Arithmetic
//----------------------------------------------------------------------
void handler_add_int64(cdts* data, char** out_err);
void handler_add_float64(cdts* data, char** out_err);
void handler_concat_strings(cdts* data, char** out_err);

//----------------------------------------------------------------------
// Array handlers
//----------------------------------------------------------------------
void handler_return_int64_array_1d(cdts* data, char** out_err);
void handler_return_int64_array_2d(cdts* data, char** out_err);
void handler_return_int64_array_3d(cdts* data, char** out_err);
void handler_return_ragged_array(cdts* data, char** out_err);
void handler_return_string_array(cdts* data, char** out_err);
void handler_sum_int64_array(cdts* data, char** out_err);
void handler_echo_int64_array(cdts* data, char** out_err);
void handler_join_strings(cdts* data, char** out_err);

//----------------------------------------------------------------------
// Handle handlers
//----------------------------------------------------------------------
void handler_create_handle(cdts* data, char** out_err);
void handler_get_handle_data(cdts* data, char** out_err);
void handler_set_handle_data(cdts* data, char** out_err);
void handler_release_handle(cdts* data, char** out_err);

//----------------------------------------------------------------------
// Callable handlers
//----------------------------------------------------------------------
void handler_call_callback_add(cdts* data, char** out_err);
void handler_call_callback_string(cdts* data, char** out_err);
void handler_return_adder_callback(cdts* data, char** out_err);

//----------------------------------------------------------------------
// Error handling handlers
//----------------------------------------------------------------------
void handler_throw_error(cdts* data, char** out_err);
void handler_throw_with_message(cdts* data, char** out_err);
void handler_error_if_negative(cdts* data, char** out_err);

//----------------------------------------------------------------------
// Any type handler
//----------------------------------------------------------------------
void handler_accept_any(cdts* data, char** out_err);

//----------------------------------------------------------------------
// Multiple return values handlers
//----------------------------------------------------------------------
void handler_return_two_values(cdts* data, char** out_err);
void handler_return_three_values(cdts* data, char** out_err);
void handler_swap_values(cdts* data, char** out_err);

//----------------------------------------------------------------------
// TestHandle class method handlers
//----------------------------------------------------------------------
void handler_get_handle_id(cdts* data, char** out_err);
void handler_append_to_data(cdts* data, char** out_err);

//----------------------------------------------------------------------
// Global variable handlers
//----------------------------------------------------------------------
void handler_get_g_name(cdts* data, char** out_err);
void handler_set_g_name(cdts* data, char** out_err);

} // namespace test_plugin
