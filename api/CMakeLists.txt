add_subdirectory(go)
add_subdirectory(jvm)
add_subdirectory(python3)
add_subdirectory(cpp)

add_custom_target(api
	DEPENDS api_go api_jvm api_python3 api_cpp
)
set(api api PARENT_SCOPE)
