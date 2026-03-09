// xcall convention wrappers for c_guest_module.
// These are loaded by xllr.cpp via dlsym/GetProcAddress.
// Compiled into c_guest_module DLL automatically via collect_c_cpp_files.

#include <runtime/cdt.h>
#include <runtime/metaffi_primitives.h>
#include "c_guest_module.h"

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vector>

extern "C" C_GUEST_API
void xcall_c_no_op(void* /*ctx*/, char** /*err*/)
{
	c_guest_no_op();
}

extern "C" C_GUEST_API
void xcall_c_returns_an_error(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	// c_guest_returns_an_error returns int (e.g. -1 on error).
	// We expose it as an int32 return value rather than throwing.
	int v = c_guest_returns_an_error();

	params_ret[1].arr[0].type = metaffi_int32_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int32_val = static_cast<metaffi_int32>(v);
}

extern "C" C_GUEST_API
void xcall_c_hello_world(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	// No input; returns string8 in params_ret[1].arr[0]
	const char* result = c_guest_hello_world();

	static thread_local char buf[4096];
	strncpy(buf, result ? result : "", sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	params_ret[1].arr[0].type = metaffi_string8_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.string8_val = reinterpret_cast<char8_t*>(buf);
}

extern "C" C_GUEST_API
void xcall_c_div_integers(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	int64_t x = params_ret[0].arr[0].cdt_val.int64_val;
	int64_t y = params_ret[0].arr[1].cdt_val.int64_val;

	double result = c_guest_div_integers(x, y);

	params_ret[1].arr[0].type = metaffi_float64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.float64_val = result;
}

extern "C" C_GUEST_API
void xcall_c_get_counter(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	int64_t result = c_guest_get_counter();

	params_ret[1].arr[0].type = metaffi_int64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int64_val = result;
}

extern "C" C_GUEST_API
void xcall_c_set_counter(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	int64_t value = params_ret[0].arr[0].cdt_val.int64_val;
	c_guest_set_counter(value);
}

extern "C" C_GUEST_API
void xcall_c_inc_counter(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	int64_t delta = params_ret[0].arr[0].cdt_val.int64_val;
	int64_t result = c_guest_inc_counter(delta);

	params_ret[1].arr[0].type = metaffi_int64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int64_val = result;
}

// --- Typed scalar returns ---

extern "C" C_GUEST_API
void xcall_c_return_int8(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_int8_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int8_val = c_guest_return_int8();
}

extern "C" C_GUEST_API
void xcall_c_return_int16(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_int16_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int16_val = c_guest_return_int16();
}

extern "C" C_GUEST_API
void xcall_c_return_int32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_int32_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int32_val = c_guest_return_int32();
}

extern "C" C_GUEST_API
void xcall_c_return_int64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_int64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int64_val = c_guest_return_int64();
}

extern "C" C_GUEST_API
void xcall_c_return_uint8(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_uint8_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.uint8_val = c_guest_return_uint8();
}

extern "C" C_GUEST_API
void xcall_c_return_uint16(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_uint16_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.uint16_val = c_guest_return_uint16();
}

extern "C" C_GUEST_API
void xcall_c_return_uint32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_uint32_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.uint32_val = c_guest_return_uint32();
}

extern "C" C_GUEST_API
void xcall_c_return_uint64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_uint64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.uint64_val = c_guest_return_uint64();
}

extern "C" C_GUEST_API
void xcall_c_return_float32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_float32_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.float32_val = c_guest_return_float32();
}

extern "C" C_GUEST_API
void xcall_c_return_float64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_float64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.float64_val = c_guest_return_float64();
}

extern "C" C_GUEST_API
void xcall_c_return_bool(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	// c_guest_return_bool returns int; store as bool_val
	params_ret[1].arr[0].type = metaffi_bool_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.bool_val = c_guest_return_bool() ? 1 : 0;
}

extern "C" C_GUEST_API
void xcall_c_return_string(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	const char* result = c_guest_return_string();

	static thread_local char buf[4096];
	strncpy(buf, result ? result : "", sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	params_ret[1].arr[0].type = metaffi_string8_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.string8_val = reinterpret_cast<char8_t*>(buf);
}

// --- Typed scalar accepts ---

extern "C" C_GUEST_API
void xcall_c_accept_int8(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_int8(params_ret[0].arr[0].cdt_val.int8_val);
}

extern "C" C_GUEST_API
void xcall_c_accept_int16(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_int16(params_ret[0].arr[0].cdt_val.int16_val);
}

extern "C" C_GUEST_API
void xcall_c_accept_int32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_int32(params_ret[0].arr[0].cdt_val.int32_val);
}

