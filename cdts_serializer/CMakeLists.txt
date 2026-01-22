

add_subdirectory(cpp)
add_subdirectory(c)
add_subdirectory(cpython3)
add_subdirectory(jvm)
add_subdirectory(go)

add_custom_target(cdts_serializer
	DEPENDS cdts_c_serializer_test cdts_cpp_serializer_test cdts_python3_serializer_test cdts_jvm_serializer_test
)
set(cdts_serializer cdts_serializer PARENT_SCOPE)
