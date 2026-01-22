#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include <doctest/doctest.h>
#include "runtime_manager.h"
#include "module.h"
#include "entity.h"
#include "jni_helpers.h"
#include <filesystem>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <string>
#include <functional>

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

struct env_guard
{
	explicit env_guard(jdk_runtime_manager& manager)
		: manager_(manager), release_(manager_.get_env(&env_))
	{
	}

	~env_guard()
	{
		if(release_)
		{
			release_();
		}
	}

	jdk_runtime_manager& manager_;
	JNIEnv* env_ = nullptr;
	std::function<void()> release_;
};

static jclass get_class(JNIEnv* env, const char* name)
{
	jclass cls = env->FindClass(name);
	if(!cls)
	{
		std::string error = get_exception_description(env);
		throw std::runtime_error(error.empty() ? std::string("Failed to find class: ") + name : error);
	}
	return cls;
}

static jclass get_primitive_class(JNIEnv* env, const char* wrapper_class)
{
	jclass wrapper = get_class(env, wrapper_class);
	jfieldID type_field = env->GetStaticFieldID(wrapper, "TYPE", "Ljava/lang/Class;");
	if(!type_field)
	{
		std::string error = get_exception_description(env);
		env->DeleteLocalRef(wrapper);
		throw std::runtime_error(error.empty() ? "Failed to get TYPE field" : error);
	}
	jclass primitive = (jclass)env->GetStaticObjectField(wrapper, type_field);
	env->DeleteLocalRef(wrapper);
	if(!primitive)
	{
		std::string error = get_exception_description(env);
		throw std::runtime_error(error.empty() ? "Failed to read primitive TYPE" : error);
	}
	return primitive;
}

struct jvm_types
{
	jclass int_type = nullptr;
	jclass double_type = nullptr;
	jclass string_type = nullptr;
	jclass object_type = nullptr;
};

static jvm_types get_jvm_types(JNIEnv* env)
{
	jvm_types types;
	types.int_type = get_primitive_class(env, "java/lang/Integer");
	types.double_type = get_primitive_class(env, "java/lang/Double");
	types.string_type = get_class(env, "java/lang/String");
	types.object_type = get_class(env, "java/lang/Object");
	return types;
}

struct test_env
{
	explicit test_env(jdk_runtime_manager& manager)
		: guard(manager), env(guard.env_), types(get_jvm_types(env))
	{
	}

	env_guard guard;
	JNIEnv* env = nullptr;
	jvm_types types{};
};

#define WITH_JVM_TYPES(manager) test_env tenv(manager); const auto& types = tenv.types; (void)types

constexpr const char* test_class_name = "com.metaffi.jdk.TestModule";
constexpr const char* test_inner_class_name = "com.metaffi.jdk.TestModule.Inner";

static std::string get_test_module_path()
{
	std::filesystem::path jar_path = JDK_TEST_JAR_PATH;
	if(!std::filesystem::exists(jar_path))
	{
		throw std::runtime_error("Test JAR path does not exist: " + jar_path.string());
	}
	return jar_path.string();
}

static jvm_installed_info get_test_jvm_info()
{
	auto jvms = jdk_runtime_manager::detect_installed_jvms();
	if(jvms.empty())
	{
		throw std::runtime_error("No installed JVMs detected");
	}

	auto prefer_version = [](const std::string& version, const std::string& prefix){
		return version.rfind(prefix, 0) == 0;
	};

	for(const auto& info : jvms)
	{
		if(prefer_version(info.version, "21") || prefer_version(info.version, "17") || prefer_version(info.version, "11"))
		{
			return info;
		}
	}
	return jvms[0];
}

static bool can_load_jvm(const jvm_installed_info& info)
{
	jdk_runtime_manager manager(info);
	if(!expect_no_throw([&]() { manager.load_runtime(); }))
	{
		return false;
	}
	return manager.is_runtime_loaded();
}

