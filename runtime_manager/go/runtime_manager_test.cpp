#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include <doctest/doctest.h>
#include "runtime_manager.h"
#include "module.h"
#include "entity.h"
#include <filesystem>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <string>
#include <cstdlib>

// Define function pointer types matching the Go exports
extern "C"
{
	typedef int (*add_func_t)(int, int);
	typedef int (*subtract_func_t)(int, int);
	typedef int (*multiply_func_t)(int, int);
	typedef double (*divide_func_t)(double, double);
	typedef double (*get_pi_func_t)();
	typedef int (*is_positive_func_t)(int);
	typedef int (*max_func_t)(int, int);
	typedef long long (*factorial_func_t)(int);
}

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

// Get Go info for tests - returns first detected Go or empty if none
static go_installed_info get_test_go_info()
{
	auto installations = go_runtime_manager::detect_installed_go();
	if (installations.empty())
	{
		return go_installed_info{};
	}
	return installations[0];
}

// Check if Go is available for testing
static bool is_go_available()
{
	auto installations = go_runtime_manager::detect_installed_go();
	return !installations.empty();
}

// Check if test module is available
static bool is_test_module_available()
{
	std::string module_path = GO_TEST_MODULE_PATH;
	return std::filesystem::exists(module_path);
}

// ============================================================================
// 1. Detection Tests
// ============================================================================

TEST_SUITE("1. Go Detection")
{
	TEST_CASE("1.1 Detect Go installations")
	{
		auto installations = go_runtime_manager::detect_installed_go();

		// We should find at least one Go installation (since we're building with Go)
		REQUIRE(installations.size() > 0);

		for (const auto& info : installations)
		{
			INFO("Found Go: " << info.version << " at " << info.goroot);
			CHECK(!info.version.empty());
			CHECK(!info.goroot.empty());
			CHECK(!info.go_exe.empty());

			// Version should start with "go"
			CHECK(info.version.rfind("go", 0) == 0);

			// Check that go executable exists
			CHECK(std::filesystem::exists(info.go_exe));
		}
	}

	TEST_CASE("1.2 Go installation has valid paths")
	{
		REQUIRE(is_go_available());

		auto info = get_test_go_info();

		// GOROOT should exist
		CHECK(std::filesystem::exists(info.goroot));

		// go executable should exist
		CHECK(std::filesystem::exists(info.go_exe));

		// bin directory should exist
		std::filesystem::path bin_dir = std::filesystem::path(info.goroot) / "bin";
		CHECK(std::filesystem::exists(bin_dir));
	}
}

// ============================================================================
// 2. Runtime Lifecycle Tests
// ============================================================================

TEST_SUITE("2. Runtime Lifecycle")
{
	TEST_CASE("2.1 Load runtime - success")
	{
		REQUIRE(is_go_available());

		auto info = get_test_go_info();
		go_runtime_manager manager(info);

		CHECK(manager.is_runtime_loaded() == false);
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(manager.is_runtime_loaded() == true);
	}

	TEST_CASE("2.2 Load runtime - idempotent")
	{
		REQUIRE(is_go_available());

		auto info = get_test_go_info();
		go_runtime_manager manager(info);

		// Multiple loads should succeed
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(manager.is_runtime_loaded() == true);
	}

	TEST_CASE("2.3 Release runtime")
	{
		REQUIRE(is_go_available());

		auto info = get_test_go_info();
		go_runtime_manager manager(info);

		manager.load_runtime();
		CHECK(manager.is_runtime_loaded() == true);

		manager.release_runtime();
		CHECK(manager.is_runtime_loaded() == false);
	}

	TEST_CASE("2.4 Release runtime - idempotent")
	{
		REQUIRE(is_go_available());

		auto info = get_test_go_info();
		go_runtime_manager manager(info);

		manager.load_runtime();

		// Multiple releases should succeed
		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
		CHECK(manager.is_runtime_loaded() == false);
	}

	TEST_CASE("2.5 Get Go info")
	{
		REQUIRE(is_go_available());

		auto info = get_test_go_info();
		go_runtime_manager manager(info);

		const auto& retrieved_info = manager.get_go_info();
		CHECK(retrieved_info.version == info.version);
		CHECK(retrieved_info.goroot == info.goroot);
		CHECK(retrieved_info.go_exe == info.go_exe);
	}
}

