cmake_minimum_required(VERSION 3.16)
project(go_idl_compiler)

# Build Go IDL compiler executable (output to METAFFI_HOME/sdk/idl_compiler/go)
go_build(go_idl_compiler_exe
	SOURCE_DIR
		${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_DIR
		sdk/idl_compiler/go
	OUTPUT_NAME
		go_idl_compiler
	OUTPUT_TYPE
		executable
	DEPENDENT
		${CMAKE_CURRENT_SOURCE_DIR}
	PACKAGE
		./cmd/go_idl_compiler
)

# Copy from SDK output to METAFFI_HOME/go when built (plugin loads from METAFFI_HOME/go)
# Source must match go_build output: METAFFI_HOME/sdk/idl_compiler/go/go_idl_compiler.exe
set(GO_IDL_COMPILER_EXE_SOURCE "$ENV{METAFFI_HOME}/sdk/idl_compiler/go/go_idl_compiler${CMAKE_EXECUTABLE_SUFFIX}")
add_custom_command(TARGET go_idl_compiler_exe POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory $ENV{METAFFI_HOME}/go
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${GO_IDL_COMPILER_EXE_SOURCE}
		$ENV{METAFFI_HOME}/go/
	COMMENT "Copy go_idl_compiler from SDK output to METAFFI_HOME/go"
)

# Run Go tests
add_test(
	NAME go_idl_compiler_tests
	COMMAND ${GOEXEC} test -v
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set_tests_properties(go_idl_compiler_tests PROPERTIES
	TIMEOUT 300
	LABELS "go;idl_compiler"
)

message(STATUS "Go IDL Compiler configured:")
message(STATUS "  Executable: go_idl_compiler${CMAKE_EXECUTABLE_SUFFIX}")
message(STATUS "  Primary location: $ENV{METAFFI_HOME}/sdk/idl_compiler/go")
message(STATUS "  C++ wrapper available at: ${CMAKE_CURRENT_SOURCE_DIR}/cpp_wrapper/go_idl_compiler_wrapper.{h,cpp}")
