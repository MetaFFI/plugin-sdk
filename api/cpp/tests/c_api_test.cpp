/**
 * c_api_test.cpp — C API (metaffi_api_c.h) integration tests
 *
 * Same 15 scenarios as cpp_api_test.cpp, exercised through the opaque-handle
 * C API.  The test file is compiled as C++ (to use cdts constructors and
 * doctest), but only calls extern "C" functions from metaffi_api_c.h.
 *
 * Runtime: "test" → "xllr.test"
 * Module path: "" (test plugin)
 * Entity paths: "test::function_name"
 */

#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#include <doctest/doctest.h>

#include <metaffi/api/metaffi_api_c.h>
#include <cdts_serializer/cpp/cdts_cpp_serializer.h>
#include <runtime/xllr_capi_loader.h>
#include <runtime/xcall.h>

#include <cmath>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

using namespace metaffi::utils;

// ---------------------------------------------------------------------------
// Global fixtures
// ---------------------------------------------------------------------------

static metaffi_runtime_h g_runtime = nullptr;
static metaffi_module_h  g_module  = nullptr;

// ---------------------------------------------------------------------------
// Callable helpers (static lifetime)
// ---------------------------------------------------------------------------

static void c_adder_fn(void* /*ctx*/, cdts* d, char** /*err*/)
{
	metaffi_int64 a = d[0].arr[0].cdt_val.int64_val;
	metaffi_int64 b = d[0].arr[1].cdt_val.int64_val;
	d[1].arr[0] = a + b;
}

static void c_echo_string_fn(void* /*ctx*/, cdts* d, char** /*err*/)
{
	metaffi_string8 s = d[0].arr[0].cdt_val.string8_val;
	d[1].arr[0].set_string(s, /*is_copy=*/false);
}

static xcall s_c_adder_xcall(reinterpret_cast<void*>(c_adder_fn), nullptr);
static xcall s_c_echo_str_xcall(reinterpret_cast<void*>(c_echo_string_fn), nullptr);

static cdt_metaffi_callable* c_make_callable(xcall* xc, int8_t np, int8_t nr)
{
	auto* cb = static_cast<cdt_metaffi_callable*>(xllr_alloc_memory(sizeof(cdt_metaffi_callable)));
	new(cb) cdt_metaffi_callable{};
	cb->val                 = xc;
	cb->params_types_length = np;
	cb->retval_types_length = nr;
	return cb;
}

// ---------------------------------------------------------------------------
// Helper: assert out_err is null, then clear it
// ---------------------------------------------------------------------------
#define CHECK_NO_ERR(err_var) \
	do { \
		if((err_var) != nullptr) { \
			fprintf(stderr, "Unexpected error: %s\n", (err_var)); \
			xllr_free_string(err_var); \
			(err_var) = nullptr; \
			FAIL("Got unexpected error (see stderr)"); \
		} \
	} while(0)

// ---------------------------------------------------------------------------
// Helpers to build type-info arrays on the stack
// ---------------------------------------------------------------------------

static metaffi_type_info make_ti(metaffi_type t)
{
	metaffi_type_info ti{};
	ti.type             = t;
	ti.alias            = nullptr;
	ti.is_free_alias    = 0;
	ti.fixed_dimensions = MIXED_OR_UNKNOWN_DIMENSIONS;
	return ti;
}

static metaffi_type_info make_ti_arr(metaffi_type t, metaffi_int64 dims)
{
	metaffi_type_info ti = make_ti(t);
	ti.fixed_dimensions  = dims;
	return ti;
}

// ---------------------------------------------------------------------------
// Shorthand: load entity via C API
// ---------------------------------------------------------------------------

static metaffi_entity_h load(const char* path,
                              const metaffi_type_info* params, int8_t np,
                              const metaffi_type_info* retvals, int8_t nr)
{
	char* err = nullptr;
	metaffi_entity_h h = metaffi_entity_load(g_module, path, params, np, retvals, nr, &err);
	if(err)
	{
		fprintf(stderr, "metaffi_entity_load(%s): %s\n", path, err);
		xllr_free_string(err);
		FAIL("metaffi_entity_load failed (see stderr)");
	}
	REQUIRE(h != nullptr);
	return h;
}

// ---------------------------------------------------------------------------
// main()
// ---------------------------------------------------------------------------

