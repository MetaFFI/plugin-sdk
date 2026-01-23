add_subdirectory(go)
add_subdirectory(jvm)
add_subdirectory(python3)

add_custom_target(api
	DEPENDS api_go api_jvm api_python3
)
set(api api PARENT_SCOPE)
