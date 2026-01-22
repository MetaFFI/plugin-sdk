add_subdirectory(guest_modules)
add_subdirectory(host_modules)

add_custom_target(test_modules
	DEPENDS guest_modules host_modules
)
set(test_modules test_modules PARENT_SCOPE)
