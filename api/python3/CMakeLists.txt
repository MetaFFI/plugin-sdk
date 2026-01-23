add_custom_target(api_python3)
set(api_python3 api_python3 PARENT_SCOPE)

# * ---- Publish python metaffi package to pypi ----
add_custom_target(python3.publish
    COMMAND ${PYTHON_EXECUTABLE} ${PYTHON_EXECUTABLE_ARG} publish_python_package.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS xllr.python3
)
