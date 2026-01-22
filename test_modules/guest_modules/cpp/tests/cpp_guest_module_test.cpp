#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "cpp_guest_module.h"

using namespace guest;

static int add_callback(int a, int b) {
	return a + b;
}

TEST_CASE("core functions and globals") {
	set_counter(0);
	hello_world();
	CHECK(get_counter() == 1);
	CHECK(CONST_FIVE_SECONDS == 5);
	CHECK(inc_counter(2) == 3);
	CHECK(div_integers(10, 2) == doctest::Approx(5.0));
	CHECK(join_strings({"a", "b", "c"}) == "a,b,c");
	CHECK(return_null() == nullptr);
	CHECK_THROWS(returns_an_error());
	CHECK(call_callback_add(add_callback) == 3);
	auto cb = return_callback_add();
	CHECK(cb(1, 2) == 3);
	CHECK(call_callback_string([](const std::string& s) { return s + "_x"; }, "a") == "a_x");
	CHECK_FALSE(call_callback_with_error([] { throw std::runtime_error("fail"); }).empty());
}

TEST_CASE("arrays and buffers") {
	CHECK(make_1d_array().size() == 3);
	CHECK(make_2d_array().size() == 2);
	CHECK(sum_3d_array(make_3d_array()) == 10);
	CHECK(sum_ragged_array(make_ragged_array()) == 21);
	auto buffers = get_three_buffers();
	CHECK(buffers.size() == 3);
	CHECK_NOTHROW(expect_three_buffers(buffers));
	auto classes = get_some_classes();
	CHECK(classes.size() == 3);
	CHECK_NOTHROW(expect_three_some_classes(classes));
	auto dims = returns_array_with_different_dimensions();
	CHECK(std::holds_alternative<std::vector<int>>(dims[0]));
	CHECK(std::holds_alternative<int>(dims[1]));
	CHECK(std::holds_alternative<std::vector<std::vector<int>>>(dims[2]));
	auto diff = returns_array_of_different_objects();
	CHECK(diff.size() == 6);
	CHECK(std::holds_alternative<std::string>(diff[1]));
	CHECK(std::holds_alternative<SomeClass>(diff[5]));
}

TEST_CASE("objects and classes") {
	SomeClass sc("x");
	CHECK(sc.print().find("SomeClass") != std::string::npos);
	CHECK(SomeClass::make_default() == SomeClass("some"));
	TestMap tm;
	tm.set("k", 1);
	CHECK(tm.contains("k"));
	CHECK(std::any_cast<int>(tm.get("k")) == 1);
	CHECK(tm.name == "name1");
	Base base = Base::make_default();
	CHECK(base.base_method() == 7);
	Derived derived(2, "extra");
	CHECK(derived.base_method() == 2);
	CHECK(derived.derived_method() == "extra");
	Outer outer(5);
	CHECK(outer.get_inner().get_value() == 5);
	SimpleGreeter greeter;
	CHECK(call_greeter(&greeter, "Bob") == "Hello Bob");
	{
		ResourceTracker tracker;
		CHECK(ResourceTracker::get_live_count() == 1);
	}
	CHECK(ResourceTracker::get_live_count() == 0);
}

TEST_CASE("overloads, defaults, variadic") {
	CHECK(add(1, 2) == 3);
	CHECK(add(1.5, 2.0) == doctest::Approx(3.5));
	CHECK(add(std::string("a"), std::string("b")) == "ab");
	CHECK(default_args(1) == 3);
	CHECK(sum_variadic(3, 1, 2, 3) == 6);
	CHECK(join_variadic("p", 2, "a", "b") == "p:a:b");
}

TEST_CASE("types and primitives") {
	auto prims = accepts_primitives(true,
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
	CHECK(std::get<0>(prims));
	CHECK(std::get<4>(prims) == 4);
	CHECK(std::get<10>(prims) == doctest::Approx(2.5));
	CHECK(std::get<11>(prims) == 'a');
	CHECK(echo_bytes({1, 2, 3}).size() == 3);
	auto map = make_string_int_map();
	CHECK(map.size() == 2);
	auto nested = make_nested_map();
	CHECK(nested["nums"].size() == 3);
	auto box = make_int_box(3);
	CHECK(box.get() == 3);
	auto sbox = make_string_box("x");
	CHECK(sbox.get() == "x");
	Vec2 v = add_vec2(Vec2(1, 2), Vec2(3, 4));
	CHECK(v == Vec2(4, 6));
}

TEST_CASE("any, variants, tuples, errors") {
	CHECK(std::any_cast<int64_t>(return_any(0)) == 1);
	CHECK(std::any_cast<std::string>(return_any(1)) == "string");
	CHECK(std::any_cast<SomeClass>(return_any(4)) == SomeClass("some"));
	CHECK_THROWS(accepts_any(0, std::any(std::string("bad"))));
	CHECK_NOTHROW(accepts_any(0, std::any(int64_t(1))));
	auto multi = return_multiple_return_values();
	CHECK(multi.i == 1);
	CHECK(multi.s == "string");
	auto tuple_val = make_tuple(1, "name");
	CHECK(std::get<0>(tuple_val) == 1);
	CHECK(std::get<1>(tuple_val) == "name");
	auto plain = make_plain_struct(2, "x");
	CHECK(plain.id == 2);
	CHECK(color_name(get_color(0)) == "RED");
	CHECK(return_error_pair(true).first);
	CHECK_FALSE(return_error_pair(false).first);
}
