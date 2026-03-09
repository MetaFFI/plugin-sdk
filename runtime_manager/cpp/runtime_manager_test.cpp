#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include <doctest/doctest.h>

#include "runtime_manager.h"
#include "module.h"
#include "entity.h"
#include "cpp_abi.h"

#include <filesystem>
#include <thread>
#include <vector>
#include <atomic>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cmath>

// CPP_TEST_LIB_PATH is injected as a compile definition by CMakeLists.txt.
// It points to the built test_lib shared library.
#ifndef CPP_TEST_LIB_PATH
#   error "CPP_TEST_LIB_PATH not defined — configure CMake correctly"
#endif

// CPP_TEST_LIB_CPP_PATH points to the C++ variant of the test library.
#ifndef CPP_TEST_LIB_CPP_PATH
#   error "CPP_TEST_LIB_CPP_PATH not defined — configure CMake correctly"
#endif

// CPP_TEST_LIB_CLASS_PATH points to the C++ class test library (Phase 2).
#ifndef CPP_TEST_LIB_CLASS_PATH
#   error "CPP_TEST_LIB_CLASS_PATH not defined — configure CMake correctly"
#endif

// ============================================================================
// Platform-specific mangled names for test_lib_class symbols.
// Extracted via dumpbin /exports (MSVC) or nm/objdump (Itanium ABI).
// ============================================================================
#ifdef _MSC_VER
// MSVC x64 mangled names
#   define MANGLED_POINT_CTOR     "??0Point@@QEAA@HN@Z"
#   define MANGLED_POINT_DTOR     "??1Point@@QEAA@XZ"
#   define MANGLED_POINT_SUM      "?sum@Point@@QEBANXZ"
#   define MANGLED_POINT_SET_X    "?set_x@Point@@QEAAXH@Z"
#   define MANGLED_POINT_STATIC   "?static_add@Point@@SAHHH@Z"
#   define MANGLED_MATH_MULTIPLY  "?multiply@math@@YAHHH@Z"
#   define MANGLED_MATH_G_SCALE   "?g_scale@math@@3HA"
#else
// GCC/Clang Itanium ABI
#   define MANGLED_POINT_CTOR     "_ZN5PointC1Eid"
#   define MANGLED_POINT_DTOR     "_ZN5PointD1Ev"
#   define MANGLED_POINT_SUM      "_ZNK5Point3sumEv"
#   define MANGLED_POINT_SET_X    "_ZN5Point5set_xEi"
#   define MANGLED_POINT_STATIC   "_ZN5Point10static_addEii"
#   define MANGLED_MATH_MULTIPLY  "_ZN4math8multiplyEii"
#   define MANGLED_MATH_G_SCALE   "_ZN4math7g_scaleE"
#endif

// ============================================================================
// C function pointer types matching test_lib.c exports
// ============================================================================
extern "C"
{
	typedef int       (*add_func_t)      (int, int);
	typedef int       (*subtract_func_t) (int, int);
	typedef int       (*multiply_func_t) (int, int);
	typedef double    (*divide_func_t)   (double, double);
	typedef double    (*get_pi_func_t)   ();
	typedef int       (*is_positive_func_t)(int);
	typedef int       (*max_of_func_t)   (int, int);
	typedef long long (*factorial_func_t)(int);
}

// ============================================================================
// Helpers
// ============================================================================

template<typename Func>
bool expect_no_throw(Func&& func)
{
	try
	{
		func();
		return true;
	}
	catch (const std::exception& e)
	{
		INFO(e.what());
		return false;
	}
}

template<typename Func>
bool expect_throw(Func&& func)
{
	try
	{
		func();
		return false;
	}
	catch (const std::exception& e)
	{
		INFO(e.what());
		return true;
	}
}

static bool is_test_lib_available()
{
	return std::filesystem::exists(CPP_TEST_LIB_PATH);
}

static bool is_cpp_test_lib_available()
{
	return std::filesystem::exists(CPP_TEST_LIB_CPP_PATH);
}

static bool is_class_test_lib_available()
{
	return std::filesystem::exists(CPP_TEST_LIB_CLASS_PATH);
}

// C-linkage helper types from test_lib_class.cpp (extern "C" wrappers)
extern "C"
{
	typedef void*       (*point_create_func_t)(int, double);
	typedef void        (*point_destroy_func_t)(void*);
	typedef double      (*point_sum_func_t)(void*);
	typedef int         (*point_get_x_func_t)(void*);
	typedef double      (*point_get_y_func_t)(void*);
	typedef std::size_t (*point_sizeof_func_t)();
	typedef std::size_t (*point_offset_x_func_t)();
	typedef std::size_t (*point_offset_y_func_t)();
}


// ============================================================================
// 1. Runtime Lifecycle
// ============================================================================

