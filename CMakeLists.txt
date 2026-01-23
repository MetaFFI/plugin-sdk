# add all C/CPP files to sdk_include_dir, sdk_src_files, sdk_header_files
# add them to the parent scope
collect_c_cpp_files("${CMAKE_CURRENT_LIST_DIR};${CMAKE_CURRENT_LIST_DIR}/compiler;${CMAKE_CURRENT_LIST_DIR}/runtime;${CMAKE_CURRENT_LIST_DIR}/utils" sdk)

# sdk_src and sdk_include to the parent scope
set(sdk_src ${sdk_src} PARENT_SCOPE)
set(sdk_include ${sdk_include} PARENT_SCOPE)
set(sdk_include_dir ${sdk_include_dir} PARENT_SCOPE)

set(metaffi_sdk_root ${CMAKE_CURRENT_LIST_DIR})
set(metaffi_sdk_root ${metaffi_sdk_root} PARENT_SCOPE)
add_subdirectory(cdts_serializer)
add_subdirectory(compiler)
add_subdirectory(api)
add_subdirectory(idl_entities)
add_subdirectory(idl_compiler)
add_subdirectory(runtime)
add_subdirectory(runtime_manager)
add_subdirectory(test_modules)

add_custom_target(metaffi_sdk
	DEPENDS cdts_serializer compiler api idl_entities idl_compiler sdk_runtime runtime_manager test_modules
)
set_target_properties(metaffi_sdk PROPERTIES EXCLUDE_FROM_ALL TRUE)

set(metaffi_sdk metaffi_sdk PARENT_SCOPE)
