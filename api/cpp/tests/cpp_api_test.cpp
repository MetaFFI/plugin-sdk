/**
 * cpp_api_test.cpp — C++ MetaFFI API integration tests
 *
 * Uses doctest with a custom main() so we can share a single runtime/module
 * fixture across all suites.
 *
 * Runtime: "test" (normalised to "xllr.test")
 * Module path: "" (the test plugin derives everything from the entity path)
 * Entity paths: "test::function_name" or "test::ClassName.method"
 */

#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#include <doctest/doctest.h>

#include <metaffi/api/metaffi_api.h>
#include <cdts_serializer/cpp/cdts_cpp_serializer.h>
#include <runtime/xllr_capi_loader.h>
#include <runtime/xcall.h>

#include <cmath>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

using namespace metaffi::api;
using namespace metaffi::utils;

// ---------------------------------------------------------------------------
// Global fixtures (shared across all suites)
// ---------------------------------------------------------------------------

static MetaFFIRuntime* g_runtime = nullptr;
static MetaFFIModule*  g_module  = nullptr;

// ---------------------------------------------------------------------------
// Callable helpers (static lifetime — alive for the full test process)
// ---------------------------------------------------------------------------

/** Native adder callback: sums two int64 params and returns the result. */
static void cpp_adder_fn(void* /*ctx*/, cdts* d, char** /*err*/)
{
	metaffi_int64 a = d[0].arr[0].cdt_val.int64_val;
	metaffi_int64 b = d[0].arr[1].cdt_val.int64_val;
	d[1].arr[0] = a + b;
}

/** Native echo-string callback: echoes the input string pointer back. */
static void cpp_echo_string_fn(void* /*ctx*/, cdts* d, char** /*err*/)
{
	// Borrow the input pointer — the plugin reads the return immediately.
	metaffi_string8 s = d[0].arr[0].cdt_val.string8_val;
	d[1].arr[0].set_string(s, /*is_copy=*/false);
}

// Static xcalls whose function pointers are always non-null.
static xcall s_adder_xcall(reinterpret_cast<void*>(cpp_adder_fn), nullptr);
static xcall s_echo_str_xcall(reinterpret_cast<void*>(cpp_echo_string_fn), nullptr);

/** Heap-allocate a cdt_metaffi_callable wrapping an xcall. */
static cdt_metaffi_callable* make_callable(xcall* xc, int8_t np, int8_t nr)
{
	auto* cb = static_cast<cdt_metaffi_callable*>(xllr_alloc_memory(sizeof(cdt_metaffi_callable)));
	new(cb) cdt_metaffi_callable{};
	cb->val                 = xc;
	cb->params_types_length = np;
	cb->retval_types_length = nr;
	// Type arrays intentionally null — the test plugin doesn't inspect them.
	return cb;
}

// ---------------------------------------------------------------------------
// Shorthand type-info helpers
// ---------------------------------------------------------------------------

static MetaFFITypeInfo ti(metaffi_type t)
{
	return MetaFFITypeInfo{t};
}

static MetaFFITypeInfo ti_arr(metaffi_type t, metaffi_int64 dims)
{
	MetaFFITypeInfo info{t};
	info.fixed_dimensions = dims;
	return info;
}

// ---------------------------------------------------------------------------
// main()
// ---------------------------------------------------------------------------

int main(int argc, char** argv)
{
	// --- Setup ---
	g_runtime = new MetaFFIRuntime("test");
	g_runtime->load_runtime_plugin();

	auto m   = g_runtime->load_module("");
	g_module = new MetaFFIModule(std::move(m));

	// --- Run all test suites ---
	doctest::Context ctx(argc, argv);
	int res = ctx.run();

	// --- Teardown ---
	delete g_module;
	g_module = nullptr;

	g_runtime->release_runtime_plugin();
	delete g_runtime;
	g_runtime = nullptr;

	return res;
}

// ===========================================================================
// Suite 1 — Void
// ===========================================================================

TEST_SUITE("Suite 1 - Void")
{
	TEST_CASE("no_op")
	{
		auto e = g_module->load_entity("test::no_op");
		CHECK_NOTHROW(e.call_cdts());
	}

	TEST_CASE("print_hello")
	{
		auto e = g_module->load_entity("test::print_hello");
		CHECK_NOTHROW(e.call_cdts());
	}
}

