add_subdirectory(go)
add_subdirectory(python3)
add_subdirectory(jvm)

add_custom_target(metaffi_compiler_tests
	DEPENDS compiler_go_tests compiler_python3_tests compiler_jvm_tests
)
set(metaffi_compiler_tests metaffi_compiler_tests PARENT_SCOPE)

add_custom_target(compiler
)
set(compiler compiler PARENT_SCOPE)
