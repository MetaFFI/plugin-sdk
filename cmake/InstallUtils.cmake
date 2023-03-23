macro(install_target TARGET DESTINATION)
	cmake_parse_arguments("install_target"
			"" # bool vals
			"PATTERN" # single val
			"" # multi-vals
			${ARGN})

	if(NOT install_target_PATTERN)
		set(PATTERN "python|boost|expat|jvm")
	else()
		set(PATTERN ${install_target_PATTERN})
	endif()

	install(TARGETS ${TARGET}
			DESTINATION ${DESTINATION}
			RUNTIME_DEPENDENCY_SET DEPENDS_SET)



	list(APPEND SEARCH_DIRS $ENV{METAFFI_HOME})
	list(APPEND SEARCH_DIRS ${Boost_INCLUDE_DIR}/../lib)
	if(WIN32) # Add Path environment variable
		cmake_path(CONVERT "$ENV{Path}" TO_CMAKE_PATH_LIST CMAKE_WIN_PATH)
		list(APPEND SEARCH_DIRS ${CMAKE_WIN_PATH})
	endif()

	install(RUNTIME_DEPENDENCY_SET DEPENDS_SET
			PRE_INCLUDE_REGEXES ${PATTERN}
			PRE_EXCLUDE_REGEXES
				[[api-ms-win-.*]] [[ext-ms-.*]] [[kernel32\.dll]]
				[[bcrypt.dll]] [[mfplat.dll]] [[msvcrt.dll]] [[ole32.dll]] [[secur32.dll]] [[user32.dll]] [[vcruntime140.dll]]
				[[ws2_32.dll]] [[wpaxholder.dll]] [[vaultcli.dll]] [[urlmon.dll]] [[unenrollhook.dll]] [[ucrtbased.dll]] [[twinapi.appcore.dll]]
				[[libgcc_s_seh-1\.dll]] [[libstdc\+\+\-6.dll]] [[certca.dll]] [[credui.dll]] [[cryptnet.dll]] [[crypttpmeksvc.dll]]
			POST_EXCLUDE_REGEXES
				".*/[Ss][Yy][Ss][Tt][Ee][Mm]32/.*\\.[Dd][Ll][Ll]"
			DIRECTORIES ${SEARCH_DIRS})

endmacro()