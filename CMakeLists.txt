
if(NOT DEFINED SDK_INCLUDE_ONCE)
	set(SDK_INCLUDE_ONCE TRUE)

	cmake_minimum_required(VERSION 3.16.3)
	project(metaffi-plugin-sdk VERSION 0.0.1)
	set(Boost_NO_WARN_NEW_VERSIONS 1)

	set(CMAKE_CXX_STANDARD 17)

	find_program(GOEXEC go)
	if(NOT GOEXEC)
		message(FATAL "Go not found")
	endif()

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)


	include_directories(${CMAKE_CURRENT_LIST_DIR})

	include(CTest)
	enable_testing()

	add_executable(cdts_wrapper_test_sdk
				${CMAKE_CURRENT_LIST_DIR}/runtime/cdts_wrapper_test.cpp
				${CMAKE_CURRENT_LIST_DIR}/runtime/cdts_wrapper.cpp
				${CMAKE_CURRENT_LIST_DIR}/runtime/cdt_capi_loader.c
			)

	target_link_libraries(cdts_wrapper_test_sdk PUBLIC Threads::Threads)
	if(NOT WIN32)
		target_link_libraries(cdts_wrapper_test_sdk PUBLIC dl)
	endif()

	add_test(NAME cdts_wrapper_ctest COMMAND cdts_wrapper_test_sdk)

	add_test(NAME idl_go_test
			COMMAND ${GOEXEC} test
			WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/compiler/go/IDL)

endif()