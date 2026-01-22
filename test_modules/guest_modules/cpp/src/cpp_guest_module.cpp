#include "cpp_guest_module.h"

#include <cstdarg>
#include <iostream>
#include <stdexcept>

namespace guest {

const int64_t CONST_FIVE_SECONDS = 5;
int64_t g_counter = 0;

int64_t get_counter() {
	return g_counter;
}

void set_counter(int64_t value) {
	g_counter = value;
}

int64_t inc_counter(int64_t delta) {
	g_counter += delta;
	return g_counter;
}

void hello_world() {
	++g_counter;
	std::cout << "Hello World, from C++" << std::endl;
}

void returns_an_error() {
	throw std::runtime_error("Error");
}

double div_integers(int64_t x, int64_t y) {
	return static_cast<double>(x) / static_cast<double>(y);
}

std::string join_strings(const std::vector<std::string>& arr) {
	std::string out;
	for (size_t i = 0; i < arr.size(); ++i) {
		out += arr[i];
		if (i + 1 < arr.size()) {
			out += ",";
		}
	}
	return out;
}

void wait_a_bit(int64_t ms) {
	(void)ms;
}

void* return_null() {
	return nullptr;
}

static int add_impl(int a, int b) {
	return a + b;
}

int call_callback_add(AddCallback cb) {
	int res = cb(1, 2);
	if (res != 3) {
		throw std::runtime_error("expected 3");
	}
	return res;
}

AddCallback return_callback_add() {
	return &add_impl;
}

std::string call_callback_string(const StringTransformer& cb, const std::string& value) {
	return cb(value);
}

std::string call_callback_with_error(const VoidCallback& cb) {
	try {
		cb();
	} catch (const std::exception& ex) {
		return ex.what();
	} catch (...) {
		return "unknown error";
	}
	return "";
}

SomeClass::SomeClass() : name("some") {}

SomeClass::SomeClass(std::string name) : name(std::move(name)) {}

std::string SomeClass::print() const {
	return "Hello from SomeClass " + name;
}

SomeClass SomeClass::make_default() {
	return SomeClass();
}

bool SomeClass::operator==(const SomeClass& other) const {
	return name == other.name;
}

TestMap::TestMap() : name("name1") {}

void TestMap::set(const std::string& key, const std::any& value) {
	data[key] = value;
}

std::any TestMap::get(const std::string& key) const {
	auto it = data.find(key);
	if (it == data.end()) {
		throw std::runtime_error("missing key");
	}
	return it->second;
}

bool TestMap::contains(const std::string& key) const {
	return data.find(key) != data.end();
}

Base::Base(int value) : base_value(value) {}

int Base::base_method() const {
	return base_value;
}

Base Base::make_default() {
	return Base(7);
}

Derived::Derived(int value, std::string extra) : Base(value), extra(std::move(extra)) {}

std::string Derived::derived_method() const {
	return extra;
}

Outer::Inner::Inner(int value) : value_(value) {}

int Outer::Inner::get_value() const {
	return value_;
}

Outer::Outer(int value) : inner_(value) {}

Outer::Inner Outer::get_inner() const {
	return inner_;
}

std::string SimpleGreeter::greet(const std::string& name) {
	return "Hello " + name;
}

std::string call_greeter(Greeter* greeter, const std::string& name) {
	return greeter->greet(name);
}

int ResourceTracker::live_count = 0;

ResourceTracker::ResourceTracker() {
	++live_count;
}

ResourceTracker::~ResourceTracker() {
	--live_count;
}

int ResourceTracker::get_live_count() {
	return live_count;
}

IntBox make_int_box(int value) {
	return IntBox(value);
}

StringBox make_string_box(const std::string& value) {
	return StringBox(value);
}

Vec2::Vec2(int x, int y) : x(x), y(y) {}

Vec2 Vec2::operator+(const Vec2& other) const {
	return Vec2{x + other.x, y + other.y};
}

bool Vec2::operator==(const Vec2& other) const {
	return x == other.x && y == other.y;
}

Vec2 add_vec2(const Vec2& a, const Vec2& b) {
	return a + b;
}

Color get_color(int idx) {
	switch (idx) {
		case 0:
			return Color::Red;
		case 1:
			return Color::Green;
		default:
			return Color::Blue;
	}
}

std::string color_name(Color color) {
	switch (color) {
		case Color::Red:
			return "RED";
		case Color::Green:
			return "GREEN";
		default:
			return "BLUE";
	}
}

int add(int a, int b) {
	return a + b;
}

double add(double a, double b) {
	return a + b;
}

std::string add(const std::string& a, const std::string& b) {
	return a + b;
}

int default_args(int a, int b) {
	return a + b;
}

int sum_variadic(int count, ...) {
	va_list args;
	va_start(args, count);
	int sum = 0;
	for (int i = 0; i < count; ++i) {
		sum += va_arg(args, int);
	}
	va_end(args);
	return sum;
}

std::string join_variadic(const std::string& prefix, int count, ...) {
	va_list args;
	va_start(args, count);
	std::string out = prefix;
	for (int i = 0; i < count; ++i) {
		const char* val = va_arg(args, const char*);
		out += ":";
		out += val ? val : "";
	}
	va_end(args);
	return out;
}

PlainStruct make_plain_struct(int id, const std::string& name) {
	return PlainStruct{id, name};
}

std::tuple<int, std::string> make_tuple(int id, const std::string& name) {
	return std::make_tuple(id, name);
}

std::vector<int> make_1d_array() {
	return {1, 2, 3};
}

std::vector<std::vector<int>> make_2d_array() {
	return {{1, 2}, {3, 4}};
}

std::vector<std::vector<std::vector<int>>> make_3d_array() {
	return {{{1}, {2}}, {{3}, {4}}};
}

std::vector<std::vector<int>> make_ragged_array() {
	return {{1, 2, 3}, {4}, {5, 6}};
}

int sum_3d_array(const std::vector<std::vector<std::vector<int>>>& arr) {
	int sum = 0;
	for (const auto& plane : arr) {
		for (const auto& row : plane) {
			for (int val : row) {
				sum += val;
			}
		}
	}
	return sum;
}

int sum_ragged_array(const std::vector<std::vector<int>>& arr) {
	int sum = 0;
	for (const auto& row : arr) {
		for (int val : row) {
			sum += val;
		}
	}
	return sum;
}

std::vector<std::vector<uint8_t>> get_three_buffers() {
	return {
		{1, 2, 3, 4},
		{5, 6, 7},
		{8, 9}
	};
}

void expect_three_buffers(const std::vector<std::vector<uint8_t>>& buffers) {
	if (buffers.size() != 3) {
		throw std::runtime_error("buffers length is not 3");
	}
	if (buffers[0].size() != 4 || buffers[1].size() != 3 || buffers[2].size() != 2) {
		throw std::runtime_error("buffer sizes mismatch");
	}
}

std::vector<SomeClass> get_some_classes() {
	return {SomeClass("a"), SomeClass("b"), SomeClass("c")};
}

void expect_three_some_classes(const std::vector<SomeClass>& classes) {
	if (classes.size() != 3) {
		throw std::runtime_error("array length is not 3");
	}
}

std::vector<DimVariant> returns_array_with_different_dimensions() {
	return {
		std::vector<int>{1, 2, 3},
		4,
		std::vector<std::vector<int>>{{5, 6}, {7, 8}}
	};
}

std::vector<AnyVariant> returns_array_of_different_objects() {
	return {
		int64_t(1),
		std::string("string"),
		3.0,
		nullptr,
		std::vector<uint8_t>{1, 2, 3},
		SomeClass("x")
	};
}

std::any return_any(int which) {
	switch (which) {
		case 0:
			return int64_t(1);
		case 1:
			return std::string("string");
		case 2:
			return 3.0;
		case 3:
			return std::vector<std::string>{"list", "of", "strings"};
		case 4:
			return SomeClass("some");
		default:
			return std::any();
	}
}

void accepts_any(int which, const std::any& value) {
	switch (which) {
		case 0:
			if (value.type() != typeid(int64_t)) {
				throw std::runtime_error("expected int64");
			}
			break;
		case 1:
			if (value.type() != typeid(std::string)) {
				throw std::runtime_error("expected string");
			}
			break;
		case 2:
			if (value.type() != typeid(double)) {
				throw std::runtime_error("expected double");
			}
			break;
		case 3:
			if (value.has_value()) {
				throw std::runtime_error("expected empty");
			}
			break;
		case 4:
			if (value.type() != typeid(std::vector<uint8_t>)) {
				throw std::runtime_error("expected bytes");
			}
			break;
		case 5:
			if (value.type() != typeid(SomeClass)) {
				throw std::runtime_error("expected SomeClass");
			}
			break;
		default:
			throw std::runtime_error("unsupported type");
	}
}

std::tuple<bool, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double, char>
accepts_primitives(bool b, int8_t i8, int16_t i16, int32_t i32, int64_t i64,
	uint8_t u8, uint16_t u16, uint32_t u32, uint64_t u64, float f32, double f64, char c) {
	return std::make_tuple(b, i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, c);
}

std::vector<uint8_t> echo_bytes(const std::vector<uint8_t>& data) {
	return data;
}

std::map<std::string, int> make_string_int_map() {
	return {{"a", 1}, {"b", 2}};
}

std::map<std::string, std::vector<int>> make_nested_map() {
	return {{"nums", {1, 2, 3}}};
}

MultiReturn return_multiple_return_values() {
	return MultiReturn{1, "string", 3.0, true, {1, 2, 3}, SomeClass("x")};
}

std::pair<bool, std::string> return_error_pair(bool ok) {
	if (ok) {
		return {true, ""};
	}
	return {false, "error"};
}

}  // namespace guest
