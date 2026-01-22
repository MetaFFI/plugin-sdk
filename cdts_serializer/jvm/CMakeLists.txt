# Find JNI
find_package(JNI REQUIRED)

# Find required packages
find_or_install_package(doctest)
find_or_install_package(Boost COMPONENTS filesystem)

# Collect source files
collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} cdts_jvm_serializer)

# Build test executable
c_cpp_exe(cdts_jvm_serializer_test
    "${cdts_jvm_serializer_src};${sdk_src};cdts_jvm_serializer_test.cpp"
    "${sdk_include_dir};${doctest_INCLUDE_DIRS};${CMAKE_CURRENT_LIST_DIR};${JNI_INCLUDE_DIRS};${Boost_INCLUDE_DIRS}"
    "doctest::doctest;Boost::filesystem;${JNI_LIBRARIES}"
    "sdk/cdts_serializer/jvm")

# Register test with CTest
add_test(NAME cdts_jvm_serializer_test
         COMMAND $ENV{METAFFI_HOME}/sdk/cdts_serializer/jvm/cdts_jvm_serializer_test${CMAKE_EXECUTABLE_SUFFIX})

set(cdts_jvm_serializer_test cdts_jvm_serializer_test PARENT_SCOPE)
