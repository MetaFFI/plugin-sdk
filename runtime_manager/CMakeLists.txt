# Runtime Manager Libraries

add_subdirectory(cpython3)
add_subdirectory(go)
add_subdirectory(jvm)

add_custom_target(runtime_manager
	DEPENDS cpython3_runtime_manager_test go_runtime_manager_test jvm_runtime_manager_test
)
set(runtime_manager runtime_manager PARENT_SCOPE)
