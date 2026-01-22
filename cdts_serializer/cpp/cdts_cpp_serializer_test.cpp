#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "cdts_cpp_serializer.h"
#include <runtime/xllr_capi_loader.h>

using namespace metaffi::utils;

TEST_SUITE("CDTS C++ Serializer")
{
	TEST_CASE("Serialize and deserialize int8")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_int8 original = -42;
		ser << original;

		ser.reset();
		metaffi_int8 extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize int16")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_int16 original = -12345;
		ser << original;

		ser.reset();
		metaffi_int16 extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize int32")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_int32 original = -123456789;
		ser << original;

		ser.reset();
		metaffi_int32 extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize int64")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_int64 original = -1234567890123456LL;
		ser << original;

		ser.reset();
		metaffi_int64 extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize uint8")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_uint8 original = 255;
		ser << original;

		ser.reset();
		metaffi_uint8 extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize uint16")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_uint16 original = 65535;
		ser << original;

		ser.reset();
		metaffi_uint16 extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize uint32")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_uint32 original = 4294967295U;
		ser << original;

		ser.reset();
		metaffi_uint32 extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize uint64")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_uint64 original = 18446744073709551615ULL;
		ser << original;

		ser.reset();
		metaffi_uint64 extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize float32")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_float32 original = 3.14159f;
		ser << original;

		ser.reset();
		metaffi_float32 extracted;
		ser >> extracted;

		CHECK(extracted == doctest::Approx(original));
	}

	TEST_CASE("Serialize and deserialize float64")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_float64 original = 2.71828182845904523536;
		ser << original;

		ser.reset();
		metaffi_float64 extracted;
		ser >> extracted;

		CHECK(extracted == doctest::Approx(original));
	}

	TEST_CASE("Serialize and deserialize bool")
	{
		cdts data(2);
		cdts_cpp_serializer ser(data);

		ser << true << false;

		ser.reset();
		bool b1, b2;
		ser >> b1 >> b2;

		CHECK(b1 == true);
		CHECK(b2 == false);
	}

	TEST_CASE("Serialize and deserialize multiple primitives")
	{
		cdts data(5);
		cdts_cpp_serializer ser(data);

		// Serialize
		ser << metaffi_int32(42) << metaffi_float32(3.14f) << metaffi_float64(2.71828) << true << metaffi_uint64(999);

		// Deserialize
		metaffi_int32 i;
		metaffi_float32 f;
		metaffi_float64 d;
		bool b;
		metaffi_uint64 u;

		ser.reset();
		ser >> i >> f >> d >> b >> u;

		CHECK(i == 42);
		CHECK(f == doctest::Approx(3.14f));
		CHECK(d == doctest::Approx(2.71828));
		CHECK(b == true);
		CHECK(u == 999);
	}

	TEST_CASE("Serialize and deserialize std::string")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		std::string original = "Hello, MetaFFI!";
		ser << original;

		ser.reset();
		std::string extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize std::u16string")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		std::u16string original = u"Hello UTF-16 מה שלומך";
		ser << original;

		ser.reset();
		std::u16string extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize std::u32string")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		std::u32string original = U"Hello UTF-32 🚀";
		ser << original;

		ser.reset();
		std::u32string extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize const char*")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		ser << "C-style string";

		ser.reset();
		std::string extracted;
		ser >> extracted;

		CHECK(extracted == "C-style string");
	}

	TEST_CASE("Serialize and deserialize empty string")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		std::string original = "";
		ser << original;

		ser.reset();
		std::string extracted;
		ser >> extracted;

		CHECK(extracted == original);
	}

	TEST_CASE("Serialize and deserialize 1D array of int32")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		std::vector<metaffi_int32> original = {1, 2, 3, 4, 5};
		ser << original;

		ser.reset();
		std::vector<metaffi_int32> extracted;
		ser >> extracted;

		REQUIRE(extracted.size() == original.size());
		for (size_t i = 0; i < original.size(); ++i)
		{
			CHECK(extracted[i] == original[i]);
		}
	}

	TEST_CASE("Serialize and deserialize 1D array of double")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		std::vector<metaffi_float64> original = {1.1, 2.2, 3.3};
		ser << original;

		ser.reset();
		std::vector<metaffi_float64> extracted;
		ser >> extracted;

		REQUIRE(extracted.size() == original.size());
		for (size_t i = 0; i < original.size(); ++i)
		{
			CHECK(extracted[i] == doctest::Approx(original[i]));
		}
	}

	TEST_CASE("Serialize and deserialize empty array")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		std::vector<metaffi_int32> original;
		ser << original;

		ser.reset();
		std::vector<metaffi_int32> extracted;
		ser >> extracted;

		CHECK(extracted.empty());
	}

	TEST_CASE("Serialize and deserialize 2D array")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		std::vector<std::vector<metaffi_int32>> original = {{1, 2, 3}, {4, 5, 6}};
		ser << original;

		ser.reset();
		std::vector<std::vector<metaffi_int32>> extracted;
		ser >> extracted;

		REQUIRE(extracted.size() == original.size());
		for (size_t i = 0; i < original.size(); ++i)
		{
			REQUIRE(extracted[i].size() == original[i].size());
			for (size_t j = 0; j < original[i].size(); ++j)
			{
				CHECK(extracted[i][j] == original[i][j]);
			}
		}
	}

	TEST_CASE("Serialize and deserialize 3D array")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		std::vector<std::vector<std::vector<metaffi_int32>>> original = {
			{{1, 2}, {3, 4}},
			{{5, 6}, {7, 8}}
		};
		ser << original;

		ser.reset();
		std::vector<std::vector<std::vector<metaffi_int32>>> extracted;
		ser >> extracted;

		REQUIRE(extracted.size() == 2);
		REQUIRE(extracted[0].size() == 2);
		REQUIRE(extracted[0][0].size() == 2);
		CHECK(extracted[0][0][0] == 1);
		CHECK(extracted[0][0][1] == 2);
		CHECK(extracted[0][1][0] == 3);
		CHECK(extracted[0][1][1] == 4);
		CHECK(extracted[1][0][0] == 5);
		CHECK(extracted[1][0][1] == 6);
		CHECK(extracted[1][1][0] == 7);
		CHECK(extracted[1][1][1] == 8);
	}

	TEST_CASE("Serialize and deserialize ragged 2D array")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		std::vector<std::vector<metaffi_int32>> original = {{1}, {2, 3}, {4, 5, 6}};
		ser << original;

		ser.reset();
		std::vector<std::vector<metaffi_int32>> extracted;
		ser >> extracted;

		REQUIRE(extracted.size() == 3);
		REQUIRE(extracted[0].size() == 1);
		REQUIRE(extracted[1].size() == 2);
		REQUIRE(extracted[2].size() == 3);
		CHECK(extracted[0][0] == 1);
		CHECK(extracted[1][0] == 2);
		CHECK(extracted[1][1] == 3);
		CHECK(extracted[2][0] == 4);
		CHECK(extracted[2][1] == 5);
		CHECK(extracted[2][2] == 6);
	}

	TEST_CASE("Serialize and deserialize null")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		ser.null();

		ser.reset();
		CHECK(ser.is_null());
		CHECK(ser.peek_type() == metaffi_null_type);
	}

	TEST_CASE("Type query with peek_type")
	{
		cdts data(3);
		cdts_cpp_serializer ser(data);

		ser << metaffi_int32(42) << std::string("hello") << metaffi_float64(3.14);

		ser.reset();

		CHECK(ser.peek_type() == metaffi_int32_type);
		metaffi_int32 i;
		ser >> i;

		CHECK(ser.peek_type() == metaffi_string8_type);
		std::string s;
		ser >> s;

		CHECK(ser.peek_type() == metaffi_float64_type);
		metaffi_float64 d;
		ser >> d;
	}

	TEST_CASE("Extract ANY type - int32")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		ser << metaffi_int32(99);

		ser.reset();
		auto value = ser.extract_any();

		CHECK(std::holds_alternative<metaffi_int32>(value));
		CHECK(std::get<metaffi_int32>(value) == 99);
	}

	TEST_CASE("Extract ANY type - string")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		ser << std::string("test string");

		ser.reset();
		auto value = ser.extract_any();

		CHECK(std::holds_alternative<std::string>(value));
		CHECK(std::get<std::string>(value) == "test string");
	}

	TEST_CASE("Extract ANY type - float64")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		ser << metaffi_float64(2.71828);

		ser.reset();
		auto value = ser.extract_any();

		CHECK(std::holds_alternative<metaffi_float64>(value));
		CHECK(std::get<metaffi_float64>(value) == doctest::Approx(2.71828));
	}

	TEST_CASE("Extract ANY type - bool")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		ser << true;

		ser.reset();
		auto value = ser.extract_any();

		CHECK(std::holds_alternative<bool>(value));
		CHECK(std::get<bool>(value) == true);
	}

	TEST_CASE("Extract ANY type - null")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		ser.null();

		ser.reset();
		auto value = ser.extract_any();

		CHECK(std::holds_alternative<std::monostate>(value));
	}

	TEST_CASE("Utility methods")
	{
		cdts data(5);
		cdts_cpp_serializer ser(data);

		CHECK(ser.get_index() == 0);
		CHECK(ser.size() == 5);
		CHECK(ser.has_more());

		ser << metaffi_int32(1) << metaffi_int32(2) << metaffi_int32(3);
		CHECK(ser.get_index() == 3);
		CHECK(ser.has_more());

		ser.reset();
		CHECK(ser.get_index() == 0);

		ser.set_index(2);
		CHECK(ser.get_index() == 2);
	}

	TEST_CASE("Deserialization from existing CDTS")
	{
		// Simulate receiving CDTS from MetaFFI call
		cdts data(3);
		data[0] = metaffi_int32(123);
		data[1].set_string((const char8_t*)u8"test", true);
		data[2] = metaffi_float64(9.99);

		// Deserialize
		cdts_cpp_serializer ser(data);
		metaffi_int32 i;
		std::string s;
		metaffi_float64 d;
		ser >> i >> s >> d;

		CHECK(i == 123);
		CHECK(s == "test");
		CHECK(d == doctest::Approx(9.99));
	}

	TEST_CASE("Error: Type mismatch on deserialization")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		ser << metaffi_int32(42);
		ser.reset();

		// Try to extract as wrong type
		std::string s;
		CHECK_THROWS_AS(ser >> s, std::runtime_error);
	}

	TEST_CASE("Error: Bounds violation on serialization")
	{
		cdts data(2);
		cdts_cpp_serializer ser(data);

		ser << metaffi_int32(1) << metaffi_int32(2);

		// Try to add a third element
		CHECK_THROWS_AS(ser << metaffi_int32(3), std::out_of_range);
	}

	TEST_CASE("Error: Bounds violation on deserialization")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		ser << metaffi_int32(42);
		ser.reset();

		metaffi_int32 i1, i2;
		ser >> i1;

		// Try to read beyond bounds - check_bounds throws out_of_range
		CHECK_THROWS_AS(ser >> i2, std::out_of_range);
	}

	TEST_CASE("Error: Peek type beyond bounds")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		ser << metaffi_int32(42);

		// Index is now at 1, which is out of bounds
		CHECK_THROWS_AS(ser.peek_type(), std::out_of_range);
	}

	TEST_CASE("Mixed types serialization/deserialization")
	{
		cdts data(6);
		cdts_cpp_serializer ser(data);

		// Serialize mixed types
		ser << metaffi_int8(10)
		    << std::string("hello")
		    << metaffi_float32(3.14f)
		    << true
		    << metaffi_uint64(9999)
		    << std::u16string(u"world");

		// Deserialize
		ser.reset();
		metaffi_int8 i8;
		std::string s;
		metaffi_float32 f32;
		bool b;
		metaffi_uint64 u64;
		std::u16string s16;

		ser >> i8 >> s >> f32 >> b >> u64 >> s16;

		CHECK(i8 == 10);
		CHECK(s == "hello");
		CHECK(f32 == doctest::Approx(3.14f));
		CHECK(b == true);
		CHECK(u64 == 9999);
		CHECK(s16 == u"world");
	}

	TEST_CASE("Chaining operations")
	{
		cdts data(3);
		cdts_cpp_serializer ser(data);

		// Chain serialization
		ser << metaffi_int32(1) << metaffi_int32(2) << metaffi_int32(3);

		// Chain deserialization
		metaffi_int32 a, b, c;
		ser.reset();
		ser >> a >> b >> c;

		CHECK(a == 1);
		CHECK(b == 2);
		CHECK(c == 3);
	}

	TEST_CASE("Handle serialization")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		int some_value = 42;
		cdt_metaffi_handle handle(&some_value, 100, nullptr);

		ser << handle;

		ser.reset();
		cdt_metaffi_handle extracted;
		ser >> extracted;

		CHECK(extracted.handle == handle.handle);
		CHECK(extracted.runtime_id == handle.runtime_id);
		CHECK(*static_cast<int*>(extracted.handle) == 42);
	}

	TEST_CASE("Callable serialization")
	{
		cdts data(1);
		cdts_cpp_serializer ser(data);

		metaffi_type param_types[] = {metaffi_int32_type, metaffi_string8_type};
		metaffi_type retval_types[] = {metaffi_float64_type};
		cdt_metaffi_callable callable((void*)0x1234ABCD, param_types, 2, retval_types, 1);  // Use non-null stubbed value

		ser << callable;

		ser.reset();
		cdt_metaffi_callable extracted;
		ser >> extracted;

		CHECK(extracted.val == callable.val);
		CHECK(extracted.params_types_length == 2);
		CHECK(extracted.retval_types_length == 1);
		CHECK(extracted.parameters_types[0] == metaffi_int32_type);
		CHECK(extracted.parameters_types[1] == metaffi_string8_type);
		CHECK(extracted.retval_types[0] == metaffi_float64_type);
	}

	// =============================================================================
	// TYPE PRESERVATION TESTS
	// Verify that CDTS stores the exact type that was serialized
	// =============================================================================

	TEST_CASE("Verify CDTS stores correct integer types after serialization")
	{
		cdts data(8);
		cdts_cpp_serializer ser(data);

		// Serialize different integer types
		ser << int8_t(-10) << int16_t(-1000) << int32_t(-100000) << int64_t(-10000000)
		    << uint8_t(10) << uint16_t(1000) << uint32_t(100000) << uint64_t(10000000);

		// Verify CDTS has correct types stored
		CHECK(data[0].type == metaffi_int8_type);
		CHECK(data[1].type == metaffi_int16_type);
		CHECK(data[2].type == metaffi_int32_type);
		CHECK(data[3].type == metaffi_int64_type);
		CHECK(data[4].type == metaffi_uint8_type);
		CHECK(data[5].type == metaffi_uint16_type);
		CHECK(data[6].type == metaffi_uint32_type);
		CHECK(data[7].type == metaffi_uint64_type);
	}

	TEST_CASE("Verify CDTS stores correct float types after serialization")
	{
		cdts data(2);
		cdts_cpp_serializer ser(data);

		// Serialize different float types
		ser << float(3.14f) << double(2.71828);

		// Verify CDTS has correct types stored
		CHECK(data[0].type == metaffi_float32_type);
		CHECK(data[1].type == metaffi_float64_type);
	}

	TEST_CASE("Verify CDTS stores correct types for mixed primitives")
	{
		cdts data(5);
		cdts_cpp_serializer ser(data);

		// Serialize mixed types
		ser << int32_t(42) << float(3.14f) << double(2.71828) << true << int16_t(1000);

		// Verify types
		CHECK(data[0].type == metaffi_int32_type);
		CHECK(data[1].type == metaffi_float32_type);
		CHECK(data[2].type == metaffi_float64_type);
		CHECK(data[3].type == metaffi_bool_type);
		CHECK(data[4].type == metaffi_int16_type);
	}

	// =============================================================================
	// DESERIALIZATION-ONLY TESTS
	// These simulate receiving pre-filled CDTS from MetaFFI cross-language calls
	// =============================================================================

	TEST_CASE("Deserialize pre-filled CDTS with all primitive types")
	{
		// Manually create CDTS as if received from MetaFFI
		cdts data(11);
		data[0] = int8_t(-10);
		data[1] = int16_t(-1000);
		data[2] = int32_t(-100000);
		data[3] = int64_t(-10000000);
		data[4] = uint8_t(10);
		data[5] = uint16_t(1000);
		data[6] = uint32_t(100000);
		data[7] = uint64_t(10000000);
		data[8] = float(3.14f);
		data[9] = double(2.71828);
		data[10] = true;

		// Deserialize
		cdts_cpp_serializer deser(data);
		int8_t i8; int16_t i16; int32_t i32; int64_t i64;
		uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
		float f; double d; bool b;

		deser >> i8 >> i16 >> i32 >> i64 >> u8 >> u16 >> u32 >> u64 >> f >> d >> b;

		CHECK(i8 == -10);
		CHECK(i16 == -1000);
		CHECK(i32 == -100000);
		CHECK(i64 == -10000000);
		CHECK(u8 == 10);
		CHECK(u16 == 1000);
		CHECK(u32 == 100000);
		CHECK(u64 == 10000000);
		CHECK(f == doctest::Approx(3.14f));
		CHECK(d == doctest::Approx(2.71828));
		CHECK(b == true);
	}

	TEST_CASE("Deserialize pre-filled CDTS with strings")
	{
		// Manually create CDTS with strings
		cdts data(3);
		data[0].set_string((const char8_t*)"Hello UTF-8", true);
		data[1].set_string(u"Hello UTF-16", true);
		data[2].set_string(U"Hello UTF-32", true);

		// Deserialize
		cdts_cpp_serializer deser(data);
		std::string s8; std::u16string s16; std::u32string s32;
		deser >> s8 >> s16 >> s32;

		CHECK(s8 == "Hello UTF-8");
		CHECK(s16 == u"Hello UTF-16");
		CHECK(s32 == U"Hello UTF-32");
	}

	TEST_CASE("Deserialize pre-filled CDTS with 1D array")
	{
		// Manually create 1D array: [10, 20, 30, 40, 50]
		cdts data(1);
		data[0].set_new_array(5, 1, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr = static_cast<cdts&>(data[0]);
		arr[0] = int32_t(10);
		arr[1] = int32_t(20);
		arr[2] = int32_t(30);
		arr[3] = int32_t(40);
		arr[4] = int32_t(50);

		// Deserialize
		cdts_cpp_serializer deser(data);
		std::vector<int32_t> result;
		deser >> result;

		REQUIRE(result.size() == 5);
		CHECK(result[0] == 10);
		CHECK(result[1] == 20);
		CHECK(result[2] == 30);
		CHECK(result[3] == 40);
		CHECK(result[4] == 50);
	}

	TEST_CASE("Deserialize pre-filled CDTS with 2D array")
	{
		// Manually create 2D array: [[1, 2, 3], [4, 5, 6]]
		cdts data(1);
		data[0].set_new_array(2, 2, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr = static_cast<cdts&>(data[0]);

		arr[0].set_new_array(3, 1, static_cast<metaffi_types>(metaffi_int32_type));
		static_cast<cdts&>(arr[0])[0] = int32_t(1);
		static_cast<cdts&>(arr[0])[1] = int32_t(2);
		static_cast<cdts&>(arr[0])[2] = int32_t(3);

		arr[1].set_new_array(3, 1, static_cast<metaffi_types>(metaffi_int32_type));
		static_cast<cdts&>(arr[1])[0] = int32_t(4);
		static_cast<cdts&>(arr[1])[1] = int32_t(5);
		static_cast<cdts&>(arr[1])[2] = int32_t(6);

		// Deserialize
		cdts_cpp_serializer deser(data);
		std::vector<std::vector<int32_t>> result;
		deser >> result;

		REQUIRE(result.size() == 2);
		REQUIRE(result[0].size() == 3);
		REQUIRE(result[1].size() == 3);
		CHECK(result[0][0] == 1);
		CHECK(result[0][1] == 2);
		CHECK(result[0][2] == 3);
		CHECK(result[1][0] == 4);
		CHECK(result[1][1] == 5);
		CHECK(result[1][2] == 6);
	}

	TEST_CASE("Deserialize pre-filled CDTS with 3D array")
	{
		// Manually create 3D array: [[[1, 2]], [[3, 4]]]
		cdts data(1);
		data[0].set_new_array(2, 3, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr3d = static_cast<cdts&>(data[0]);

		// First 2D element
		arr3d[0].set_new_array(1, 2, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr2d_0 = static_cast<cdts&>(arr3d[0]);
		arr2d_0[0].set_new_array(2, 1, static_cast<metaffi_types>(metaffi_int32_type));
		static_cast<cdts&>(arr2d_0[0])[0] = int32_t(1);
		static_cast<cdts&>(arr2d_0[0])[1] = int32_t(2);

		// Second 2D element
		arr3d[1].set_new_array(1, 2, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr2d_1 = static_cast<cdts&>(arr3d[1]);
		arr2d_1[0].set_new_array(2, 1, static_cast<metaffi_types>(metaffi_int32_type));
		static_cast<cdts&>(arr2d_1[0])[0] = int32_t(3);
		static_cast<cdts&>(arr2d_1[0])[1] = int32_t(4);

		// Deserialize
		cdts_cpp_serializer deser(data);
		std::vector<std::vector<std::vector<int32_t>>> result;
		deser >> result;

		REQUIRE(result.size() == 2);
		REQUIRE(result[0].size() == 1);
		REQUIRE(result[0][0].size() == 2);
		CHECK(result[0][0][0] == 1);
		CHECK(result[0][0][1] == 2);
		CHECK(result[1][0][0] == 3);
		CHECK(result[1][0][1] == 4);
	}

	TEST_CASE("Deserialize pre-filled CDTS with ragged array")
	{
		// Manually create ragged array: [[1], [2, 3], [4, 5, 6]]
		cdts data(1);
		data[0].set_new_array(3, 2, static_cast<metaffi_types>(metaffi_int32_type));
		cdts& arr = static_cast<cdts&>(data[0]);

		arr[0].set_new_array(1, 1, static_cast<metaffi_types>(metaffi_int32_type));
		static_cast<cdts&>(arr[0])[0] = int32_t(1);

		arr[1].set_new_array(2, 1, static_cast<metaffi_types>(metaffi_int32_type));
		static_cast<cdts&>(arr[1])[0] = int32_t(2);
		static_cast<cdts&>(arr[1])[1] = int32_t(3);

		arr[2].set_new_array(3, 1, static_cast<metaffi_types>(metaffi_int32_type));
		static_cast<cdts&>(arr[2])[0] = int32_t(4);
		static_cast<cdts&>(arr[2])[1] = int32_t(5);
		static_cast<cdts&>(arr[2])[2] = int32_t(6);

		// Deserialize
		cdts_cpp_serializer deser(data);
		std::vector<std::vector<int32_t>> result;
		deser >> result;

		REQUIRE(result.size() == 3);
		REQUIRE(result[0].size() == 1);
		REQUIRE(result[1].size() == 2);
		REQUIRE(result[2].size() == 3);
		CHECK(result[0][0] == 1);
		CHECK(result[1][0] == 2);
		CHECK(result[1][1] == 3);
		CHECK(result[2][0] == 4);
		CHECK(result[2][1] == 5);
		CHECK(result[2][2] == 6);
	}

	TEST_CASE("Deserialize pre-filled CDTS with NULL value")
	{
		// Manually create CDTS with null
		cdts data(1);
		data[0].type = metaffi_null_type;
		data[0].free_required = false;

		// Deserialize
		cdts_cpp_serializer deser(data);

		CHECK(deser.is_null());
		CHECK(deser.peek_type() == metaffi_null_type);
	}

	TEST_CASE("Deserialize pre-filled CDTS with handle")
	{
		// Manually create CDTS with handle
		cdts data(1);
		int some_value = 99;
		cdt_metaffi_handle orig_handle(&some_value, 123, nullptr);
		data[0].set_handle(&orig_handle);

		// Deserialize
		cdts_cpp_serializer deser(data);
		cdt_metaffi_handle extracted;
		deser >> extracted;

		CHECK(extracted.handle == orig_handle.handle);
		CHECK(extracted.runtime_id == 123);
		CHECK(*static_cast<int*>(extracted.handle) == 99);
	}

	TEST_CASE("Deserialize pre-filled CDTS with callable")
	{
		// Manually create CDTS with callable
		cdts data(1);
		
		// Create callable with xllr-allocated arrays
		cdt_metaffi_callable* callable = (cdt_metaffi_callable*)xllr_alloc_memory(sizeof(cdt_metaffi_callable));
		callable->val = (void*)0x5678EFAB;
		callable->params_types_length = 2;
		callable->retval_types_length = 1;
		callable->parameters_types = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * 2);
		callable->parameters_types[0] = metaffi_int32_type;
		callable->parameters_types[1] = metaffi_bool_type;
		callable->retval_types = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * 1);
		callable->retval_types[0] = metaffi_string8_type;
		
		data[0].type = metaffi_callable_type;
		data[0].cdt_val.callable_val = callable;
		data[0].free_required = true;

		// Deserialize
		cdts_cpp_serializer deser(data);
		cdt_metaffi_callable extracted;
		deser >> extracted;

		CHECK(extracted.val == callable->val);
		CHECK(extracted.params_types_length == 2);
		CHECK(extracted.retval_types_length == 1);
		CHECK(extracted.parameters_types[0] == metaffi_int32_type);
		CHECK(extracted.parameters_types[1] == metaffi_bool_type);
		CHECK(extracted.retval_types[0] == metaffi_string8_type);
	}

	TEST_CASE("Type query on pre-filled CDTS before deserialization")
	{
		// Manually create CDTS with various types
		cdts data(4);
		data[0] = int32_t(42);
		data[1].set_string((const char8_t*)"test", true);
		data[2] = double(3.14);
		data[3].type = metaffi_null_type;
		data[3].free_required = false;

		cdts_cpp_serializer deser(data);

		// Query types before deserializing
		CHECK(deser.peek_type() == metaffi_int32_type);
		int32_t i; deser >> i;

		CHECK(deser.peek_type() == metaffi_string8_type);
		std::string s; deser >> s;

		CHECK(deser.peek_type() == metaffi_float64_type);
		double d; deser >> d;

		CHECK(deser.is_null());
		CHECK(deser.peek_type() == metaffi_null_type);

		CHECK(i == 42);
		CHECK(s == "test");
		CHECK(d == doctest::Approx(3.14));
	}

	TEST_CASE("Extract ANY from pre-filled CDTS")
	{
		// Manually create CDTS
		cdts data(4);
		data[0] = int32_t(42);
		data[1].set_string((const char8_t*)"hello", true);
		data[2] = double(3.14);
		data[3] = true;

		cdts_cpp_serializer deser(data);

		// Extract using ANY
		auto v1 = deser.extract_any();
		auto v2 = deser.extract_any();
		auto v3 = deser.extract_any();
		auto v4 = deser.extract_any();

		CHECK(std::get<int32_t>(v1) == 42);
		CHECK(std::get<std::string>(v2) == "hello");
		CHECK(std::get<double>(v3) == doctest::Approx(3.14));
		CHECK(std::get<bool>(v4) == true);
	}
}