// ===========================================================================
// Suite 2 — Return primitives (no params)
// ===========================================================================

TEST_SUITE("Suite 2 - Return primitives")
{
	TEST_CASE("return_int8")
	{
		auto e = g_module->load_entity("test::return_int8", {}, {metaffi_int8_type});
		auto [v] = e.call<metaffi_int8>();
		CHECK(v == 42);
	}

	TEST_CASE("return_int16")
	{
		auto e = g_module->load_entity("test::return_int16", {}, {metaffi_int16_type});
		auto [v] = e.call<metaffi_int16>();
		CHECK(v == 1000);
	}

	TEST_CASE("return_int32")
	{
		auto e = g_module->load_entity("test::return_int32", {}, {metaffi_int32_type});
		auto [v] = e.call<metaffi_int32>();
		CHECK(v == 100000);
	}

	TEST_CASE("return_int64")
	{
		auto e = g_module->load_entity("test::return_int64", {}, {metaffi_int64_type});
		auto [v] = e.call<metaffi_int64>();
		CHECK(v == std::numeric_limits<metaffi_int64>::max());
	}

	TEST_CASE("return_uint8")
	{
		auto e = g_module->load_entity("test::return_uint8", {}, {metaffi_uint8_type});
		auto [v] = e.call<metaffi_uint8>();
		CHECK(v == metaffi_uint8(255));
	}

	TEST_CASE("return_uint16")
	{
		auto e = g_module->load_entity("test::return_uint16", {}, {metaffi_uint16_type});
		auto [v] = e.call<metaffi_uint16>();
		CHECK(v == metaffi_uint16(65535));
	}

	TEST_CASE("return_uint32")
	{
		auto e = g_module->load_entity("test::return_uint32", {}, {metaffi_uint32_type});
		auto [v] = e.call<metaffi_uint32>();
		CHECK(v == std::numeric_limits<metaffi_uint32>::max());
	}

	TEST_CASE("return_uint64")
	{
		auto e = g_module->load_entity("test::return_uint64", {}, {metaffi_uint64_type});
		auto [v] = e.call<metaffi_uint64>();
		CHECK(v == std::numeric_limits<metaffi_uint64>::max());
	}

	TEST_CASE("return_float32")
	{
		auto e = g_module->load_entity("test::return_float32", {}, {metaffi_float32_type});
		auto [v] = e.call<metaffi_float32>();
		CHECK(std::fabs(static_cast<double>(v) - 3.14159) < 0.001);
	}

	TEST_CASE("return_float64")
	{
		auto e = g_module->load_entity("test::return_float64", {}, {metaffi_float64_type});
		auto [v] = e.call<metaffi_float64>();
		CHECK(std::fabs(v - 3.141592653589793) < 1e-12);
	}

	TEST_CASE("return_bool_true")
	{
		auto e = g_module->load_entity("test::return_bool_true", {}, {metaffi_bool_type});
		auto [v] = e.call<bool>();
		CHECK(v == true);
	}

	TEST_CASE("return_bool_false")
	{
		auto e = g_module->load_entity("test::return_bool_false", {}, {metaffi_bool_type});
		auto [v] = e.call<bool>();
		CHECK(v == false);
	}

	TEST_CASE("return_string8")
	{
		auto e = g_module->load_entity("test::return_string8", {}, {metaffi_string8_type});
		auto [v] = e.call<std::string>();
		CHECK(v == "Hello from test plugin");
	}

	TEST_CASE("return_null")
	{
		auto e = g_module->load_entity_with_info("test::return_null", {}, {ti(metaffi_null_type)});
		cdts retvals = e.call_cdts();
		REQUIRE(retvals.length == 1);
		CHECK(retvals[0].type == metaffi_null_type);
	}
}

// ===========================================================================
// Suite 3 — Accept primitives (params only, no return)
// ===========================================================================

