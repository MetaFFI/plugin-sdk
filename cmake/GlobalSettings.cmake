macro(add_global_policies)
	cmake_policy(SET CMP0110 NEW) # Allow arbitrary names in CTest names
	cmake_policy(SET CMP0022 NEW) # Enable INTERFACE_LINK_LIBRARIES
endmacro()

macro(c_cpp_global_settings)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
	set(CMAKE_CXX_STANDARD 20)
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endmacro()

macro(c_cpp_debug_settings)
	if (CMAKE_BUILD_TYPE MATCHES Debug)
		add_definitions(-DEBUG) # add -DEBUG to "Debug" builds
		set(CMAKE_DEBUG_POSTFIX "")
	endif()
endmacro()