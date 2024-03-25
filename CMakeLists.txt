
#------------------------------- metaffi-plugin-sdk Project ------------------------------------------
cmake_minimum_required(VERSION 3.16.3)
project(metaffi-plugin-sdk VERSION 0.0.1)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/PackageManagementUtils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Utils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Go.cmake)

add_Threads_package()

# Copy "include" code to METAFFI_HOME
add_custom_target(sdk_copy_headers_to_include)
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/metaffi_primitives.h" "$ENV{METAFFI_HOME}/include/metaffi_primitives.h")
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/cdt.h" "$ENV{METAFFI_HOME}/include/cdt.h")
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/cdt_capi_loader.h" "$ENV{METAFFI_HOME}/include/cdt_capi_loader.h")
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/cdt_capi_loader.c.h" "$ENV{METAFFI_HOME}/include/cdt_capi_loader.c.h")

install(FILES   ${CMAKE_CURRENT_LIST_DIR}/runtime/metaffi_primitives.h
		${CMAKE_CURRENT_LIST_DIR}/runtime/cdt.h
		${CMAKE_CURRENT_LIST_DIR}/runtime/xllr_capi_loader.h
		${CMAKE_CURRENT_LIST_DIR}/runtime/xllr_capi_loader.c
		DESTINATION "include")