TEST_SUITE("1. Runtime Lifecycle")
{
	TEST_CASE("1.1 Load runtime - success")
	{
		cpp_runtime_manager manager;

		CHECK(manager.is_runtime_loaded() == false);
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(manager.is_runtime_loaded() == true);
	}

	TEST_CASE("1.2 Load runtime - idempotent")
	{
		cpp_runtime_manager manager;

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(manager.is_runtime_loaded() == true);
	}

	TEST_CASE("1.3 Release runtime")
	{
		cpp_runtime_manager manager;

		manager.load_runtime();
		CHECK(manager.is_runtime_loaded() == true);

		manager.release_runtime();
		CHECK(manager.is_runtime_loaded() == false);
	}

	TEST_CASE("1.4 Release runtime - idempotent")
	{
		cpp_runtime_manager manager;
		manager.load_runtime();

		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
		CHECK(manager.is_runtime_loaded() == false);
	}

	TEST_CASE("1.5 get_abi returns a valid value")
	{
		cpp_runtime_manager manager;
		cpp_abi abi = manager.get_abi();

		// Must be one of the known values
		bool valid = (abi == cpp_abi::unknown  ||
		              abi == cpp_abi::c_only   ||
		              abi == cpp_abi::itanium  ||
		              abi == cpp_abi::msvc);
		CHECK(valid);
		INFO("Plugin ABI: " << cpp_abi_name(abi));
	}
}


// ============================================================================
// 2. Module Loading
// ============================================================================

TEST_SUITE("2. Module Loading")
{
	TEST_CASE("2.1 Load valid C module")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		manager.load_runtime();

		std::shared_ptr<Module> module;
		CHECK(expect_no_throw([&]() { module = manager.load_module(CPP_TEST_LIB_PATH); }));
		CHECK(module != nullptr);
		CHECK(module->get_module_path() == std::string(CPP_TEST_LIB_PATH));
	}

	TEST_CASE("2.2 Load module - auto-loads runtime")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		CHECK(manager.is_runtime_loaded() == false);

		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		CHECK(manager.is_runtime_loaded() == true);
		CHECK(module != nullptr);
	}

	TEST_CASE("2.3 Load module - nonexistent path throws")
	{
		cpp_runtime_manager manager;
		manager.load_runtime();

		CHECK(expect_throw([&]() {
			manager.load_module("/nonexistent/path/to/module.dll");
		}));
	}

	TEST_CASE("2.4 Load same module twice returns same instance")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module1 = manager.load_module(CPP_TEST_LIB_PATH);
		auto module2 = manager.load_module(CPP_TEST_LIB_PATH);

		// Same cached shared_ptr
		CHECK(module1.get() == module2.get());
	}

	TEST_CASE("2.5 Module has_symbol - existing and nonexistent")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		CHECK(module->has_symbol("add")        == true);
		CHECK(module->has_symbol("multiply")   == true);
		CHECK(module->has_symbol("g_counter")  == true);
		CHECK(module->has_symbol("no_such_fn") == false);
	}

	TEST_CASE("2.6 Module detected ABI is c_only for pure-C library")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		// test_lib.c has no C++ runtime dependency — must be c_only
		cpp_abi detected = module->get_detected_abi();
		INFO("Detected ABI: " << cpp_abi_name(detected));
		CHECK(detected == cpp_abi::c_only);
	}

	TEST_CASE("2.7 release_runtime clears module cache — new load_module returns new instance")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module1 = manager.load_module(CPP_TEST_LIB_PATH);

		// Releasing and re-initialising the runtime must flush the module cache.
		manager.release_runtime();
		manager.load_runtime();

		auto module2 = manager.load_module(CPP_TEST_LIB_PATH);

		// The cache was cleared, so a fresh Module object must have been created.
		CHECK(module1.get() != module2.get());
	}

	TEST_CASE("2.8 release_runtime — held module remains functional")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		// Release the runtime; the caller still holds the last shared_ptr reference,
		// so the underlying library handle must remain alive and callable.
		manager.release_runtime();

		auto fn = reinterpret_cast<add_func_t>(
			module->load_entity("callable=add")->get_function_pointer());
		CHECK(fn(5, 3) == 8);
	}
}


// ============================================================================
// 3. Entity Loading
// ============================================================================

