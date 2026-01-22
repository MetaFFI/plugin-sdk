# Find required packages
find_or_install_package(doctest)
find_or_install_package(Boost COMPONENTS filesystem)

# Collect source files
collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} cdts_cpp_serializer)

# Build test executable
c_cpp_exe(cdts_cpp_serializer_test
    "${cdts_cpp_serializer_src};${sdk_src};cdts_cpp_serializer_test.cpp"
    "${sdk_include_dir};${doctest_INCLUDE_DIRS};${CMAKE_CURRENT_LIST_DIR};${Boost_INCLUDE_DIRS}"
    "doctest::doctest;Boost::filesystem"
    "sdk/cdts_serializer/cpp")

# Register test with CTest
add_test(NAME cdts_cpp_serializer_test
         COMMAND $ENV{METAFFI_HOME}/sdk/cdts_serializer/cpp/cdts_cpp_serializer_test${CMAKE_EXECUTABLE_SUFFIX})

set(cdts_cpp_serializer_test cdts_cpp_serializer_test PARENT_SCOPE)