TEST_SUITE("Suite 3 - Accept primitives")
{
	TEST_CASE("accept_int8")
	{
		auto e = g_module->load_entity("test::accept_int8", {metaffi_int8_type}, {});
		CHECK_NOTHROW(e.call_cdts(metaffi_int8(42)));
	}

	TEST_CASE("accept_int16")
	{
		auto e = g_module->load_entity("test::accept_int16", {metaffi_int16_type}, {});
		CHECK_NOTHROW(e.call_cdts(metaffi_int16(1000)));
	}

	TEST_CASE("accept_int32")
	{
		auto e = g_module->load_entity("test::accept_int32", {metaffi_int32_type}, {});
		CHECK_NOTHROW(e.call_cdts(metaffi_int32(100000)));
	}

	TEST_CASE("accept_int64")
	{
		auto e = g_module->load_entity("test::accept_int64", {metaffi_int64_type}, {});
		CHECK_NOTHROW(e.call_cdts(std::numeric_limits<metaffi_int64>::max()));
	}

	TEST_CASE("accept_float32")
	{
		auto e = g_module->load_entity("test::accept_float32", {metaffi_float32_type}, {});
		CHECK_NOTHROW(e.call_cdts(metaffi_float32(3.14159f)));
	}

	TEST_CASE("accept_float64")
	{
		auto e = g_module->load_entity("test::accept_float64", {metaffi_float64_type}, {});
		CHECK_NOTHROW(e.call_cdts(metaffi_float64(3.141592653589793)));
	}

	TEST_CASE("accept_bool")
	{
		auto e = g_module->load_entity("test::accept_bool", {metaffi_bool_type}, {});
		CHECK_NOTHROW(e.call_cdts(true));
	}

	TEST_CASE("accept_string8")
	{
		auto e = g_module->load_entity("test::accept_string8", {metaffi_string8_type}, {});
		CHECK_NOTHROW(e.call_cdts(std::string("test string")));
	}
}

// ===========================================================================
// Suite 4 — Echo (param in, same param out)
// ===========================================================================

TEST_SUITE("Suite 4 - Echo")
{
	TEST_CASE("echo_int64")
	{
		auto e = g_module->load_entity("test::echo_int64",
		                               {metaffi_int64_type}, {metaffi_int64_type});
		auto [v] = e.call<metaffi_int64>(metaffi_int64(42));
		CHECK(v == 42);
	}

	TEST_CASE("echo_float64")
	{
		auto e = g_module->load_entity("test::echo_float64",
		                               {metaffi_float64_type}, {metaffi_float64_type});
		auto [v] = e.call<metaffi_float64>(metaffi_float64(3.14));
		CHECK(std::fabs(v - 3.14) < 1e-10);
	}

	TEST_CASE("echo_string8")
	{
		auto e = g_module->load_entity("test::echo_string8",
		                               {metaffi_string8_type}, {metaffi_string8_type});
		auto [v] = e.call<std::string>(std::string("hello"));
		CHECK(v == "hello");
	}

	TEST_CASE("echo_bool")
	{
		auto e = g_module->load_entity("test::echo_bool",
		                               {metaffi_bool_type}, {metaffi_bool_type});
		auto [v] = e.call<bool>(true);
		CHECK(v == true);
	}
}

// ===========================================================================
// Suite 5 — Arithmetic
// ===========================================================================

TEST_SUITE("Suite 5 - Arithmetic")
{
	TEST_CASE("add_int64")
	{
		auto e = g_module->load_entity("test::add_int64",
		                               {metaffi_int64_type, metaffi_int64_type},
		                               {metaffi_int64_type});
		auto [v] = e.call<metaffi_int64>(metaffi_int64(10), metaffi_int64(20));
		CHECK(v == 30);
	}

	TEST_CASE("add_float64")
	{
		auto e = g_module->load_entity("test::add_float64",
		                               {metaffi_float64_type, metaffi_float64_type},
		                               {metaffi_float64_type});
		auto [v] = e.call<metaffi_float64>(metaffi_float64(1.5), metaffi_float64(2.5));
		CHECK(std::fabs(v - 4.0) < 1e-10);
	}

	TEST_CASE("concat_strings")
	{
		auto e = g_module->load_entity("test::concat_strings",
		                               {metaffi_string8_type, metaffi_string8_type},
		                               {metaffi_string8_type});
		auto [v] = e.call<std::string>(std::string("hello"), std::string("world"));
		CHECK(v == "helloworld");
	}
}

