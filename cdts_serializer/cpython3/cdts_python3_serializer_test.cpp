#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "cdts_python3_serializer.h"
#include <runtime_manager/cpython3/runtime_manager.h>
#include <runtime_manager/cpython3/python_api_wrapper.h>
#include <runtime/xllr_capi_loader.h>
#include <runtime/xcall.h>
#include <cdts_serializer/cpython3/runtime_id.h>
#include <cstdlib>
#include <utils/safe_func.h>
#include <utils/logger.hpp>

using namespace metaffi::utils;

// Global runtime manager for tests
static std::shared_ptr<cpython3_runtime_manager> g_runtime;
static std::unique_ptr<cpython3_runtime_manager::scoped_gil> g_gil;

static auto LOG = metaffi::get_logger("cpython3.serializer");

#define TEST_LOG(msg) \
	do { METAFFI_DEBUG(LOG, "{}", msg); } while(0)

static void dummy_xcall(void*, cdts*, char**)
{
}

// Main function with Python initialization
int main(int argc, char** argv)
{
	// Detect and create Python runtime
	auto versions = cpython3_runtime_manager::detect_installed_python3();
	if(versions.empty())
	{
		METAFFI_ERROR(LOG, "No Python 3 installation found");
		return 1;
	}
	
	g_runtime = cpython3_runtime_manager::create(versions[0]);
	g_gil = std::make_unique<cpython3_runtime_manager::scoped_gil>();

	char* source_root = metaffi_getenv_alloc("METAFFI_SOURCE_ROOT");
	if(source_root && *source_root)
	{
		std::string metaffi_path = std::string(source_root) + "/sdk/api/python3";
		TEST_LOG(("Adding to sys.path: " + metaffi_path).c_str());
		g_runtime->add_sys_path(metaffi_path);
	}
	else
	{
		TEST_LOG("METAFFI_SOURCE_ROOT is not set; metaffi package may not be importable");
	}
	metaffi_free_env(source_root);

	// Run doctest
	doctest::Context context;
	context.applyCommandLine(argc, argv);
	int res = context.run();

	// Release runtime
	g_gil.reset();
	g_runtime.reset();

	return res;
}