TEST_SUITE("3. Entity Loading")
{
	TEST_CASE("3.1 Load callable entity - add function")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		std::shared_ptr<Entity> entity;
		CHECK(expect_no_throw([&]() { entity = module->load_entity("callable=add"); }));
		CHECK(entity != nullptr);
		CHECK(entity->get_name() == "add");
		CHECK(entity->get_function_pointer() != nullptr);
	}

	TEST_CASE("3.2 Load callable entity - nonexistent symbol throws")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		CHECK(expect_throw([&]() {
			module->load_entity("callable=nonexistent_function_xyz");
		}));
	}

	TEST_CASE("3.3 Load entity with unsupported path format throws")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		CHECK(expect_throw([&]() {
			// Missing 'callable=' or 'global=' key
			module->load_entity("method=SomeClass.foo");
		}));
	}

	TEST_CASE("3.4 Load global getter entity")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		std::shared_ptr<Entity> entity;
		CHECK(expect_no_throw([&]() {
			entity = module->load_entity("global=g_counter,getter=true");
		}));
		CHECK(entity != nullptr);
		CHECK(entity->get_name() == "g_counter");
		CHECK(entity->get_function_pointer() != nullptr);
	}

	TEST_CASE("3.5 Load global setter entity")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		std::shared_ptr<Entity> entity;
		CHECK(expect_no_throw([&]() {
			entity = module->load_entity("global=g_counter,setter=true");
		}));
		CHECK(entity != nullptr);
		CHECK(entity->get_name() == "g_counter");
	}

	TEST_CASE("3.6 Load global entity without getter/setter throws")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		CHECK(expect_throw([&]() {
			module->load_entity("global=g_counter");
		}));
	}

	TEST_CASE("3.7 Entity is cached — same pointer on second call")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		auto e1 = module->load_entity("callable=add");
		auto e2 = module->load_entity("callable=add");
		CHECK(e1.get() == e2.get());
	}

	TEST_CASE("3.8 Callable entity path with whitespace around value is trimmed")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		// "callable= add " — leading/trailing spaces around the symbol name must
		// be stripped by boost::trim_copy before the dlsym lookup.
		std::shared_ptr<Entity> entity;
		CHECK(expect_no_throw([&]() {
			entity = module->load_entity("callable= add ");
		}));
		REQUIRE(entity != nullptr);

		auto fn = reinterpret_cast<add_func_t>(entity->get_function_pointer());
		CHECK(fn(4, 6) == 10);
	}
}


// ============================================================================
// 4. Function Execution
// ============================================================================

TEST_SUITE("4. Function Execution")
{
	TEST_CASE("4.1 Call add")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto entity = module->load_entity("callable=add");
		auto fn     = reinterpret_cast<add_func_t>(entity->get_function_pointer());
		REQUIRE(fn != nullptr);

		CHECK(fn(2, 3)   == 5);
		CHECK(fn(0, 0)   == 0);
		CHECK(fn(-5, 5)  == 0);
		CHECK(fn(100, 200) == 300);
	}

	TEST_CASE("4.2 Call subtract")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto fn = reinterpret_cast<subtract_func_t>(
			module->load_entity("callable=subtract")->get_function_pointer());
		REQUIRE(fn != nullptr);

		CHECK(fn(5, 3) == 2);
		CHECK(fn(0, 0) == 0);
		CHECK(fn(3, 5) == -2);
	}

	TEST_CASE("4.3 Call multiply")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto fn = reinterpret_cast<multiply_func_t>(
			module->load_entity("callable=multiply")->get_function_pointer());
		REQUIRE(fn != nullptr);

		CHECK(fn(3, 4)  == 12);
		CHECK(fn(0, 100) == 0);
		CHECK(fn(-2, 3) == -6);
	}

	TEST_CASE("4.4 Call divide")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto fn = reinterpret_cast<divide_func_t>(
			module->load_entity("callable=divide")->get_function_pointer());
		REQUIRE(fn != nullptr);

		CHECK(fn(10.0, 2.0) == doctest::Approx(5.0));
		CHECK(fn(7.0, 2.0)  == doctest::Approx(3.5));
		CHECK(fn(10.0, 0.0) == doctest::Approx(0.0)); // div by zero → 0 per test_lib
	}

	TEST_CASE("4.5 Call get_pi")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto fn = reinterpret_cast<get_pi_func_t>(
			module->load_entity("callable=get_pi")->get_function_pointer());
		REQUIRE(fn != nullptr);

		CHECK(fn() == doctest::Approx(3.14159265358979).epsilon(1e-10));
	}

	TEST_CASE("4.6 Call is_positive")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto fn = reinterpret_cast<is_positive_func_t>(
			module->load_entity("callable=is_positive")->get_function_pointer());
		REQUIRE(fn != nullptr);

		CHECK(fn(5)  == 1);
		CHECK(fn(0)  == 0);
		CHECK(fn(-5) == 0);
	}

	TEST_CASE("4.7 Call max_of")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto fn = reinterpret_cast<max_of_func_t>(
			module->load_entity("callable=max_of")->get_function_pointer());
		REQUIRE(fn != nullptr);

		CHECK(fn(3, 7)   == 7);
		CHECK(fn(7, 3)   == 7);
		CHECK(fn(5, 5)   == 5);
		CHECK(fn(-10, -5) == -5);
	}

	TEST_CASE("4.8 Call factorial")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto fn = reinterpret_cast<factorial_func_t>(
			module->load_entity("callable=factorial")->get_function_pointer());
		REQUIRE(fn != nullptr);

		CHECK(fn(0)  == 1LL);
		CHECK(fn(1)  == 1LL);
		CHECK(fn(5)  == 120LL);
		CHECK(fn(10) == 3628800LL);
	}
}


// ============================================================================
// 5. Global Variable Access
// ============================================================================