// ===========================================================================
// Suite 6 — Arrays
// ===========================================================================

TEST_SUITE("Suite 6 - Arrays")
{
	TEST_CASE("return_int64_array_1d")
	{
		auto e = g_module->load_entity_with_info("test::return_int64_array_1d",
		                                          {},
		                                          {ti_arr(metaffi_int64_packed_array_type, 1)});
		cdts retvals = e.call_cdts();
		REQUIRE(retvals.length == 1);

		std::vector<metaffi_int64> arr;
		cdts_cpp_serializer ds(retvals);
		ds >> arr;

		REQUIRE(arr.size() == 3);
		CHECK(arr[0] == 1); CHECK(arr[1] == 2); CHECK(arr[2] == 3);
	}

	TEST_CASE("return_int64_array_2d")
	{
		auto e = g_module->load_entity_with_info("test::return_int64_array_2d",
		                                          {},
		                                          {ti_arr(metaffi_int64_array_type, 2)});
		cdts retvals = e.call_cdts();
		REQUIRE(retvals.length == 1);

		std::vector<std::vector<metaffi_int64>> arr;
		cdts_cpp_serializer ds(retvals);
		ds >> arr;

		REQUIRE(arr.size() == 2);
		REQUIRE(arr[0].size() == 2); REQUIRE(arr[1].size() == 2);
		CHECK(arr[0][0] == 1); CHECK(arr[0][1] == 2);
		CHECK(arr[1][0] == 3); CHECK(arr[1][1] == 4);
	}

	TEST_CASE("return_int64_array_3d")
	{
		auto e = g_module->load_entity_with_info("test::return_int64_array_3d",
		                                          {},
		                                          {ti_arr(metaffi_int64_array_type, 3)});
		cdts retvals = e.call_cdts();
		REQUIRE(retvals.length == 1);

		std::vector<std::vector<std::vector<metaffi_int64>>> arr;
		cdts_cpp_serializer ds(retvals);
		ds >> arr;

		REQUIRE(arr.size() == 2);
		CHECK(arr[0][0][0] == 1); CHECK(arr[0][0][1] == 2);
		CHECK(arr[0][1][0] == 3); CHECK(arr[0][1][1] == 4);
		CHECK(arr[1][0][0] == 5); CHECK(arr[1][0][1] == 6);
		CHECK(arr[1][1][0] == 7); CHECK(arr[1][1][1] == 8);
	}

	TEST_CASE("return_ragged_array")
	{
		auto e = g_module->load_entity_with_info("test::return_ragged_array",
		                                          {},
		                                          {ti_arr(metaffi_int64_array_type, 2)});
		cdts retvals = e.call_cdts();
		REQUIRE(retvals.length == 1);

		std::vector<std::vector<metaffi_int64>> arr;
		cdts_cpp_serializer ds(retvals);
		ds >> arr;

		REQUIRE(arr.size() == 3);
		REQUIRE(arr[0].size() == 3);
		CHECK(arr[0][0] == 1); CHECK(arr[0][1] == 2); CHECK(arr[0][2] == 3);
		REQUIRE(arr[1].size() == 1); CHECK(arr[1][0] == 4);
		REQUIRE(arr[2].size() == 2); CHECK(arr[2][0] == 5); CHECK(arr[2][1] == 6);
	}

	TEST_CASE("return_string_array")
	{
		auto e = g_module->load_entity_with_info("test::return_string_array",
		                                          {},
		                                          {ti_arr(metaffi_string8_packed_array_type, 1)});
		cdts retvals = e.call_cdts();
		REQUIRE(retvals.length == 1);

		std::vector<std::string> arr;
		cdts_cpp_serializer ds(retvals);
		ds >> arr;

		REQUIRE(arr.size() == 3);
		CHECK(arr[0] == "one"); CHECK(arr[1] == "two"); CHECK(arr[2] == "three");
	}

	TEST_CASE("sum_int64_array")
	{
		// sum([1,2,3,4,5]) → 15
		auto e = g_module->load_entity_with_info("test::sum_int64_array",
		                                          {ti_arr(metaffi_int64_array_type, 1)},
		                                          {ti(metaffi_int64_type)});

		std::vector<metaffi_int64> input{1, 2, 3, 4, 5};
		auto [v] = e.call<metaffi_int64>(input);
		CHECK(v == 15);
	}

	TEST_CASE("echo_int64_array")
	{
		// echo([10,20,30]) → [10,20,30]
		auto e = g_module->load_entity_with_info("test::echo_int64_array",
		                                          {ti_arr(metaffi_int64_array_type, 1)},
		                                          {ti_arr(metaffi_int64_array_type, 1)});

		std::vector<metaffi_int64> input{10, 20, 30};

		// Serialize params manually so we can use call_raw
		cdts params(1);
		cdts_cpp_serializer sp(params);
		sp << input;

		cdts retvals = e.call_raw(std::move(params));

		std::vector<metaffi_int64> out;
		cdts_cpp_serializer ds(retvals);
		ds >> out;

		REQUIRE(out.size() == 3);
		CHECK(out[0] == 10); CHECK(out[1] == 20); CHECK(out[2] == 30);
	}

	TEST_CASE("join_strings")
	{
		// join(["one","two","three"]) → "one, two, three"
		auto e = g_module->load_entity_with_info("test::join_strings",
		                                          {ti_arr(metaffi_string8_array_type, 1)},
		                                          {ti(metaffi_string8_type)});

		std::vector<std::string> input{"one", "two", "three"};
		auto [v] = e.call<std::string>(input);
		CHECK(v == "one, two, three");
	}
}

