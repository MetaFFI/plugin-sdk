set(PY_GUEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(PY_GUEST_OUT_DIR ${CMAKE_CURRENT_LIST_DIR}/test_bin)

file(GLOB_RECURSE PY_GUEST_MODULE_PY RELATIVE ${PY_GUEST_SRC_DIR} ${PY_GUEST_SRC_DIR}/module/*.py)
set(PY_GUEST_PY_FILES ${PY_GUEST_MODULE_PY} single_file_module.py)
set(PY_GUEST_PY_OUT_FILES "")
foreach(PY_FILE IN LISTS PY_GUEST_PY_FILES)
	list(APPEND PY_GUEST_PY_OUT_FILES ${PY_GUEST_OUT_DIR}/${PY_FILE})
endforeach()

add_custom_target(python3_guest_module ALL
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PY_GUEST_OUT_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${PY_GUEST_SRC_DIR}/module ${PY_GUEST_OUT_DIR}/module
	COMMAND ${CMAKE_COMMAND} -E copy ${PY_GUEST_SRC_DIR}/single_file_module.py ${PY_GUEST_OUT_DIR}/single_file_module.py
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

if(EXISTS "${PYTHON_EXECUTABLE}")
	add_custom_command(
		TARGET python3_guest_module
		POST_BUILD
		COMMAND ${PYTHON_EXECUTABLE} -m compileall -q -b ${PY_GUEST_OUT_DIR}
		COMMAND ${CMAKE_COMMAND} -E rm -f ${PY_GUEST_PY_OUT_FILES}
		WORKING_DIRECTORY ${PY_GUEST_OUT_DIR}
	)
endif()

add_py_test(tests.test_entities WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
