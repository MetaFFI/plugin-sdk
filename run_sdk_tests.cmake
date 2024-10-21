
find_or_install_package(doctest)
find_or_install_package(Boost COMPONENTS filesystem)

collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} sdk)

# compile and add CTest for ./runtime/cdts_test.cpp with sdk_src and sdk_include
c_cpp_exe(cdts_test
		"${CMAKE_CURRENT_LIST_DIR}/runtime/cdts_test.cpp;${sdk_src}"
		"${sdk_include_dir};${Boost_INCLUDE_DIRS};${doctest_INCLUDE_DIRS}"
		"doctest::doctest;Boost::filesystem"
		".")
add_test(NAME cdts_test COMMAND cdts_test)
set(cdts_test cdts_test PARENT_SCOPE)


c_cpp_exe(xllr_capi_test
		"${CMAKE_CURRENT_LIST_DIR}/runtime/xllr_capi_test.cpp;${sdk_src}"
		"${sdk_include_dir};${Boost_INCLUDE_DIRS};${doctest_INCLUDE_DIRS}"
		"doctest::doctest;Boost::filesystem"
		".")
add_test(NAME xllr_capi_test COMMAND cdts_test)
set(xllr_capi_test xllr_capi_test PARENT_SCOPE)