extern "C" C_GUEST_API
void xcall_c_accept_int64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_int64(params_ret[0].arr[0].cdt_val.int64_val);
}

extern "C" C_GUEST_API
void xcall_c_accept_uint8(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_uint8(params_ret[0].arr[0].cdt_val.uint8_val);
}

extern "C" C_GUEST_API
void xcall_c_accept_uint16(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_uint16(params_ret[0].arr[0].cdt_val.uint16_val);
}

extern "C" C_GUEST_API
void xcall_c_accept_uint32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_uint32(params_ret[0].arr[0].cdt_val.uint32_val);
}

extern "C" C_GUEST_API
void xcall_c_accept_uint64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_uint64(params_ret[0].arr[0].cdt_val.uint64_val);
}

extern "C" C_GUEST_API
void xcall_c_accept_float32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_float32(params_ret[0].arr[0].cdt_val.float32_val);
}

extern "C" C_GUEST_API
void xcall_c_accept_float64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_float64(params_ret[0].arr[0].cdt_val.float64_val);
}

extern "C" C_GUEST_API
void xcall_c_accept_bool(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	c_guest_accept_bool(params_ret[0].arr[0].cdt_val.bool_val ? 1 : 0);
}

extern "C" C_GUEST_API
void xcall_c_accept_string(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	const char* s = reinterpret_cast<const char*>(params_ret[0].arr[0].cdt_val.string8_val);
	c_guest_accept_string(s ? s : "");
}

// --- Echo (round-trip) ---

extern "C" C_GUEST_API
void xcall_c_echo_int64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	int64_t result = c_guest_echo_int64(params_ret[0].arr[0].cdt_val.int64_val);

	params_ret[1].arr[0].type = metaffi_int64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int64_val = result;
}

extern "C" C_GUEST_API
void xcall_c_echo_float64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	double result = c_guest_echo_float64(params_ret[0].arr[0].cdt_val.float64_val);

	params_ret[1].arr[0].type = metaffi_float64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.float64_val = result;
}

extern "C" C_GUEST_API
void xcall_c_echo_bool(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	int result = c_guest_echo_bool(params_ret[0].arr[0].cdt_val.bool_val ? 1 : 0);

	params_ret[1].arr[0].type = metaffi_bool_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.bool_val = result ? 1 : 0;
}

extern "C" C_GUEST_API
void xcall_c_echo_string(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	const char* s = reinterpret_cast<const char*>(params_ret[0].arr[0].cdt_val.string8_val);

	// c_guest_echo_string heap-allocates the result — copy to thread_local buf then free
	const char* result = c_guest_echo_string(s ? s : "");
	static thread_local char buf[4096];
	strncpy(buf, result ? result : "", sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	free(const_cast<char*>(result));

	params_ret[1].arr[0].type = metaffi_string8_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.string8_val = reinterpret_cast<char8_t*>(buf);
}

// --- 1D int64 array helpers ---

extern "C" C_GUEST_API
void xcall_c_make_1d_int64_array(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	size_t out_len = 0;
	int64_t* data_raw = c_guest_make_1d_int64_array(&out_len);

	// Allocate packed array; copy data to our own malloc buffer
	auto* pa = static_cast<cdt_packed_array*>(malloc(sizeof(cdt_packed_array)));
	auto* data = static_cast<int64_t*>(malloc(out_len * sizeof(int64_t)));
	memcpy(data, data_raw, out_len * sizeof(int64_t));
	free(data_raw);

	pa->data = data;
	pa->length = static_cast<metaffi_size>(out_len);

	params_ret[1].arr[0].set_packed_array(pa, metaffi_int64_type);
	params_ret[1].arr[0].free_required = 0; // caller manages lifetime
}

extern "C" C_GUEST_API
void xcall_c_sum_1d_int64_array(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	cdt_packed_array* pa = params_ret[0].arr[0].get_packed_array();
	auto* data = static_cast<int64_t*>(pa->data);

	int64_t result = c_guest_sum_1d_int64_array(data, static_cast<size_t>(pa->length));

	params_ret[1].arr[0].type = metaffi_int64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int64_val = result;
}