TEST_SUITE("5. Global Variable Access")
{
	TEST_CASE("5.1 Read g_counter via getter")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto entity = module->load_entity("global=g_counter,getter=true");

		auto* getter = dynamic_cast<CppGlobalGetter*>(entity.get());
		REQUIRE(getter != nullptr);

		int* ptr = static_cast<int*>(getter->get());
		REQUIRE(ptr != nullptr);

		// g_counter initialised to 42 in test_lib.c
		CHECK(*ptr == 42);
	}

	TEST_CASE("5.2 Write g_counter via setter then read back via getter")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		auto getter_entity = module->load_entity("global=g_counter,getter=true");
		auto setter_entity = module->load_entity("global=g_counter,setter=true");

		auto* getter = dynamic_cast<CppGlobalGetter*>(getter_entity.get());
		auto* setter = dynamic_cast<CppGlobalSetter*>(setter_entity.get());
		REQUIRE(getter != nullptr);
		REQUIRE(setter != nullptr);

		// Write 99
		int new_value = 99;
		setter->set(&new_value, sizeof(int));

		// Read back — should see 99
		int* ptr = static_cast<int*>(getter->get());
		CHECK(*ptr == 99);

		// Restore original value so later tests see 42
		int restore = 42;
		setter->set(&restore, sizeof(int));
	}

	TEST_CASE("5.3 Read g_ratio via getter")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto entity = module->load_entity("global=g_ratio,getter=true");

		auto* getter = dynamic_cast<CppGlobalGetter*>(entity.get());
		REQUIRE(getter != nullptr);

		double* ptr = static_cast<double*>(getter->get());
		REQUIRE(ptr != nullptr);

		CHECK(*ptr == doctest::Approx(2.71828).epsilon(1e-5));
	}

	TEST_CASE("5.4 CppGlobalSetter::set with null value throws")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto entity = module->load_entity("global=g_counter,setter=true");
		auto* setter = dynamic_cast<CppGlobalSetter*>(entity.get());
		REQUIRE(setter != nullptr);

		CHECK(expect_throw([&]() { setter->set(nullptr, sizeof(int)); }));
	}

	TEST_CASE("5.5 CppGlobalSetter::set with zero size throws")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto entity = module->load_entity("global=g_counter,setter=true");
		auto* setter = dynamic_cast<CppGlobalSetter*>(entity.get());
		REQUIRE(setter != nullptr);

		int v = 1;
		CHECK(expect_throw([&]() { setter->set(&v, 0); }));
	}

	TEST_CASE("5.6 Read g_message (const char*) via getter")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto entity = module->load_entity("global=g_message,getter=true");

		auto* getter = dynamic_cast<CppGlobalGetter*>(entity.get());
		REQUIRE(getter != nullptr);

		// dlsym returns the address of g_message (a const char**).
		// Dereference once to get the const char* string pointer.
		const char** ptr = static_cast<const char**>(getter->get());
		REQUIRE(ptr != nullptr);
		REQUIRE(*ptr != nullptr);

		CHECK(std::string(*ptr) == "hello from test_lib");
	}
}


// ============================================================================
// 6. ABI Detection
// ============================================================================

TEST_SUITE("6. ABI Detection")
{
	TEST_CASE("6.1 detect_module_abi - pure-C test lib is c_only")
	{
		REQUIRE(is_test_lib_available());

		cpp_abi abi = detect_module_abi(CPP_TEST_LIB_PATH);
		INFO("Detected ABI: " << cpp_abi_name(abi));
		CHECK(abi == cpp_abi::c_only);
	}

	TEST_CASE("6.2 detect_module_abi - nonexistent file throws")
	{
		CHECK_THROWS_AS(
			detect_module_abi("/nonexistent/file/does_not_exist.dll"),
			std::runtime_error);
	}

	TEST_CASE("6.3 get_plugin_abi returns a known ABI")
	{
		cpp_abi abi = get_plugin_abi();
		INFO("Plugin ABI: " << cpp_abi_name(abi));

		bool valid = (abi == cpp_abi::msvc     ||
		              abi == cpp_abi::itanium   ||
		              abi == cpp_abi::unknown);
		CHECK(valid);
	}

	TEST_CASE("6.4 cpp_abi_name for all values")
	{
		CHECK(std::string(cpp_abi_name(cpp_abi::unknown))  == "unknown");
		CHECK(std::string(cpp_abi_name(cpp_abi::c_only))   == "c_only");
		CHECK(std::string(cpp_abi_name(cpp_abi::itanium))  == "itanium");
		CHECK(std::string(cpp_abi_name(cpp_abi::msvc))     == "msvc");
	}

	TEST_CASE("6.5 detect_module_abi - C++ test lib matches plugin ABI (msvc or itanium)")
	{
		REQUIRE(is_cpp_test_lib_available());

		// The C++ test library uses std::string, pulling in the C++ runtime.
		// Its ABI must therefore equal the plugin ABI (msvc on MSVC, itanium on GCC/Clang).
		cpp_abi detected  = detect_module_abi(CPP_TEST_LIB_CPP_PATH);
		cpp_abi plugin    = get_plugin_abi();

		INFO("Detected ABI : " << cpp_abi_name(detected));
		INFO("Plugin ABI   : " << cpp_abi_name(plugin));

		CHECK(detected != cpp_abi::c_only);
		CHECK(detected == plugin);
	}
}


