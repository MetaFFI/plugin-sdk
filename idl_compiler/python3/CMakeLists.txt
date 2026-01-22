# Add Python unit test (single comprehensive test file)
add_py_test(sdk.idl_compiler.python3.compiler_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDENCIES "jsonschema")
