
macro(load_hunter_pm)
	# Hunter CMake package manager https://hunter.readthedocs.io/
	include("${CMAKE_CURRENT_LIST_DIR}/cmake/HunterGate.cmake")
	HunterGate(
			URL "https://github.com/cpp-pm/hunter/archive/v0.24.15.tar.gz"
			SHA1 "8010d63d5ae611c564889d5fe12d3cb7a45703ac"
			FILEPATH "${CMAKE_CURRENT_LIST_DIR}/cmake/config.cmake"
	)

	set(HUNTER_BUILD_SHARED_LIBS ON)
endmacro()

# add Threads package, prefer using pthread (if available)
macro(add_Threads_package)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
endmacro()

# add Catch2 & CTest packages
macro(add_ctest_and_catch2_packages)
	find_package(Catch2 CONFIG REQUIRED)
	include(CTest)
	enable_testing()
	include(Catch)
	get_target_property(catch2_SOURCE_DIR Catch2::Catch2 INTERFACE_INCLUDE_DIRECTORIES)
endmacro()

macro(add_boost libraries)
	set(_LIBS ${libraries} ${ARGN} )

	find_package(Boost CONFIG 1.79.0 REQUIRED COMPONENTS ${_LIBS}) # Boost library
	set(Boost_USE_STATIC_LIBS OFF) # (default is OFF)
	set(Boost_USE_DEBUG_LIBS OFF) # (default is ON)
	set(Boost_USE_RELEASE_LIBS ON) # (default is ON)
#	set(Boost_USE_MULTITHREADED ON) # (default is ON)
#	set(Boost_USE_STATIC_RUNTIME OFF) # (default is platform dependent)
#	set(Boost_USE_DEBUG_RUNTIME ON) # (default is ON)
#	set(Boost_USE_DEBUG_PYTHON OFF) # (default is OFF)

	if(WIN32)
		set(BOOST_INCLUDEDIR    $ENV{Boost_INCLUDE_DIR})
		set(BOOST_LIBRARYDIR    $ENV{Boost_INCLUDE_DIR}/lib64-msvc-14.3)
		set(BOOST_ROOT          $ENV{Boost_INCLUDE_DIR}/boost)
	endif()

endmacro()

macro(add_python3)
	find_package(Python3 REQUIRED COMPONENTS Development) # Python library
endmacro()

macro(add_jni)
	find_package(JNI REQUIRED) # JNI library
endmacro()

macro(add_java)
	find_package(Java 11 REQUIRED COMPONENTS Development)
	include(UseJava)
endmacro()