// ============================================================================
// 7. Module Semantics (copy / move)
// ============================================================================

TEST_SUITE("7. Module Semantics")
{
	TEST_CASE("7.1 Module copy constructor")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module1 = manager.load_module(CPP_TEST_LIB_PATH);
		Module module2(*module1);

		CHECK(module2.get_module_path() == module1->get_module_path());

		// Both should resolve the same symbol correctly
		auto fn1 = reinterpret_cast<add_func_t>(
			module1->load_entity("callable=add")->get_function_pointer());
		auto fn2 = reinterpret_cast<add_func_t>(
			module2.load_entity("callable=add")->get_function_pointer());

		CHECK(fn1(1, 2) == 3);
		CHECK(fn2(1, 2) == 3);
	}

	TEST_CASE("7.2 Module move constructor")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module1 = manager.load_module(CPP_TEST_LIB_PATH);

		Module module2(std::move(*module1));
		CHECK(module2.get_module_path() == std::string(CPP_TEST_LIB_PATH));

		auto fn = reinterpret_cast<add_func_t>(
			module2.load_entity("callable=add")->get_function_pointer());
		CHECK(fn(10, 20) == 30);
	}
}


// ============================================================================
// 8. Thread Safety
// ============================================================================

TEST_SUITE("8. Thread Safety")
{
	TEST_CASE("8.1 Concurrent function calls from multiple threads")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);
		auto entity = module->load_entity("callable=add");

		auto add_fn = reinterpret_cast<add_func_t>(entity->get_function_pointer());
		REQUIRE(add_fn != nullptr);

		const int num_threads  = 10;
		const int iterations   = 100;
		std::atomic<int> success_count{0};
		std::vector<std::thread> threads;

		for (int t = 0; t < num_threads; ++t)
		{
			threads.emplace_back([&, t]()
			{
				for (int i = 0; i < iterations; ++i)
				{
					int a = t * iterations + i;
					int b = i;
					if (add_fn(a, b) == a + b) ++success_count;
				}
			});
		}

		for (auto& th : threads) th.join();

		CHECK(success_count == num_threads * iterations);
	}

	TEST_CASE("8.2 Concurrent load_entity calls are safe")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		const int num_threads = 8;
		std::atomic<int> success_count{0};
		std::vector<std::thread> threads;

		for (int t = 0; t < num_threads; ++t)
		{
			threads.emplace_back([&]()
			{
				try
				{
					auto e = module->load_entity("callable=multiply");
					if (e && e->get_function_pointer() != nullptr) ++success_count;
				}
				catch (...) {}
			});
		}

		for (auto& th : threads) th.join();
		CHECK(success_count == num_threads);
	}

	TEST_CASE("8.3 Multiple managers loading the same module path")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager mgr1;
		cpp_runtime_manager mgr2;

		auto mod1 = mgr1.load_module(CPP_TEST_LIB_PATH);
		auto mod2 = mgr2.load_module(CPP_TEST_LIB_PATH);

		auto fn1 = reinterpret_cast<add_func_t>(
			mod1->load_entity("callable=add")->get_function_pointer());
		auto fn2 = reinterpret_cast<add_func_t>(
			mod2->load_entity("callable=add")->get_function_pointer());

		CHECK(fn1(5, 3)  == 8);
		CHECK(fn2(10, 20) == 30);
	}
}


// ============================================================================
// 9. Module Unload & get_symbol
// ============================================================================

TEST_SUITE("9. Module Unload")
{
	TEST_CASE("9.1 unload — load_entity throws after unload")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		module->unload();

		// The library handle is gone; attempting to resolve an entity must throw.
		CHECK(expect_throw([&]() {
			module->load_entity("callable=add");
		}));
	}

	TEST_CASE("9.2 unload — has_symbol returns false after unload")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		// Symbol is visible before unload.
		CHECK(module->has_symbol("add") == true);

		module->unload();

		// After unload the library handle is invalid; has_symbol must return false.
		CHECK(module->has_symbol("add") == false);
	}

	TEST_CASE("9.3 get_symbol — returns non-null for known symbol")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		void* sym = module->get_symbol("add");
		CHECK(sym != nullptr);
	}

	TEST_CASE("9.4 get_symbol — returns nullptr for unknown symbol")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_PATH);

		void* sym = module->get_symbol("this_symbol_does_not_exist_xyz");
		CHECK(sym == nullptr);
	}
}


// ============================================================================
// 10. Module Assignment Operators
// ============================================================================

