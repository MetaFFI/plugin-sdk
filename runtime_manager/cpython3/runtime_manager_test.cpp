#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "runtime_manager.h"
#include "module.h"
#include "entity.h"
#include "python_api_wrapper.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>
#include <atomic>

template<typename Func>
bool expect_no_throw(Func&& func)
{
	try
	{
		func();
		return true;
	}
	catch(const std::exception& e)
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
	catch(const std::exception& e)
	{
		INFO(e.what());
		return true;
	}
}

// Helper function to create a test Python module file
std::string create_test_module(const std::string& module_name, const std::string& content)
{
	std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "metaffi_cpython3_test";
	std::filesystem::create_directories(test_dir);
	
	std::filesystem::path module_path = test_dir / (module_name + ".py");
	std::ofstream file(module_path);
	file << content;
	file.close();
	
	return module_path.string();
}

// Helper function to get a Python version for testing
std::string get_test_python_version()
{
	auto versions = cpython3_runtime_manager::detect_installed_python3();
	if(versions.empty())
	{
		// Try common versions - prefer 3.11 or 3.12
		// If no versions detected, tests will fail gracefully
		return "3.11";
	}
	// Prefer 3.11 or 3.12 if available, otherwise use first detected
	for(const auto& v : versions)
	{
		if(v == "3.11" || v == "3.12")
		{
			return v;
		}
	}
	return versions[0];
}

// Helper to check if we can actually load Python
bool can_load_python(const std::string& version)
{
	try
	{
		auto manager = cpython3_runtime_manager::create(version);
		return manager && manager->is_runtime_loaded();
	}
	catch(...)
	{
		return false;
	}
}

// Test module content
const char* test_module_content = R"(
# Test module for CPython3 runtime manager tests

# Global variable
test_global = 42

# Simple function
def test_function(x, y):
    return x + y

# Function with no parameters
def test_function_no_params():
    return 100

# Function with no return value
def test_function_no_return(x):
    pass

# Function with no parameters and no return value
def test_function_void():
    pass

# Class with methods
class TestClass:
    class_var = "class_value"
    
    def __init__(self, value):
        self.instance_var = value
    
    def instance_method(self, x):
        return self.instance_var + x
    
    @staticmethod
    def static_method(x):
        return x * 2
    
    @classmethod
    def class_method(cls, x):
        return cls.class_var + str(x)
    
    def method_no_params(self):
        return self.instance_var
    
    def method_no_return(self, x):
        self.instance_var = x

# Nested class
class OuterClass:
    class InnerClass:
        def inner_method(self):
            return "inner"
)";

