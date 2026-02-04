# JDK Runtime Manager Library (source-only, no build needed)
# Tests are built and run here

find_or_install_package(Boost COMPONENTS filesystem system)
find_or_install_package(doctest)

collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} jvm_runtime_manager)

collect_c_cpp_files("${metaffi_sdk_root}/utils" sdk_utils)
list(APPEND sdk_utils_src "${metaffi_sdk_root}/utils/env_utils.cpp")

build_jar(jvm_runtime_manager_test_jar
	SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/java/com/metaffi/jvm/*.java"
	OUTPUT_DIR "sdk/runtime_manager/jvm/tests"
	OUTPUT_NAME "jvm_runtime_manager_test"
)

set(JDK_TEST_JAR_PATH "$ENV{METAFFI_HOME}/sdk/runtime_manager/jvm/tests/jvm_runtime_manager_test.jar")
string(REPLACE "\\" "/" JDK_TEST_JAR_PATH "${JDK_TEST_JAR_PATH}")

set(jvm_runtime_manager_test_libs "doctest::doctest;Boost::filesystem;Boost::system")

c_cpp_exe(jvm_runtime_manager_test
	"${jvm_runtime_manager_src};${sdk_utils_src};runtime_manager_test.cpp"
	"${jvm_runtime_manager_include_dir};${sdk_utils_include_dir};${CMAKE_SOURCE_DIR}/sdk;${doctest_INCLUDE_DIRS};${Boost_INCLUDE_DIRS};${JNI_INCLUDE_DIRS}"
	"${jvm_runtime_manager_test_libs}"
	"$ENV{METAFFI_HOME}/sdk/runtime_manager/jvm")

add_dependencies(jvm_runtime_manager_test jvm_runtime_manager_test_jar)
target_compile_definitions(jvm_runtime_manager_test PRIVATE JDK_TEST_JAR_PATH=\"${JDK_TEST_JAR_PATH}\")

add_test(NAME jvm_runtime_manager_test COMMAND $ENV{METAFFI_HOME}/sdk/runtime_manager/jvm/jvm_runtime_manager_test${CMAKE_EXECUTABLE_SUFFIX})
set(jvm_runtime_manager_test jvm_runtime_manager_test PARENT_SCOPE)
