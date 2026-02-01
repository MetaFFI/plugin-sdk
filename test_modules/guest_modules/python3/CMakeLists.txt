set(PY_GUEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})

add_custom_target(python3_guest_module ALL
	COMMAND ${CMAKE_COMMAND} -E echo "Python3 guest module uses source files from ${PY_GUEST_SRC_DIR}"
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_py_test(tests.test_entities WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