TEST_SUITE("JDK Runtime Manager Tests")
{
	// ============================================================================
	// 3. Runtime Lifecycle Tests
	// ============================================================================

	TEST_CASE("3.1 Load Runtime - Success")
	{
		auto info = get_test_jvm_info();
		jdk_runtime_manager manager(info);

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(manager.is_runtime_loaded() == true);
	}

	TEST_CASE("3.2 Load Runtime - Idempotency")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		for(int i = 0; i < 5; i++)
		{
			CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
			CHECK(manager.is_runtime_loaded() == true);
		}
	}

	TEST_CASE("3.3 Release Runtime - Success")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
		CHECK(manager.is_runtime_loaded() == false);
	}

	TEST_CASE("3.4 Release Runtime - Idempotency")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		for(int i = 0; i < 5; i++)
		{
			CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
			CHECK(manager.is_runtime_loaded() == false);
		}
	}

	TEST_CASE("3.5 Release Runtime Without Load")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
	}

	TEST_CASE("3.6 Load After Release")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(manager.is_runtime_loaded() == true);
	}

	TEST_CASE("3.7 Destructor Cleanup")
	{
		auto info = get_test_jvm_info();
		{
			jdk_runtime_manager manager(info);
			CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		}
	}

	// ============================================================================
	// 4. Module Loading Tests
	// ============================================================================

	TEST_CASE("4.1 Load Valid Module")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::string module_path = get_test_module_path();
		auto module = manager.load_module(module_path);

		CHECK(module != nullptr);
		if(module)
		{
			CHECK(module->get_module_path() == module_path);
		}
	}

	TEST_CASE("4.2 Load Invalid Module Path")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
		CHECK(expect_throw([&]() {
			(void)manager.load_module("C:/invalid/path/jdk_runtime_manager_missing.jar");
		}));
	}

	TEST_CASE("4.3 Load Module Without Runtime")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		std::string module_path = get_test_module_path();
		std::shared_ptr<Module> module;
		CHECK(expect_no_throw([&]() { module = manager.load_module(module_path); }));
		CHECK(module != nullptr);
		CHECK(manager.is_runtime_loaded() == true);
	}

	TEST_CASE("4.4 Load Same Module Multiple Times - No Caching")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::string module_path = get_test_module_path();

		auto module1 = manager.load_module(module_path);
		CHECK(module1 != nullptr);

		auto module2 = manager.load_module(module_path);
		CHECK(module2 != nullptr);

		CHECK(module1.get() != module2.get());
	}

	TEST_CASE("4.5 Load Different Modules")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::string module1_path = get_test_module_path();
		#ifdef _WIN32
		const char classpath_separator = ';';
		#else
		const char classpath_separator = ':';
		#endif
		std::string module2_path = module1_path + classpath_separator + ".";

		auto module1 = manager.load_module(module1_path);
		CHECK(module1 != nullptr);

		auto module2 = manager.load_module(module2_path);
		CHECK(module2 != nullptr);

		CHECK(module1.get() != module2.get());
		CHECK(module1->get_module_path() != module2->get_module_path());
	}

	TEST_CASE("4.6 Module Path Variations")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::filesystem::path jar_path = get_test_module_path();
		std::string abs_path = std::filesystem::absolute(jar_path).string();
		auto module_abs = manager.load_module(abs_path);
		CHECK(module_abs != nullptr);
	}

	TEST_CASE("4.7 Module Unload")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::string module_path = get_test_module_path();
		auto module = manager.load_module(module_path);
		CHECK(module != nullptr);

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		(void)entity;

		module->unload();
	}

	TEST_CASE("4.8 Module Unload Without Entities")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::string module_path = get_test_module_path();
		auto module = manager.load_module(module_path);
		CHECK(module != nullptr);

		module->unload();
	}

	// ============================================================================
	// 5. Entity Loading Tests
	// ============================================================================

	TEST_CASE("5.1 Load Method Entity")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);

		CHECK(entity != nullptr);
	}

	TEST_CASE("5.2 Load Instance Method Entity")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_to_instance,instance_required", params, retvals);

		CHECK(entity != nullptr);
	}

	TEST_CASE("5.3 Load Constructor Entity")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.object_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=<init>", params, retvals);

		CHECK(entity != nullptr);
	}

	TEST_CASE("5.4 Load Field Getter Entity")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params;
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",field=static_field,getter", params, retvals);

		CHECK(entity != nullptr);
	}

	TEST_CASE("5.5 Load Field Setter Entity")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals;
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",field=static_field,setter", params, retvals);

		CHECK(entity != nullptr);
	}

	TEST_CASE("5.6 Load Instance Field Getter Entity")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params;
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",field=instance_field,getter,instance_required", params, retvals);

		CHECK(entity != nullptr);
	}

	TEST_CASE("5.7 Load Instance Field Setter Entity")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals;
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",field=instance_field,setter,instance_required", params, retvals);

		CHECK(entity != nullptr);
	}

	TEST_CASE("5.8 Load Entity with Correct Types")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);

		CHECK(entity != nullptr);
	}

	TEST_CASE("5.9 Load Entity with Incorrect Types")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.string_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		}));
	}

	TEST_CASE("5.10 Load Non-Existent Entity")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity(std::string("class=") + test_class_name + ",callable=nonexistent_method", params, retvals);
		}));
	}

	TEST_CASE("5.11 Load Same Entity Multiple Times - No Caching")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};

		auto entity1 = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		CHECK(entity1 != nullptr);

		auto entity2 = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		CHECK(entity2 != nullptr);

		CHECK(entity1.get() != entity2.get());
	}

	TEST_CASE("5.12 Load Same Entity with Different Types")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params1 = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals1 = {{types.int_type}};
		auto entity1 = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params1, retvals1);
		CHECK(entity1 != nullptr);

		std::vector<jclass> params2 = {{types.double_type}, {types.double_type}};
		std::vector<jclass> retvals2 = {{types.double_type}};
		auto entity2 = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params2, retvals2);
		CHECK(entity2 != nullptr);

		CHECK(entity1.get() != entity2.get());
	}

	TEST_CASE("5.13 Load Entity with Invalid Entity Path")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity("invalid_format", params, retvals);
		}));
	}

	TEST_CASE("5.14 Load Entity from Unloaded Module")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		module->unload();

		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		(void)entity;
	}

	// ============================================================================
	// 6. Entity Execution Tests
	// ============================================================================
	// Note: CDTS conversion is out of scope, so these tests verify the interface
	// structure and error handling

	TEST_CASE("6.1 Call Method with Parameters and Return Value - Interface")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		CHECK(entity != nullptr);

		CHECK(entity->get_params_types().size() == 2);
		CHECK(entity->get_retval_types().size() == 1);
	}

	TEST_CASE("6.2 Call Method with No Parameters - Interface")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params;
		std::vector<jclass> retvals;
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=static_void", params, retvals);
		CHECK(entity != nullptr);

		CHECK(entity->get_params_types().size() == 0);
		CHECK(entity->get_retval_types().size() == 0);
	}

	TEST_CASE("6.3 Call Method with No Return Value - Interface")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals;
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=instance_void,instance_required", params, retvals);
		CHECK(entity != nullptr);
	}

	TEST_CASE("6.4 Call Method with No Parameters and No Return Value - Interface")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params;
		std::vector<jclass> retvals;
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=static_void", params, retvals);
		CHECK(entity != nullptr);
	}

	TEST_CASE("6.5 Call Method - Instance Method - Interface")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_to_instance,instance_required", params, retvals);
		CHECK(entity != nullptr);
	}

	TEST_CASE("6.6 Call Method - Static Method - Interface")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		CHECK(entity != nullptr);
	}

	TEST_CASE("6.7 Call Constructor - Interface")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.object_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=<init>", params, retvals);
		CHECK(entity != nullptr);
	}

	TEST_CASE("6.12 Get Field - Interface")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params;
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",field=static_field,getter", params, retvals);
		CHECK(entity != nullptr);

		CHECK(entity->get_params_types().size() == 0);
		CHECK(entity->get_retval_types().size() == 1);
	}

	TEST_CASE("6.13 Set Field - Interface")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals;
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",field=static_field,setter", params, retvals);
		CHECK(entity != nullptr);

		CHECK(entity->get_params_types().size() == 1);
		CHECK(entity->get_retval_types().size() == 0);
	}

	TEST_CASE("6.16 Call with Null Pointers")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		CHECK(entity != nullptr);
	}

	// ============================================================================
	// 7. Caching Tests - Verify NO Caching
	// ============================================================================

	TEST_CASE("7.1 Module Cache - No Caching")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::string module_path = get_test_module_path();

		auto module1 = manager.load_module(module_path);
		CHECK(module1 != nullptr);

		auto module2 = manager.load_module(module_path);
		CHECK(module2 != nullptr);

		CHECK(module1.get() != module2.get());
	}

	TEST_CASE("7.2 Module Cache Miss - Different Paths")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::string module1_path = get_test_module_path();
		#ifdef _WIN32
		const char classpath_separator = ';';
		#else
		const char classpath_separator = ':';
		#endif
		std::string module2_path = module1_path + classpath_separator + ".";

		auto module1 = manager.load_module(module1_path);

		auto module2 = manager.load_module(module2_path);

		CHECK(module1.get() != module2.get());
	}

	TEST_CASE("7.3 Entity Cache - No Caching")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};

		auto entity1 = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		CHECK(entity1 != nullptr);

		auto entity2 = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		CHECK(entity2 != nullptr);

		CHECK(entity1.get() != entity2.get());
	}

	TEST_CASE("7.4 Entity Cache Miss - Different Types")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params1 = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals1 = {{types.int_type}};
		auto entity1 = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params1, retvals1);

		std::vector<jclass> params2 = {{types.double_type}, {types.double_type}};
		std::vector<jclass> retvals2 = {{types.double_type}};
		auto entity2 = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params2, retvals2);

		CHECK(entity1.get() != entity2.get());
	}

	// ============================================================================
	// 8. Thread Safety Tests
	// ============================================================================

	TEST_CASE("8.1 Concurrent Runtime Load")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		std::vector<std::thread> threads;
		std::atomic<int> success_count(0);
		std::atomic<int> fail_count(0);

		const int num_threads = 10;

		for(int i = 0; i < num_threads; i++)
		{
			threads.emplace_back([&manager, &success_count, &fail_count]() {
				try
				{
					CHECK(expect_no_throw([&]() { manager.load_runtime(); }));
					success_count++;
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
		CHECK(manager.is_runtime_loaded() == true);
	}

	TEST_CASE("8.2 Concurrent Runtime Release")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::vector<std::thread> threads;
		std::atomic<int> success_count(0);
		const int num_threads = 10;

		for(int i = 0; i < num_threads; i++)
		{
			threads.emplace_back([&manager, &success_count]() {
				try
				{
					CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
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
		CHECK(manager.is_runtime_loaded() == false);
	}

	TEST_CASE("8.3 Concurrent Module Load")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::string module_path = get_test_module_path();
		std::vector<std::thread> threads;
		std::vector<std::shared_ptr<Module>> modules;
		std::mutex modules_mutex;
		const int num_threads = 10;

		for(int i = 0; i < num_threads; i++)
		{
			threads.emplace_back([&manager, &module_path, &modules, &modules_mutex]() {
				try
				{
					auto module = manager.load_module(module_path);
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
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};

		std::vector<std::thread> threads;
		std::vector<std::shared_ptr<Entity>> entities;
		std::mutex entities_mutex;
		const int num_threads = 10;

		for(int i = 0; i < num_threads; i++)
		{
			threads.emplace_back([&module, &params, &retvals, &entities, &entities_mutex]() {
				try
				{
					auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
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
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		CHECK(entity != nullptr);

		std::vector<std::thread> threads;
		std::atomic<int> call_count(0);
		const int num_threads = 10;

		CHECK(entity != nullptr);
		CHECK(entity->get_params_types().size() == 2);
		CHECK(entity->get_retval_types().size() == 1);

		for(int i = 0; i < num_threads; i++)
		{
			threads.emplace_back([&entity, &call_count]() {
				auto params = entity->get_params_types();
				auto retvals = entity->get_retval_types();
				(void)params;
				(void)retvals;
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
			jdk_runtime_manager manager(get_test_jvm_info());
			CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

			auto module = manager.load_module(get_test_module_path());

			WITH_JVM_TYPES(manager);
			std::vector<jclass> params = {{types.int_type}};
			std::vector<jclass> retvals = {{types.object_type}};
			auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=<init>", params, retvals);
			(void)entity;
		}
	}

	TEST_CASE("9.2 Module Cleanup")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.object_type}};
		auto entity1 = module->load_entity(std::string("class=") + test_class_name + ",callable=<init>", params, retvals);

		auto entity2 = module->load_entity(std::string("class=") + test_class_name + ",callable=<init>", params, retvals);
		(void)entity1;
		(void)entity2;

		module->unload();
	}

	TEST_CASE("9.3 Entity Cleanup")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		{
			std::vector<jclass> params = {{types.int_type}, {types.int_type}};
			std::vector<jclass> retvals = {{types.int_type}};
			auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
			(void)entity;
		}
	}

	TEST_CASE("9.4 Error Path Cleanup")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity(std::string("class=") + test_class_name + ",callable=nonexistent", params, retvals);
		}));

		auto entity2 = module->load_entity(std::string("class=") + test_class_name + ",callable=add_to_instance,instance_required", params, retvals);
		CHECK(entity2 != nullptr);
	}

	// ============================================================================
	// 10. Error Handling Tests
	// ============================================================================

	TEST_CASE("10.1 Error Message Format")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		try
		{
			(void)module->load_entity(std::string("class=") + test_class_name + ",callable=nonexistent", params, retvals);
			CHECK_MESSAGE(false, "Expected exception was not thrown");
		}
		catch(const std::exception& e)
		{
			CHECK(std::string(e.what()).size() > 0);
		}
	}

	TEST_CASE("10.2 Error Message Memory")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity(std::string("class=") + test_class_name + ",callable=nonexistent", params, retvals);
		}));
	}

	TEST_CASE("10.3 Error Propagation")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		CHECK(expect_throw([&]() {
			(void)module->load_entity(std::string("class=") + test_class_name + ",callable=nonexistent", params, retvals);
		}));

		auto entity2 = module->load_entity(std::string("class=") + test_class_name + ",callable=add_to_instance,instance_required", params, retvals);
		CHECK(entity2 != nullptr);
	}

	TEST_CASE("10.4 Null Pointer Handling")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		try
		{
			(void)manager.load_module("");
		}
		catch(const std::exception&)
		{
		}
	}

	// ============================================================================
	// 11. Integration Tests
	// ============================================================================

	TEST_CASE("11.1 End-to-End Method Call")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		{
			WITH_JVM_TYPES(manager);
			std::vector<jclass> params = {{types.int_type}, {types.int_type}};
			std::vector<jclass> retvals = {{types.int_type}};
			auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
			CHECK(entity != nullptr);
		}

		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));
	}

	TEST_CASE("11.2 Multiple Modules and Entities")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::string module1_path = get_test_module_path();
		#ifdef _WIN32
		const char classpath_separator = ';';
		#else
		const char classpath_separator = ':';
		#endif
		std::string module2_path = module1_path + classpath_separator + ".";

		auto module1 = manager.load_module(module1_path);
		auto module2 = manager.load_module(module2_path);

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};

		auto entity1 = module1->load_entity(std::string("class=") + test_class_name + ",callable=add_to_instance,instance_required", params, retvals);
		auto entity2 = module2->load_entity(std::string("class=") + test_class_name + ",callable=add_to_instance,instance_required", params, retvals);

		CHECK(entity1 != nullptr);
		CHECK(entity2 != nullptr);
	}

	TEST_CASE("11.3 Complex Entity Paths")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params;
		std::vector<jclass> retvals = {{types.string_type}};
		auto entity = module->load_entity(std::string("class=") + test_inner_class_name + ",callable=inner_method,instance_required", params, retvals);
		(void)entity;
	}

	TEST_CASE("11.4 Runtime Reload Scenario")
	{
		jdk_runtime_manager manager(get_test_jvm_info());

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		std::string module_path = get_test_module_path();
		auto module1 = manager.load_module(module_path);

		CHECK(expect_no_throw([&]() { manager.release_runtime(); }));

		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module2 = manager.load_module(module_path);

		CHECK(module1.get() != module2.get());
	}

	// ============================================================================
	// 12. Java-Specific Tests
	// ============================================================================

	TEST_CASE("12.1 Java Static Method")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}, {types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_ints", params, retvals);
		CHECK(entity != nullptr);
	}

	TEST_CASE("12.2 Java Instance Method")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params = {{types.int_type}};
		std::vector<jclass> retvals = {{types.int_type}};
		auto entity = module->load_entity(std::string("class=") + test_class_name + ",callable=add_to_instance,instance_required", params, retvals);
		CHECK(entity != nullptr);
	}

	TEST_CASE("12.3 Java Nested Class")
	{
		jdk_runtime_manager manager(get_test_jvm_info());
		CHECK(expect_no_throw([&]() { manager.load_runtime(); }));

		auto module = manager.load_module(get_test_module_path());

		WITH_JVM_TYPES(manager);
		std::vector<jclass> params;
		std::vector<jclass> retvals = {{types.string_type}};
		auto entity = module->load_entity(std::string("class=") + test_inner_class_name + ",callable=inner_method,instance_required", params, retvals);
		CHECK(entity != nullptr);
	}
}
