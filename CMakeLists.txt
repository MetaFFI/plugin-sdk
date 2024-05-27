
#------------------------------- metaffi-plugin-sdk  ------------------------------------------
# This is the main CMakeLists.txt file for the metaffi-plugin-sdk project.


include(${CMAKE_CURRENT_LIST_DIR}/cmake/GlobalSettings.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/PackageManagementUtils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Utils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Go.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Python3.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/InstallUtils.cmake)

find_program(METAFFI_EXECUTABLE metaffi)
if(METAFFI_EXECUTABLE)
	include(${CMAKE_CURRENT_LIST_DIR}/cmake/MetaFFI.cmake)
endif()


# Copy "include" code to METAFFI_HOME
#add_custom_target(sdk_copy_headers_to_include)
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/metaffi_primitives.h" "$ENV{METAFFI_HOME}/include/metaffi_primitives.h")
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/cdt.h" "$ENV{METAFFI_HOME}/include/cdt.h")
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/cdt_capi_loader.h" "$ENV{METAFFI_HOME}/include/cdt_capi_loader.h")
#copy_file_from_project_binary(sdk_copy_headers_to_include "runtime/cdt_capi_loader.c.h" "$ENV{METAFFI_HOME}/include/cdt_capi_loader.c.h")

#install(FILES   ${CMAKE_CURRENT_LIST_DIR}/runtime/metaffi_primitives.h
#		${CMAKE_CURRENT_LIST_DIR}/runtime/cdt.h
#		${CMAKE_CURRENT_LIST_DIR}/runtime/xcall.h
#		${CMAKE_CURRENT_LIST_DIR}/runtime/xllr_capi_loader.h
#		${CMAKE_CURRENT_LIST_DIR}/runtime/xllr_capi_loader.c
#		${CMAKE_CURRENT_LIST_DIR}/runtime/cdts_traverse_construct.h
#		DESTINATION "include")

