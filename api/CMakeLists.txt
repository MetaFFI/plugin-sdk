add_subdirectory(go)
add_subdirectory(jvm)
add_subdirectory(python3)
add_subdirectory(cpp)

add_custom_target(api_tests)
add_dependencies(api_tests ${cpp_api_test} ${c_api_test})
set(api_tests api_tests PARENT_SCOPE)

add_custom_target(api)
add_dependencies(api api_python3 api_cpp api_jvm api_go api_tests)
set(api api PARENT_SCOPE)