// ===========================================================================
// Suite 7 — Handles
// ===========================================================================

TEST_SUITE("Suite 7 - Handles")
{
	TEST_CASE("handle lifecycle: create / get / set / release")
	{
		// --- Create handle ---
		auto create_e = g_module->load_entity("test::create_handle", {}, {metaffi_handle_type});
		cdts retvals_create = create_e.call_cdts();
		REQUIRE(retvals_create.length == 1);
		REQUIRE(retvals_create[0].type == metaffi_handle_type);
		cdt_metaffi_handle handle = *retvals_create[0].cdt_val.handle_val;
		REQUIRE(handle.handle != nullptr);

		auto get_e = g_module->load_entity("test::get_handle_data",
		                                   {metaffi_handle_type}, {metaffi_string8_type});
		auto set_e = g_module->load_entity("test::set_handle_data",
		                                   {metaffi_handle_type, metaffi_string8_type}, {});
		auto rel_e = g_module->load_entity("test::release_handle", {metaffi_handle_type}, {});

		// --- Get data → "test_data" ---
		{
			cdts p(1);
			p[0].set_handle(&handle);
			cdts r = get_e.call_raw(std::move(p));
			std::string data;
			cdts_cpp_serializer ds(r);
			ds >> data;
			CHECK(data == "test_data");
		}

		// --- Set data → "new_data" ---
		{
			cdts p(2);
			p[0].set_handle(&handle);
			cdts_cpp_serializer sp(p);
			sp.set_index(1);
			sp << std::string("new_data");
			CHECK_NOTHROW(set_e.call_raw(std::move(p)));
		}

		// --- Verify updated value ---
		{
			cdts p(1);
			p[0].set_handle(&handle);
			cdts r = get_e.call_raw(std::move(p));
			std::string data;
			cdts_cpp_serializer ds(r);
			ds >> data;
			CHECK(data == "new_data");
		}

		// --- Release ---
		{
			cdts p(1);
			p[0].set_handle(&handle);
			CHECK_NOTHROW(rel_e.call_raw(std::move(p)));
		}
	}

	TEST_CASE("TestHandle.get_id")
	{
		auto create_e = g_module->load_entity("test::create_handle", {}, {metaffi_handle_type});
		cdts rv        = create_e.call_cdts();
		cdt_metaffi_handle handle = *rv[0].cdt_val.handle_val;

		auto get_id_e = g_module->load_entity("test::TestHandle.get_id",
		                                      {metaffi_handle_type}, {metaffi_int64_type});
		cdts p(1);
		p[0].set_handle(&handle);
		cdts r = get_id_e.call_raw(std::move(p));
		metaffi_int64 id;
		cdts_cpp_serializer ds(r);
		ds >> id;
		CHECK(id > 0);

		auto rel_e = g_module->load_entity("test::release_handle", {metaffi_handle_type}, {});
		cdts pr(1);
		pr[0].set_handle(&handle);
		rel_e.call_raw(std::move(pr));
	}

	TEST_CASE("TestHandle.append_to_data")
	{
		auto create_e = g_module->load_entity("test::create_handle", {}, {metaffi_handle_type});
		cdts rv        = create_e.call_cdts();
		cdt_metaffi_handle handle = *rv[0].cdt_val.handle_val;

		// Append "_suffix"
		auto append_e = g_module->load_entity("test::TestHandle.append_to_data",
		                                      {metaffi_handle_type, metaffi_string8_type}, {});
		{
			cdts p(2);
			p[0].set_handle(&handle);
			cdts_cpp_serializer sp(p);
			sp.set_index(1);
			sp << std::string("_suffix");
			CHECK_NOTHROW(append_e.call_raw(std::move(p)));
		}

		// Verify → "test_data_suffix"
		auto get_e = g_module->load_entity("test::get_handle_data",
		                                   {metaffi_handle_type}, {metaffi_string8_type});
		{
			cdts p(1);
			p[0].set_handle(&handle);
			cdts r = get_e.call_raw(std::move(p));
			std::string data;
			cdts_cpp_serializer ds(r);
			ds >> data;
			CHECK(data == "test_data_suffix");
		}

		auto rel_e = g_module->load_entity("test::release_handle", {metaffi_handle_type}, {});
		cdts pr(1);
		pr[0].set_handle(&handle);
		rel_e.call_raw(std::move(pr));
	}
}

