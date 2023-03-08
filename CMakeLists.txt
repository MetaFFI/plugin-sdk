
#------------------------------- metaffi-plugin-sdk Project ------------------------------------------
cmake_minimum_required(VERSION 3.16.3)
project(metaffi-plugin-sdk VERSION 0.0.1)

add_Threads_package()

# Copy "include" code to METAFFI_HOME
add_custom_target(sdk_copy_headers_to_include)
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/metaffi_primitives.h" "$ENV{METAFFI_HOME}/include/metaffi_primitives.h")
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/cdt_structs.h" "$ENV{METAFFI_HOME}/include/cdt_structs.h")
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/cdt_capi_loader.h" "$ENV{METAFFI_HOME}/include/cdt_capi_loader.h")
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/cdt_capi_loader.c.h" "$ENV{METAFFI_HOME}/include/cdt_capi_loader.c.h")

# unitests

prepend_dir_to_files(expand_env_test_src "${CMAKE_CURRENT_LIST_DIR}/utils" "expand_env.cpp expand_env_test.cpp")
add_executable(expand_env_test ${expand_env_test_src})
target_link_libraries(expand_env_test PRIVATE Catch2::Catch2)
add_test(NAME "Expand Environment Variables" COMMAND expand_env_test)

add_go_test("MetaFFI IDL Test" WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/compiler/go/IDL)

install(FILES   ${CMAKE_CURRENT_LIST_DIR}/runtime/metaffi_primitives.h
				${CMAKE_CURRENT_LIST_DIR}/runtime/cdt_structs.h
				${CMAKE_CURRENT_LIST_DIR}/runtime/cdt_capi_loader.h
				${CMAKE_CURRENT_LIST_DIR}/runtime/cdt_capi_loader.c
		DESTINATION "include")