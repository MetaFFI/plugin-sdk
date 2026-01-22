
find_or_install_package(doctest)
find_or_install_package(Boost COMPONENTS filesystem)

# compile and add CTest for ./cdts_test.cpp with sdk_src and sdk_include
c_cpp_exe(cdts_test
		"${CMAKE_CURRENT_LIST_DIR}/cdts_test.cpp;${sdk_src}"
		"${sdk_include_dir};${Boost_INCLUDE_DIRS};${doctest_INCLUDE_DIRS}"
		"doctest::doctest;Boost::filesystem"
		"$ENV{METAFFI_HOME}/sdk/runtime")
add_test(NAME cdts_test COMMAND $ENV{METAFFI_HOME}/sdk/runtime/cdts_test)
set(cdts_test cdts_test PARENT_SCOPE)


c_cpp_exe(xllr_capi_test
		"${CMAKE_CURRENT_LIST_DIR}/xllr_capi_test.cpp;${sdk_src}"
		"${sdk_include_dir};${Boost_INCLUDE_DIRS};${doctest_INCLUDE_DIRS}"
		"doctest::doctest;Boost::filesystem"
		"$ENV{METAFFI_HOME}/sdk/runtime")
add_test(NAME xllr_capi_test COMMAND $ENV{METAFFI_HOME}/sdk/runtime/xllr_capi_test)
set(xllr_capi_test xllr_capi_test PARENT_SCOPE)