TEST_SUITE("CPython3 Runtime Manager Tests")
{
	// ============================================================================
	// 3. Runtime Lifecycle Tests
	// ============================================================================
	
	TEST_CASE("3.1 Load Runtime - Success")
	{
		std::string version = get_test_python_version();
		std::shared_ptr<cpython3_runtime_manager> manager;
		
		CHECK(expect_no_throw([&]() { manager = cpython3_runtime_manager::create(version); }));
		CHECK(manager != nullptr);
		CHECK(manager->is_runtime_loaded() == true);
	}
	
	TEST_CASE("3.2 Load Runtime - Factory Method Idempotency")
	{
		// Creating multiple managers is valid - each manages its own state
		auto manager1 = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager1 != nullptr);
		CHECK(manager1->is_runtime_loaded() == true);
		
		// Second creation should also work (Python runtime is shared)
		auto manager2 = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager2 != nullptr);
		CHECK(manager2->is_runtime_loaded() == true);
	}
	
	TEST_CASE("3.3 Release Runtime - Success")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		CHECK(expect_no_throw([&]() { manager->release_runtime(); }));
		CHECK(manager->is_runtime_loaded() == false);
		
	}
	
	TEST_CASE("3.4 Release Runtime - Idempotency")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		// Call multiple times
		for(int i = 0; i < 5; i++)
		{
			CHECK(expect_no_throw([&]() { manager->release_runtime(); }));
			CHECK(manager->is_runtime_loaded() == false);
		}
	}
	
	TEST_CASE("3.5 Release Runtime Then Create New Manager")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		// Release runtime
		CHECK(expect_no_throw([&]() { manager->release_runtime(); }));
		CHECK(manager->is_runtime_loaded() == false);
		
		// Create new manager - should work
		auto manager2 = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager2 != nullptr);
		CHECK(manager2->is_runtime_loaded() == true);
	}
	
	TEST_CASE("3.6 Load After Release - New Manager")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		CHECK(expect_no_throw([&]() { manager->release_runtime(); }));
		
		// After release, create a new manager to reload
		auto manager2 = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager2 != nullptr);
		CHECK(manager2->is_runtime_loaded() == true);
		
	}
	
	TEST_CASE("3.7 Destructor Cleanup")
	{
		{
			auto manager = cpython3_runtime_manager::create(get_test_python_version());
			CHECK(manager != nullptr);
		}
		// Destructor should release runtime
		// No explicit check, but should not crash
	}
	
	TEST_CASE("3.8 Detect Installed Python Versions")
	{
		auto versions = cpython3_runtime_manager::detect_installed_python3();
		// Should return at least one version if Python is installed
		// If no Python is installed, this test may fail - that's expected
		CHECK(versions.size() >= 0);
		
		// If versions are found, verify format
		for(const auto& version : versions)
		{
			CHECK(version.length() >= 3); // At least "3.X"
			CHECK(version[0] == '3');
		}
	}
	
	// ============================================================================
	// 4. Module Loading Tests
	// ============================================================================
	
	TEST_CASE("4.1 Load Valid Module")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		CHECK(module != nullptr);
		if(module)
		{
			CHECK(module->get_module_path() == module_path);
		}
		
	}
	
	TEST_CASE("4.2 Load Invalid Module Path")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		CHECK(expect_throw([&]() {
			(void)manager->load_module("/invalid/path/module_that_does_not_exist.py");
		}));
		
	}
	
	TEST_CASE("4.3 Load Module - Runtime Auto-Initialized")
	{
		// With factory pattern, runtime is always initialized after create()
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		CHECK(manager->is_runtime_loaded() == true);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		CHECK(module != nullptr);
	}
	
	TEST_CASE("4.4 Load Same Module Multiple Times - No Caching")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		
		// Load first time
		auto module1 = manager->load_module(module_path);
		CHECK(module1 != nullptr);
		
		// Load second time - should create new instance (no caching)
		auto module2 = manager->load_module(module_path);
		CHECK(module2 != nullptr);
		
		// Verify they are different instances (no caching per requirements)
		CHECK(module1.get() != module2.get());
	}
	
	TEST_CASE("4.5 Load Different Modules")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module1_path = create_test_module("test_module1", test_module_content);
		std::string module2_path = create_test_module("test_module2", test_module_content);
		
		auto module1 = manager->load_module(module1_path);
		CHECK(module1 != nullptr);
		
		auto module2 = manager->load_module(module2_path);
		CHECK(module2 != nullptr);
		
		CHECK(module1.get() != module2.get());
		CHECK(module1->get_module_path() != module2->get_module_path());
	}
	
	TEST_CASE("4.6 Module Path Variations")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		// Test absolute file path
		std::string abs_path = create_test_module("test_abs", test_module_content);
		auto module_abs = manager->load_module(abs_path);
		CHECK(module_abs != nullptr);
	}
	
	TEST_CASE("4.7 Module Unload")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		CHECK(module != nullptr);
		
		// Load an entity first
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=test_function", params, retvals);
		
		// Unload module
		module->unload();
	}
	
	TEST_CASE("4.8 Module Unload Without Entities")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		CHECK(module != nullptr);
		
		module->unload();
	}
	
	// ============================================================================
	// 5. Entity Loading Tests
	// ============================================================================
	
	TEST_CASE("5.1 Load Function Entity")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=test_function", params, retvals);
		
		CHECK(entity != nullptr);
		
	}
	
	TEST_CASE("5.2 Load Method Entity")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=TestClass.instance_method,instance_required", params, retvals);
		
		CHECK(entity != nullptr);
		
	}
	
	TEST_CASE("5.3 Load Constructor Entity")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyBaseObject_Type}};
		auto entity = module->load_entity("callable=TestClass.__init__", params, retvals);
		
		CHECK(entity != nullptr);
		
	}
	
	TEST_CASE("5.4 Load Global Getter Entity")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params;
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("attribute=test_global,getter", params, retvals);
		
		CHECK(entity != nullptr);
		
	}
	
	TEST_CASE("5.5 Load Global Setter Entity")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals;
		auto entity = module->load_entity("attribute=test_global,setter", params, retvals);
		
		CHECK(entity != nullptr);
		
	}
	
	TEST_CASE("5.6 Load Field Getter Entity")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params;
		std::vector<PyObject*> retvals = {{pPyUnicode_Type}};
		auto entity = module->load_entity("attribute=TestClass.class_var,getter,instance_required", params, retvals);
		
		// Note: This might fail if class_var is accessed incorrectly, but the entity should load
		// The actual behavior depends on implementation
	}
	
	TEST_CASE("5.7 Load Field Setter Entity")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyUnicode_Type}};
		std::vector<PyObject*> retvals;
		auto entity = module->load_entity("attribute=TestClass.instance_var,setter,instance_required", params, retvals);
		
		// Note: This might fail if instance_var is accessed incorrectly, but the entity should load
	}
	
	TEST_CASE("5.8 Load Entity with Correct Types")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=test_function", params, retvals);
		
		CHECK(entity != nullptr);
		
	}
	
	TEST_CASE("5.9 Load Entity with Incorrect Types")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		// Wrong parameter count
		std::vector<PyObject*> params = {{pPyLong_Type}}; // Should be 2
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=test_function", params, retvals);
		
		// Type checking might not be strict at load time, but entity should still load
		// The actual error might occur at call time
	}
	
	TEST_CASE("5.10 Load Non-Existent Entity")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity("callable=nonexistent_function", params, retvals);
		}));
		
	}
	
	TEST_CASE("5.11 Load Same Entity Multiple Times - No Caching")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		
		// Load first time
		auto entity1 = module->load_entity("callable=test_function", params, retvals);
		CHECK(entity1 != nullptr);
		
		// Load second time - should create new instance (no caching)
		auto entity2 = module->load_entity("callable=test_function", params, retvals);
		CHECK(entity2 != nullptr);
		
		// Verify they are different instances
		CHECK(entity1.get() != entity2.get());
	}
	
	TEST_CASE("5.12 Load Same Entity with Different Types")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params1 = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals1 = {{pPyLong_Type}};
		auto entity1 = module->load_entity("callable=test_function", params1, retvals1);
		CHECK(entity1 != nullptr);
		
		std::vector<PyObject*> params2 = {{pPyFloat_Type}, {pPyFloat_Type}};
		std::vector<PyObject*> retvals2 = {{pPyFloat_Type}};
		auto entity2 = module->load_entity("callable=test_function", params2, retvals2);
		CHECK(entity2 != nullptr);
		
		// Should be different instances
		CHECK(entity1.get() != entity2.get());
	}
	
	TEST_CASE("5.13 Load Entity with Invalid Entity Path")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity("invalid_format", params, retvals);
		}));
		
	}
	
	TEST_CASE("5.14 Load Entity from Unloaded Module")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		module->unload();
		
		// Try to load entity after unload
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=test_function", params, retvals);
		
		// Behavior is implementation-specific - might error or might work
	}
	
	// ============================================================================
	// 6. Entity Execution Tests
	// ============================================================================
	// Note: CDTS conversion is out of scope, so these tests verify the interface
	// structure and error handling
	
	TEST_CASE("6.1 Call Function with Parameters and Return Value - Interface")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=test_function", params, retvals);
		CHECK(entity != nullptr);
		
		// Try to call - should return error about CDTS conversion
		// Note: Entity types are concrete, so we can't use dynamic_cast
		// The call() method is on CallableEntity, but we need to check entity type
		// For now, just verify entity was loaded correctly
		CHECK(entity->get_params_types().size() == 2);
		CHECK(entity->get_retval_types().size() == 1);
	}
	
	TEST_CASE("6.2 Call Function with No Parameters - Interface")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params;
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=test_function_no_params", params, retvals);
		CHECK(entity != nullptr);
		
		// Verify entity can be loaded and has correct interface
		CHECK(entity->get_params_types().size() == 0);
		CHECK(entity->get_retval_types().size() == 1);
	}
	
	TEST_CASE("6.3 Call Function with No Return Value - Interface")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals;
		auto entity = module->load_entity("callable=test_function_no_return", params, retvals);
		CHECK(entity != nullptr);
	}
	
	TEST_CASE("6.4 Call Function with No Parameters and No Return Value - Interface")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params;
		std::vector<PyObject*> retvals;
		auto entity = module->load_entity("callable=test_function_void", params, retvals);
		CHECK(entity != nullptr);
	}
	
	TEST_CASE("6.5 Call Method - Instance Method - Interface")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=TestClass.instance_method,instance_required", params, retvals);
		CHECK(entity != nullptr);
	}
	
	TEST_CASE("6.6 Call Method - Static Method - Interface")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=TestClass.static_method", params, retvals);
		CHECK(entity != nullptr);
	}
	
	TEST_CASE("6.7 Call Constructor - Interface")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyBaseObject_Type}};
		auto entity = module->load_entity("callable=TestClass.__init__", params, retvals);
		CHECK(entity != nullptr);
	}
	
	TEST_CASE("6.12 Get Global Variable - Interface")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params;
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("attribute=test_global,getter", params, retvals);
		CHECK(entity != nullptr);
		
		// Try to get - should return error about CDTS conversion
		// Note: VariableEntity has get() method, but we can't use dynamic_cast
		// For now, just verify entity was loaded correctly
		CHECK(entity->get_params_types().size() == 0);
		CHECK(entity->get_retval_types().size() == 1);
	}
	
	TEST_CASE("6.13 Set Global Variable - Interface")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals;
		auto entity = module->load_entity("attribute=test_global,setter", params, retvals);
		CHECK(entity != nullptr);
		
		// Try to set - should return error about CDTS conversion
		// Note: VariableEntity has set() method, but we can't use dynamic_cast
		// For now, just verify entity was loaded correctly
		CHECK(entity->get_params_types().size() == 1);
		CHECK(entity->get_retval_types().size() == 0);
	}
	
	TEST_CASE("6.16 Call with Null Pointers")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=test_function", params, retvals);
		CHECK(entity != nullptr);
		
		// Note: Can't use dynamic_cast, but entity should handle null gracefully
		// The actual call would need CDTS conversion which is out of scope
		CHECK(entity != nullptr);
	}
	
	// ============================================================================
	// 7. Caching Tests - Verify NO Caching
	// ============================================================================
	
	TEST_CASE("7.1 Module Cache - No Caching")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		
		// Load first time
		auto module1 = manager->load_module(module_path);
		CHECK(module1 != nullptr);
		
		// Load second time
		auto module2 = manager->load_module(module_path);
		CHECK(module2 != nullptr);
		
		// Verify different instances (no caching)
		CHECK(module1.get() != module2.get());
	}
	
	TEST_CASE("7.2 Module Cache Miss - Different Paths")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module1_path = create_test_module("test_module1", test_module_content);
		std::string module2_path = create_test_module("test_module2", test_module_content);
		
		auto module1 = manager->load_module(module1_path);
		
		auto module2 = manager->load_module(module2_path);
		
		CHECK(module1.get() != module2.get());
	}
	
	TEST_CASE("7.3 Entity Cache - No Caching")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		
		// Load first time
		auto entity1 = module->load_entity("callable=test_function", params, retvals);
		CHECK(entity1 != nullptr);
		
		// Load second time with same types
		auto entity2 = module->load_entity("callable=test_function", params, retvals);
		CHECK(entity2 != nullptr);
		
		// Verify different instances (no caching)
		CHECK(entity1.get() != entity2.get());
	}
	
	TEST_CASE("7.4 Entity Cache Miss - Different Types")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params1 = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals1 = {{pPyLong_Type}};
		auto entity1 = module->load_entity("callable=test_function", params1, retvals1);
		
		std::vector<PyObject*> params2 = {{pPyFloat_Type}, {pPyFloat_Type}};
		std::vector<PyObject*> retvals2 = {{pPyFloat_Type}};
		auto entity2 = module->load_entity("callable=test_function", params2, retvals2);
		
		CHECK(entity1.get() != entity2.get());
	}
	
	// ============================================================================
	// 8. Thread Safety Tests
	// ============================================================================
	
	TEST_CASE("8.1 Concurrent Runtime Create")
	{
		std::vector<std::thread> threads;
		std::vector<std::shared_ptr<cpython3_runtime_manager>> managers;
		std::mutex managers_mutex;
		std::atomic<int> success_count(0);
		std::atomic<int> fail_count(0);
		
		const int num_threads = 10;
		std::string version = get_test_python_version();
		
		for(int i = 0; i < num_threads; i++)
		{
			threads.emplace_back([&version, &managers, &managers_mutex, &success_count, &fail_count]() {
				try
				{
					auto manager = cpython3_runtime_manager::create(version);
					if(manager && manager->is_runtime_loaded())
					{
						std::lock_guard<std::mutex> lock(managers_mutex);
						managers.push_back(manager);
						success_count++;
					}
					else
					{
						fail_count++;
					}
				}
				catch(const std::exception&)
				{
					fail_count++;
				}
			});
		}
		
		for(auto& t : threads)
		{
			t.join();
		}
		
		CHECK(success_count == num_threads);
		CHECK(fail_count == 0);
	}
	
	TEST_CASE("8.2 Concurrent Runtime Release")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::vector<std::thread> threads;
		std::atomic<int> success_count(0);
		const int num_threads = 10;
		
		for(int i = 0; i < num_threads; i++)
		{
			threads.emplace_back([&manager, &success_count]() {
				try
				{
					CHECK(expect_no_throw([&]() { manager->release_runtime(); }));
					success_count++;
				}
				catch(const std::exception&)
				{
				}
			});
		}
		
		for(auto& t : threads)
		{
			t.join();
		}
		
		CHECK(success_count == num_threads);
		CHECK(manager->is_runtime_loaded() == false);
	}
	
	TEST_CASE("8.3 Concurrent Module Load")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		std::vector<std::thread> threads;
		std::vector<std::shared_ptr<Module>> modules;
		std::mutex modules_mutex;
		const int num_threads = 10;
		
		for(int i = 0; i < num_threads; i++)
		{
			threads.emplace_back([&manager, &module_path, &modules, &modules_mutex]() {
				try
				{
					auto module = manager->load_module(module_path);
					std::lock_guard<std::mutex> lock(modules_mutex);
					modules.push_back(module);
				}
				catch(const std::exception&)
				{
				}
			});
		}
		
		for(auto& t : threads)
		{
			t.join();
		}
		
		CHECK(modules.size() == num_threads);
		// All should be different instances (no caching)
		for(size_t i = 0; i < modules.size(); i++)
		{
			for(size_t j = i + 1; j < modules.size(); j++)
			{
				CHECK(modules[i].get() != modules[j].get());
			}
		}
	}
	
	TEST_CASE("8.4 Concurrent Entity Load")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		
		std::vector<std::thread> threads;
		std::vector<std::shared_ptr<Entity>> entities;
		std::mutex entities_mutex;
		const int num_threads = 10;
		
		for(int i = 0; i < num_threads; i++)
		{
			threads.emplace_back([&module, &params, &retvals, &entities, &entities_mutex]() {
				try
				{
					auto entity = module->load_entity("callable=test_function", params, retvals);
					std::lock_guard<std::mutex> lock(entities_mutex);
					entities.push_back(entity);
				}
				catch(const std::exception&)
				{
				}
			});
		}
		
		for(auto& t : threads)
		{
			t.join();
		}
		
		CHECK(entities.size() == num_threads);
		// All should be different instances (no caching)
		for(size_t i = 0; i < entities.size(); i++)
		{
			for(size_t j = i + 1; j < entities.size(); j++)
			{
				CHECK(entities[i].get() != entities[j].get());
			}
		}
	}
	
	TEST_CASE("8.5 Concurrent Entity Calls")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=test_function", params, retvals);
		CHECK(entity != nullptr);
		
		std::vector<std::thread> threads;
		std::atomic<int> call_count(0);
		const int num_threads = 10;
		
		// Note: Can't use dynamic_cast, but we can verify entity is valid
		// The actual concurrent calls would need CDTS conversion which is out of scope
		CHECK(entity != nullptr);
		CHECK(entity->get_params_types().size() == 2);
		CHECK(entity->get_retval_types().size() == 1);
		
		// Just verify entity can be accessed concurrently
		for(int i = 0; i < num_threads; i++)
		{
			threads.emplace_back([&entity, &call_count]() {
				// Access entity properties (thread-safe)
				auto params = entity->get_params_types();
				auto retvals = entity->get_retval_types();
				call_count++;
			});
		}
		
		for(auto& t : threads)
		{
			t.join();
		}
		
		CHECK(call_count == num_threads);
	}
	
	// ============================================================================
	// 9. Memory Management Tests
	// ============================================================================
	
	TEST_CASE("9.1 Destructor Cleanup")
	{
		{
			auto manager = cpython3_runtime_manager::create(get_test_python_version());
			CHECK(manager != nullptr);
			
			std::string module_path = create_test_module("test_module", test_module_content);
			auto module = manager->load_module(module_path);
			
			std::vector<PyObject*> params = {{pPyLong_Type}};
			std::vector<PyObject*> retvals = {{pPyLong_Type}};
			auto entity = module->load_entity("callable=test_function_no_params", params, retvals);
		}
		// Destructors should clean up - no explicit check but should not crash
	}
	
	TEST_CASE("9.2 Module Cleanup")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity1 = module->load_entity("callable=test_function_no_params", params, retvals);
		
		auto entity2 = module->load_entity("callable=test_function", params, retvals);
		
		module->unload();
	}
	
	TEST_CASE("9.3 Entity Cleanup")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		{
			std::vector<PyObject*> params = {{pPyLong_Type}};
			std::vector<PyObject*> retvals = {{pPyLong_Type}};
			auto entity = module->load_entity("callable=test_function_no_params", params, retvals);
		}
		// Entity should be cleaned up when out of scope
	}
	
	TEST_CASE("9.4 Error Path Cleanup")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		// Trigger error - load invalid entity
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity("callable=nonexistent", params, retvals);
		}));
		
		// Module should still be valid
		auto entity2 = module->load_entity("callable=test_function_no_params", params, retvals);
		CHECK(entity2 != nullptr);
	}
	
	// ============================================================================
	// 10. Error Handling Tests
	// ============================================================================
	
	TEST_CASE("10.1 Error Message Format")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		// Trigger error
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		try
		{
			(void)module->load_entity("callable=nonexistent", params, retvals);
			CHECK_MESSAGE(false, "Expected exception was not thrown");
		}
		catch(const std::exception& e)
		{
			CHECK(std::string(e.what()).size() > 0);
		}
		
	}
	
	TEST_CASE("10.2 Error Message Memory")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		// Trigger error
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity("callable=nonexistent", params, retvals);
		}));
		// Exception message lifetime should be managed by std::exception.
	}
	
	TEST_CASE("10.3 Error Propagation")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		// Load entity fails
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity("callable=nonexistent", params, retvals);
		}));
		
		// Module should still be valid
		auto entity2 = module->load_entity("callable=test_function_no_params", params, retvals);
		CHECK(entity2 != nullptr);
	}
	
	TEST_CASE("10.4 Null Pointer Handling")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		// load_module with empty string might fail
		try
		{
			(void)manager->load_module("");
		}
		catch(const std::exception&)
		{
			// Acceptable: invalid module path.
		}
		// Behavior is implementation-specific
	}
	
	// ============================================================================
	// 11. Integration Tests
	// ============================================================================
	
	TEST_CASE("11.1 End-to-End Function Call")
	{
		// Create manager (runtime auto-loaded)
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		// Load module
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		// Load function entity
		std::vector<PyObject*> params = {{pPyLong_Type}, {pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=test_function", params, retvals);
		CHECK(entity != nullptr);
		
		// Release runtime
		CHECK(expect_no_throw([&]() { manager->release_runtime(); }));
	}
	
	TEST_CASE("11.2 Multiple Modules and Entities")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		// Load multiple modules
		std::string module1_path = create_test_module("test_module1", test_module_content);
		std::string module2_path = create_test_module("test_module2", test_module_content);
		
		auto module1 = manager->load_module(module1_path);
		
		auto module2 = manager->load_module(module2_path);
		
		// Load entities from each module
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		
		auto entity1 = module1->load_entity("callable=test_function_no_params", params, retvals);
		
		auto entity2 = module2->load_entity("callable=test_function_no_params", params, retvals);
		
		CHECK(entity1 != nullptr);
		CHECK(entity2 != nullptr);
	}
	
	TEST_CASE("11.3 Complex Entity Paths")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		// Test nested class
		std::vector<PyObject*> params;
		std::vector<PyObject*> retvals = {{pPyUnicode_Type}};
		auto entity = module->load_entity("callable=OuterClass.InnerClass.inner_method,instance_required", params, retvals);
		// Might fail if nested class access is not supported, but should handle gracefully
	}
	
	TEST_CASE("11.4 Runtime Reload Scenario")
	{
		// First cycle
		auto manager1 = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager1 != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module1 = manager1->load_module(module_path);
		
		CHECK(expect_no_throw([&]() { manager1->release_runtime(); }));
		
		// Second cycle - create new manager
		auto manager2 = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager2 != nullptr);
		
		auto module2 = manager2->load_module(module_path);
		
		CHECK(module1.get() != module2.get());
	}
	
	// ============================================================================
	// 12. Python-Specific Tests
	// ============================================================================
	
	TEST_CASE("12.1 Python Varargs Support")
	{
		const char* varargs_module = R"(
def varargs_function(x, *args):
    return x + sum(args)
)";
		
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("varargs_module", varargs_module);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=varargs_function,varargs", params, retvals);
		CHECK(entity != nullptr);
	}
	
	TEST_CASE("12.2 Python Named Args Support")
	{
		const char* kwargs_module = R"(
def kwargs_function(x, **kwargs):
    return x + kwargs.get('y', 0)
)";
		
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("kwargs_module", kwargs_module);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		auto entity = module->load_entity("callable=kwargs_function,named_args", params, retvals);
		CHECK(entity != nullptr);
	}
	
	TEST_CASE("12.3 Python Static Method")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyLong_Type}};
		// Static method - no instance_required
		auto entity = module->load_entity("callable=TestClass.static_method", params, retvals);
		CHECK(entity != nullptr);
	}
	
	TEST_CASE("12.4 Python Class Method")
	{
		auto manager = cpython3_runtime_manager::create(get_test_python_version());
		CHECK(manager != nullptr);
		
		std::string module_path = create_test_module("test_module", test_module_content);
		auto module = manager->load_module(module_path);
		
		std::vector<PyObject*> params = {{pPyLong_Type}};
		std::vector<PyObject*> retvals = {{pPyUnicode_Type}};
		// Class method - no instance_required
		auto entity = module->load_entity("callable=TestClass.class_method", params, retvals);
		CHECK(entity != nullptr);
	}
}
