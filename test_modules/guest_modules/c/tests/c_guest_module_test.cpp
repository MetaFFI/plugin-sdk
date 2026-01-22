#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "c_guest_module.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static int add_callback(int a, int b) {
	return a + b;
}

static int bad_add_callback(int a, int b) {
	return a - b;
}

static int g_void_called = 0;

static void void_callback(void) {
	g_void_called = 1;
}

static const char* string_callback(const char* value) {
	static char buffer[64];
	std::snprintf(buffer, sizeof(buffer), "%s_x", value ? value : "");
	return buffer;
}

TEST_CASE("core functions and globals") {
	c_guest_set_counter(0);
	c_guest_hello_world();
	CHECK(c_guest_get_counter() == 1);
	CHECK(C_GUEST_CONST_FIVE_SECONDS == 5);
	CHECK(c_guest_inc_counter(2) == 3);
	CHECK(c_guest_div_integers(10, 4) == doctest::Approx(2.5));

	const char* parts[] = {"a", "b", "c"};
	CGuestStringArray arr = {parts, 3};
	const char* joined = c_guest_join_strings(arr);
	REQUIRE(joined != nullptr);
	CHECK(std::string(joined) == "a,b,c");
	free((void*)joined);

	CHECK(c_guest_returns_an_error() == -1);
	CHECK(c_guest_call_callback_add(&add_callback) == 0);
	CHECK(c_guest_call_callback_add(&bad_add_callback) == -1);
	c_add_callback cb = c_guest_return_callback_add();
	CHECK(cb(2, 3) == 5);
	CHECK(std::string(c_guest_call_callback_string(&string_callback, "in")) == "in_x");

	CHECK(std::string(c_guest_call_callback_with_error(NULL)) == "callback is null");
	g_void_called = 0;
	CHECK(std::string(c_guest_call_callback_with_error(&void_callback)).empty());
	CHECK(g_void_called == 1);

	c_guest_wait_a_bit(10);
	CHECK(c_guest_return_null() == nullptr);
}

TEST_CASE("arrays and buffers") {
	CGuestIntArray arr1 = c_guest_make_1d_array();
	CHECK(arr1.len == 3);
	CHECK(arr1.data[0] == 1);
	c_guest_free_int_array(arr1);

	CGuestInt2DArray arr2 = c_guest_make_2d_array();
	CHECK(arr2.len == 2);
	CHECK(arr2.rows[0].len == 3);
	CHECK(arr2.rows[1].len == 2);
	CHECK(c_guest_sum_ragged_array(arr2) == 13);
	c_guest_free_int2d(arr2);

	CGuestInt3DArray arr3 = c_guest_make_3d_array();
	CHECK(arr3.len == 2);
	CHECK(c_guest_sum_3d_array(arr3) == 26);
	c_guest_free_int3d(arr3);

	CGuestInt2DArray ragged = c_guest_make_ragged_array();
	CHECK(ragged.len == 3);
	CHECK(c_guest_sum_ragged_array(ragged) == 21);
	c_guest_free_int2d(ragged);

	CGuestByte2DArray buffers = c_guest_get_three_buffers();
	CHECK(buffers.len == 3);
	CHECK(c_guest_expect_three_buffers(buffers) == 0);
	c_guest_free_byte2d(buffers);

	size_t classes_len = 0;
	CGuestSomeClass* classes = c_guest_get_some_classes(&classes_len);
	REQUIRE(classes != nullptr);
	CHECK(classes_len == 3);
	CHECK(classes[0].name != nullptr);
	c_guest_expect_three_some_classes(classes, classes_len);
	c_guest_free_some_classes(classes, classes_len);

	size_t dims_len = 0;
	CGuestAny* dims = c_guest_returns_array_with_different_dimensions(&dims_len);
	REQUIRE(dims != nullptr);
	CHECK(dims_len == 3);
	CHECK(dims[1].val.i64 == 4);
	CGuestIntArray dim1 = {(int*)dims[0].val.bytes.data, dims[0].val.bytes.len / sizeof(int)};
	c_guest_free_int_array(dim1);
	CGuestInt2DArray dim2 = {(CGuestIntArray*)dims[2].val.bytes.data, dims[2].val.bytes.len};
	c_guest_free_int2d(dim2);
	free(dims);

	size_t diff_len = 0;
	CGuestAny* diff = c_guest_returns_array_of_different_objects(&diff_len);
	REQUIRE(diff != nullptr);
	CHECK(diff_len == 6);
	CHECK(diff[1].type == 2);
	CHECK(diff[5].type == 5);
	free(diff[4].val.bytes.data);
	c_guest_free_any_array(diff, diff_len);

	CGuestStringIntMap map = c_guest_make_string_int_map();
	CHECK(map.len == 2);
	CHECK(std::string(map.items[0].key) == "a");
	CHECK(map.items[1].value == 2);
	c_guest_free_string_int_map(map);

	uint8_t raw[] = {1, 2, 3};
	CGuestByteArray echo = c_guest_echo_bytes(CGuestByteArray{raw, 3});
	CHECK(echo.len == 3);
	CHECK(echo.data[2] == 3);
	free(echo.data);
}