int main(int argc, char** argv)
{
	// --- Setup ---
	char* err = nullptr;

	g_runtime = metaffi_runtime_create("test", &err);
	if(err) { fprintf(stderr, "runtime_create: %s\n", err); return 1; }

	metaffi_runtime_load_plugin(g_runtime, &err);
	if(err) { fprintf(stderr, "load_plugin: %s\n", err); return 1; }

	g_module = metaffi_module_load(g_runtime, "", &err);
	if(err) { fprintf(stderr, "module_load: %s\n", err); return 1; }

	// --- Run ---
	doctest::Context ctx(argc, argv);
	int res = ctx.run();

	// --- Teardown ---
	metaffi_module_free(g_module);
	g_module = nullptr;

	metaffi_runtime_release_plugin(g_runtime, &err);
	if(err) xllr_free_string(err);

	metaffi_runtime_free(g_runtime);
	g_runtime = nullptr;

	return res;
}

// ===========================================================================
// Suite 1 — Void
// ===========================================================================

TEST_SUITE("C Suite 1 - Void")
{
	TEST_CASE("no_op")
	{
		metaffi_entity_h e = load("test::no_op", nullptr, 0, nullptr, 0);
		cdts pr[2] = {cdts(), cdts()};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("print_hello")
	{
		metaffi_entity_h e = load("test::print_hello", nullptr, 0, nullptr, 0);
		cdts pr[2] = {cdts(), cdts()};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}
}

// ===========================================================================
// Suite 2 — Return primitives
// ===========================================================================

TEST_SUITE("C Suite 2 - Return primitives")
{
	TEST_CASE("return_int8")
	{
		metaffi_type_info rti = make_ti(metaffi_int8_type);
		metaffi_entity_h e = load("test::return_int8", nullptr, 0, &rti, 1);

		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.int8_val == 42);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_int16")
	{
		metaffi_type_info rti = make_ti(metaffi_int16_type);
		metaffi_entity_h e = load("test::return_int16", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.int16_val == 1000);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_int32")
	{
		metaffi_type_info rti = make_ti(metaffi_int32_type);
		metaffi_entity_h e = load("test::return_int32", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.int32_val == 100000);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_int64")
	{
		metaffi_type_info rti = make_ti(metaffi_int64_type);
		metaffi_entity_h e = load("test::return_int64", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.int64_val == std::numeric_limits<metaffi_int64>::max());
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_uint8")
	{
		metaffi_type_info rti = make_ti(metaffi_uint8_type);
		metaffi_entity_h e = load("test::return_uint8", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.uint8_val == 255u);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_uint16")
	{
		metaffi_type_info rti = make_ti(metaffi_uint16_type);
		metaffi_entity_h e = load("test::return_uint16", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.uint16_val == 65535u);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_uint32")
	{
		metaffi_type_info rti = make_ti(metaffi_uint32_type);
		metaffi_entity_h e = load("test::return_uint32", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.uint32_val == std::numeric_limits<metaffi_uint32>::max());
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_uint64")
	{
		metaffi_type_info rti = make_ti(metaffi_uint64_type);
		metaffi_entity_h e = load("test::return_uint64", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.uint64_val == std::numeric_limits<metaffi_uint64>::max());
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_float32")
	{
		metaffi_type_info rti = make_ti(metaffi_float32_type);
		metaffi_entity_h e = load("test::return_float32", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(std::fabs(static_cast<double>(pr[1].arr[0].cdt_val.float32_val) - 3.14159) < 0.001);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_float64")
	{
		metaffi_type_info rti = make_ti(metaffi_float64_type);
		metaffi_entity_h e = load("test::return_float64", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(std::fabs(pr[1].arr[0].cdt_val.float64_val - 3.141592653589793) < 1e-12);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_bool_true")
	{
		metaffi_type_info rti = make_ti(metaffi_bool_type);
		metaffi_entity_h e = load("test::return_bool_true", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.bool_val != 0);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_bool_false")
	{
		metaffi_type_info rti = make_ti(metaffi_bool_type);
		metaffi_entity_h e = load("test::return_bool_false", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.bool_val == 0);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_string8")
	{
		metaffi_type_info rti = make_ti(metaffi_string8_type);
		metaffi_entity_h e = load("test::return_string8", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		REQUIRE(pr[1].arr[0].cdt_val.string8_val != nullptr);
		CHECK(std::string(reinterpret_cast<const char*>(pr[1].arr[0].cdt_val.string8_val))
		      == "Hello from test plugin");
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_null")
	{
		metaffi_type_info rti = make_ti(metaffi_null_type);
		metaffi_entity_h e = load("test::return_null", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts(1)};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].type == metaffi_null_type);
		metaffi_entity_free(e, nullptr);
	}
}

// ===========================================================================
// Suite 3 — Accept primitives
// ===========================================================================

TEST_SUITE("C Suite 3 - Accept primitives")
{
	TEST_CASE("accept_int8")
	{
		metaffi_type_info pti = make_ti(metaffi_int8_type);
		metaffi_entity_h e = load("test::accept_int8", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = metaffi_int8(42);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("accept_int16")
	{
		metaffi_type_info pti = make_ti(metaffi_int16_type);
		metaffi_entity_h e = load("test::accept_int16", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = metaffi_int16(1000);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("accept_int32")
	{
		metaffi_type_info pti = make_ti(metaffi_int32_type);
		metaffi_entity_h e = load("test::accept_int32", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = metaffi_int32(100000);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("accept_int64")
	{
		metaffi_type_info pti = make_ti(metaffi_int64_type);
		metaffi_entity_h e = load("test::accept_int64", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = std::numeric_limits<metaffi_int64>::max();
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("accept_float32")
	{
		metaffi_type_info pti = make_ti(metaffi_float32_type);
		metaffi_entity_h e = load("test::accept_float32", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = metaffi_float32(3.14159f);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("accept_float64")
	{
		metaffi_type_info pti = make_ti(metaffi_float64_type);
		metaffi_entity_h e = load("test::accept_float64", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = metaffi_float64(3.141592653589793);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("accept_bool")
	{
		metaffi_type_info pti = make_ti(metaffi_bool_type);
		metaffi_entity_h e = load("test::accept_bool", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = true;
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("accept_string8")
	{
		metaffi_type_info pti = make_ti(metaffi_string8_type);
		metaffi_entity_h e = load("test::accept_string8", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0].set_string(reinterpret_cast<const char8_t*>("test string"), /*is_copy=*/true);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}
}

// ===========================================================================
// Suite 4 — Echo
// ===========================================================================

TEST_SUITE("C Suite 4 - Echo")
{
	TEST_CASE("echo_int64")
	{
		metaffi_type_info pti = make_ti(metaffi_int64_type);
		metaffi_type_info rti = make_ti(metaffi_int64_type);
		metaffi_entity_h e = load("test::echo_int64", &pti, 1, &rti, 1);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = metaffi_int64(42);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.int64_val == 42);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("echo_float64")
	{
		metaffi_type_info pti = make_ti(metaffi_float64_type);
		metaffi_type_info rti = make_ti(metaffi_float64_type);
		metaffi_entity_h e = load("test::echo_float64", &pti, 1, &rti, 1);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = metaffi_float64(3.14);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(std::fabs(pr[1].arr[0].cdt_val.float64_val - 3.14) < 1e-10);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("echo_string8")
	{
		metaffi_type_info pti = make_ti(metaffi_string8_type);
		metaffi_type_info rti = make_ti(metaffi_string8_type);
		metaffi_entity_h e = load("test::echo_string8", &pti, 1, &rti, 1);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0].set_string(reinterpret_cast<const char8_t*>("hello"), /*is_copy=*/true);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(std::string(reinterpret_cast<const char*>(pr[1].arr[0].cdt_val.string8_val)) == "hello");
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("echo_bool")
	{
		metaffi_type_info pti = make_ti(metaffi_bool_type);
		metaffi_type_info rti = make_ti(metaffi_bool_type);
		metaffi_entity_h e = load("test::echo_bool", &pti, 1, &rti, 1);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = true;
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.bool_val != 0);
		metaffi_entity_free(e, nullptr);
	}
}

// ===========================================================================
// Suite 5 — Arithmetic
// ===========================================================================

TEST_SUITE("C Suite 5 - Arithmetic")
{
	TEST_CASE("add_int64")
	{
		metaffi_type_info ptis[2] = {make_ti(metaffi_int64_type), make_ti(metaffi_int64_type)};
		metaffi_type_info rti     = make_ti(metaffi_int64_type);
		metaffi_entity_h e = load("test::add_int64", ptis, 2, &rti, 1);
		cdts pr[2] = {cdts(2), cdts()};
		pr[0].arr[0] = metaffi_int64(10);
		pr[0].arr[1] = metaffi_int64(20);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.int64_val == 30);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("add_float64")
	{
		metaffi_type_info ptis[2] = {make_ti(metaffi_float64_type), make_ti(metaffi_float64_type)};
		metaffi_type_info rti     = make_ti(metaffi_float64_type);
		metaffi_entity_h e = load("test::add_float64", ptis, 2, &rti, 1);
		cdts pr[2] = {cdts(2), cdts()};
		pr[0].arr[0] = metaffi_float64(1.5);
		pr[0].arr[1] = metaffi_float64(2.5);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(std::fabs(pr[1].arr[0].cdt_val.float64_val - 4.0) < 1e-10);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("concat_strings")
	{
		metaffi_type_info ptis[2] = {make_ti(metaffi_string8_type), make_ti(metaffi_string8_type)};
		metaffi_type_info rti     = make_ti(metaffi_string8_type);
		metaffi_entity_h e = load("test::concat_strings", ptis, 2, &rti, 1);
		cdts pr[2] = {cdts(2), cdts()};
		pr[0].arr[0].set_string(reinterpret_cast<const char8_t*>("hello"), true);
		pr[0].arr[1].set_string(reinterpret_cast<const char8_t*>("world"), true);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(std::string(reinterpret_cast<const char*>(pr[1].arr[0].cdt_val.string8_val)) == "helloworld");
		metaffi_entity_free(e, nullptr);
	}
}

// ===========================================================================
// Suite 6 — Arrays (use cdts_cpp_serializer for complex types)
// ===========================================================================

TEST_SUITE("C Suite 6 - Arrays")
{
	TEST_CASE("return_int64_array_1d")
	{
		metaffi_type_info rti = make_ti_arr(metaffi_int64_packed_array_type, 1);
		metaffi_entity_h e = load("test::return_int64_array_1d", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts()};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);

		std::vector<metaffi_int64> arr;
		cdts_cpp_serializer ds(pr[1]);
		ds >> arr;
		REQUIRE(arr.size() == 3);
		CHECK(arr[0] == 1); CHECK(arr[1] == 2); CHECK(arr[2] == 3);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_string_array")
	{
		metaffi_type_info rti = make_ti_arr(metaffi_string8_packed_array_type, 1);
		metaffi_entity_h e = load("test::return_string_array", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts()};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);

		std::vector<std::string> arr;
		cdts_cpp_serializer ds(pr[1]);
		ds >> arr;
		REQUIRE(arr.size() == 3);
		CHECK(arr[0] == "one"); CHECK(arr[1] == "two"); CHECK(arr[2] == "three");
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("sum_int64_array")
	{
		metaffi_type_info pti = make_ti_arr(metaffi_int64_array_type, 1);
		metaffi_type_info rti = make_ti(metaffi_int64_type);
		metaffi_entity_h e = load("test::sum_int64_array", &pti, 1, &rti, 1);

		cdts pr[2] = {cdts(1), cdts()};
		std::vector<metaffi_int64> input{1, 2, 3, 4, 5};
		cdts_cpp_serializer sp(pr[0]);
		sp << input;

		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.int64_val == 15);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("join_strings")
	{
		metaffi_type_info pti = make_ti_arr(metaffi_string8_array_type, 1);
		metaffi_type_info rti = make_ti(metaffi_string8_type);
		metaffi_entity_h e = load("test::join_strings", &pti, 1, &rti, 1);

		cdts pr[2] = {cdts(1), cdts()};
		std::vector<std::string> input{"one", "two", "three"};
		cdts_cpp_serializer sp(pr[0]);
		sp << input;

		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(std::string(reinterpret_cast<const char*>(pr[1].arr[0].cdt_val.string8_val))
		      == "one, two, three");
		metaffi_entity_free(e, nullptr);
	}
}

// ===========================================================================
// Suite 7 — Handles
// ===========================================================================

TEST_SUITE("C Suite 7 - Handles")
{
	TEST_CASE("handle lifecycle: create / get / set / release")
	{
		// --- Create ---
		metaffi_type_info h_ti = make_ti(metaffi_handle_type);
		metaffi_entity_h create_e = load("test::create_handle", nullptr, 0, &h_ti, 1);
		cdts pr_create[2] = {cdts(), cdts()};
		char* err = nullptr;
		metaffi_entity_call(create_e, pr_create, &err);
		CHECK_NO_ERR(err);
		REQUIRE(pr_create[1].arr[0].type == metaffi_handle_type);
		cdt_metaffi_handle handle = *pr_create[1].arr[0].cdt_val.handle_val;
		REQUIRE(handle.handle != nullptr);
		metaffi_entity_free(create_e, nullptr);

		metaffi_type_info ptis_get[1]  = {make_ti(metaffi_handle_type)};
		metaffi_type_info rti_str      = make_ti(metaffi_string8_type);
		metaffi_type_info ptis_set[2]  = {make_ti(metaffi_handle_type), make_ti(metaffi_string8_type)};
		metaffi_type_info ptis_rel[1]  = {make_ti(metaffi_handle_type)};

		metaffi_entity_h get_e = load("test::get_handle_data", ptis_get, 1, &rti_str, 1);
		metaffi_entity_h set_e = load("test::set_handle_data", ptis_set, 2, nullptr, 0);
		metaffi_entity_h rel_e = load("test::release_handle",  ptis_rel, 1, nullptr, 0);

		// --- Get data → "test_data" ---
		{
			cdts pr[2] = {cdts(1), cdts()};
			pr[0].arr[0].set_handle(&handle);
			metaffi_entity_call(get_e, pr, &err);
			CHECK_NO_ERR(err);
			CHECK(std::string(reinterpret_cast<const char*>(pr[1].arr[0].cdt_val.string8_val))
			      == "test_data");
		}

		// --- Set data → "new_data" ---
		{
			cdts pr[2] = {cdts(2), cdts()};
			pr[0].arr[0].set_handle(&handle);
			pr[0].arr[1].set_string(reinterpret_cast<const char8_t*>("new_data"), true);
			metaffi_entity_call(set_e, pr, &err);
			CHECK_NO_ERR(err);
		}

		// --- Verify ---
		{
			cdts pr[2] = {cdts(1), cdts()};
			pr[0].arr[0].set_handle(&handle);
			metaffi_entity_call(get_e, pr, &err);
			CHECK_NO_ERR(err);
			CHECK(std::string(reinterpret_cast<const char*>(pr[1].arr[0].cdt_val.string8_val))
			      == "new_data");
		}

		// --- Release ---
		{
			cdts pr[2] = {cdts(1), cdts()};
			pr[0].arr[0].set_handle(&handle);
			metaffi_entity_call(rel_e, pr, &err);
			CHECK_NO_ERR(err);
		}

		metaffi_entity_free(get_e, nullptr);
		metaffi_entity_free(set_e, nullptr);
		metaffi_entity_free(rel_e, nullptr);
	}

	TEST_CASE("TestHandle.get_id")
	{
		metaffi_type_info h_ti = make_ti(metaffi_handle_type);
		metaffi_entity_h create_e = load("test::create_handle", nullptr, 0, &h_ti, 1);
		cdts pr_c[2] = {cdts(), cdts()};
		char* err = nullptr;
		metaffi_entity_call(create_e, pr_c, &err);
		CHECK_NO_ERR(err);
		cdt_metaffi_handle handle = *pr_c[1].arr[0].cdt_val.handle_val;
		metaffi_entity_free(create_e, nullptr);

		metaffi_type_info pti_h = make_ti(metaffi_handle_type);
		metaffi_type_info rti_i = make_ti(metaffi_int64_type);
		metaffi_entity_h get_id_e = load("test::TestHandle.get_id", &pti_h, 1, &rti_i, 1);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0].set_handle(&handle);
		metaffi_entity_call(get_id_e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.int64_val > 0);
		metaffi_entity_free(get_id_e, nullptr);

		metaffi_type_info pti_rel = make_ti(metaffi_handle_type);
		metaffi_entity_h rel_e = load("test::release_handle", &pti_rel, 1, nullptr, 0);
		cdts pr_r[2] = {cdts(1), cdts()};
		pr_r[0].arr[0].set_handle(&handle);
		metaffi_entity_call(rel_e, pr_r, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(rel_e, nullptr);
	}
}

// ===========================================================================
// Suite 8 — Callables
// ===========================================================================

TEST_SUITE("C Suite 8 - Callables")
{
	TEST_CASE("call_callback_add")
	{
		metaffi_type_info pti = make_ti(metaffi_callable_type);
		metaffi_type_info rti = make_ti(metaffi_int64_type);
		metaffi_entity_h e = load("test::call_callback_add", &pti, 1, &rti, 1);

		cdt_metaffi_callable* cb = c_make_callable(&s_c_adder_xcall, 2, 1);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0].type                 = metaffi_callable_type;
		pr[0].arr[0].cdt_val.callable_val = cb;
		pr[0].arr[0].free_required        = true;

		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.int64_val == 7);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("call_callback_string")
	{
		metaffi_type_info pti = make_ti(metaffi_callable_type);
		metaffi_type_info rti = make_ti(metaffi_string8_type);
		metaffi_entity_h e = load("test::call_callback_string", &pti, 1, &rti, 1);

		cdt_metaffi_callable* cb = c_make_callable(&s_c_echo_str_xcall, 1, 1);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0].type                 = metaffi_callable_type;
		pr[0].arr[0].cdt_val.callable_val = cb;
		pr[0].arr[0].free_required        = true;

		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(std::string(reinterpret_cast<const char*>(pr[1].arr[0].cdt_val.string8_val)) == "test");
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_adder_callback")
	{
		metaffi_type_info rti = make_ti(metaffi_callable_type);
		metaffi_entity_h e = load("test::return_adder_callback", nullptr, 0, &rti, 1);
		cdts pr[2] = {cdts(), cdts()};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		REQUIRE(pr[1].arr[0].type == metaffi_callable_type);

		cdt_metaffi_callable* cb = pr[1].arr[0].cdt_val.callable_val;
		REQUIRE(cb != nullptr);
		REQUIRE(cb->val != nullptr);

		auto* xc = static_cast<xcall*>(cb->val);
		cdts call_data[2] = {cdts(2), cdts(1)};
		call_data[0][0] = static_cast<metaffi_int64>(10);
		call_data[0][1] = static_cast<metaffi_int64>(20);
		char* cb_err = nullptr;
		(*xc)(call_data, &cb_err);
		REQUIRE(cb_err == nullptr);
		CHECK(call_data[1].arr[0].cdt_val.int64_val == 30);
		metaffi_entity_free(e, nullptr);
	}
}

// ===========================================================================
// Suite 9 — Errors
// ===========================================================================

TEST_SUITE("C Suite 9 - Errors")
{
	TEST_CASE("throw_error")
	{
		metaffi_entity_h e = load("test::throw_error", nullptr, 0, nullptr, 0);
		cdts pr[2] = {cdts(), cdts()};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		REQUIRE(err != nullptr);
		// Just verify it's non-null (error was propagated)
		xllr_free_string(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("throw_with_message")
	{
		metaffi_type_info pti = make_ti(metaffi_string8_type);
		metaffi_entity_h e = load("test::throw_with_message", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0].set_string(reinterpret_cast<const char8_t*>("Custom error message"), true);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		REQUIRE(err != nullptr);
		bool msg_found = std::string(err).find("Custom error message") != std::string::npos;
		CHECK(msg_found);
		xllr_free_string(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("error_if_negative — positive succeeds")
	{
		metaffi_type_info pti = make_ti(metaffi_int64_type);
		metaffi_entity_h e = load("test::error_if_negative", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = metaffi_int64(42);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("error_if_negative — negative fails")
	{
		metaffi_type_info pti = make_ti(metaffi_int64_type);
		metaffi_entity_h e = load("test::error_if_negative", &pti, 1, nullptr, 0);
		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = metaffi_int64(-1);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		REQUIRE(err != nullptr);
		xllr_free_string(err);
		metaffi_entity_free(e, nullptr);
	}
}

// ===========================================================================
// Suite 10 — ANY type
// ===========================================================================

TEST_SUITE("C Suite 10 - ANY type")
{
	TEST_CASE("accept_any — int64: 42 → 142")
	{
		metaffi_type_info pti = make_ti(metaffi_any_type);
		metaffi_type_info rti = make_ti(metaffi_any_type);
		metaffi_entity_h e = load("test::accept_any", &pti, 1, &rti, 1);

		cdts pr[2] = {cdts(1), cdts()};
		pr[0].arr[0] = metaffi_int64(42);

		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		CHECK(pr[1].arr[0].cdt_val.int64_val == 142);
		metaffi_entity_free(e, nullptr);
	}
}

// ===========================================================================
// Suite 11 — Multiple return values
// ===========================================================================

TEST_SUITE("C Suite 11 - Multiple returns")
{
	TEST_CASE("return_two_values")
	{
		metaffi_type_info rtis[2] = {make_ti(metaffi_int64_type), make_ti(metaffi_string8_type)};
		metaffi_entity_h e = load("test::return_two_values", nullptr, 0, rtis, 2);
		cdts pr[2] = {cdts(), cdts()};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		REQUIRE(pr[1].length == 2);
		CHECK(pr[1].arr[0].cdt_val.int64_val == 42);
		CHECK(std::string(reinterpret_cast<const char*>(pr[1].arr[1].cdt_val.string8_val)) == "answer");
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("return_three_values")
	{
		metaffi_type_info rtis[3] = {
			make_ti(metaffi_int64_type),
			make_ti(metaffi_float64_type),
			make_ti(metaffi_bool_type)
		};
		metaffi_entity_h e = load("test::return_three_values", nullptr, 0, rtis, 3);
		cdts pr[2] = {cdts(), cdts()};
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		REQUIRE(pr[1].length == 3);
		CHECK(pr[1].arr[0].cdt_val.int64_val == 1);
		CHECK(std::fabs(pr[1].arr[1].cdt_val.float64_val - 2.5) < 1e-10);
		CHECK(pr[1].arr[2].cdt_val.bool_val != 0);
		metaffi_entity_free(e, nullptr);
	}

	TEST_CASE("swap_values")
	{
		metaffi_type_info ptis[2] = {make_ti(metaffi_int64_type), make_ti(metaffi_string8_type)};
		metaffi_type_info rtis[2] = {make_ti(metaffi_string8_type), make_ti(metaffi_int64_type)};
		metaffi_entity_h e = load("test::swap_values", ptis, 2, rtis, 2);
		cdts pr[2] = {cdts(2), cdts()};
		pr[0].arr[0] = metaffi_int64(42);
		pr[0].arr[1].set_string(reinterpret_cast<const char8_t*>("hello"), true);
		char* err = nullptr;
		metaffi_entity_call(e, pr, &err);
		CHECK_NO_ERR(err);
		REQUIRE(pr[1].length == 2);
		CHECK(std::string(reinterpret_cast<const char*>(pr[1].arr[0].cdt_val.string8_val)) == "hello");
		CHECK(pr[1].arr[1].cdt_val.int64_val == 42);
		metaffi_entity_free(e, nullptr);
	}
}

// ===========================================================================
// Suite 12 — Global variables
// ===========================================================================

TEST_SUITE("C Suite 12 - Globals")
{
	TEST_CASE("get/set g_name")
	{
		metaffi_type_info rti_str  = make_ti(metaffi_string8_type);
		metaffi_type_info pti_str  = make_ti(metaffi_string8_type);
		metaffi_entity_h get_e = load("test::get_g_name", nullptr,   0, &rti_str, 1);
		metaffi_entity_h set_e = load("test::set_g_name", &pti_str,  1, nullptr,  0);

		// Save original
		char* err = nullptr;
		cdts pr_get[2] = {cdts(), cdts()};
		metaffi_entity_call(get_e, pr_get, &err);
		CHECK_NO_ERR(err);
		std::string orig(reinterpret_cast<const char*>(pr_get[1].arr[0].cdt_val.string8_val));

		// Set new value
		{
			cdts pr_set[2] = {cdts(1), cdts()};
			pr_set[0].arr[0].set_string(reinterpret_cast<const char8_t*>("c_api_test_value"), true);
			metaffi_entity_call(set_e, pr_set, &err);
			CHECK_NO_ERR(err);
		}

		// Verify
		{
			cdts pr_v[2] = {cdts(), cdts()};
			metaffi_entity_call(get_e, pr_v, &err);
			CHECK_NO_ERR(err);
			CHECK(std::string(reinterpret_cast<const char*>(pr_v[1].arr[0].cdt_val.string8_val))
			      == "c_api_test_value");
		}

		// Restore original
		{
			cdts pr_r[2] = {cdts(1), cdts()};
			pr_r[0].arr[0].set_string(reinterpret_cast<const char8_t*>(orig.c_str()), true);
			metaffi_entity_call(set_e, pr_r, &err);
			CHECK_NO_ERR(err);
		}

		metaffi_entity_free(get_e, nullptr);
		metaffi_entity_free(set_e, nullptr);
	}
}
