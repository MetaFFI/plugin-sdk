# JDK Runtime Manager Library (source-only, no build needed)
# Tests are built and run here

find_or_install_package(Boost COMPONENTS filesystem system)
find_or_install_package(doctest)

collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} jvm_runtime_manager)

collect_c_cpp_files("${metaffi_sdk_root}/utils" sdk_utils)
list(APPEND sdk_utils_src "${metaffi_sdk_root}/utils/env_utils.cpp")

# Runtime sources required by cdts_java_wrapper (cdts, traversal, allocators, primitives)
collect_c_cpp_files("${metaffi_sdk_root}/runtime" sdk_runtime)

build_jar(jvm_runtime_manager_test_jar
	SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/java/com/metaffi/jvm/*.java"
	OUTPUT_DIR "sdk/runtime_manager/jvm/tests"
	OUTPUT_NAME "jvm_runtime_manager_test"
)

set(JDK_TEST_JAR_PATH "$ENV{METAFFI_HOME}/sdk/runtime_manager/jvm/tests/jvm_runtime_manager_test.jar")
string(REPLACE "\\" "/" JDK_TEST_JAR_PATH "${JDK_TEST_JAR_PATH}")

set(jvm_runtime_manager_test_libs "doctest::doctest;Boost::filesystem;Boost::system")

c_cpp_exe(jvm_runtime_manager_test
	"${jvm_runtime_manager_src};${sdk_utils_src};${sdk_runtime_src};runtime_manager_test.cpp"
	"${jvm_runtime_manager_include_dir};${sdk_utils_include_dir};${CMAKE_SOURCE_DIR}/sdk;${metaffi_sdk_root}/runtime;${doctest_INCLUDE_DIRS};${Boost_INCLUDE_DIRS};${JNI_INCLUDE_DIRS}"
	"${jvm_runtime_manager_test_libs}"
	"$ENV{METAFFI_HOME}/sdk/runtime_manager/jvm")

add_dependencies(jvm_runtime_manager_test jvm_runtime_manager_test_jar)
target_compile_definitions(jvm_runtime_manager_test PRIVATE JDK_TEST_JAR_PATH=\"${JDK_TEST_JAR_PATH}\")

add_test(NAME jvm_runtime_manager_test COMMAND $ENV{METAFFI_HOME}/sdk/runtime_manager/jvm/jvm_runtime_manager_test${CMAKE_EXECUTABLE_SUFFIX})

if(WIN32)
	get_jvm_dll_search_paths(jvm_runtime_manager_jvm_paths)

	set(jvm_runtime_manager_test_path "$ENV{METAFFI_HOME}/sdk/runtime_manager/jvm;$ENV{METAFFI_HOME}/sdk/api/cpp;$ENV{METAFFI_HOME}/jvm")
	if(jvm_runtime_manager_jvm_paths)
		list(JOIN jvm_runtime_manager_jvm_paths ";" jvm_runtime_manager_jvm_paths_joined)
		set(jvm_runtime_manager_test_path "${jvm_runtime_manager_test_path};${jvm_runtime_manager_jvm_paths_joined}")
	endif()
	set(jvm_runtime_manager_test_path "${jvm_runtime_manager_test_path};$ENV{PATH}")
	string(REPLACE ";" "\\;" jvm_runtime_manager_test_path "${jvm_runtime_manager_test_path}")

	set_tests_properties(jvm_runtime_manager_test PROPERTIES
		ENVIRONMENT "PATH=${jvm_runtime_manager_test_path}"
	)
endif()

set(jvm_runtime_manager_test jvm_runtime_manager_test PARENT_SCOPE)
