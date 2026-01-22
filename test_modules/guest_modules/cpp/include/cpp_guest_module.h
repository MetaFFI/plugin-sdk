#pragma once

#include <any>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#ifdef _WIN32
#	ifdef CPP_GUEST_MODULE_EXPORTS
#		define CPP_GUEST_MODULE_API __declspec(dllexport)
#	else
#		define CPP_GUEST_MODULE_API __declspec(dllimport)
#	endif
#else
#	define CPP_GUEST_MODULE_API
#endif

namespace guest {

using AddCallback = int(*)(int, int);
using StringTransformer = std::function<std::string(const std::string&)>;
using VoidCallback = std::function<void()>;

CPP_GUEST_MODULE_API extern const int64_t CONST_FIVE_SECONDS;
CPP_GUEST_MODULE_API extern int64_t g_counter;

CPP_GUEST_MODULE_API int64_t get_counter();
CPP_GUEST_MODULE_API void set_counter(int64_t value);
CPP_GUEST_MODULE_API int64_t inc_counter(int64_t delta);

CPP_GUEST_MODULE_API void hello_world();
CPP_GUEST_MODULE_API void returns_an_error();
CPP_GUEST_MODULE_API double div_integers(int64_t x, int64_t y);
CPP_GUEST_MODULE_API std::string join_strings(const std::vector<std::string>& arr);
CPP_GUEST_MODULE_API void wait_a_bit(int64_t ms);
CPP_GUEST_MODULE_API void* return_null();

CPP_GUEST_MODULE_API int call_callback_add(AddCallback cb);
CPP_GUEST_MODULE_API AddCallback return_callback_add();
CPP_GUEST_MODULE_API std::string call_callback_string(const StringTransformer& cb, const std::string& value);
CPP_GUEST_MODULE_API std::string call_callback_with_error(const VoidCallback& cb);

struct CPP_GUEST_MODULE_API SomeClass {
	std::string name;
	SomeClass();
	explicit SomeClass(std::string name);
	std::string print() const;
	static SomeClass make_default();
	bool operator==(const SomeClass& other) const;
};

struct CPP_GUEST_MODULE_API TestMap {
	std::map<std::string, std::any> data;
	std::string name;
	TestMap();
	void set(const std::string& key, const std::any& value);
	std::any get(const std::string& key) const;
	bool contains(const std::string& key) const;
};

struct CPP_GUEST_MODULE_API Base {
	int base_value;
	explicit Base(int value = 0);
	int base_method() const;
	static Base make_default();
};

struct CPP_GUEST_MODULE_API Derived : public Base {
	std::string extra;
	Derived(int value, std::string extra);
	std::string derived_method() const;
};

class CPP_GUEST_MODULE_API Outer {
public:
	class CPP_GUEST_MODULE_API Inner {
	public:
		explicit Inner(int value);
		int get_value() const;
	private:
		int value_;
	};