TEST_SUITE("10. Module Assignment Operators")
{
	TEST_CASE("10.1 Module copy assignment operator")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module1 = manager.load_module(CPP_TEST_LIB_PATH);

		// Build two independent copies, then copy-assign one over the other.
		Module lhs(*module1);
		Module rhs(*module1);

		lhs = rhs;  // copy assignment

		CHECK(lhs.get_module_path() == std::string(CPP_TEST_LIB_PATH));

		auto fn = reinterpret_cast<add_func_t>(
			lhs.load_entity("callable=add")->get_function_pointer());
		CHECK(fn(7, 8) == 15);
	}

	TEST_CASE("10.2 Module move assignment operator")
	{
		REQUIRE(is_test_lib_available());

		cpp_runtime_manager manager;
		auto module1 = manager.load_module(CPP_TEST_LIB_PATH);

		// Build two independent copies, then move-assign one over the other.
		Module lhs(*module1);
		Module rhs(*module1);

		lhs = std::move(rhs);  // move assignment

		CHECK(lhs.get_module_path() == std::string(CPP_TEST_LIB_PATH));

		auto fn = reinterpret_cast<add_func_t>(
			lhs.load_entity("callable=add")->get_function_pointer());
		CHECK(fn(3, 4) == 7);
	}
}


// ============================================================================
// 11. Namespaces
// ============================================================================

TEST_SUITE("11. Namespaces")
{
	TEST_CASE("11.1 Load namespaced free function (math::multiply)")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		// The mangled name encodes the namespace; the optional namespace= key
		// is stored but does not affect symbol lookup.
		std::string path = std::string("callable=") + MANGLED_MATH_MULTIPLY + ",namespace=math";

		std::shared_ptr<Entity> entity;
		CHECK(expect_no_throw([&]() { entity = module->load_entity(path); }));
		REQUIRE(entity != nullptr);

		// Verify it is a plain free function (no instance flag was set)
		CHECK(dynamic_cast<CppFreeFunction*>(entity.get()) != nullptr);

		// Call it: multiply(3, 7) == 21
		typedef int (*mul_fn_t)(int, int);
		auto fn = reinterpret_cast<mul_fn_t>(entity->get_function_pointer());
		REQUIRE(fn != nullptr);
		CHECK(fn(3, 7) == 21);
	}

	TEST_CASE("11.2 Load namespaced global getter (math::g_scale)")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		std::string path = std::string("global=") + MANGLED_MATH_G_SCALE + ",getter=true,namespace=math";

		std::shared_ptr<Entity> entity;
		CHECK(expect_no_throw([&]() { entity = module->load_entity(path); }));
		REQUIRE(entity != nullptr);

		auto* getter = dynamic_cast<CppGlobalGetter*>(entity.get());
		REQUIRE(getter != nullptr);

		int* ptr = static_cast<int*>(getter->get());
		REQUIRE(ptr != nullptr);
		CHECK(*ptr == 10);  // g_scale initialised to 10
	}

	TEST_CASE("11.3 Load namespaced global setter (math::g_scale)")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		std::string getter_path = std::string("global=") + MANGLED_MATH_G_SCALE + ",getter=true,namespace=math";
		std::string setter_path = std::string("global=") + MANGLED_MATH_G_SCALE + ",setter=true,namespace=math";

		auto getter_entity = module->load_entity(getter_path);
		auto setter_entity = module->load_entity(setter_path);

		auto* getter = dynamic_cast<CppGlobalGetter*>(getter_entity.get());
		auto* setter = dynamic_cast<CppGlobalSetter*>(setter_entity.get());
		REQUIRE(getter != nullptr);
		REQUIRE(setter != nullptr);

		// Write 99, read back
		int new_value = 99;
		setter->set(&new_value, sizeof(int));
		CHECK(*static_cast<int*>(getter->get()) == 99);

		// Restore
		int restore = 10;
		setter->set(&restore, sizeof(int));
	}
}


// ============================================================================
// 12. Instance Methods
// ============================================================================

