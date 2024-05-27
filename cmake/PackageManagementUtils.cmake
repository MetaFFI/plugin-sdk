
# if windows, make sure msbuild.exe is available
if(WIN32)
	find_program(CMAKE_VS_MSBUILD_COMMAND msbuild)
	if(NOT CMAKE_VS_MSBUILD_COMMAND)
		message(FATAL_ERROR "msbuild.exe is not found")
	endif()
endif()

# import vcpkg
macro(add_vcpkg)
	file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" VCPKG_ROOT_CMAKE_PATH)
	set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT_CMAKE_PATH}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
endmacro()

# add Threads package, prefer using pthread (if available)
macro(add_threads_package)
	if(NOT WIN32)
		set(THREADS_PREFER_PTHREAD_FLAG ON)
		find_package(Threads REQUIRED)
	endif()
endmacro()

# add doctest & CTest packages
macro(add_ctest_and_unitest_libs)
	enable_testing()
	find_package(doctest CONFIG REQUIRED)
	include(doctest)
	include(CTest)

	# doctest include dir property:
	get_target_property(DOCTEST_INCLUDE_DIRS doctest::doctest INTERFACE_INCLUDE_DIRECTORIES)
	message(STATUS "Including doctest. Include Dir: ${DOCTEST_INCLUDE_DIRS}")
endmacro()


macro(add_boost libraries)
	set(_LIBS ${libraries} ${ARGN} )

	find_package(Boost CONFIG 1.85.0 REQUIRED COMPONENTS ${_LIBS}) # Boost library
	set(Boost_USE_STATIC_LIBS OFF) # (default is OFF)
	set(Boost_USE_DEBUG_LIBS OFF) # (default is ON)
	set(Boost_USE_RELEASE_LIBS ON) # (default is ON)

	# print out the detected boost libraries and properties from Boost::
	message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
	message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
endmacro()

macro(add_python3)
	find_package(Python3 REQUIRED COMPONENTS Development) # Python library

	# print out the detected python libraries and properties from Python3::
	message(STATUS "Python3_LIBRARIES: ${Python3_LIBRARIES}")
	message(STATUS "Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
	message(STATUS "Python3_LIBRARY_DIRS: ${Python3_LIBRARY_DIRS}")
	message(STATUS "Python3_VERSION: ${Python3_VERSION}")
	message(STATUS "Python3_Development_FOUND: ${Python3_Development_FOUND}")
	message(STATUS "Python3 executable: ${Python3_EXECUTABLE}")

	message(STATUS "Python3_LIBRARY_RELEASE: ${Python3_LIBRARY_RELEASE}")
	message(STATUS "Python3_LIBRARY_DEBUG: ${Python3_LIBRARY_DEBUG}")
endmacro()

macro(add_jni)
	find_package(JNI REQUIRED) # JNI library
endmacro()

macro(add_java)
	find_package(Java 11 REQUIRED COMPONENTS Development)
	include(UseJava)
endmacro()

