set(C_GUEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(C_GUEST_OUT_DIR ${C_GUEST_SRC_DIR}/test_bin)
set(C_GUEST_INCLUDE_DIR ${C_GUEST_SRC_DIR}/include)

find_or_install_package(doctest)

collect_c_cpp_files("${C_GUEST_SRC_DIR}/src;${C_GUEST_INCLUDE_DIR}" c_guest_module)

c_cpp_shared_lib(c_guest_module
	"${c_guest_module_src}"
	"${C_GUEST_INCLUDE_DIR}"
	""
	"${C_GUEST_OUT_DIR}"
)
target_compile_definitions(c_guest_module PRIVATE C_GUEST_MODULE_EXPORTS)

file(GLOB_RECURSE C_GUEST_HEADERS
	"${C_GUEST_INCLUDE_DIR}/*.h"
	"${C_GUEST_INCLUDE_DIR}/*.hpp"
)

add_custom_command(TARGET c_guest_module POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${C_GUEST_OUT_DIR}/include
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${C_GUEST_HEADERS} ${C_GUEST_OUT_DIR}/include
	COMMAND_EXPAND_LISTS
)

c_cpp_exe(c_guest_module_tests
	"${C_GUEST_SRC_DIR}/tests/c_guest_module_test.cpp"
	"${C_GUEST_INCLUDE_DIR};${doctest_INCLUDE_DIRS}"
	"doctest::doctest;c_guest_module"
	"${CMAKE_CURRENT_BINARY_DIR}"
	SKIP_DEPS
)

add_dependencies(c_guest_module_tests c_guest_module)

add_custom_command(TARGET c_guest_module_tests POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:c_guest_module> $<TARGET_FILE_DIR:c_guest_module_tests>
)

add_test(NAME c_guest_module_tests COMMAND $<TARGET_FILE:c_guest_module_tests>)
