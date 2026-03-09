
add_subdirectory(python3)
add_subdirectory(jvm)
add_subdirectory(go)
add_subdirectory(cpp)

add_custom_target(idl_compiler)
add_dependencies(idl_compiler jvm_idl_compiler go_idl_compiler_exe test_classes cpp_idl_compiler_test)
set(idl_compiler idl_compiler PARENT_SCOPE)
