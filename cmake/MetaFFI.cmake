include(${CMAKE_CURRENT_LIST_DIR}/Utils.cmake)
get_app_path("metaffi" METAFFI_EXEC)

macro(metaffi_compile_host TARGET IDL HOST_LANG)
	cmake_parse_arguments("metaffi_compile_host"
			"" # bool vals
			"WORKING_DIRECTORY" # single val
			"" # multi-vals
			${ARGN})

	if("${metaffi_compile_host_WORKING_DIRECTORY}" STREQUAL "")
		set(metaffi_compile_host_WORKING_DIRECTORY .)
	endif()

	add_custom_command(TARGET ${TARGET}
			WORKING_DIRECTORY ${metaffi_compile_host_WORKING_DIRECTORY}
			COMMAND ${METAFFI_EXEC} -c --idl ${IDL} -h ${HOST_LANG}
			COMMENT "Building MetaFFI \"${HOST_LANG}\" host wrapper for ${IDL}"
			USES_TERMINAL )
endmacro()

macro(metaffi_compile_guest TARGET IDL)
	cmake_parse_arguments("metaffi_compile_host"
			"" # bool vals
			"WORKING_DIRECTORY" # single val
			"" # multi-vals
			${ARGN})

	if("${metaffi_compile_host_WORKING_DIRECTORY}" STREQUAL "")
		set(metaffi_compile_host_WORKING_DIRECTORY .)
	endif()

	add_custom_command(TARGET ${TARGET}
			WORKING_DIRECTORY ${metaffi_compile_host_WORKING_DIRECTORY}
			COMMAND ${METAFFI_EXEC} -c --idl ${IDL} -g
			COMMENT "Building MetaFFI guest entrypoint for ${IDL}"
			USES_TERMINAL )
endmacro()

macro(metaffi_pack ROOT FILES)
	cmake_parse_arguments("metaffi_pack"
			"" # bool vals
			"WORKING_DIRECTORY" # single val
			"" # multi-vals
			${ARGN})

	if("${metaffi_pack_WORKING_DIRECTORY}" STREQUAL "")
		set(metaffi_pack_WORKING_DIRECTORY .)
	endif()

	execute_process(
			WORKING_DIRECTORY ${metaffi_pack_WORKING_DIRECTORY}
			COMMAND ${METAFFI_EXEC} --plugin --pack --root ${ROOT} --files ${FILES}
			COMMAND_ECHO STDOUT
			COMMAND_ERROR_IS_FATAL ANY
	)
endmacro()

macro(metaffi_confirm_plugin plugin_name)
	cmake_parse_arguments("metaffi_pack"
			"" # bool vals
			"WORKING_DIRECTORY" # single val
			"" # multi-vals
			${ARGN})

	if("${metaffi_pack_WORKING_DIRECTORY}" STREQUAL "")
		set(metaffi_pack_WORKING_DIRECTORY .)
	endif()

	execute_process(
			WORKING_DIRECTORY ${metaffi_pack_WORKING_DIRECTORY}
			COMMAND ${METAFFI_EXEC} --plugin --confirm ${plugin_name}
			COMMAND_ECHO STDOUT
			COMMAND_ERROR_IS_FATAL ANY
	)
endmacro()