TEST_SUITE("CDTS Python3 Serializer")
{
	// ========================================================================
	// PHASE 1 TESTS: Core Infrastructure
	// ========================================================================

	TEST_CASE("Constructor and basic utilities")
	{
		cdts data(5);
		cdts_python3_serializer ser(*g_runtime, data);

		CHECK(ser.get_index() == 0);
		CHECK(ser.size() == 5);
		CHECK(ser.has_more() == true);

		ser.set_index(2);
		CHECK(ser.get_index() == 2);

		ser.reset();
		CHECK(ser.get_index() == 0);
	}

	TEST_CASE("Bounds checking")
	{
		cdts data(2);
		cdts_python3_serializer ser(*g_runtime, data);

		// Valid index
		CHECK_NOTHROW(ser.set_index(0));
		CHECK_NOTHROW(ser.set_index(1));

		// Out of bounds
		CHECK_THROWS_AS(ser.set_index(2), std::out_of_range);
		CHECK_THROWS_AS(ser.set_index(100), std::out_of_range);
	}

	TEST_CASE("has_more utility")
	{
		cdts data(3);
		cdts_python3_serializer ser(*g_runtime, data);

		TEST_LOG("Start has_more utility test");
		CHECK(ser.has_more() == true);

		ser.set_index(2);
		TEST_LOG("After set_index(2)");
		CHECK(ser.has_more() == true);

		// Add a value then extract to move index to 3
		TEST_LOG("Creating PyLong for sanity");
		PyObject* val = pPyLong_FromLong(42);
		data[0] = int32_t(42);
		data[1] = int32_t(43);
		data[2] = int32_t(44);

		ser.reset();
		TEST_LOG("About to extract v1");
		PyObject* v1 = ser.extract_pyobject();
		TEST_LOG("Extracted v1");
		PyObject* v2 = ser.extract_pyobject();
		TEST_LOG("Extracted v2");
		PyObject* v3 = ser.extract_pyobject();
		TEST_LOG("Extracted v3");

		Py_DECREF(v1);
		Py_DECREF(v2);
		Py_DECREF(v3);
		Py_DECREF(val);

		// Now index should be 3, has_more should be false
		TEST_LOG("Checking has_more after extracts");
		CHECK(ser.has_more() == false);
	}

	// ========================================================================
	// PHASE 2 TESTS: Primitives
	// ========================================================================

	TEST_CASE("Serialize and deserialize int64")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize with explicit int64 type
		PyObject* original = pPyLong_FromLongLong(42);
		ser.add(original, metaffi_int64_type);
		Py_DECREF(original);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();
		CHECK(pPyLong_AsLongLong(extracted) == 42);
		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize and deserialize negative int64")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize with explicit int64 type
		PyObject* original = pPyLong_FromLongLong(-12345);
		ser.add(original, metaffi_int64_type);
		Py_DECREF(original);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();
		CHECK(pPyLong_AsLongLong(extracted) == -12345);
		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize and deserialize float64")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize with explicit float64 type
		PyObject* original = pPyFloat_FromDouble(3.14159);
		ser.add(original, metaffi_float64_type);
		Py_DECREF(original);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();
		CHECK(pPyFloat_AsDouble(extracted) == doctest::Approx(3.14159));
		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize and deserialize bool")
	{
		cdts data(2);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize True and False (bool type is unambiguous)
		ser.add(pPy_True, metaffi_bool_type).add(pPy_False, metaffi_bool_type);

		// Deserialize
		ser.reset();
		PyObject* b1 = ser.extract_pyobject();
		PyObject* b2 = ser.extract_pyobject();

		CHECK(b1 == pPy_True);
		CHECK(b2 == pPy_False);

		Py_DECREF(b1);
		Py_DECREF(b2);
	}

	TEST_CASE("Serialize and deserialize null")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize None (null type is unambiguous)
		ser.add(pPy_None, metaffi_null_type);

		// Deserialize
		ser.reset();
		CHECK(ser.is_null() == true);
		CHECK(ser.peek_type() == metaffi_null_type);

		PyObject* extracted = ser.extract_pyobject();
		CHECK(extracted == pPy_None);
		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize multiple primitives")
	{
		cdts data(5);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize with explicit types
		PyObject* i = pPyLong_FromLongLong(42);
		PyObject* f = pPyFloat_FromDouble(2.71828);

		ser.add(i, metaffi_int64_type)
		   .add(f, metaffi_float64_type)
		   .add(pPy_True, metaffi_bool_type)
		   .add(pPy_False, metaffi_bool_type)
		   .add(pPy_None, metaffi_null_type);

		Py_DECREF(i);
		Py_DECREF(f);

		// Deserialize
		ser.reset();

		PyObject* i_out = ser.extract_pyobject();
		PyObject* f_out = ser.extract_pyobject();
		PyObject* b1_out = ser.extract_pyobject();
		PyObject* b2_out = ser.extract_pyobject();
		PyObject* n_out = ser.extract_pyobject();

		CHECK(pPyLong_AsLongLong(i_out) == 42);
		CHECK(pPyFloat_AsDouble(f_out) == doctest::Approx(2.71828));
		CHECK(b1_out == pPy_True);
		CHECK(b2_out == pPy_False);
		CHECK(n_out == pPy_None);

		Py_DECREF(i_out);
		Py_DECREF(f_out);
		Py_DECREF(b1_out);
		Py_DECREF(b2_out);
		Py_DECREF(n_out);
	}

	TEST_CASE("peek_type without extraction")
	{
		cdts data(3);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize with explicit types
		PyObject* i = pPyLong_FromLongLong(42);
		PyObject* f = pPyFloat_FromDouble(3.14);

		ser.add(i, metaffi_int64_type)
		   .add(f, metaffi_float64_type)
		   .add(pPy_None, metaffi_null_type);

		Py_DECREF(i);
		Py_DECREF(f);

		// Peek types without extracting
		ser.reset();

		CHECK(ser.peek_type() == metaffi_int64_type);
		PyObject* i_out = ser.extract_pyobject();
		Py_DECREF(i_out);

		CHECK(ser.peek_type() == metaffi_float64_type);
		PyObject* f_out = ser.extract_pyobject();
		Py_DECREF(f_out);

		CHECK(ser.is_null() == true);
		CHECK(ser.peek_type() == metaffi_null_type);
	}

	TEST_CASE("extract_as_tuple")
	{
		cdts data(3);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize with explicit types
		PyObject* i = pPyLong_FromLongLong(10);
		PyObject* f = pPyFloat_FromDouble(20.5);

		ser.add(i, metaffi_int64_type)
		   .add(f, metaffi_float64_type)
		   .add(pPy_True, metaffi_bool_type);

		Py_DECREF(i);
		Py_DECREF(f);

		// Extract as tuple
		ser.reset();
		PyObject* tuple = ser.extract_as_tuple();

		CHECK(pPyTuple_Check(tuple));
		CHECK(pPyTuple_Size(tuple) == 3);

		CHECK(pPyLong_AsLongLong(pPyTuple_GetItem(tuple, 0)) == 10);
		CHECK(pPyFloat_AsDouble(pPyTuple_GetItem(tuple, 1)) == doctest::Approx(20.5));
		CHECK(pPyTuple_GetItem(tuple, 2) == pPy_True);

		Py_DECREF(tuple);
	}

	TEST_CASE("Deserialization from existing CDTS (primitives)")
	{
		// Manually create CDTS as if received from MetaFFI
		cdts data(3);
		data[0] = int32_t(123);
		data[1] = double(9.99);
		data[2].type = metaffi_bool_type;
		data[2].cdt_val.bool_val = 1;
		data[2].free_required = false;

		// Deserialize only
		cdts_python3_serializer deser(*g_runtime, data);

		PyObject* i = deser.extract_pyobject();
		CHECK(pPyLong_AsLongLong(i) == 123);
		Py_DECREF(i);

		PyObject* d = deser.extract_pyobject();
		CHECK(pPyFloat_AsDouble(d) == doctest::Approx(9.99));
		Py_DECREF(d);

		PyObject* b = deser.extract_pyobject();
		CHECK(b == pPy_True);
		Py_DECREF(b);
	}

	TEST_CASE("Error: Bounds violation on serialization")
	{
		cdts data(2);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* i1 = pPyLong_FromLongLong(1);
		PyObject* i2 = pPyLong_FromLongLong(2);
		PyObject* i3 = pPyLong_FromLongLong(3);

		CHECK_NOTHROW(ser.add(i1, metaffi_int64_type));
		CHECK_NOTHROW(ser.add(i2, metaffi_int64_type));
		CHECK_THROWS_AS(ser.add(i3, metaffi_int64_type), std::out_of_range);

		Py_DECREF(i1);
		Py_DECREF(i2);
		Py_DECREF(i3);
	}

	TEST_CASE("Error: Bounds violation on deserialization")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* i = pPyLong_FromLongLong(42);
		ser.add(i, metaffi_int64_type);
		Py_DECREF(i);

		ser.reset();
		PyObject* i1 = ser.extract_pyobject();
		Py_DECREF(i1);

		CHECK_THROWS_AS(ser.extract_pyobject(), std::out_of_range);
	}

	TEST_CASE("Error: Peek type beyond bounds")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* i = pPyLong_FromLongLong(42);
		ser.add(i, metaffi_int64_type);
		Py_DECREF(i);

		// Index is now at 1, which is out of bounds
		CHECK_THROWS_AS(ser.peek_type(), std::out_of_range);
	}

	// ========================================================================
	// PHASE 3 TESTS: Strings
	// ========================================================================

	TEST_CASE("Serialize and deserialize string8 (UTF-8)")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize with explicit string8 type
		PyObject* original = pPyUnicode_FromString("Hello, MetaFFI!");
		ser.add(original, metaffi_string8_type);
		Py_DECREF(original);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();

		const char* extracted_str = pPyUnicode_AsUTF8(extracted);
		CHECK(std::string(extracted_str) == "Hello, MetaFFI!");

		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize and deserialize empty string")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize with explicit string8 type
		PyObject* original = pPyUnicode_FromString("");
		ser.add(original, metaffi_string8_type);
		Py_DECREF(original);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();

		const char* extracted_str = pPyUnicode_AsUTF8(extracted);
		CHECK(std::string(extracted_str) == "");

		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize and deserialize UTF-8 with non-ASCII")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize string with Hebrew and emoji
		PyObject* original = pPyUnicode_FromString("שלום 🚀");
		ser.add(original, metaffi_string8_type);
		Py_DECREF(original);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();

		const char* extracted_str = pPyUnicode_AsUTF8(extracted);
		CHECK(std::string(extracted_str) == "שלום 🚀");

		Py_DECREF(extracted);
	}

	TEST_CASE("Deserialize pre-filled CDTS with string")
	{
		// Manually create CDTS with string
		cdts data(1);

		// Allocate string using xllr
		const char* test_str = "Hello from CDTS";
		char8_t* allocated = xllr_alloc_string8((const char8_t*)test_str, strlen(test_str));

		data[0].type = metaffi_string8_type;
		data[0].cdt_val.string8_val = allocated;
		data[0].free_required = true;

		// Deserialize
		cdts_python3_serializer deser(*g_runtime, data);

		PyObject* extracted = deser.extract_pyobject();
		const char* extracted_str = pPyUnicode_AsUTF8(extracted);
		CHECK(std::string(extracted_str) == "Hello from CDTS");

		Py_DECREF(extracted);
	}

	// ========================================================================
	// PHASE 4 TESTS: Arrays
	// ========================================================================

	TEST_CASE("Serialize and deserialize 1D array of int64")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Create Python list [1, 2, 3, 4, 5]
		PyObject* list = pPyList_New(5);
		for(int i = 0; i < 5; i++)
		{
			pPyList_SetItem(list, i, pPyLong_FromLongLong(i + 1));
		}

		// Serialize with explicit int64 element type
		ser.add(list, metaffi_int64_type);
		Py_DECREF(list);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();

		CHECK(pPyList_Check(extracted));
		CHECK(pPyList_Size(extracted) == 5);
		for(int i = 0; i < 5; i++)
		{
			CHECK(pPyLong_AsLongLong(pPyList_GetItem(extracted, i)) == i + 1);
		}

		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize and deserialize 1D array of float64")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Create Python list [1.1, 2.2, 3.3]
		PyObject* list = pPyList_New(3);
		pPyList_SetItem(list, 0, pPyFloat_FromDouble(1.1));
		pPyList_SetItem(list, 1, pPyFloat_FromDouble(2.2));
		pPyList_SetItem(list, 2, pPyFloat_FromDouble(3.3));

		// Serialize with explicit float64 element type
		ser.add(list, metaffi_float64_type);
		Py_DECREF(list);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();

		CHECK(pPyList_Check(extracted));
		CHECK(pPyList_Size(extracted) == 3);
		CHECK(pPyFloat_AsDouble(pPyList_GetItem(extracted, 0)) == doctest::Approx(1.1));
		CHECK(pPyFloat_AsDouble(pPyList_GetItem(extracted, 1)) == doctest::Approx(2.2));
		CHECK(pPyFloat_AsDouble(pPyList_GetItem(extracted, 2)) == doctest::Approx(3.3));

		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize and deserialize empty array")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* list = pPyList_New(0);
		// Empty array - use any_type
		ser.add(list, metaffi_any_type);
		Py_DECREF(list);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();

		CHECK(pPyList_Check(extracted));
		CHECK(pPyList_Size(extracted) == 0);

		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize and deserialize 2D array")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Create [[1, 2, 3], [4, 5, 6]]
		PyObject* list = pPyList_New(2);

		PyObject* row1 = pPyList_New(3);
		pPyList_SetItem(row1, 0, pPyLong_FromLongLong(1));
		pPyList_SetItem(row1, 1, pPyLong_FromLongLong(2));
		pPyList_SetItem(row1, 2, pPyLong_FromLongLong(3));

		PyObject* row2 = pPyList_New(3);
		pPyList_SetItem(row2, 0, pPyLong_FromLongLong(4));
		pPyList_SetItem(row2, 1, pPyLong_FromLongLong(5));
		pPyList_SetItem(row2, 2, pPyLong_FromLongLong(6));

		pPyList_SetItem(list, 0, row1);
		pPyList_SetItem(list, 1, row2);

		// Serialize with explicit int64 element type for 2D array
		ser.add(list, metaffi_int64_type);
		Py_DECREF(list);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();

		CHECK(pPyList_Check(extracted));
		CHECK(pPyList_Size(extracted) == 2);

		PyObject* r1 = pPyList_GetItem(extracted, 0);
		CHECK(pPyList_Check(r1));
		CHECK(pPyList_Size(r1) == 3);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r1, 0)) == 1);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r1, 1)) == 2);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r1, 2)) == 3);

		PyObject* r2 = pPyList_GetItem(extracted, 1);
		CHECK(pPyList_Check(r2));
		CHECK(pPyList_Size(r2) == 3);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r2, 0)) == 4);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r2, 1)) == 5);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r2, 2)) == 6);

		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize and deserialize 3D array")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Create [[[1, 2]], [[3, 4]]]
		PyObject* list = pPyList_New(2);

		// First 2D element
		PyObject* arr2d_0 = pPyList_New(1);
		PyObject* arr1d_0 = pPyList_New(2);
		pPyList_SetItem(arr1d_0, 0, pPyLong_FromLongLong(1));
		pPyList_SetItem(arr1d_0, 1, pPyLong_FromLongLong(2));
		pPyList_SetItem(arr2d_0, 0, arr1d_0);

		// Second 2D element
		PyObject* arr2d_1 = pPyList_New(1);
		PyObject* arr1d_1 = pPyList_New(2);
		pPyList_SetItem(arr1d_1, 0, pPyLong_FromLongLong(3));
		pPyList_SetItem(arr1d_1, 1, pPyLong_FromLongLong(4));
		pPyList_SetItem(arr2d_1, 0, arr1d_1);

		pPyList_SetItem(list, 0, arr2d_0);
		pPyList_SetItem(list, 1, arr2d_1);

		// Serialize with explicit int64 element type for 3D array
		ser.add(list, metaffi_int64_type);
		Py_DECREF(list);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();

		CHECK(pPyList_Check(extracted));
		CHECK(pPyList_Size(extracted) == 2);

		// Check first element
		PyObject* e0 = pPyList_GetItem(extracted, 0);
		CHECK(pPyList_Size(e0) == 1);
		PyObject* e00 = pPyList_GetItem(e0, 0);
		CHECK(pPyList_Size(e00) == 2);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(e00, 0)) == 1);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(e00, 1)) == 2);

		// Check second element
		PyObject* e1 = pPyList_GetItem(extracted, 1);
		CHECK(pPyList_Size(e1) == 1);
		PyObject* e10 = pPyList_GetItem(e1, 0);
		CHECK(pPyList_Size(e10) == 2);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(e10, 0)) == 3);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(e10, 1)) == 4);

		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize and deserialize ragged 2D array")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Create [[1], [2, 3], [4, 5, 6]]
		PyObject* list = pPyList_New(3);

		PyObject* row1 = pPyList_New(1);
		pPyList_SetItem(row1, 0, pPyLong_FromLongLong(1));

		PyObject* row2 = pPyList_New(2);
		pPyList_SetItem(row2, 0, pPyLong_FromLongLong(2));
		pPyList_SetItem(row2, 1, pPyLong_FromLongLong(3));

		PyObject* row3 = pPyList_New(3);
		pPyList_SetItem(row3, 0, pPyLong_FromLongLong(4));
		pPyList_SetItem(row3, 1, pPyLong_FromLongLong(5));
		pPyList_SetItem(row3, 2, pPyLong_FromLongLong(6));

		pPyList_SetItem(list, 0, row1);
		pPyList_SetItem(list, 1, row2);
		pPyList_SetItem(list, 2, row3);

		// Serialize with explicit int64 element type for ragged array
		ser.add(list, metaffi_int64_type);
		Py_DECREF(list);

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();

		CHECK(pPyList_Check(extracted));
		CHECK(pPyList_Size(extracted) == 3);

		PyObject* r1 = pPyList_GetItem(extracted, 0);
		CHECK(pPyList_Size(r1) == 1);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r1, 0)) == 1);

		PyObject* r2 = pPyList_GetItem(extracted, 1);
		CHECK(pPyList_Size(r2) == 2);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r2, 0)) == 2);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r2, 1)) == 3);

		PyObject* r3 = pPyList_GetItem(extracted, 2);
		CHECK(pPyList_Size(r3) == 3);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r3, 0)) == 4);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r3, 1)) == 5);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(r3, 2)) == 6);

		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize mixed-type array with metaffi_any_type")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Create [1, "two", 3.0]
		PyObject* list = pPyList_New(3);
		pPyList_SetItem(list, 0, pPyLong_FromLongLong(1));
		pPyList_SetItem(list, 1, pPyUnicode_FromString("two"));
		pPyList_SetItem(list, 2, pPyFloat_FromDouble(3.0));

		ser.add(list, metaffi_any_type);
		Py_DECREF(list);

		CHECK((data[0].type & metaffi_array_type) == metaffi_array_type);
		CHECK((data[0].type & metaffi_any_type) == metaffi_any_type);

		cdts& arr = static_cast<cdts&>(data[0]);
		CHECK(arr.length == 3);
		CHECK(arr[0].type == metaffi_int64_type);
		CHECK(arr[1].type == metaffi_string8_type);
		CHECK(arr[2].type == metaffi_float64_type);

		ser.reset();
		PyObject* extracted = ser.extract_pyobject();
		CHECK(pPyList_Check(extracted));
		CHECK(pPyList_Size(extracted) == 3);
		CHECK(pPyLong_AsLongLong(pPyList_GetItem(extracted, 0)) == 1);
		CHECK(std::string(pPyUnicode_AsUTF8(pPyList_GetItem(extracted, 1))) == "two");
		CHECK(pPyFloat_AsDouble(pPyList_GetItem(extracted, 2)) == doctest::Approx(3.0));
		Py_DECREF(extracted);
	}

	TEST_CASE("Serialize mixed-dimension array with metaffi_any_type")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Create [ [1,2], 3, [[4]] ]
		PyObject* list = pPyList_New(3);

		PyObject* row = pPyList_New(2);
		pPyList_SetItem(row, 0, pPyLong_FromLongLong(1));
		pPyList_SetItem(row, 1, pPyLong_FromLongLong(2));
		pPyList_SetItem(list, 0, row);

		pPyList_SetItem(list, 1, pPyLong_FromLongLong(3));

		PyObject* nested = pPyList_New(1);
		PyObject* inner = pPyList_New(1);
		pPyList_SetItem(inner, 0, pPyLong_FromLongLong(4));
		pPyList_SetItem(nested, 0, inner);
		pPyList_SetItem(list, 2, nested);

		ser.add(list, metaffi_any_type);
		Py_DECREF(list);

		cdts& arr = static_cast<cdts&>(data[0]);
		CHECK(arr.fixed_dimensions == MIXED_OR_UNKNOWN_DIMENSIONS);
		CHECK(arr.length == 3);
		CHECK((arr[0].type & metaffi_array_type) != 0);
		CHECK(arr[1].type == metaffi_int64_type);
		CHECK((arr[2].type & metaffi_array_type) != 0);
	}

	// ========================================================================
	// TYPE PRESERVATION TESTS
	// Verify that CDTS stores the exact type that was specified
	// ========================================================================

	TEST_CASE("Verify CDTS stores correct integer types after serialization")
	{
		cdts data(8);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize same value (42) as different integer types
		PyObject* val = pPyLong_FromLong(42);
		
		ser.add(val, metaffi_int8_type)
		   .add(val, metaffi_int16_type)
		   .add(val, metaffi_int32_type)
		   .add(val, metaffi_int64_type)
		   .add(val, metaffi_uint8_type)
		   .add(val, metaffi_uint16_type)
		   .add(val, metaffi_uint32_type)
		   .add(val, metaffi_uint64_type);

		Py_DECREF(val);

		// Verify CDTS has correct types stored
		CHECK(data[0].type == metaffi_int8_type);
		CHECK(data[1].type == metaffi_int16_type);
		CHECK(data[2].type == metaffi_int32_type);
		CHECK(data[3].type == metaffi_int64_type);
		CHECK(data[4].type == metaffi_uint8_type);
		CHECK(data[5].type == metaffi_uint16_type);
		CHECK(data[6].type == metaffi_uint32_type);
		CHECK(data[7].type == metaffi_uint64_type);

		// Verify values are correct too
		CHECK(data[0].cdt_val.int8_val == 42);
		CHECK(data[1].cdt_val.int16_val == 42);
		CHECK(data[2].cdt_val.int32_val == 42);
		CHECK(data[3].cdt_val.int64_val == 42);
		CHECK(data[4].cdt_val.uint8_val == 42);
		CHECK(data[5].cdt_val.uint16_val == 42);
		CHECK(data[6].cdt_val.uint32_val == 42);
		CHECK(data[7].cdt_val.uint64_val == 42);
	}

	TEST_CASE("Verify CDTS stores correct float types after serialization")
	{
		cdts data(2);
		cdts_python3_serializer ser(*g_runtime, data);

		// Serialize same value as different float types
		PyObject* val = pPyFloat_FromDouble(3.14);
		
		ser.add(val, metaffi_float32_type)
		   .add(val, metaffi_float64_type);

		Py_DECREF(val);

		// Verify CDTS has correct types stored
		CHECK(data[0].type == metaffi_float32_type);
		CHECK(data[1].type == metaffi_float64_type);

		// Verify values are approximately correct
		CHECK(data[0].cdt_val.float32_val == doctest::Approx(3.14f));
		CHECK(data[1].cdt_val.float64_val == doctest::Approx(3.14));
	}

	TEST_CASE("Serialize int as int32 (not int64)")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* val = pPyLong_FromLong(1000);
		ser.add(val, metaffi_int32_type);
		Py_DECREF(val);

		// Verify int32 type (not default int64)
		CHECK(data[0].type == metaffi_int32_type);
		CHECK(data[0].cdt_val.int32_val == 1000);
	}

	TEST_CASE("Serialize float as float32 (not float64)")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* val = pPyFloat_FromDouble(2.5);
		ser.add(val, metaffi_float32_type);
		Py_DECREF(val);

		// Verify float32 type (not default float64)
		CHECK(data[0].type == metaffi_float32_type);
		CHECK(data[0].cdt_val.float32_val == doctest::Approx(2.5f));
	}

	// ========================================================================
	// VALIDATION TESTS - Range checking
	// ========================================================================

	TEST_CASE("Error: Value out of range for int8")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* val = pPyLong_FromLong(300);  // Too large for int8 [-128, 127]
		CHECK_THROWS_WITH(ser.add(val, metaffi_int8_type), 
		                  doctest::Contains("out of range for int8"));
		Py_DECREF(val);
	}

	TEST_CASE("Error: Negative value for uint8")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* val = pPyLong_FromLong(-10);  // Negative not allowed for unsigned
		CHECK_THROWS_WITH(ser.add(val, metaffi_uint8_type), 
		                  doctest::Contains("out of range for uint8"));
		Py_DECREF(val);
	}

	TEST_CASE("Error: Value out of range for int16")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* val = pPyLong_FromLong(50000);  // Too large for int16 [-32768, 32767]
		CHECK_THROWS_WITH(ser.add(val, metaffi_int16_type), 
		                  doctest::Contains("out of range for int16"));
		Py_DECREF(val);
	}

	TEST_CASE("Error: Value out of range for uint32")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* val = pPyLong_FromLongLong(5000000000LL);  // Too large for uint32
		CHECK_THROWS_WITH(ser.add(val, metaffi_uint32_type), 
		                  doctest::Contains("out of range for uint32"));
		Py_DECREF(val);
	}

	TEST_CASE("Boundary values: int8 min/max")
	{
		cdts data(2);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* min_val = pPyLong_FromLong(-128);
		PyObject* max_val = pPyLong_FromLong(127);
		
		CHECK_NOTHROW(ser.add(min_val, metaffi_int8_type));
		CHECK_NOTHROW(ser.add(max_val, metaffi_int8_type));
		
		Py_DECREF(min_val);
		Py_DECREF(max_val);

		CHECK(data[0].cdt_val.int8_val == -128);
		CHECK(data[1].cdt_val.int8_val == 127);
	}

	TEST_CASE("Boundary values: uint8 min/max")
	{
		cdts data(2);
		cdts_python3_serializer ser(*g_runtime, data);

		PyObject* min_val = pPyLong_FromLong(0);
		PyObject* max_val = pPyLong_FromLong(255);
		
		CHECK_NOTHROW(ser.add(min_val, metaffi_uint8_type));
		CHECK_NOTHROW(ser.add(max_val, metaffi_uint8_type));
		
		Py_DECREF(min_val);
		Py_DECREF(max_val);

		CHECK(data[0].cdt_val.uint8_val == 0);
		CHECK(data[1].cdt_val.uint8_val == 255);
	}

	// ========================================================================
	// PYTHON ERROR MESSAGE TESTS
	// Verify that Python error messages are properly captured
	// ========================================================================

	TEST_CASE("Python error messages are captured: integer overflow")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Create a Python integer that's too large for long long
		PyObject* huge_val = pPyLong_FromString("99999999999999999999999999999999999999", nullptr, 10);
		
		try
		{
			ser.add(huge_val, metaffi_int64_type);
			Py_DECREF(huge_val);
			FAIL("Expected exception was not thrown");
		}
		catch(const std::runtime_error& e)
		{
			Py_DECREF(huge_val);
			std::string error_msg(e.what());
			
			// Verify that the error message contains our context message
			// This proves we're using the new error handling that extracts Python errors
			CHECK(error_msg.find("Failed to convert Python int to long long:") != std::string::npos);
			
			// Verify the message is not just our hardcoded part - it should have additional info
			// The old code would have said "Integer overflow in PyLong_AsLongLong" without the colon
			CHECK(error_msg.length() > strlen("Failed to convert Python int to long long: "));
		}
	}

	// ========================================================================
	// PHASE 5 TESTS: Handles & Callables
	// ========================================================================

	TEST_CASE("Serialize and deserialize handle")
	{
		cdts data(1);
		cdts_python3_serializer ser(*g_runtime, data);

		// Create a Python dict as an arbitrary object to wrap in a handle
		PyObject* original_dict = pPyDict_New();
		pPyDict_SetItemString(original_dict, "key", pPyLong_FromLong(42));

		// Handles are unambiguous - use handle type
		ser.add(original_dict, metaffi_handle_type);
		Py_DECREF(original_dict);  // Serializer should have incremented refcount

		// Deserialize
		ser.reset();
		PyObject* extracted = ser.extract_pyobject();

		// Should get the same dict back
		CHECK(pPyDict_Check(extracted));
		PyObject* value = pPyDict_GetItemString(extracted, "key");
		CHECK(pPyLong_AsLong(value) == 42);

		Py_DECREF(extracted);
	}

	TEST_CASE("Deserialize pre-filled CDTS with handle")
	{
		// Manually create CDTS with handle
		cdts data(1);

		// Create a Python list and wrap it in a handle
		PyObject* py_list = pPyList_New(2);
		pPyList_SetItem(py_list, 0, pPyLong_FromLong(10));
		pPyList_SetItem(py_list, 1, pPyLong_FromLong(20));

		data[0].type = metaffi_handle_type;
		void* handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
		REQUIRE(handle_mem != nullptr);
		data[0].cdt_val.handle_val = new (handle_mem) cdt_metaffi_handle();
		data[0].cdt_val.handle_val->handle = py_list;
		data[0].cdt_val.handle_val->runtime_id = PYTHON3_RUNTIME_ID;
		data[0].cdt_val.handle_val->release = cpython3_runtime_manager::py_object_releaser;
		data[0].free_required = true;

		// Deserialize
		cdts_python3_serializer deser(*g_runtime, data);
		PyObject* extracted = deser.extract_pyobject();

		CHECK(pPyList_Check(extracted));
		CHECK(pPyList_Size(extracted) == 2);
		CHECK(pPyLong_AsLong(pPyList_GetItem(extracted, 0)) == 10);
		CHECK(pPyLong_AsLong(pPyList_GetItem(extracted, 1)) == 20);

		Py_DECREF(extracted);
	}

	TEST_CASE("Deserialize pre-filled CDTS with callable")
	{
		// Manually create CDTS with callable
		cdts data(1);

		// Create a mock callable structure using xllr allocation
		data[0].type = metaffi_callable_type;
		data[0].cdt_val.callable_val = (cdt_metaffi_callable*)xllr_alloc_memory(sizeof(cdt_metaffi_callable));
		void* xcall_mem = xllr_alloc_memory(sizeof(xcall));
		REQUIRE(xcall_mem != nullptr);
		auto* xcall_ptr = new (xcall_mem) xcall();
		xcall_ptr->pxcall_and_context[0] = reinterpret_cast<void*>(&dummy_xcall);
		xcall_ptr->pxcall_and_context[1] = nullptr;
		data[0].cdt_val.callable_val->val = static_cast<metaffi_callable>(xcall_ptr);

		// Allocate parameter and return type arrays using xllr
		data[0].cdt_val.callable_val->parameters_types = (metaffi_type*)xllr_alloc_memory(2 * sizeof(metaffi_type));
		data[0].cdt_val.callable_val->parameters_types[0] = metaffi_int32_type;
		data[0].cdt_val.callable_val->parameters_types[1] = metaffi_string8_type;
		data[0].cdt_val.callable_val->params_types_length = 2;

		data[0].cdt_val.callable_val->retval_types = (metaffi_type*)xllr_alloc_memory(1 * sizeof(metaffi_type));
		data[0].cdt_val.callable_val->retval_types[0] = metaffi_float64_type;
		data[0].cdt_val.callable_val->retval_types_length = 1;

		data[0].free_required = true;  // CDT destructor will handle cleanup

		// Deserialize
		TEST_LOG("About to extract callable");
		cdts_python3_serializer deser(*g_runtime, data);
		PyObject* extracted = deser.extract_pyobject();

		// Should be callable
		CHECK(pPyCallable_Check(extracted));
		TEST_LOG("Extracted callable");

		Py_DECREF(extracted);

		// No manual cleanup needed - CDT destructor handles it via free_required=true
	}
}
