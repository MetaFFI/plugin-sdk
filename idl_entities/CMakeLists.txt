add_subdirectory(python3)
add_subdirectory(jvm)
add_subdirectory(cpp)

set(IDL_ENTITIES_OUT_DIR "$ENV{METAFFI_HOME}/sdk/idl_entities")

add_custom_target(idl_entities_specs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${IDL_ENTITIES_OUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/idl.schema.json ${IDL_ENTITIES_OUT_DIR}/idl.schema.json
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/entity_path_specs.json ${IDL_ENTITIES_OUT_DIR}/entity_path_specs.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(idl_entities
    DEPENDS idl_entities_specs python3_idl_entities jvm_idl_entities cpp_idl_entities cpp_idl_entities_test
)

set(idl_entities idl_entities PARENT_SCOPE)
