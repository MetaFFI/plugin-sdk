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

if(WIN32)
	get_jvm_dll_search_paths(cdts_jvm_jvm_paths)

	set(cdts_jvm_serializer_test_path "$ENV{METAFFI_HOME}/sdk/cdts_serializer/jvm;$ENV{METAFFI_HOME}/sdk/api/cpp;$ENV{METAFFI_HOME}/jvm")
	if(cdts_jvm_jvm_paths)
		list(JOIN cdts_jvm_jvm_paths ";" cdts_jvm_jvm_paths_joined)
		set(cdts_jvm_serializer_test_path "${cdts_jvm_serializer_test_path};${cdts_jvm_jvm_paths_joined}")
	endif()
	set(cdts_jvm_serializer_test_path "${cdts_jvm_serializer_test_path};$ENV{PATH}")
	string(REPLACE ";" "\\;" cdts_jvm_serializer_test_path "${cdts_jvm_serializer_test_path}")

	set_tests_properties(cdts_jvm_serializer_test PROPERTIES
		ENVIRONMENT "PATH=${cdts_jvm_serializer_test_path}"
	)
endif()

set(cdts_jvm_serializer_test cdts_jvm_serializer_test PARENT_SCOPE)
