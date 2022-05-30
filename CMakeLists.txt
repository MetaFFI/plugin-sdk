cmake_minimum_required(VERSION 3.17.3)
project(metaffi-plugin-sdk VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


include_directories(${CMAKE_CURRENT_LIST_DIR})

include(CTest)
enable_testing()

add_executable(cdts_wrapper_test
			${CMAKE_CURRENT_LIST_DIR}/runtime/cdts_wrapper_test.cpp
			${CMAKE_CURRENT_LIST_DIR}/runtime/cdts_wrapper.cpp
			${CMAKE_CURRENT_LIST_DIR}/runtime/cdt_capi_loader.c
		)

target_link_libraries(cdts_wrapper_test PUBLIC Threads::Threads)
if(NOT WIN32)
	target_link_libraries(cdts_wrapper_test PUBLIC dl)
endif()

add_test(NAME cdts_wrapper_test COMMAND cdts_wrapper_test)