// ============================================================================
// 3. Module Loading Tests
// ============================================================================

TEST_SUITE("3. Module Loading")
{
	TEST_CASE("3.1 Load valid Go module")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		manager.load_runtime();

		std::shared_ptr<Module> module;
		CHECK(expect_no_throw([&]() { module = manager.load_module(module_path); }));
		CHECK(module != nullptr);
		CHECK(module->get_module_path() == module_path);
	}

	TEST_CASE("3.2 Load module - auto-loads runtime")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);

		CHECK(manager.is_runtime_loaded() == false);

		// load_module should auto-load runtime
		auto module = manager.load_module(module_path);
		CHECK(manager.is_runtime_loaded() == true);
		CHECK(module != nullptr);
	}

	TEST_CASE("3.3 Load module - invalid path throws")
	{
		REQUIRE(is_go_available());

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		manager.load_runtime();

		CHECK(expect_throw([&]() {
			manager.load_module("/nonexistent/path/to/module.dll");
		}));
	}

	TEST_CASE("3.4 Module - check symbol exists")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);

		CHECK(module->has_symbol("Add") == true);
		CHECK(module->has_symbol("Subtract") == true);
		CHECK(module->has_symbol("Multiply") == true);
		CHECK(module->has_symbol("Divide") == true);
		CHECK(module->has_symbol("GetPi") == true);
		CHECK(module->has_symbol("NonExistentFunction") == false);
	}
}

// ============================================================================
// 4. Entity Loading Tests
// ============================================================================

TEST_SUITE("4. Entity Loading")
{
	TEST_CASE("4.1 Load entity - callable format")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);

		std::shared_ptr<Entity> entity;
		CHECK(expect_no_throw([&]() { entity = module->load_entity("callable=Add"); }));
		CHECK(entity != nullptr);
		CHECK(entity->get_name() == "Add");
		CHECK(entity->get_function_pointer() != nullptr);
	}

	TEST_CASE("4.2 Load entity - bare function name")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);

		// Should also work without "callable=" prefix
		std::shared_ptr<Entity> entity;
		CHECK(expect_no_throw([&]() { entity = module->load_entity("Multiply"); }));
		CHECK(entity != nullptr);
		CHECK(entity->get_name() == "Multiply");
	}

	TEST_CASE("4.3 Load entity - nonexistent symbol throws")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);

		CHECK(expect_throw([&]() {
			module->load_entity("callable=NonExistentFunction");
		}));
	}
}

// ============================================================================
// 5. Function Execution Tests
// ============================================================================

