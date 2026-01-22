# CPython3 Runtime Manager Library (source-only, no build needed)
# Tests are built and run here

find_or_install_package(Boost COMPONENTS filesystem)
find_or_install_package(doctest)

collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} cpython3_runtime_manager)

# Build test executable only
# Need to include SDK source files for entity_path_parser
collect_c_cpp_files("${CMAKE_CURRENT_SOURCE_DIR}/../../utils" sdk_utils)
if(WIN32)
	set(cpython3_runtime_manager_test_libs "doctest::doctest;Boost::filesystem;version")
else()
	set(cpython3_runtime_manager_test_libs "doctest::doctest;Boost::filesystem")
endif()

c_cpp_exe(cpython3_runtime_manager_test
		"${cpython3_runtime_manager_src};${sdk_utils_src};runtime_manager_test.cpp"
		"${sdk_include_dir};${doctest_INCLUDE_DIRS};${Boost_INCLUDE_DIRS}"
		"${cpython3_runtime_manager_test_libs}"
		"$ENV{METAFFI_HOME}/sdk/runtime_manager/cpython3")

add_test(NAME cpython3_runtime_manager_test COMMAND $ENV{METAFFI_HOME}/sdk/runtime_manager/cpython3/cpython3_runtime_manager_test${CMAKE_EXECUTABLE_SUFFIX})
set(cpython3_runtime_manager_test cpython3_runtime_manager_test PARENT_SCOPE)