TEST_CASE("objects and handles") {
	CGuestSomeClass obj = c_guest_someclass_create("x");
	const char* printed = c_guest_someclass_print(&obj);
	REQUIRE(printed != nullptr);
	CHECK(std::string(printed).find("SomeClass") != std::string::npos);
	free((void*)printed);
	c_guest_someclass_destroy(&obj);

	CGuestSomeClass def = c_guest_someclass_make_default();
	CHECK(def.name != nullptr);
	c_guest_someclass_destroy(&def);

	CGuestTestMap* map = c_guest_testmap_create();
	REQUIRE(map != nullptr);
	CHECK(std::string(c_guest_testmap_get_name(map)) == "name1");
	CGuestAny value = {0};
	value.type = 0;
	value.val.i64 = 42;
	c_guest_testmap_set(map, "k", value);
	CHECK(c_guest_testmap_contains(map, "k") == 1);
	CGuestAny got = c_guest_testmap_get(map, "k");
	CHECK(got.val.i64 == 42);
	c_guest_testmap_set_name(map, "name2");
	CHECK(std::string(c_guest_testmap_get_name(map)) == "name2");
	c_guest_testmap_destroy(map);
}

TEST_CASE("types, overloads, variadic, errors") {
	CGuestAny any0 = c_guest_return_any(0);
	CHECK(any0.type == 0);
	CHECK(any0.val.i64 == 1);
	CGuestAny any1 = c_guest_return_any(1);
	CHECK(any1.type == 2);
	CHECK(std::string(any1.val.str) == "string");
	CGuestAny any3 = c_guest_return_any(3);
	CHECK(any3.type == 3);
	CHECK(any3.val.str == nullptr);
	CGuestAny any4 = c_guest_return_any(4);
	CHECK(any4.type == 5);
	c_guest_someclass_destroy(&any4.val.obj);

	CGuestAny accept0 = {0};
	accept0.type = 0;
	accept0.val.i64 = 1;
	CHECK(c_guest_accepts_any(0, accept0) == 0);
	CHECK(c_guest_accepts_any(1, accept0) == -1);
	CGuestAny accept_bytes = {0};
	accept_bytes.type = 4;
	uint8_t single[] = {'a'};
	accept_bytes.val.bytes = CGuestByteArray{single, 1};
	CHECK(c_guest_accepts_any(4, accept_bytes) == 0);
	CGuestAny accept_obj = {0};
	accept_obj.type = 5;
	accept_obj.val.obj = c_guest_someclass_create("y");
	CHECK(c_guest_accepts_any(5, accept_obj) == 0);
	c_guest_someclass_destroy(&accept_obj.val.obj);

	CGuestAny prims = c_guest_accepts_primitives(true,
		static_cast<int8_t>(1),
		static_cast<int16_t>(2),
		static_cast<int32_t>(3),
		static_cast<int64_t>(4),
		static_cast<uint8_t>(5),
		static_cast<uint16_t>(6),
		static_cast<uint32_t>(7),
		static_cast<uint64_t>(8),
		1.5f,
		2.5,
		'a');
	CHECK(prims.type == 99);
	CHECK(prims.val.i64 == 137);

	CGuestVec2 v = c_guest_add_vec2(CGuestVec2{1, 2}, CGuestVec2{3, 4});
	CHECK(c_guest_vec2_equals(v, CGuestVec2{4, 6}) == 1);
	CHECK(std::string(c_guest_color_name(c_guest_get_color(0))) == "RED");

	CHECK(c_guest_add_int(1, 2) == 3);
	CHECK(c_guest_add_double(1.5, 2.0) == doctest::Approx(3.5));
	const char* added = c_guest_add_string("a", "b");
	REQUIRE(added != nullptr);
	CHECK(std::string(added) == "ab");
	free((void*)added);
	CHECK(c_guest_default_args(1, 2) == 3);
	CHECK(c_guest_sum_variadic(3, 1, 2, 3) == 6);
	const char* joined = c_guest_join_variadic("p", 2, "a", "b");
	REQUIRE(joined != nullptr);
	CHECK(std::string(joined) == "p:a:b");
	free((void*)joined);

	CGuestPlainStruct plain = c_guest_make_plain_struct(7, "name");
	CHECK(plain.id == 7);
	CHECK(std::string(plain.name) == "name");

	CGuestMultiReturn multi = c_guest_return_multiple_return_values();
	CHECK(multi.i == 1);
	CHECK(std::string(multi.s) == "string");
	CHECK(multi.d == doctest::Approx(3.0));
	CHECK(multi.bytes_len == 3);
	c_guest_someclass_destroy(&multi.obj);

	const char* error = NULL;
	CHECK(c_guest_return_error_pair(true, &error) == 0);
	CHECK(error == NULL);
	CHECK(c_guest_return_error_pair(false, &error) == -1);
	REQUIRE(error != NULL);
	CHECK(std::string(error) == "error");
}
