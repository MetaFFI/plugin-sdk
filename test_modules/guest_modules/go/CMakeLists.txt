set(GO_GUEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(GO_GUEST_OUT_DIR ${CMAKE_CURRENT_LIST_DIR}/test_bin)

add_custom_target(go_guest_module ALL
	COMMAND ${CMAKE_COMMAND} -E make_directory ${GO_GUEST_OUT_DIR}
	WORKING_DIRECTORY ${GO_GUEST_SRC_DIR}
)

add_custom_target(go_guest_module_tests)

add_dependencies(go_guest_module_tests go_guest_module)

add_go_test(go_guest_module_tests WORKING_DIRECTORY ${GO_GUEST_SRC_DIR})
