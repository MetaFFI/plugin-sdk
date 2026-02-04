#include "entity_handlers.h"
#include "logging.h"
#include "xcall.h"
#include "xllr_capi_loader.h"
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <limits>
#include <new>

namespace test_plugin
{

// Helper to set error message
static void set_error(char** out_err, const std::string& msg)
{
	if(out_err)
	{
		*out_err = xllr_alloc_string(msg.c_str(), msg.size());
	}
}

// Test handle structure
struct TestHandle
{
	int64_t id;
	std::string data;

	TestHandle(int64_t i, const std::string& d) : id(i), data(d) {}
};

// Global handle counter
static int64_t g_next_handle_id = 1;

// Global name variable for testing globals
static std::string g_name = "default_name";

// Test runtime ID for handles
static const uint64_t TEST_RUNTIME_ID = 0xDEADBEEF;

//======================================================================
// NO_PARAMS_NO_RET handlers
//======================================================================

void handler_no_op(cdts* data, char** out_err)
{
	log_entity("test::no_op", "executed (no-op)");
}

void handler_print_hello(cdts* data, char** out_err)
{
	METAFFI_INFO(LOG, "Hello from test plugin!");
	log_entity("test::print_hello", "printed hello message");
}

//======================================================================
// NO_PARAMS_RET handlers - Return primitives
//======================================================================

void handler_return_int8(cdts* data, char** out_err)
{
	// data[1] is the return values cdts - use type-specific assignment
	data[1].arr[0] = static_cast<metaffi_int8>(42);
	log_entity("test::return_int8", "returning 42");
}

void handler_return_int16(cdts* data, char** out_err)
{
	data[1].arr[0] = static_cast<metaffi_int16>(1000);
	log_entity("test::return_int16", "returning 1000");
}

void handler_return_int32(cdts* data, char** out_err)
{
	data[1].arr[0] = static_cast<metaffi_int32>(100000);
	log_entity("test::return_int32", "returning 100000");
}

void handler_return_int64(cdts* data, char** out_err)
{
	data[1].arr[0] = static_cast<metaffi_int64>(9223372036854775807LL);
	log_entity("test::return_int64", "returning 9223372036854775807");
}

void handler_return_uint8(cdts* data, char** out_err)
{
	data[1].arr[0] = static_cast<metaffi_uint8>(255);
	log_entity("test::return_uint8", "returning 255");
}

void handler_return_uint16(cdts* data, char** out_err)
{
	data[1].arr[0] = static_cast<metaffi_uint16>(65535);
	log_entity("test::return_uint16", "returning 65535");
}

void handler_return_uint32(cdts* data, char** out_err)
{
	data[1].arr[0] = static_cast<metaffi_uint32>(4294967295U);
	log_entity("test::return_uint32", "returning 4294967295");
}

void handler_return_uint64(cdts* data, char** out_err)
{
	data[1].arr[0] = static_cast<metaffi_uint64>(18446744073709551615ULL);
	log_entity("test::return_uint64", "returning 18446744073709551615");
}

void handler_return_float32(cdts* data, char** out_err)
{
	data[1].arr[0] = static_cast<metaffi_float32>(3.14159f);
	log_entity("test::return_float32", "returning 3.14159");
}

void handler_return_float64(cdts* data, char** out_err)
{
	data[1].arr[0] = static_cast<metaffi_float64>(3.141592653589793);
	log_entity("test::return_float64", "returning 3.141592653589793");
}

void handler_return_bool_true(cdts* data, char** out_err)
{
	data[1].arr[0] = true;
	log_entity("test::return_bool_true", "returning true");
}

void handler_return_bool_false(cdts* data, char** out_err)
{
	data[1].arr[0] = false;
	log_entity("test::return_bool_false", "returning false");
}

void handler_return_string8(cdts* data, char** out_err)
{
	data[1].arr[0].set_string(reinterpret_cast<const char8_t*>("Hello from test plugin"), true);
	log_entity("test::return_string8", "returning \"Hello from test plugin\"");
}

void handler_return_null(cdts* data, char** out_err)
{
	data[1].arr[0].type = metaffi_null_type;
	log_entity("test::return_null", "returning null");
}

//======================================================================
// PARAMS_NO_RET handlers - Accept primitives
//======================================================================

void handler_accept_int8(cdts* data, char** out_err)
{
	cdt& param = data[0].arr[0];
	if(param.type != metaffi_int8_type)
	{
		const char* type_str = get_type_name(param.type);
		std::string error_msg = "test::accept_int8: expected int8, got " + std::string(type_str);
		log_error("test::accept_int8", error_msg);
		set_error(out_err, error_msg);
		return;
	}
	metaffi_int8 val = param.cdt_val.int8_val;
	log_entity("test::accept_int8", "received " + std::to_string(val));
}

void handler_accept_int16(cdts* data, char** out_err)
{
	cdt& param = data[0].arr[0];
	if(param.type != metaffi_int16_type)
	{
		const char* type_str = get_type_name(param.type);
		std::string error_msg = "test::accept_int16: expected int16, got " + std::string(type_str);
		log_error("test::accept_int16", error_msg);
		set_error(out_err, error_msg);
		return;
	}
	metaffi_int16 val = param.cdt_val.int16_val;
	log_entity("test::accept_int16", "received " + std::to_string(val));
}

void handler_accept_int32(cdts* data, char** out_err)
{
	cdt& param = data[0].arr[0];
	if(param.type != metaffi_int32_type)
	{
		const char* type_str = get_type_name(param.type);
		std::string error_msg = "test::accept_int32: expected int32, got " + std::string(type_str);
		log_error("test::accept_int32", error_msg);
		set_error(out_err, error_msg);
		return;
	}
	metaffi_int32 val = param.cdt_val.int32_val;
	log_entity("test::accept_int32", "received " + std::to_string(val));
}

void handler_accept_int64(cdts* data, char** out_err)
{
	cdt& param = data[0].arr[0];
	if(param.type != metaffi_int64_type)
	{
		const char* type_str = get_type_name(param.type);
		std::string error_msg = "test::accept_int64: expected int64, got " + std::string(type_str);
		log_error("test::accept_int64", error_msg);
		set_error(out_err, error_msg);
		return;
	}
	metaffi_int64 val = param.cdt_val.int64_val;
	log_entity("test::accept_int64", "received " + std::to_string(val));
}

void handler_accept_float32(cdts* data, char** out_err)
{
	cdt& param = data[0].arr[0];
	if(param.type != metaffi_float32_type)
	{
		const char* type_str = get_type_name(param.type);
		std::string error_msg = "test::accept_float32: expected float32, got " + std::string(type_str);
		log_error("test::accept_float32", error_msg);
		set_error(out_err, error_msg);
		return;
	}
	metaffi_float32 val = param.cdt_val.float32_val;
	log_entity("test::accept_float32", "received " + std::to_string(val));
}

void handler_accept_float64(cdts* data, char** out_err)
{
	metaffi_float64 val = data[0].arr[0].cdt_val.float64_val;
	log_entity("test::accept_float64", "received " + std::to_string(val));
}

void handler_accept_bool(cdts* data, char** out_err)
{
	cdt& param = data[0].arr[0];
	if(param.type != metaffi_bool_type)
	{
		const char* type_str = get_type_name(param.type);
		std::string error_msg = "test::accept_bool: expected bool, got " + std::string(type_str);
		log_error("test::accept_bool", error_msg);
		set_error(out_err, error_msg);
		return;
	}
	metaffi_bool val = param.cdt_val.bool_val;
	log_entity("test::accept_bool", "received " + std::string(val ? "true" : "false"));
}

void handler_accept_string8(cdts* data, char** out_err)
{
	cdt& param = data[0].arr[0];
	if(param.type != metaffi_string8_type)
	{
		const char* type_str = get_type_name(param.type);
		std::string error_msg = "test::accept_string8: expected string8, got " + std::string(type_str);
		log_error("test::accept_string8", error_msg);
		set_error(out_err, error_msg);
		return;
	}
	const char* val = reinterpret_cast<const char*>(param.cdt_val.string8_val);
	log_entity("test::accept_string8", "received \"" + std::string(val ? val : "null") + "\"");
}

//======================================================================
// PARAMS_RET handlers - Echo functions
//======================================================================

void handler_echo_int64(cdts* data, char** out_err)
{
	// data[0] = params, data[1] = returns
	metaffi_int64 val = data[0].arr[0].cdt_val.int64_val;
	data[1].arr[0] = val;
	log_entity("test::echo_int64", "echoing " + std::to_string(val));
}

void handler_echo_float64(cdts* data, char** out_err)
{
	metaffi_float64 val = data[0].arr[0].cdt_val.float64_val;
	data[1].arr[0] = val;
	log_entity("test::echo_float64", "echoing " + std::to_string(val));
}

void handler_echo_string8(cdts* data, char** out_err)
{
	const char* val = reinterpret_cast<const char*>(data[0].arr[0].cdt_val.string8_val);
	if(val)
	{
		data[1].arr[0].set_string(reinterpret_cast<const char8_t*>(val), true);
		log_entity("test::echo_string8", "echoing \"" + std::string(val) + "\"");
	}
	else
	{
		data[1].arr[0].type = metaffi_null_type;
		log_entity("test::echo_string8", "echoing null");
	}
}

void handler_echo_bool(cdts* data, char** out_err)
{
	metaffi_bool val = data[0].arr[0].cdt_val.bool_val;
	data[1].arr[0] = (val != 0);
	log_entity("test::echo_bool", "echoing " + std::string(val ? "true" : "false"));
}

//======================================================================
// PARAMS_RET handlers - Arithmetic
//======================================================================

void handler_add_int64(cdts* data, char** out_err)
{
	metaffi_int64 a = data[0].arr[0].cdt_val.int64_val;
	metaffi_int64 b = data[0].arr[1].cdt_val.int64_val;
	metaffi_int64 result = a + b;
	data[1].arr[0] = result;
	log_entity("test::add_int64", std::to_string(a) + " + " + std::to_string(b) + " = " + std::to_string(result));
}

void handler_add_float64(cdts* data, char** out_err)
{
	metaffi_float64 a = data[0].arr[0].cdt_val.float64_val;
	metaffi_float64 b = data[0].arr[1].cdt_val.float64_val;
	metaffi_float64 result = a + b;
	data[1].arr[0] = result;
	log_entity("test::add_float64", std::to_string(a) + " + " + std::to_string(b) + " = " + std::to_string(result));
}

void handler_concat_strings(cdts* data, char** out_err)
{
	const char* a = reinterpret_cast<const char*>(data[0].arr[0].cdt_val.string8_val);
	const char* b = reinterpret_cast<const char*>(data[0].arr[1].cdt_val.string8_val);

	std::string result;
	if(a) result += a;
	if(b) result += b;

	data[1].arr[0].set_string(reinterpret_cast<const char8_t*>(result.c_str()), true);
	log_entity("test::concat_strings", "concatenated to \"" + result + "\"");
}

//======================================================================
// Array handlers
//======================================================================

void handler_return_int64_array_1d(cdts* data, char** out_err)
{
	// Create array [1, 2, 3]
	data[1].arr[0].set_new_array(3, 1, metaffi_int64_type);
	cdts& arr = *data[1].arr[0].cdt_val.array_val;
	arr[0] = static_cast<metaffi_int64>(1);
	arr[1] = static_cast<metaffi_int64>(2);
	arr[2] = static_cast<metaffi_int64>(3);
	log_entity("test::return_int64_array_1d", "returning [1, 2, 3]");
}

void handler_return_int64_array_2d(cdts* data, char** out_err)
{
	// Create 2D array [[1, 2], [3, 4]]
	data[1].arr[0].set_new_array(2, 2, metaffi_int64_type);
	cdts& outer = *data[1].arr[0].cdt_val.array_val;

	// First row [1, 2]
	outer[0].set_new_array(2, 1, metaffi_int64_type);
	(*outer[0].cdt_val.array_val)[0] = static_cast<metaffi_int64>(1);
	(*outer[0].cdt_val.array_val)[1] = static_cast<metaffi_int64>(2);

	// Second row [3, 4]
	outer[1].set_new_array(2, 1, metaffi_int64_type);
	(*outer[1].cdt_val.array_val)[0] = static_cast<metaffi_int64>(3);
	(*outer[1].cdt_val.array_val)[1] = static_cast<metaffi_int64>(4);

	log_entity("test::return_int64_array_2d", "returning [[1, 2], [3, 4]]");
}

void handler_return_int64_array_3d(cdts* data, char** out_err)
{
	// Create 3D array [[[1,2],[3,4]], [[5,6],[7,8]]]
	data[1].arr[0].set_new_array(2, 3, metaffi_int64_type);
	cdts& d1 = *data[1].arr[0].cdt_val.array_val;

	// First 2D block [[1,2],[3,4]]
	d1[0].set_new_array(2, 2, metaffi_int64_type);
	cdts& d1_0 = *d1[0].cdt_val.array_val;
	d1_0[0].set_new_array(2, 1, metaffi_int64_type);
	(*d1_0[0].cdt_val.array_val)[0] = static_cast<metaffi_int64>(1);
	(*d1_0[0].cdt_val.array_val)[1] = static_cast<metaffi_int64>(2);
	d1_0[1].set_new_array(2, 1, metaffi_int64_type);
	(*d1_0[1].cdt_val.array_val)[0] = static_cast<metaffi_int64>(3);
	(*d1_0[1].cdt_val.array_val)[1] = static_cast<metaffi_int64>(4);

	// Second 2D block [[5,6],[7,8]]
	d1[1].set_new_array(2, 2, metaffi_int64_type);
	cdts& d1_1 = *d1[1].cdt_val.array_val;
	d1_1[0].set_new_array(2, 1, metaffi_int64_type);
	(*d1_1[0].cdt_val.array_val)[0] = static_cast<metaffi_int64>(5);
	(*d1_1[0].cdt_val.array_val)[1] = static_cast<metaffi_int64>(6);
	d1_1[1].set_new_array(2, 1, metaffi_int64_type);
	(*d1_1[1].cdt_val.array_val)[0] = static_cast<metaffi_int64>(7);
	(*d1_1[1].cdt_val.array_val)[1] = static_cast<metaffi_int64>(8);

	log_entity("test::return_int64_array_3d", "returning [[[1,2],[3,4]],[[5,6],[7,8]]]");
}

void handler_return_ragged_array(cdts* data, char** out_err)
{
	// Create ragged array [[1,2,3], [4], [5,6]]
	data[1].arr[0].set_new_array(3, 2, metaffi_int64_type);
	cdts& outer = *data[1].arr[0].cdt_val.array_val;

	// Row 0: [1, 2, 3]
	outer[0].set_new_array(3, 1, metaffi_int64_type);
	(*outer[0].cdt_val.array_val)[0] = static_cast<metaffi_int64>(1);
	(*outer[0].cdt_val.array_val)[1] = static_cast<metaffi_int64>(2);
	(*outer[0].cdt_val.array_val)[2] = static_cast<metaffi_int64>(3);

	// Row 1: [4]
	outer[1].set_new_array(1, 1, metaffi_int64_type);
	(*outer[1].cdt_val.array_val)[0] = static_cast<metaffi_int64>(4);

	// Row 2: [5, 6]
	outer[2].set_new_array(2, 1, metaffi_int64_type);
	(*outer[2].cdt_val.array_val)[0] = static_cast<metaffi_int64>(5);
	(*outer[2].cdt_val.array_val)[1] = static_cast<metaffi_int64>(6);

	log_entity("test::return_ragged_array", "returning [[1,2,3],[4],[5,6]]");
}

void handler_return_string_array(cdts* data, char** out_err)
{
	// Create array ["one", "two", "three"]
	data[1].arr[0].set_new_array(3, 1, metaffi_string8_type);
	cdts& arr = *data[1].arr[0].cdt_val.array_val;
	arr[0].set_string(reinterpret_cast<const char8_t*>("one"), true);
	arr[1].set_string(reinterpret_cast<const char8_t*>("two"), true);
	arr[2].set_string(reinterpret_cast<const char8_t*>("three"), true);
	log_entity("test::return_string_array", "returning [\"one\", \"two\", \"three\"]");
}

void handler_sum_int64_array(cdts* data, char** out_err)
{
	// data[0] = params, data[1] = returns
	cdts* arr = data[0].arr[0].cdt_val.array_val;
	metaffi_int64 sum = 0;

	if(arr && arr->arr)
	{
		for(metaffi_size i = 0; i < arr->length; ++i)
		{
			sum += arr->arr[i].cdt_val.int64_val;
		}
	}

	data[1].arr[0] = sum;
	log_entity("test::sum_int64_array", "sum = " + std::to_string(sum));
}

void handler_echo_int64_array(cdts* data, char** out_err)
{
	// data[0] = params, data[1] = returns
	cdts* input_arr = data[0].arr[0].cdt_val.array_val;

	if(!input_arr || !input_arr->arr)
	{
		data[1].arr[0].type = metaffi_null_type;
		log_entity("test::echo_int64_array", "echoing null array");
		return;
	}

	// Create output array with same size
	data[1].arr[0].set_new_array(input_arr->length, input_arr->fixed_dimensions, metaffi_int64_type);
	cdts& output_arr = *data[1].arr[0].cdt_val.array_val;

	for(metaffi_size i = 0; i < input_arr->length; ++i)
	{
		output_arr[i] = input_arr->arr[i].cdt_val.int64_val;
	}

	log_entity("test::echo_int64_array", "echoing array of length " + std::to_string(input_arr->length));
}

void handler_join_strings(cdts* data, char** out_err)
{
	// data[0] = params, data[1] = returns
	cdts* arr = data[0].arr[0].cdt_val.array_val;
	std::string result;

	if(arr && arr->arr)
	{
		for(metaffi_size i = 0; i < arr->length; ++i)
		{
			if(i > 0) result += ", ";
			const char* s = reinterpret_cast<const char*>(arr->arr[i].cdt_val.string8_val);
			if(s) result += s;
		}
	}

	data[1].arr[0].set_string(reinterpret_cast<const char8_t*>(result.c_str()), true);
	log_entity("test::join_strings", "joined to \"" + result + "\"");
}

//======================================================================
// Handle handlers
//======================================================================

static void release_test_handle(cdt_metaffi_handle* h)
{
	if(h && h->handle)
	{
		auto* th = static_cast<TestHandle*>(h->handle);
		log_entity("handle_release", "releasing handle id=" + std::to_string(th->id));
		th->~TestHandle();
		xllr_free_memory(th);
		h->handle = nullptr;
	}
}

void handler_create_handle(cdts* data, char** out_err)
{
	void* handle_mem = xllr_alloc_memory(sizeof(TestHandle));
	if(!handle_mem)
	{
		set_error(out_err, "Failed to allocate memory for TestHandle");
		return;
	}
	auto* handle = new (handle_mem) TestHandle(g_next_handle_id++, "test_data");

	void* cdt_handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
	if(!cdt_handle_mem)
	{
		handle->~TestHandle();
		xllr_free_memory(handle_mem);
		set_error(out_err, "Failed to allocate memory for cdt_metaffi_handle");
		return;
	}
	auto* cdt_handle = new (cdt_handle_mem) cdt_metaffi_handle(
		handle,
		TEST_RUNTIME_ID,
		release_test_handle
	);

	data[1].arr[0].set_handle(cdt_handle);
	log_entity("test::create_handle", "created handle id=" + std::to_string(handle->id));
}

void handler_get_handle_data(cdts* data, char** out_err)
{
	// data[0] = params, data[1] = returns
	cdt_metaffi_handle* h = data[0].arr[0].cdt_val.handle_val;

	if(!h || !h->handle)
	{
		log_error("test::get_handle_data", "null handle");
		set_error(out_err, "get_handle_data: null handle");
		return;
	}

	auto* th = static_cast<TestHandle*>(h->handle);
	data[1].arr[0].set_string(reinterpret_cast<const char8_t*>(th->data.c_str()), true);
	log_entity("test::get_handle_data", "returning \"" + th->data + "\" from handle id=" + std::to_string(th->id));
}

void handler_set_handle_data(cdts* data, char** out_err)
{
	// data[0] = params (handle, string)
	cdt_metaffi_handle* h = data[0].arr[0].cdt_val.handle_val;
	const char* new_data = reinterpret_cast<const char*>(data[0].arr[1].cdt_val.string8_val);

	if(!h || !h->handle)
	{
		log_error("test::set_handle_data", "null handle");
		set_error(out_err, "set_handle_data: null handle");
		return;
	}

	auto* th = static_cast<TestHandle*>(h->handle);
	th->data = new_data ? new_data : "";
	log_entity("test::set_handle_data", "set handle id=" + std::to_string(th->id) + " data to \"" + th->data + "\"");
}

void handler_release_handle(cdts* data, char** out_err)
{
	// data[0] = params (handle)
	cdt_metaffi_handle* h = data[0].arr[0].cdt_val.handle_val;

	if(h && h->handle)
	{
		auto* th = static_cast<TestHandle*>(h->handle);
		log_entity("test::release_handle", "release requested for handle id=" + std::to_string(th->id));
		// Note: actual release happens via the release callback in cdt_metaffi_handle
	}
	else
	{
		log_entity("test::release_handle", "release requested for null handle");
	}
}

//======================================================================
// Callable handlers
//======================================================================

void handler_call_callback_add(cdts* data, char** out_err)
{
	// data[0] = params (callable), data[1] = returns
	cdt_metaffi_callable* callable = data[0].arr[0].cdt_val.callable_val;

	if(!callable || !callable->val)
	{
		log_error("test::call_callback_add", "null callable");
		set_error(out_err, "call_callback_add: null callable");
		return;
	}

	log_entity("test::call_callback_add", "calling callback with (3, 4)");

	// The callable->val is an xcall pointer
	auto* cb_xcall = static_cast<xcall*>(callable->val);

	// Create CDTS for the callback call: params[0] and returns[1]
	cdts call_data[2] = {cdts(2, 1), cdts(1, 1)};
	call_data[0][0] = static_cast<metaffi_int64>(3);
	call_data[0][1] = static_cast<metaffi_int64>(4);

	char* cb_err = nullptr;
	(*cb_xcall)(call_data, &cb_err);

	if(cb_err)
	{
		log_error("test::call_callback_add", std::string("callback error: ") + cb_err);
		*out_err = cb_err;
		return;
	}

	// Get result from callback
	metaffi_int64 result = call_data[1].arr[0].cdt_val.int64_val;
	data[1].arr[0] = result;
	log_entity("test::call_callback_add", "callback returned " + std::to_string(result));
}

void handler_call_callback_string(cdts* data, char** out_err)
{
	// data[0] = params (callable), data[1] = returns
	cdt_metaffi_callable* callable = data[0].arr[0].cdt_val.callable_val;

	if(!callable || !callable->val)
	{
		log_error("test::call_callback_string", "null callable");
		set_error(out_err, "call_callback_string: null callable");
		return;
	}

	log_entity("test::call_callback_string", "calling callback with \"test\"");

	auto* cb_xcall = static_cast<xcall*>(callable->val);

	// Create CDTS for the callback call
	cdts call_data[2] = {cdts(1, 1), cdts(1, 1)};
	call_data[0][0].set_string(reinterpret_cast<const char8_t*>("test"), true);

	char* cb_err = nullptr;
	(*cb_xcall)(call_data, &cb_err);

	if(cb_err)
	{
		log_error("test::call_callback_string", std::string("callback error: ") + cb_err);
		*out_err = cb_err;
		return;
	}

	// Get result from callback
	const char* result = reinterpret_cast<const char*>(call_data[1].arr[0].cdt_val.string8_val);
	data[1].arr[0].set_string(reinterpret_cast<const char8_t*>(result ? result : ""), true);
	log_entity("test::call_callback_string", "callback returned \"" + std::string(result ? result : "") + "\"");
}

// Internal adder callback implementation
static void adder_callback_impl(void* context, cdts* call_data, char** cb_err)
{
	// call_data[0] = params (int64, int64), call_data[1] = returns (int64)
	metaffi_int64 a = call_data[0].arr[0].cdt_val.int64_val;
	metaffi_int64 b = call_data[0].arr[1].cdt_val.int64_val;
	call_data[1].arr[0] = a + b;
	log_entity("adder_callback", std::to_string(a) + " + " + std::to_string(b) + " = " + std::to_string(a + b));
}

void handler_return_adder_callback(cdts* data, char** out_err)
{
	// Create an xcall for the adder callback
	void* adder_xcall_mem = xllr_alloc_memory(sizeof(xcall));
	if(!adder_xcall_mem)
	{
		set_error(out_err, "Failed to allocate memory for xcall");
		return;
	}
	auto* adder_xcall = new (adder_xcall_mem) xcall(reinterpret_cast<void*>(adder_callback_impl), nullptr);

	void* callable_mem = xllr_alloc_memory(sizeof(cdt_metaffi_callable));
	if(!callable_mem)
	{
		adder_xcall->~xcall();
		xllr_free_memory(adder_xcall_mem);
		set_error(out_err, "Failed to allocate memory for cdt_metaffi_callable");
		return;
	}
	auto* callable = new (callable_mem) cdt_metaffi_callable();
	callable->val = adder_xcall;
	callable->params_types_length = 2;
	callable->retval_types_length = 1;

	void* params_types_mem = xllr_alloc_memory(sizeof(metaffi_type) * callable->params_types_length);
	if(!params_types_mem)
	{
		callable->~cdt_metaffi_callable();
		xllr_free_memory(callable_mem);
		adder_xcall->~xcall();
		xllr_free_memory(adder_xcall_mem);
		set_error(out_err, "Failed to allocate memory for callable parameter types");
		return;
	}
	callable->parameters_types = static_cast<metaffi_type*>(params_types_mem);
	callable->parameters_types[0] = metaffi_int64_type;
	callable->parameters_types[1] = metaffi_int64_type;

	void* retval_types_mem = xllr_alloc_memory(sizeof(metaffi_type) * callable->retval_types_length);
	if(!retval_types_mem)
	{
		xllr_free_memory(callable->parameters_types);
		callable->~cdt_metaffi_callable();
		xllr_free_memory(callable_mem);
		adder_xcall->~xcall();
		xllr_free_memory(adder_xcall_mem);
		set_error(out_err, "Failed to allocate memory for callable return types");
		return;
	}
	callable->retval_types = static_cast<metaffi_type*>(retval_types_mem);
	callable->retval_types[0] = metaffi_int64_type;

	// Set the CDT type and value directly
	data[1].arr[0].type = metaffi_callable_type;
	data[1].arr[0].cdt_val.callable_val = callable;
	data[1].arr[0].free_required = true;
	log_entity("test::return_adder_callback", "returning adder callback");
}

//======================================================================
// Error handling handlers
//======================================================================

void handler_throw_error(cdts* data, char** out_err)
{
	log_error("test::throw_error", "intentionally throwing error");
	set_error(out_err, "Test error thrown intentionally");
}

void handler_throw_with_message(cdts* data, char** out_err)
{
	const char* msg = reinterpret_cast<const char*>(data[0].arr[0].cdt_val.string8_val);
	std::string error_msg = msg ? msg : "No message provided";
	log_error("test::throw_with_message", error_msg);
	set_error(out_err, error_msg);
}

void handler_error_if_negative(cdts* data, char** out_err)
{
	metaffi_int64 val = data[0].arr[0].cdt_val.int64_val;
	log_entity("test::error_if_negative", "received " + std::to_string(val));

	if(val < 0)
	{
		log_error("test::error_if_negative", "value is negative: " + std::to_string(val));
		set_error(out_err, "Value is negative: " + std::to_string(val));
	}
}

//======================================================================
// Any type handler
//======================================================================

void handler_accept_any(cdts* data, char** out_err)
{
	// data[0] = params (any), data[1] = returns (any) - actual type set dynamically to match input type
	cdt& param = data[0].arr[0];
	cdt& ret = data[1].arr[0];

	switch(param.type)
	{
		case metaffi_int64_type:
		{
			metaffi_int64 input_val = param.cdt_val.int64_val;
			metaffi_int64 return_val = input_val + 100;  // Return different value
			ret.type = metaffi_int64_type;
			ret.cdt_val.int64_val = return_val;
			ret.free_required = false;
			log_entity("test::accept_any", "received int64: " + std::to_string(input_val) + ", returning int64: " + std::to_string(return_val));
			break;
		}

		case metaffi_float64_type:
		{
			metaffi_float64 input_val = param.cdt_val.float64_val;
			metaffi_float64 return_val = input_val * 2.0;  // Return different value
			ret.type = metaffi_float64_type;
			ret.cdt_val.float64_val = return_val;
			ret.free_required = false;
			log_entity("test::accept_any", "received float64: " + std::to_string(input_val) + ", returning float64: " + std::to_string(return_val));
			break;
		}

		case metaffi_string8_type:
		{
			const char* input_str = reinterpret_cast<const char*>(param.cdt_val.string8_val);
			std::string return_str = input_str ? std::string("echoed: ") + input_str : "echoed: null";
			ret.set_string(reinterpret_cast<const char8_t*>(return_str.c_str()), true);
			log_entity("test::accept_any", "received string8: \"" + std::string(input_str ? input_str : "null") + "\", returning string8: \"" + return_str + "\"");
			break;
		}

		case metaffi_int32_array_type:
		{
			// Return a different int32 array
			ret.set_new_array(3, 1, metaffi_int32_type);
			cdts& arr = *ret.cdt_val.array_val;
			arr[0] = static_cast<metaffi_int32>(4);
			arr[1] = static_cast<metaffi_int32>(5);
			arr[2] = static_cast<metaffi_int32>(6);
			log_entity("test::accept_any", "received int32[]: " + format_int32_array(param.cdt_val.array_val) + ", returning int32[]: [4, 5, 6]");
			break;
		}

		case metaffi_int64_array_type:
		{
			// Return a different int64 array
			ret.set_new_array(3, 1, metaffi_int64_type);
			cdts& arr = *ret.cdt_val.array_val;
			arr[0] = static_cast<metaffi_int64>(10);
			arr[1] = static_cast<metaffi_int64>(20);
			arr[2] = static_cast<metaffi_int64>(30);
			log_entity("test::accept_any", "received int64[]: " + format_int64_array(param.cdt_val.array_val) + ", returning int64[]: [10, 20, 30]");
			break;
		}

		default:
		{
			const char* type_str = get_type_name(param.type);
			std::string error_msg = "accept_any: unexpected type " + std::string(type_str);
			log_error("test::accept_any", error_msg);
			set_error(out_err, error_msg);
			return;
		}
	}
}

//======================================================================
// Multiple return values handlers
//======================================================================

void handler_return_two_values(cdts* data, char** out_err)
{
	// data[1] = returns (int64, string8)
	data[1].arr[0] = static_cast<metaffi_int64>(42);
	data[1].arr[1].set_string(reinterpret_cast<const char8_t*>("answer"), true);
	log_entity("test::return_two_values", "returning (42, \"answer\")");
}

void handler_return_three_values(cdts* data, char** out_err)
{
	// data[1] = returns (int64, float64, bool)
	data[1].arr[0] = static_cast<metaffi_int64>(1);
	data[1].arr[1] = static_cast<metaffi_float64>(2.5);
	data[1].arr[2] = true;
	log_entity("test::return_three_values", "returning (1, 2.5, true)");
}

void handler_swap_values(cdts* data, char** out_err)
{
	// data[0] = params (int64, string8), data[1] = returns (string8, int64)
	metaffi_int64 int_val = data[0].arr[0].cdt_val.int64_val;
	const char* str_val = reinterpret_cast<const char*>(data[0].arr[1].cdt_val.string8_val);

	data[1].arr[0].set_string(reinterpret_cast<const char8_t*>(str_val ? str_val : ""), true);
	data[1].arr[1] = int_val;

	log_entity("test::swap_values", "swapping (" + std::to_string(int_val) + ", \"" +
	           std::string(str_val ? str_val : "") + "\") -> (\"" +
	           std::string(str_val ? str_val : "") + "\", " + std::to_string(int_val) + ")");
}

//======================================================================
// TestHandle class method handlers
//======================================================================

void handler_get_handle_id(cdts* data, char** out_err)
{
	// data[0] = params (handle), data[1] = returns (int64)
	cdt_metaffi_handle* h = data[0].arr[0].cdt_val.handle_val;

	if(!h || !h->handle)
	{
		log_error("test::TestHandle.get_id", "null handle");
		set_error(out_err, "get_handle_id: null handle");
		return;
	}

	auto* th = static_cast<TestHandle*>(h->handle);
	data[1].arr[0] = th->id;
	log_entity("test::TestHandle.get_id", "returning id=" + std::to_string(th->id) + " from handle");
}

void handler_append_to_data(cdts* data, char** out_err)
{
	// data[0] = params (handle, string8)
	cdt_metaffi_handle* h = data[0].arr[0].cdt_val.handle_val;
	const char* append_str = reinterpret_cast<const char*>(data[0].arr[1].cdt_val.string8_val);

	if(!h || !h->handle)
	{
		log_error("test::TestHandle.append_to_data", "null handle");
		set_error(out_err, "append_to_data: null handle");
		return;
	}

	auto* th = static_cast<TestHandle*>(h->handle);
	std::string old_data = th->data;
	th->data += (append_str ? append_str : "");
	log_entity("test::TestHandle.append_to_data",
	           "appended \"" + std::string(append_str ? append_str : "") +
	           "\" to handle id=" + std::to_string(th->id) +
	           ", data: \"" + old_data + "\" -> \"" + th->data + "\"");
}

//======================================================================
// Global variable handlers
//======================================================================

void handler_get_g_name(cdts* data, char** out_err)
{
	// data[0] = params (empty), data[1] = returns (string8)
	data[1].arr[0].set_string(reinterpret_cast<const char8_t*>(g_name.c_str()), true);
	log_entity("test::get_g_name", "returning g_name=\"" + g_name + "\"");
}

void handler_set_g_name(cdts* data, char** out_err)
{
	// data[0] = params (string8)
	const char* new_name = reinterpret_cast<const char*>(data[0].arr[0].cdt_val.string8_val);
	std::string old_name = g_name;
	g_name = new_name ? new_name : "";
	log_entity("test::set_g_name", "set g_name: \"" + old_name + "\" -> \"" + g_name + "\"");
}

} // namespace test_plugin
