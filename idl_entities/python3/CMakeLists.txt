set(IDL_PY_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(IDL_PY_OUT_DIR "$ENV{METAFFI_HOME}/sdk/idl_entities/python3")

add_custom_target(python3_idl_entities
    COMMAND ${CMAKE_COMMAND} -E make_directory ${IDL_PY_OUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${IDL_PY_SRC_DIR}/idl_entities ${IDL_PY_OUT_DIR}/idl_entities
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

set(python3_idl_entities python3_idl_entities PARENT_SCOPE)
