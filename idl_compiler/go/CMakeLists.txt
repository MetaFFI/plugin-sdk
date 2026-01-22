cmake_minimum_required(VERSION 3.16)
project(go_idl_compiler)

# Find Go compiler
find_program(GO_EXECUTABLE go REQUIRED)
if(NOT GO_EXECUTABLE)
    message(FATAL_ERROR "Go compiler not found. Please install Go: https://golang.org/dl/")
endif()

message(STATUS "Found Go compiler: ${GO_EXECUTABLE}")

# Get Go version
execute_process(
    COMMAND ${GO_EXECUTABLE} version
    OUTPUT_VARIABLE GO_VERSION_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Go version: ${GO_VERSION_OUTPUT}")

# Determine output executable name
if(WIN32)
    set(GO_IDL_COMPILER_EXE "go_idl_compiler.exe")
else()
    set(GO_IDL_COMPILER_EXE "go_idl_compiler")
endif()


# Build Go executable
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${GO_IDL_COMPILER_EXE}
    COMMAND ${GO_EXECUTABLE} build
        -o ${CMAKE_CURRENT_BINARY_DIR}/${GO_IDL_COMPILER_EXE}
        ./cmd/go_idl_compiler
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/cmd/go_idl_compiler/main.go
        ${CMAKE_CURRENT_SOURCE_DIR}/compiler.go
        ${CMAKE_CURRENT_SOURCE_DIR}/extractor.go
        ${CMAKE_CURRENT_SOURCE_DIR}/functions.go
        ${CMAKE_CURRENT_SOURCE_DIR}/structs.go
        ${CMAKE_CURRENT_SOURCE_DIR}/globals.go
        ${CMAKE_CURRENT_SOURCE_DIR}/idl_generator.go
        ${CMAKE_CURRENT_SOURCE_DIR}/type_mapper.go
        ${CMAKE_CURRENT_SOURCE_DIR}/entity_path.go
        ${CMAKE_CURRENT_SOURCE_DIR}/types.go
    COMMENT "Building Go IDL Compiler executable"
    VERBATIM
)

# Create custom target for the executable
add_custom_target(go_idl_compiler_exe ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GO_IDL_COMPILER_EXE}
)

# Copy executable to $METAFFI_HOME/sdk/idl_compiler/go
add_custom_command(TARGET go_idl_compiler_exe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{METAFFI_HOME}/sdk/idl_compiler/go"
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${GO_IDL_COMPILER_EXE}
        "$ENV{METAFFI_HOME}/sdk/idl_compiler/go/${GO_IDL_COMPILER_EXE}"
    COMMENT "Copying go_idl_compiler to $ENV{METAFFI_HOME}/sdk/idl_compiler/go"
)


# Run Go tests
add_test(
    NAME go_idl_compiler_tests
    COMMAND ${GO_EXECUTABLE} test -v
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set test properties
set_tests_properties(go_idl_compiler_tests PROPERTIES
    TIMEOUT 300
    LABELS "go;idl_compiler"
)

# Note: C++ wrapper files (go_idl_compiler_wrapper.h/cpp) are NOT compiled here
# They will be used by other components that need to invoke the Go IDL compiler
message(STATUS "Go IDL Compiler configured:")
message(STATUS "  Executable: ${GO_IDL_COMPILER_EXE}")
message(STATUS "  Primary location: $ENV{METAFFI_HOME}/sdk/idl_compiler/go")
message(STATUS "  C++ wrapper available at: ${CMAKE_CURRENT_SOURCE_DIR}/cpp_wrapper/go_idl_compiler_wrapper.{h,cpp}")