TEST_SUITE("5. Function Execution")
{
	TEST_CASE("5.1 Call Add function")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);
		auto entity = module->load_entity("Add");

		auto add_func = reinterpret_cast<add_func_t>(entity->get_function_pointer());
		CHECK(add_func != nullptr);

		CHECK(add_func(2, 3) == 5);
		CHECK(add_func(0, 0) == 0);
		CHECK(add_func(-5, 5) == 0);
		CHECK(add_func(100, 200) == 300);
	}

	TEST_CASE("5.2 Call Subtract function")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);
		auto entity = module->load_entity("Subtract");

		auto subtract_func = reinterpret_cast<subtract_func_t>(entity->get_function_pointer());
		CHECK(subtract_func != nullptr);

		CHECK(subtract_func(5, 3) == 2);
		CHECK(subtract_func(0, 0) == 0);
		CHECK(subtract_func(3, 5) == -2);
	}

	TEST_CASE("5.3 Call Multiply function")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);
		auto entity = module->load_entity("Multiply");

		auto multiply_func = reinterpret_cast<multiply_func_t>(entity->get_function_pointer());
		CHECK(multiply_func != nullptr);

		CHECK(multiply_func(3, 4) == 12);
		CHECK(multiply_func(0, 100) == 0);
		CHECK(multiply_func(-2, 3) == -6);
	}

	TEST_CASE("5.4 Call Divide function")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);
		auto entity = module->load_entity("Divide");

		auto divide_func = reinterpret_cast<divide_func_t>(entity->get_function_pointer());
		CHECK(divide_func != nullptr);

		CHECK(divide_func(10.0, 2.0) == doctest::Approx(5.0));
		CHECK(divide_func(7.0, 2.0) == doctest::Approx(3.5));
		CHECK(divide_func(10.0, 0.0) == doctest::Approx(0.0));  // Our Go returns 0 for div by 0
	}

	TEST_CASE("5.5 Call GetPi function")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);
		auto entity = module->load_entity("GetPi");

		auto get_pi_func = reinterpret_cast<get_pi_func_t>(entity->get_function_pointer());
		CHECK(get_pi_func != nullptr);

		CHECK(get_pi_func() == doctest::Approx(3.14159265358979).epsilon(0.0001));
	}

	TEST_CASE("5.6 Call IsPositive function")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);
		auto entity = module->load_entity("IsPositive");

		auto is_positive_func = reinterpret_cast<is_positive_func_t>(entity->get_function_pointer());
		CHECK(is_positive_func != nullptr);

		CHECK(is_positive_func(5) == 1);
		CHECK(is_positive_func(0) == 0);
		CHECK(is_positive_func(-5) == 0);
	}

	TEST_CASE("5.7 Call Max function")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);
		auto entity = module->load_entity("Max");

		auto max_func = reinterpret_cast<max_func_t>(entity->get_function_pointer());
		CHECK(max_func != nullptr);

		CHECK(max_func(3, 7) == 7);
		CHECK(max_func(7, 3) == 7);
		CHECK(max_func(5, 5) == 5);
		CHECK(max_func(-10, -5) == -5);
	}

	TEST_CASE("5.8 Call Factorial function")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);
		auto entity = module->load_entity("Factorial");

		auto factorial_func = reinterpret_cast<factorial_func_t>(entity->get_function_pointer());
		CHECK(factorial_func != nullptr);

		CHECK(factorial_func(0) == 1);
		CHECK(factorial_func(1) == 1);
		CHECK(factorial_func(5) == 120);
		CHECK(factorial_func(10) == 3628800);
	}
}

// ============================================================================
// 6. Threading Tests
// ============================================================================

TEST_SUITE("6. Threading")
{
	TEST_CASE("6.1 Concurrent function calls")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module = manager.load_module(module_path);
		auto entity = module->load_entity("Add");

		auto add_func = reinterpret_cast<add_func_t>(entity->get_function_pointer());

		const int num_threads = 10;
		const int iterations = 100;
		std::atomic<int> success_count{0};
		std::vector<std::thread> threads;

		for (int t = 0; t < num_threads; t++)
		{
			threads.emplace_back([&, t]()
			{
				for (int i = 0; i < iterations; i++)
				{
					int a = t * iterations + i;
					int b = i;
					int expected = a + b;
					int result = add_func(a, b);
					if (result == expected)
					{
						success_count++;
					}
				}
			});
		}

		for (auto& thread : threads)
		{
			thread.join();
		}

		CHECK(success_count == num_threads * iterations);
	}

	TEST_CASE("6.2 Multiple managers with same module")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();

		// Create multiple managers
		go_runtime_manager manager1(info);
		go_runtime_manager manager2(info);

		auto module1 = manager1.load_module(module_path);
		auto module2 = manager2.load_module(module_path);

		auto entity1 = module1->load_entity("Add");
		auto entity2 = module2->load_entity("Add");

		auto add_func1 = reinterpret_cast<add_func_t>(entity1->get_function_pointer());
		auto add_func2 = reinterpret_cast<add_func_t>(entity2->get_function_pointer());

		// Both should work correctly
		CHECK(add_func1(5, 3) == 8);
		CHECK(add_func2(10, 20) == 30);
	}
}

// ============================================================================
// 7. Module Copy/Move Tests
// ============================================================================