// ===========================================================================
// Suite 8 — Callables
// ===========================================================================

TEST_SUITE("Suite 8 - Callables")
{
	TEST_CASE("call_callback_add")
	{
		// call_callback_add(adder) → plugin calls adder(3,4) → returns 7
		auto e = g_module->load_entity("test::call_callback_add",
		                               {metaffi_callable_type}, {metaffi_int64_type});

		cdt_metaffi_callable* cb = make_callable(&s_adder_xcall, 2, 1);

		cdts params(1);
		params[0].type                 = metaffi_callable_type;
		params[0].cdt_val.callable_val = cb;
		params[0].free_required        = true;

		cdts retvals = e.call_raw(std::move(params));
		metaffi_int64 result;
		cdts_cpp_serializer ds(retvals);
		ds >> result;
		CHECK(result == 7);
	}

	TEST_CASE("call_callback_string")
	{
		// call_callback_string(echo) → plugin calls echo("test") → returns "test"
		auto e = g_module->load_entity("test::call_callback_string",
		                               {metaffi_callable_type}, {metaffi_string8_type});

		cdt_metaffi_callable* cb = make_callable(&s_echo_str_xcall, 1, 1);

		cdts params(1);
		params[0].type                 = metaffi_callable_type;
		params[0].cdt_val.callable_val = cb;
		params[0].free_required        = true;

		cdts retvals = e.call_raw(std::move(params));
		std::string result;
		cdts_cpp_serializer ds(retvals);
		ds >> result;
		CHECK(result == "test");
	}

	TEST_CASE("return_adder_callback")
	{
		// Plugin returns a callable that sums two int64 values.
		auto e = g_module->load_entity_with_info("test::return_adder_callback",
		                                          {}, {ti(metaffi_callable_type)});
		cdts retvals = e.call_cdts();
		REQUIRE(retvals.length == 1);
		REQUIRE(retvals[0].type == metaffi_callable_type);

		cdt_metaffi_callable* cb = retvals[0].cdt_val.callable_val;
		REQUIRE(cb != nullptr);
		REQUIRE(cb->val != nullptr);

		// Invoke: (10 + 20) → 30
		auto* xc = static_cast<xcall*>(cb->val);
		cdts call_data[2] = {cdts(2), cdts(1)};
		call_data[0][0] = static_cast<metaffi_int64>(10);
		call_data[0][1] = static_cast<metaffi_int64>(20);

		char* cb_err = nullptr;
		(*xc)(call_data, &cb_err);
		REQUIRE(cb_err == nullptr);
		CHECK(call_data[1].arr[0].cdt_val.int64_val == 30);
	}
}

// ===========================================================================
// Suite 9 — Errors
// ===========================================================================

