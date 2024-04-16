include(${CMAKE_CURRENT_LIST_DIR}/Utils.cmake)

# if windows
if(WIN32)
	set(PYEXECNAME "py")
else()
	set(PYEXECNAME "python3")
endif()

get_app_path(${PYEXECNAME} PYEXEC)

macro(add_py_test NAME)
	cmake_parse_arguments("add_py_test"
			"" # bool vals
			"WORKING_DIRECTORY" # single val
			"" # multi-vals
			${ARGN})

	cmake_parse_arguments("add_py_test"
			"" # bool vals
			"" # single val
			"DEPENDENCIES" # multi-vals
			${ARGN})

	if("${add_py_test_WORKING_DIRECTORY}" STREQUAL "")
		set(add_go_test_WORKING_DIRECTORY .)
	endif()

	if(NOT "${add_py_test_DEPENDENCIES}" STREQUAL "")
		foreach(DEP ${add_py_test_DEPENDENCIES})
			execute_process(COMMAND ${PYEXEC} -m pip install ${DEP} --upgrade)
		endforeach()
	endif()

	add_test(NAME "(python3 test) ${NAME}"
			COMMAND ${PYEXEC} -m unittest ${NAME}
			WORKING_DIRECTORY ${add_py_test_WORKING_DIRECTORY})
endmacro()
