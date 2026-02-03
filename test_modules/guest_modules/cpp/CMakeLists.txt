set(CPP_GUEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CPP_GUEST_OUT_DIR ${CMAKE_CURRENT_LIST_DIR}/test_bin)
set(CPP_GUEST_INCLUDE_DIR ${CPP_GUEST_SRC_DIR}/include)

find_or_install_package(doctest)

collect_c_cpp_files("${CPP_GUEST_SRC_DIR}/src;${CPP_GUEST_INCLUDE_DIR}" cpp_guest_module)

c_cpp_shared_lib(cpp_guest_module
	"${cpp_guest_module_src}"
	"${CPP_GUEST_INCLUDE_DIR};${metaffi_sdk_root}"
	""
	"${CPP_GUEST_OUT_DIR}"
)
target_compile_definitions(cpp_guest_module PRIVATE CPP_GUEST_MODULE_EXPORTS)

file(GLOB_RECURSE CPP_GUEST_HEADERS
	"${CPP_GUEST_INCLUDE_DIR}/*.h"
	"${CPP_GUEST_INCLUDE_DIR}/*.hpp"
)

add_custom_command(TARGET cpp_guest_module POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CPP_GUEST_OUT_DIR}/include
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CPP_GUEST_HEADERS} ${CPP_GUEST_OUT_DIR}/include
	COMMAND_EXPAND_LISTS
)

c_cpp_exe(cpp_guest_module_tests
	"${CPP_GUEST_SRC_DIR}/tests/cpp_guest_module_test.cpp"
	"${CPP_GUEST_INCLUDE_DIR};${metaffi_sdk_root};${doctest_INCLUDE_DIRS}"
	"doctest::doctest;cpp_guest_module"
	"${CMAKE_CURRENT_BINARY_DIR}"
	SKIP_DEPS
)

add_dependencies(cpp_guest_module_tests cpp_guest_module)

add_custom_command(TARGET cpp_guest_module_tests POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cpp_guest_module> $<TARGET_FILE_DIR:cpp_guest_module_tests>
)

add_test(NAME cpp_guest_module_tests COMMAND $<TARGET_FILE:cpp_guest_module_tests>)
