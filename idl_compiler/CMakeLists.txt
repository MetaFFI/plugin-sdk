
add_subdirectory(python3)
add_subdirectory(jvm)
add_subdirectory(go)

add_custom_target(idl_compiler
	DEPENDS jvm_idl_compiler go_idl_compiler_exe
)
set(idl_compiler idl_compiler PARENT_SCOPE)
