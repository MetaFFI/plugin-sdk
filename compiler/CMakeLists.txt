add_subdirectory(go)
add_subdirectory(python3)

add_custom_target(metaffi_compiler_tests
	DEPENDS compiler_go_tests compiler_python3_tests
)
set(metaffi_compiler_tests metaffi_compiler_tests PARENT_SCOPE)

add_custom_target(compiler
	DEPENDS metaffi_compiler_tests
)
set(compiler compiler PARENT_SCOPE)
