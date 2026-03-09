// xcall convention wrappers for cpp_guest_module.
// These are loaded by xllr.cpp via dlsym/GetProcAddress.
// Compiled into cpp_guest_module DLL automatically via collect_c_cpp_files.

#include <runtime/cdt.h>
#include <runtime/metaffi_primitives.h>
#include "cpp_guest_module.h"

#include <cstdlib>
#include <cstring>
#include <utils/safe_func.h>
#include <stdexcept>
#include <vector>

extern "C" CPP_GUEST_MODULE_API
void xcall_no_op(void* /*ctx*/, char** /*err*/)
{
	guest::no_op();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_returns_an_error(void* /*ctx*/, char** err)
{
	try
	{
		guest::returns_an_error();
	}
	catch (const std::exception& e)
	{
		// Must malloc — xllr_free_string calls free() on the error pointer
		const char* msg = e.what();
		const std::size_t len = std::strlen(msg);
		char* buf = static_cast<char*>(malloc(len + 1));
		if (buf) std::memcpy(buf, msg, len + 1);
		*err = buf;
	}
}

extern "C" CPP_GUEST_MODULE_API
void xcall_hello_world(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	// No input; returns string8 in params_ret[1].arr[0]
	std::string result = guest::hello_world();

	static thread_local char buf[4096];
	metaffi_strncpy(buf, sizeof(buf), result.c_str(), sizeof(buf) - 1);

	params_ret[1].arr[0].type = metaffi_string8_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.string8_val = reinterpret_cast<char8_t*>(buf);
}

extern "C" CPP_GUEST_MODULE_API
void xcall_div_integers(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	// params_ret[0]: 2 int64; params_ret[1]: 1 float64
	int64_t x = params_ret[0].arr[0].cdt_val.int64_val;
	int64_t y = params_ret[0].arr[1].cdt_val.int64_val;

	double result = guest::div_integers(x, y);

	params_ret[1].arr[0].type = metaffi_float64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.float64_val = result;
}

extern "C" CPP_GUEST_MODULE_API
void xcall_get_counter(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	// No input; returns int64
	int64_t result = guest::get_counter();

	params_ret[1].arr[0].type = metaffi_int64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int64_val = result;
}

extern "C" CPP_GUEST_MODULE_API
void xcall_set_counter(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	// params_ret[0]: 1 int64; no return
	int64_t value = params_ret[0].arr[0].cdt_val.int64_val;
	guest::set_counter(value);
}

extern "C" CPP_GUEST_MODULE_API
void xcall_inc_counter(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	// params_ret[0]: 1 int64 delta; returns int64
	int64_t delta = params_ret[0].arr[0].cdt_val.int64_val;
	int64_t result = guest::inc_counter(delta);

	params_ret[1].arr[0].type = metaffi_int64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int64_val = result;
}

// --- Typed scalar returns ---

extern "C" CPP_GUEST_MODULE_API
void xcall_return_int8(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_int8_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int8_val = guest::return_int8();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_int16(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_int16_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int16_val = guest::return_int16();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_int32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_int32_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int32_val = guest::return_int32();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_int64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_int64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int64_val = guest::return_int64();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_uint8(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_uint8_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.uint8_val = guest::return_uint8();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_uint16(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_uint16_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.uint16_val = guest::return_uint16();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_uint32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_uint32_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.uint32_val = guest::return_uint32();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_uint64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_uint64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.uint64_val = guest::return_uint64();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_float32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_float32_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.float32_val = guest::return_float32();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_float64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_float64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.float64_val = guest::return_float64();
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_bool(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	params_ret[1].arr[0].type = metaffi_bool_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.bool_val = guest::return_bool() ? 1 : 0;
}

extern "C" CPP_GUEST_MODULE_API
void xcall_return_string(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	std::string result = guest::return_string();
	static thread_local char buf[4096];
	metaffi_strncpy(buf, sizeof(buf), result.c_str(), sizeof(buf) - 1);

	params_ret[1].arr[0].type = metaffi_string8_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.string8_val = reinterpret_cast<char8_t*>(buf);
}

// --- Typed scalar accepts ---

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_int8(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_int8(static_cast<int8_t>(params_ret[0].arr[0].cdt_val.int8_val));
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_int16(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_int16(static_cast<int16_t>(params_ret[0].arr[0].cdt_val.int16_val));
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_int32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_int32(static_cast<int32_t>(params_ret[0].arr[0].cdt_val.int32_val));
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_int64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_int64(params_ret[0].arr[0].cdt_val.int64_val);
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_uint8(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_uint8(params_ret[0].arr[0].cdt_val.uint8_val);
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_uint16(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_uint16(params_ret[0].arr[0].cdt_val.uint16_val);
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_uint32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_uint32(params_ret[0].arr[0].cdt_val.uint32_val);
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_uint64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_uint64(params_ret[0].arr[0].cdt_val.uint64_val);
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_float32(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_float32(params_ret[0].arr[0].cdt_val.float32_val);
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_float64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_float64(params_ret[0].arr[0].cdt_val.float64_val);
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_bool(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	guest::accept_bool(params_ret[0].arr[0].cdt_val.bool_val != 0);
}

extern "C" CPP_GUEST_MODULE_API
void xcall_accept_string(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	const char* s = reinterpret_cast<const char*>(params_ret[0].arr[0].cdt_val.string8_val);
	guest::accept_string(std::string(s ? s : ""));
}

// --- Echo (round-trip) ---

extern "C" CPP_GUEST_MODULE_API
void xcall_echo_int64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	int64_t val = params_ret[0].arr[0].cdt_val.int64_val;
	int64_t result = guest::echo_int64(val);

	params_ret[1].arr[0].type = metaffi_int64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int64_val = result;
}

extern "C" CPP_GUEST_MODULE_API
void xcall_echo_float64(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	double val = params_ret[0].arr[0].cdt_val.float64_val;
	double result = guest::echo_float64(val);

	params_ret[1].arr[0].type = metaffi_float64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.float64_val = result;
}

extern "C" CPP_GUEST_MODULE_API
void xcall_echo_bool(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	bool val = params_ret[0].arr[0].cdt_val.bool_val != 0;
	bool result = guest::echo_bool(val);

	params_ret[1].arr[0].type = metaffi_bool_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.bool_val = result ? 1 : 0;
}

extern "C" CPP_GUEST_MODULE_API
void xcall_echo_string(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	const char* s = reinterpret_cast<const char*>(params_ret[0].arr[0].cdt_val.string8_val);
	std::string result = guest::echo_string(std::string(s ? s : ""));

	static thread_local char buf[4096];
	metaffi_strncpy(buf, sizeof(buf), result.c_str(), sizeof(buf) - 1);

	params_ret[1].arr[0].type = metaffi_string8_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.string8_val = reinterpret_cast<char8_t*>(buf);
}

// --- 1D int64 array helpers ---

extern "C" CPP_GUEST_MODULE_API
void xcall_make_1d_int64_array(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	// No input; returns packed int64 array in params_ret[1].arr[0]
	std::vector<int64_t> result = guest::make_1d_int64_array();

	// Allocate packed array and its data buffer via malloc
	auto* pa = static_cast<cdt_packed_array*>(malloc(sizeof(cdt_packed_array)));
	auto* data = static_cast<int64_t*>(malloc(result.size() * sizeof(int64_t)));
	for (size_t i = 0; i < result.size(); ++i)
	{
		data[i] = result[i];
	}
	pa->data = data;
	pa->length = static_cast<metaffi_size>(result.size());

	params_ret[1].arr[0].set_packed_array(pa, metaffi_int64_type);
	params_ret[1].arr[0].free_required = 0; // caller manages lifetime
}

extern "C" CPP_GUEST_MODULE_API
void xcall_sum_1d_int64_array(void* /*ctx*/, cdts* params_ret, char** /*err*/)
{
	// params_ret[0]: packed int64 array; returns int64
	cdt_packed_array* pa = params_ret[0].arr[0].get_packed_array();
	auto* data = static_cast<int64_t*>(pa->data);
	std::vector<int64_t> arr(data, data + pa->length);

	int64_t result = guest::sum_1d_int64_array(arr);

	params_ret[1].arr[0].type = metaffi_int64_type;
	params_ret[1].arr[0].free_required = 0;
	params_ret[1].arr[0].cdt_val.int64_val = result;
}
