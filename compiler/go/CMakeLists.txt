set(GO_COMPILER_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})

# Go compiler executable (same pattern as go_idl_compiler); C++ plugin runs it via cpp_wrapper
go_build(go_compiler_exe
	SOURCE_DIR ${GO_COMPILER_SRC_DIR}
	OUTPUT_DIR sdk/compiler/go
	OUTPUT_NAME go_compiler
	OUTPUT_TYPE executable
	PACKAGE ./cmd/go_compiler
	DEPENDENT ${GO_COMPILER_SRC_DIR}
)

# Copy from SDK output to METAFFI_HOME/go when built (plugin loads from METAFFI_HOME/go)
# Source must match go_build output: METAFFI_HOME/sdk/compiler/go/go_compiler.exe
set(GO_COMPILER_EXE_SOURCE "$ENV{METAFFI_HOME}/sdk/compiler/go/go_compiler${CMAKE_EXECUTABLE_SUFFIX}")
add_custom_command(TARGET go_compiler_exe POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory $ENV{METAFFI_HOME}/go
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${GO_COMPILER_EXE_SOURCE}
		$ENV{METAFFI_HOME}/go/
	COMMENT "Copy go_compiler from SDK output to METAFFI_HOME/go"
)

add_custom_target(compiler_go_tests)

add_go_test(compiler_go_tests WORKING_DIRECTORY ${GO_COMPILER_SRC_DIR})

set(compiler_go_tests compiler_go_tests PARENT_SCOPE)
