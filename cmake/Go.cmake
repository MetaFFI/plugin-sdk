include(${CMAKE_CURRENT_LIST_DIR}/Utils.cmake)
get_app_path("go" GOEXEC)

macro(add_go_target NAME)
	add_custom_target(${NAME} ALL)
endmacro()

function(go_get TARGET)
	cmake_parse_arguments("add_go"
			"" # bool vals
			"WORKING_DIRECTORY" # single val
			"" # multi-vals
			${ARGN})

	if("${add_go_test_WORKING_DIRECTORY}" STREQUAL "")
		set(add_go_test_WORKING_DIRECTORY ".")
	endif()
	add_custom_command(TARGET ${TARGET}
			WORKING_DIRECTORY ${add_go_WORKING_DIRECTORY}
			COMMAND ${GOEXEC} get -v -u -t
			COMMENT "Running \"go get\" for target ${target_name}"
			USES_TERMINAL )
endfunction()

macro(go_build TARGET)
	cmake_parse_arguments("go_build"
			"" # bool vals
			"NAME;WORKING_DIRECTORY" # single val
			"" # multi-vals
			${ARGN})

	if("${go_build_NAME}" STREQUAL "")
		set(output_file ${TARGET})
	else()
		set(output_file ${go_build_NAME})
	endif()

	if("${go_build_WORKING_DIRECTORY}" STREQUAL "")
		set(go_build_WORKING_DIRECTORY ".")
	endif()

	add_custom_command(TARGET ${TARGET}
			WORKING_DIRECTORY ${go_build_WORKING_DIRECTORY}
			COMMAND ${GOEXEC} build -buildmode=c-shared -gcflags=-shared -o ${PROJECT_BINARY_DIR}/${output_file}${CMAKE_SHARED_LIBRARY_SUFFIX}
			COMMENT "Building go C-Shared dynamic library for target ${target_name} to ${PROJECT_BINARY_DIR}/${output_file}${CMAKE_SHARED_LIBRARY_SUFFIX}"
			USES_TERMINAL )
endmacro()

# Add "go test" for the Target
macro(add_go_test NAME)
	cmake_parse_arguments("add_go_test"
			"" # bool vals
			"WORKING_DIRECTORY" # single val
			"" # multi-vals
			${ARGN})

	if("${add_go_test_WORKING_DIRECTORY}" STREQUAL "")
		set(add_go_test_WORKING_DIRECTORY .)
	endif()

	add_test(NAME "(go test) ${NAME}"
			COMMAND ${GOEXEC} test
			WORKING_DIRECTORY ${add_go_test_WORKING_DIRECTORY})
endmacro()