TEST_SUITE("Suite 9 - Errors")
{
	TEST_CASE("throw_error")
	{
		auto e = g_module->load_entity("test::throw_error");
		CHECK_THROWS_AS(e.call_cdts(), std::runtime_error);
	}

	TEST_CASE("throw_with_message")
	{
		auto e = g_module->load_entity("test::throw_with_message", {metaffi_string8_type}, {});
		bool threw = false;
		try
		{
			e.call_cdts(std::string("Custom error message"));
		}
		catch(const std::runtime_error& ex)
		{
			threw = true;
			CHECK(std::string(ex.what()).find("Custom error message") != std::string::npos);
		}
		CHECK(threw);
	}

	TEST_CASE("error_if_negative — positive input succeeds")
	{
		auto e = g_module->load_entity("test::error_if_negative", {metaffi_int64_type}, {});
		CHECK_NOTHROW(e.call_cdts(metaffi_int64(42)));
	}

	TEST_CASE("error_if_negative — negative input throws")
	{
		auto e = g_module->load_entity("test::error_if_negative", {metaffi_int64_type}, {});
		CHECK_THROWS_AS(e.call_cdts(metaffi_int64(-1)), std::runtime_error);
	}
}

// ===========================================================================
// Suite 10 — ANY type
// ===========================================================================

TEST_SUITE("Suite 10 - ANY type")
{
	TEST_CASE("accept_any — int64: 42 → 142")
	{
		auto e = g_module->load_entity_with_info("test::accept_any",
		                                          {ti(metaffi_any_type)},
		                                          {ti(metaffi_any_type)});

		cdts params(1);
		cdts_cpp_serializer sp(params);
		sp << metaffi_variant{metaffi_int64(42)};

		cdts retvals = e.call_raw(std::move(params));

		metaffi_variant ret;
		cdts_cpp_serializer ds(retvals);
		ds >> ret;
		CHECK(std::get<metaffi_int64>(ret) == 142);
	}
}

// ===========================================================================
// Suite 11 — Multiple return values
// ===========================================================================

TEST_SUITE("Suite 11 - Multiple returns")
{
	TEST_CASE("return_two_values")
	{
		auto e = g_module->load_entity_with_info("test::return_two_values",
		                                          {},
		                                          {ti(metaffi_int64_type), ti(metaffi_string8_type)});
		auto [n, s] = e.call<metaffi_int64, std::string>();
		CHECK(n == 42);
		CHECK(s == "answer");
	}

	TEST_CASE("return_three_values")
	{
		auto e = g_module->load_entity_with_info("test::return_three_values",
		                                          {},
		                                          {ti(metaffi_int64_type),
		                                           ti(metaffi_float64_type),
		                                           ti(metaffi_bool_type)});
		auto [n, f, b] = e.call<metaffi_int64, metaffi_float64, bool>();
		CHECK(n == 1);
		CHECK(std::fabs(f - 2.5) < 1e-10);
		CHECK(b == true);
	}

	TEST_CASE("swap_values")
	{
		auto e = g_module->load_entity_with_info("test::swap_values",
		                                          {ti(metaffi_int64_type), ti(metaffi_string8_type)},
		                                          {ti(metaffi_string8_type), ti(metaffi_int64_type)});
		auto [s, n] = e.call<std::string, metaffi_int64>(metaffi_int64(42), std::string("hello"));
		CHECK(s == "hello");
		CHECK(n == 42);
	}
}

// ===========================================================================
// Suite 12 — Global variables
// ===========================================================================

TEST_SUITE("Suite 12 - Globals")
{
	TEST_CASE("get/set g_name")
	{
		auto get_e = g_module->load_entity("test::get_g_name", {}, {metaffi_string8_type});
		auto set_e = g_module->load_entity("test::set_g_name", {metaffi_string8_type}, {});

		// Save original value
		auto [orig] = get_e.call<std::string>();

		// Set new value
		const std::string new_val = "cpp_api_test_value";
		CHECK_NOTHROW(set_e.call_cdts(new_val));

		// Verify new value
		auto [current] = get_e.call<std::string>();
		CHECK(current == new_val);

		// Restore original
		CHECK_NOTHROW(set_e.call_cdts(orig));
	}
}