	explicit Outer(int value);
	Inner get_inner() const;
private:
	Inner inner_;
};

class CPP_GUEST_MODULE_API Greeter {
public:
	virtual ~Greeter() = default;
	virtual std::string greet(const std::string& name) = 0;
};

class CPP_GUEST_MODULE_API SimpleGreeter : public Greeter {
public:
	std::string greet(const std::string& name) override;
};

CPP_GUEST_MODULE_API std::string call_greeter(Greeter* greeter, const std::string& name);

class CPP_GUEST_MODULE_API ResourceTracker {
public:
	ResourceTracker();
	~ResourceTracker();
	static int get_live_count();
private:
	static int live_count;
};

template <typename T>
class Box {
public:
	explicit Box(T value) : value_(std::move(value)) {}
	const T& get() const { return value_; }
	void set(T value) { value_ = std::move(value); }
private:
	T value_;
};

using IntBox = Box<int>;
using StringBox = Box<std::string>;

CPP_GUEST_MODULE_API IntBox make_int_box(int value);
CPP_GUEST_MODULE_API StringBox make_string_box(const std::string& value);

struct CPP_GUEST_MODULE_API Vec2 {
	int x;
	int y;
	Vec2(int x, int y);
	Vec2 operator+(const Vec2& other) const;
	bool operator==(const Vec2& other) const;
};

CPP_GUEST_MODULE_API Vec2 add_vec2(const Vec2& a, const Vec2& b);

enum class Color {
	Red,
	Green,
	Blue
};

CPP_GUEST_MODULE_API Color get_color(int idx);
CPP_GUEST_MODULE_API std::string color_name(Color color);

CPP_GUEST_MODULE_API int add(int a, int b);
CPP_GUEST_MODULE_API double add(double a, double b);
CPP_GUEST_MODULE_API std::string add(const std::string& a, const std::string& b);
CPP_GUEST_MODULE_API int default_args(int a, int b = 2);
CPP_GUEST_MODULE_API int sum_variadic(int count, ...);
CPP_GUEST_MODULE_API std::string join_variadic(const std::string& prefix, int count, ...);

struct CPP_GUEST_MODULE_API PlainStruct {
	int id;
	std::string name;
};

CPP_GUEST_MODULE_API PlainStruct make_plain_struct(int id, const std::string& name);
CPP_GUEST_MODULE_API std::tuple<int, std::string> make_tuple(int id, const std::string& name);

using AnyVariant = std::variant<int64_t, std::string, double, std::nullptr_t, std::vector<uint8_t>, SomeClass>;
using DimVariant = std::variant<std::vector<int>, int, std::vector<std::vector<int>>>;

CPP_GUEST_MODULE_API std::vector<int> make_1d_array();
CPP_GUEST_MODULE_API std::vector<std::vector<int>> make_2d_array();
CPP_GUEST_MODULE_API std::vector<std::vector<std::vector<int>>> make_3d_array();
CPP_GUEST_MODULE_API std::vector<std::vector<int>> make_ragged_array();
CPP_GUEST_MODULE_API int sum_3d_array(const std::vector<std::vector<std::vector<int>>>& arr);
CPP_GUEST_MODULE_API int sum_ragged_array(const std::vector<std::vector<int>>& arr);

CPP_GUEST_MODULE_API std::vector<std::vector<uint8_t>> get_three_buffers();
CPP_GUEST_MODULE_API void expect_three_buffers(const std::vector<std::vector<uint8_t>>& buffers);

CPP_GUEST_MODULE_API std::vector<SomeClass> get_some_classes();
CPP_GUEST_MODULE_API void expect_three_some_classes(const std::vector<SomeClass>& classes);

CPP_GUEST_MODULE_API std::vector<DimVariant> returns_array_with_different_dimensions();
CPP_GUEST_MODULE_API std::vector<AnyVariant> returns_array_of_different_objects();

CPP_GUEST_MODULE_API std::any return_any(int which);
CPP_GUEST_MODULE_API void accepts_any(int which, const std::any& value);

CPP_GUEST_MODULE_API std::tuple<bool, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, float, double, char>
accepts_primitives(bool b, int8_t i8, int16_t i16, int32_t i32, int64_t i64,
	uint8_t u8, uint16_t u16, uint32_t u32, uint64_t u64, float f32, double f64, char c);

CPP_GUEST_MODULE_API std::vector<uint8_t> echo_bytes(const std::vector<uint8_t>& data);
CPP_GUEST_MODULE_API std::map<std::string, int> make_string_int_map();
CPP_GUEST_MODULE_API std::map<std::string, std::vector<int>> make_nested_map();

struct CPP_GUEST_MODULE_API MultiReturn {
	int64_t i;
	std::string s;
	double d;
	bool is_null;
	std::vector<uint8_t> bytes;
	SomeClass obj;
};

CPP_GUEST_MODULE_API MultiReturn return_multiple_return_values();
CPP_GUEST_MODULE_API std::pair<bool, std::string> return_error_pair(bool ok);

}  // namespace guest