TEST_SUITE("12. Instance Methods")
{
	TEST_CASE("12.1 Load Point::sum as CppInstanceMethod and call it")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		// Use the extern "C" helpers to construct a Point without mangled names
		auto create_fn = reinterpret_cast<point_create_func_t>(
			module->load_entity("callable=point_create")->get_function_pointer());
		auto destroy_fn = reinterpret_cast<point_destroy_func_t>(
			module->load_entity("callable=point_destroy")->get_function_pointer());
		REQUIRE(create_fn != nullptr);
		REQUIRE(destroy_fn != nullptr);

		// Load Point::sum as CppInstanceMethod
		std::string sum_path = std::string("callable=") + MANGLED_POINT_SUM + ",instance_required=true";
		auto sum_entity = module->load_entity(sum_path);
		REQUIRE(sum_entity != nullptr);
		CHECK(dynamic_cast<CppInstanceMethod*>(sum_entity.get()) != nullptr);

		// Create a Point(3, 4.0) via extern "C" helper
		void* point_ptr = create_fn(3, 4.0);
		REQUIRE(point_ptr != nullptr);

		// Call sum through the instance method entity: this* is first arg
		typedef double (*sum_fn_t)(void*);
		auto sum_fn = reinterpret_cast<sum_fn_t>(sum_entity->get_function_pointer());
		REQUIRE(sum_fn != nullptr);

		double result = sum_fn(point_ptr);
		CHECK(result == doctest::Approx(7.0));

		destroy_fn(point_ptr);
	}

	TEST_CASE("12.2 instance_required flag recognised — entity is CppInstanceMethod not CppFreeFunction")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		// Same symbol, with vs without instance_required
		std::string with_flag    = std::string("callable=") + MANGLED_POINT_SUM + ",instance_required=true";
		std::string without_flag = std::string("callable=") + MANGLED_POINT_SUM;

		auto e_instance = module->load_entity(with_flag);
		auto e_free     = module->load_entity(without_flag);

		CHECK(dynamic_cast<CppInstanceMethod*>(e_instance.get()) != nullptr);
		CHECK(dynamic_cast<CppFreeFunction*>(e_free.get())       != nullptr);
	}
}


// ============================================================================
// 13. Constructors & Destructors
// ============================================================================

TEST_SUITE("13. Constructors and Destructors")
{
	TEST_CASE("13.1 CppConstructor::allocate() returns non-null pointer")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		// Get sizeof(Point) from the library's extern "C" helper
		auto sizeof_fn = reinterpret_cast<point_sizeof_func_t>(
			module->load_entity("callable=point_sizeof")->get_function_pointer());
		REQUIRE(sizeof_fn != nullptr);
		std::size_t class_size = sizeof_fn();
		REQUIRE(class_size > 0);

		std::string ctor_path = std::string("callable=") + MANGLED_POINT_CTOR
		                      + ",constructor=true,class_size=" + std::to_string(class_size);

		auto ctor_entity = module->load_entity(ctor_path);
		REQUIRE(ctor_entity != nullptr);

		auto* ctor = dynamic_cast<CppConstructor*>(ctor_entity.get());
		REQUIRE(ctor != nullptr);

		void* mem = ctor->allocate();
		CHECK(mem != nullptr);
		std::free(mem);
	}

	TEST_CASE("13.2 Full construct + call + destroy cycle")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		// Get sizeof(Point) from library
		auto sizeof_fn = reinterpret_cast<point_sizeof_func_t>(
			module->load_entity("callable=point_sizeof")->get_function_pointer());
		REQUIRE(sizeof_fn != nullptr);
		std::size_t class_size = sizeof_fn();

		// Load constructor, destructor, and sum method
		std::string ctor_path = std::string("callable=") + MANGLED_POINT_CTOR
		                      + ",constructor=true,class_size=" + std::to_string(class_size);
		std::string dtor_path = std::string("callable=") + MANGLED_POINT_DTOR + ",destructor=true";
		std::string sum_path  = std::string("callable=") + MANGLED_POINT_SUM + ",instance_required=true";

		auto ctor_entity = module->load_entity(ctor_path);
		auto dtor_entity = module->load_entity(dtor_path);
		auto sum_entity  = module->load_entity(sum_path);

		REQUIRE(ctor_entity != nullptr);
		REQUIRE(dtor_entity != nullptr);
		REQUIRE(sum_entity  != nullptr);

		auto* ctor = dynamic_cast<CppConstructor*>(ctor_entity.get());
		auto* dtor = dynamic_cast<CppDestructor*>(dtor_entity.get());
		REQUIRE(ctor != nullptr);
		REQUIRE(dtor != nullptr);

		// Allocate memory and run the constructor: Point(3, 4.0)
		void* instance = ctor->allocate();
		REQUIRE(instance != nullptr);

		typedef void (*ctor_fn_t)(void*, int, double);
		auto ctor_fn = reinterpret_cast<ctor_fn_t>(ctor->get_function_pointer());
		REQUIRE(ctor_fn != nullptr);
		ctor_fn(instance, 3, 4.0);

		// Call sum(): should return 3 + 4.0 = 7.0
		typedef double (*sum_fn_t)(void*);
		auto sum_fn = reinterpret_cast<sum_fn_t>(sum_entity->get_function_pointer());
		REQUIRE(sum_fn != nullptr);
		double result = sum_fn(instance);
		CHECK(result == doctest::Approx(7.0));

		// Destroy the instance
		CHECK(expect_no_throw([&]() { dtor->destroy(instance); }));
	}

	TEST_CASE("13.3 CppDestructor::destroy with null instance throws")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		auto sizeof_fn = reinterpret_cast<point_sizeof_func_t>(
			module->load_entity("callable=point_sizeof")->get_function_pointer());
		REQUIRE(sizeof_fn != nullptr);
		std::size_t class_size = sizeof_fn();

		std::string dtor_path = std::string("callable=") + MANGLED_POINT_DTOR + ",destructor=true";
		auto dtor_entity = module->load_entity(dtor_path);
		auto* dtor = dynamic_cast<CppDestructor*>(dtor_entity.get());
		REQUIRE(dtor != nullptr);

		CHECK(expect_throw([&]() { dtor->destroy(nullptr); }));
	}
}


