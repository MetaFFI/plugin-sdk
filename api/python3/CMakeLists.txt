add_custom_target(api_python3)
set(api_python3 api_python3 PARENT_SCOPE)

# Add Python API test to CTest
add_py_test(sdk.api.python3.unittest.test_python3_api
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Set test dependency on xllr.python3 being built
set(_python_api_test_name "(python3 test) sdk.api.python3.unittest.test_python3_api")
if(TEST "${_python_api_test_name}")
    set_tests_properties("${_python_api_test_name}" PROPERTIES
        DEPENDS xllr.python3
    )
endif()

# * ---- Publish python metaffi package to pypi ----
add_custom_target(python3.publish
    COMMAND ${PYTHON_EXECUTABLE} ${PYTHON_EXECUTABLE_ARG} publish_python_package.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS xllr.python3
)
