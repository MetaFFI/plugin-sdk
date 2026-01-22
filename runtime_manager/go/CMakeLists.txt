# Go Runtime Manager Library (source-only, no build needed)
# Tests are built and run here

include(${CMAKE_SOURCE_DIR}/cmake/Go.cmake)

find_or_install_package(Boost COMPONENTS filesystem system)
find_or_install_package(doctest)

collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} go_runtime_manager)
collect_c_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/../../utils" sdk_utils)

# Build test Go shared library using go_build macro
go_build(go_runtime_manager_test_module
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/go"
	OUTPUT_DIR "sdk/runtime_manager/go/tests"
	OUTPUT_NAME "go_runtime_manager_test"
)

set(GO_TEST_MODULE_PATH "$ENV{METAFFI_HOME}/sdk/runtime_manager/go/tests/go_runtime_manager_test${CMAKE_SHARED_LIBRARY_SUFFIX}")
string(REPLACE "\\" "/" GO_TEST_MODULE_PATH "${GO_TEST_MODULE_PATH}")

set(go_runtime_manager_test_libs "doctest::doctest;Boost::filesystem;Boost::system")

c_cpp_exe(go_runtime_manager_test
	"${go_runtime_manager_src};${sdk_utils_src};runtime_manager_test.cpp"
	"${go_runtime_manager_include_dir};${sdk_utils_include_dir};${CMAKE_SOURCE_DIR}/sdk;${doctest_INCLUDE_DIRS};${Boost_INCLUDE_DIRS}"
	"${go_runtime_manager_test_libs}"
	"$ENV{METAFFI_HOME}/sdk/runtime_manager/go")

add_dependencies(go_runtime_manager_test go_runtime_manager_test_module)
target_compile_definitions(go_runtime_manager_test PRIVATE GO_TEST_MODULE_PATH="${GO_TEST_MODULE_PATH}")

add_test(NAME go_runtime_manager_test COMMAND $ENV{METAFFI_HOME}/sdk/runtime_manager/go/go_runtime_manager_test${CMAKE_EXECUTABLE_SUFFIX})
set(go_runtime_manager_test go_runtime_manager_test PARENT_SCOPE)