TEST_SUITE("7. Module Semantics")
{
	TEST_CASE("7.1 Module copy constructor")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module1 = manager.load_module(module_path);

		// Copy the module
		Module module2(*module1);

		CHECK(module2.get_module_path() == module1->get_module_path());

		// Both should work
		auto entity1 = module1->load_entity("Add");
		auto entity2 = module2.load_entity("Add");

		auto func1 = reinterpret_cast<add_func_t>(entity1->get_function_pointer());
		auto func2 = reinterpret_cast<add_func_t>(entity2->get_function_pointer());

		CHECK(func1(1, 2) == 3);
		CHECK(func2(1, 2) == 3);
	}

	TEST_CASE("7.2 Module move constructor")
	{
		REQUIRE(is_go_available());
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		auto info = get_test_go_info();
		go_runtime_manager manager(info);
		auto module1 = manager.load_module(module_path);

		// Move the module
		Module module2(std::move(*module1));

		CHECK(module2.get_module_path() == module_path);

		// module2 should work
		auto entity = module2.load_entity("Add");
		auto func = reinterpret_cast<add_func_t>(entity->get_function_pointer());
		CHECK(func(10, 20) == 30);
	}
}

// ============================================================================
// 8. Go Shared Library Detection Tests
// ============================================================================

TEST_SUITE("8. Go Library Detection")
{
	TEST_CASE("8.1 Detect Go shared library - test module")
	{
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		go_detect_result result = go_runtime_manager::is_go_shared_library(module_path);

		INFO("Confidence: " << static_cast<int>(result.confidence));
		INFO("Reason: " << result.reason);
		if (result.go_version)
		{
			INFO("Go version: " << *result.go_version);
		}
		INFO("File offset: " << result.file_offset);

		// Should detect as Go library with high confidence
		CHECK(result.confidence != go_detect_confidence::no);

		// Should find the buildinfo magic at some offset
		CHECK(result.file_offset > 0);

		// If Go 1.18+, should have version string
		if (result.confidence == go_detect_confidence::yes)
		{
			REQUIRE(result.go_version.has_value());
			CHECK(result.go_version->rfind("go", 0) == 0);  // Starts with "go"
		}
	}

	TEST_CASE("8.2 Detect non-Go library - boost DLL")
	{
		// Find a non-Go DLL to test against (boost filesystem DLL)
		std::filesystem::path boost_dll;

#ifdef _WIN32
		// Look for boost DLL in output directory
		std::filesystem::path output_dir = std::filesystem::path(GO_TEST_MODULE_PATH).parent_path().parent_path();
		for (const auto& entry : std::filesystem::directory_iterator(output_dir))
		{
			if (entry.path().extension() == ".dll" &&
			    entry.path().filename().string().find("boost") != std::string::npos)
			{
				boost_dll = entry.path();
				break;
			}
		}
#endif

		if (boost_dll.empty() || !std::filesystem::exists(boost_dll))
		{
			// Skip if we can't find a non-Go library to test
			WARN("Skipping non-Go library test - no suitable test file found");
			return;
		}

		go_detect_result result = go_runtime_manager::is_go_shared_library(boost_dll.string());

		INFO("Testing non-Go library: " << boost_dll.string());
		INFO("Confidence: " << static_cast<int>(result.confidence));
		INFO("Reason: " << result.reason);

		// Should NOT detect as Go library
		CHECK(result.confidence == go_detect_confidence::no);
		CHECK(!result.go_version.has_value());
	}

	TEST_CASE("8.3 Detect Go library - nonexistent file throws")
	{
		CHECK_THROWS_AS(
			go_runtime_manager::is_go_shared_library("/nonexistent/path/to/library.dll"),
			std::runtime_error
		);
	}

	TEST_CASE("8.4 Detection result has meaningful reason")
	{
		REQUIRE(is_test_module_available());

		std::string module_path = GO_TEST_MODULE_PATH;

		go_detect_result result = go_runtime_manager::is_go_shared_library(module_path);

		// Reason should not be empty
		CHECK(!result.reason.empty());

		// Reason should mention buildinfo
		CHECK(result.reason.find("buildinfo") != std::string::npos);
	}
}
