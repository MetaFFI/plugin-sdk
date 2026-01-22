cmake_minimum_required(VERSION 3.16)

# Find required packages
find_package(Python3 COMPONENTS Development REQUIRED)
find_or_install_package(doctest)
find_or_install_package(Boost COMPONENTS filesystem)

# Collect source files
collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} cdts_python3_serializer)

# Build test executable
c_cpp_exe(cdts_python3_serializer_test
	"${cdts_python3_serializer_src};${sdk_src};cdts_python3_serializer_test.cpp"
	"${sdk_include_dir};${doctest_INCLUDE_DIRS};${CMAKE_CURRENT_LIST_DIR};${Boost_INCLUDE_DIRS};${Python3_INCLUDE_DIRS}"
	"doctest::doctest;Boost::filesystem;Python3::Python"
	"sdk/cdts_serializer/cpython3")

# Copy vcpkg runtime dependencies to $METAFFI_HOME
copy_vcpkg_runtime_dependencies(cdts_python3_serializer_test "sdk/cdts_serializer/cpython3")

# Register test with CTest
add_test(NAME cdts_python3_serializer_test
         COMMAND $ENV{METAFFI_HOME}/sdk/cdts_serializer/cpython3/cdts_python3_serializer_test${CMAKE_EXECUTABLE_SUFFIX})

set(cdts_python3_serializer_test cdts_python3_serializer_test PARENT_SCOPE)