// ============================================================================
// 14. Field Access
// ============================================================================

TEST_SUITE("14. Field Access")
{
	TEST_CASE("14.1 CppFieldGetter reads Point::x correctly")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		// Get field offset from the library
		auto offset_fn = reinterpret_cast<point_offset_x_func_t>(
			module->load_entity("callable=point_offset_x")->get_function_pointer());
		REQUIRE(offset_fn != nullptr);
		std::size_t offset = offset_fn();

		// Build entity path for field getter
		std::string path = "field=x,getter=true,instance_required=true,field_offset=" + std::to_string(offset);
		auto entity = module->load_entity(path);
		REQUIRE(entity != nullptr);

		auto* getter = dynamic_cast<CppFieldGetter*>(entity.get());
		REQUIRE(getter != nullptr);

		// Create a Point via extern "C" helper
		auto create_fn  = reinterpret_cast<point_create_func_t>(
			module->load_entity("callable=point_create")->get_function_pointer());
		auto destroy_fn = reinterpret_cast<point_destroy_func_t>(
			module->load_entity("callable=point_destroy")->get_function_pointer());
		REQUIRE(create_fn != nullptr);

		void* point = create_fn(42, 3.14);
		REQUIRE(point != nullptr);

		int* x_ptr = static_cast<int*>(getter->get(point));
		REQUIRE(x_ptr != nullptr);
		CHECK(*x_ptr == 42);

		destroy_fn(point);
	}

	TEST_CASE("14.2 CppFieldSetter writes Point::x, CppFieldGetter reads back the new value")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		auto offset_fn = reinterpret_cast<point_offset_x_func_t>(
			module->load_entity("callable=point_offset_x")->get_function_pointer());
		REQUIRE(offset_fn != nullptr);
		std::size_t offset = offset_fn();

		// Load getter and setter
		std::string getter_path = "field=x,getter=true,instance_required=true,field_offset=" + std::to_string(offset);
		std::string setter_path = "field=x,setter=true,instance_required=true,field_offset=" + std::to_string(offset) + ",field_size=4";

		auto getter_entity = module->load_entity(getter_path);
		auto setter_entity = module->load_entity(setter_path);

		auto* getter = dynamic_cast<CppFieldGetter*>(getter_entity.get());
		auto* setter = dynamic_cast<CppFieldSetter*>(setter_entity.get());
		REQUIRE(getter != nullptr);
		REQUIRE(setter != nullptr);

		// Create Point(1, 0.0)
		auto create_fn  = reinterpret_cast<point_create_func_t>(
			module->load_entity("callable=point_create")->get_function_pointer());
		auto destroy_fn = reinterpret_cast<point_destroy_func_t>(
			module->load_entity("callable=point_destroy")->get_function_pointer());
		REQUIRE(create_fn != nullptr);

		void* point = create_fn(1, 0.0);
		REQUIRE(point != nullptr);

		// Set x = 99
		int new_x = 99;
		setter->set(point, &new_x, sizeof(int));

		// Read back via getter
		int* x_ptr = static_cast<int*>(getter->get(point));
		CHECK(*x_ptr == 99);

		destroy_fn(point);
	}

	TEST_CASE("14.3 CppFieldGetter reads Point::y correctly")
	{
		REQUIRE(is_class_test_lib_available());

		cpp_runtime_manager manager;
		auto module = manager.load_module(CPP_TEST_LIB_CLASS_PATH);

		auto offset_fn = reinterpret_cast<point_offset_y_func_t>(
			module->load_entity("callable=point_offset_y")->get_function_pointer());
		REQUIRE(offset_fn != nullptr);
		std::size_t offset = offset_fn();

		std::string path = "field=y,getter=true,instance_required=true,field_offset=" + std::to_string(offset);
		auto entity = module->load_entity(path);
		REQUIRE(entity != nullptr);

		auto* getter = dynamic_cast<CppFieldGetter*>(entity.get());
		REQUIRE(getter != nullptr);

		auto create_fn  = reinterpret_cast<point_create_func_t>(
			module->load_entity("callable=point_create")->get_function_pointer());
		auto destroy_fn = reinterpret_cast<point_destroy_func_t>(
			module->load_entity("callable=point_destroy")->get_function_pointer());
		REQUIRE(create_fn != nullptr);

		void* point = create_fn(0, 2.718);
		REQUIRE(point != nullptr);

		double* y_ptr = static_cast<double*>(getter->get(point));
		REQUIRE(y_ptr != nullptr);
		CHECK(*y_ptr == doctest::Approx(2.718).epsilon(1e-10));

		destroy_fn(point);
	}
}
