# C/C++ Runtime Manager — tests
# (The runtime_manager sources are header/source-only; no library target is needed.
#  The test executable links them directly.)

find_or_install_package(Boost COMPONENTS filesystem system)
find_or_install_package(doctest)

# Collect cpp_runtime_manager sources (excludes *_test.cpp files automatically)
collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} cpp_runtime_manager)
collect_c_cpp_files("${metaffi_sdk_root}/utils" sdk_utils)

# ---------------------------------------------------------------------------
# Test library: a plain C shared library with no C++ runtime dependency.
# This lets ABI detection tests confirm it is classified as 'c_only'.
# We use a plain add_library (not the c_cpp_shared_lib macro) because this
# library does NOT need MetaFFI headers, the logger shim, or spdlog.
# ---------------------------------------------------------------------------
set(CPP_TEST_LIB_OUTPUT_DIR "$ENV{METAFFI_HOME}/sdk/runtime_manager/cpp/tests")
set(CPP_TEST_LIB_PATH "${CPP_TEST_LIB_OUTPUT_DIR}/test_lib${CMAKE_SHARED_LIBRARY_SUFFIX}")
string(REPLACE "\\" "/" CPP_TEST_LIB_PATH "${CPP_TEST_LIB_PATH}")

add_library(cpp_runtime_manager_test_lib SHARED
	tests/test_lib.c
)

set_target_properties(cpp_runtime_manager_test_lib PROPERTIES
	EXCLUDE_FROM_ALL TRUE
	OUTPUT_NAME "test_lib"
	PREFIX       ""
)

if(WIN32)
	set_target_properties(cpp_runtime_manager_test_lib PROPERTIES
		WINDOWS_EXPORT_ALL_SYMBOLS ON
	)
elseif(UNIX)
	target_compile_options(cpp_runtime_manager_test_lib PRIVATE
		-fvisibility=default
	)
	set_target_properties(cpp_runtime_manager_test_lib PROPERTIES
		BUILD_WITH_INSTALL_RPATH TRUE
		INSTALL_RPATH_USE_LINK_PATH FALSE
		INSTALL_RPATH "$ORIGIN"
	)
endif()

# Copy test lib to METAFFI_HOME after build
add_custom_command(TARGET cpp_runtime_manager_test_lib POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CPP_TEST_LIB_OUTPUT_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:cpp_runtime_manager_test_lib>
		"${CPP_TEST_LIB_OUTPUT_DIR}/$<TARGET_FILE_NAME:cpp_runtime_manager_test_lib>"
	COMMENT "Copying test_lib to ${CPP_TEST_LIB_OUTPUT_DIR}"
)

# ---------------------------------------------------------------------------
# Test library (C++): uses std::string to force a C++ runtime dependency.
# detect_module_abi() should classify this as 'msvc' or 'itanium'.
# ---------------------------------------------------------------------------
set(CPP_TEST_LIB_CPP_OUTPUT_DIR "$ENV{METAFFI_HOME}/sdk/runtime_manager/cpp/tests")
set(CPP_TEST_LIB_CPP_PATH "${CPP_TEST_LIB_CPP_OUTPUT_DIR}/test_lib_cpp${CMAKE_SHARED_LIBRARY_SUFFIX}")
string(REPLACE "\\" "/" CPP_TEST_LIB_CPP_PATH "${CPP_TEST_LIB_CPP_PATH}")

add_library(cpp_runtime_manager_test_lib_cpp SHARED
	tests/test_lib_cpp.cpp
)

set_target_properties(cpp_runtime_manager_test_lib_cpp PROPERTIES
	EXCLUDE_FROM_ALL TRUE
	OUTPUT_NAME "test_lib_cpp"
	PREFIX       ""
)

if(WIN32)
	set_target_properties(cpp_runtime_manager_test_lib_cpp PROPERTIES
		WINDOWS_EXPORT_ALL_SYMBOLS ON
	)
elseif(UNIX)
	target_compile_options(cpp_runtime_manager_test_lib_cpp PRIVATE
		-fvisibility=default
	)
	set_target_properties(cpp_runtime_manager_test_lib_cpp PROPERTIES
		BUILD_WITH_INSTALL_RPATH TRUE
		INSTALL_RPATH_USE_LINK_PATH FALSE
		INSTALL_RPATH "$ORIGIN"
	)
endif()

# Copy C++ test lib to METAFFI_HOME after build
add_custom_command(TARGET cpp_runtime_manager_test_lib_cpp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CPP_TEST_LIB_CPP_OUTPUT_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:cpp_runtime_manager_test_lib_cpp>
		"${CPP_TEST_LIB_CPP_OUTPUT_DIR}/$<TARGET_FILE_NAME:cpp_runtime_manager_test_lib_cpp>"
	COMMENT "Copying test_lib_cpp to ${CPP_TEST_LIB_CPP_OUTPUT_DIR}"
)

# ---------------------------------------------------------------------------
# Test library (C++ class): exports Point class and math namespace for Phase 2 tests.
# ---------------------------------------------------------------------------
set(CPP_TEST_LIB_CLASS_OUTPUT_DIR "$ENV{METAFFI_HOME}/sdk/runtime_manager/cpp/tests")
set(CPP_TEST_LIB_CLASS_PATH "${CPP_TEST_LIB_CLASS_OUTPUT_DIR}/test_lib_class${CMAKE_SHARED_LIBRARY_SUFFIX}")
string(REPLACE "\\" "/" CPP_TEST_LIB_CLASS_PATH "${CPP_TEST_LIB_CLASS_PATH}")

add_library(cpp_runtime_manager_test_lib_class SHARED
	tests/test_lib_class.cpp
)

set_target_properties(cpp_runtime_manager_test_lib_class PROPERTIES
	EXCLUDE_FROM_ALL TRUE
	OUTPUT_NAME "test_lib_class"
	PREFIX       ""
)

if(WIN32)
	set_target_properties(cpp_runtime_manager_test_lib_class PROPERTIES
		WINDOWS_EXPORT_ALL_SYMBOLS ON
	)
elseif(UNIX)
	target_compile_options(cpp_runtime_manager_test_lib_class PRIVATE
		-fvisibility=default
	)
	set_target_properties(cpp_runtime_manager_test_lib_class PROPERTIES
		BUILD_WITH_INSTALL_RPATH TRUE
		INSTALL_RPATH_USE_LINK_PATH FALSE
		INSTALL_RPATH "$ORIGIN"
	)
endif()

add_custom_command(TARGET cpp_runtime_manager_test_lib_class POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CPP_TEST_LIB_CLASS_OUTPUT_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:cpp_runtime_manager_test_lib_class>
		"${CPP_TEST_LIB_CLASS_OUTPUT_DIR}/$<TARGET_FILE_NAME:cpp_runtime_manager_test_lib_class>"
	COMMENT "Copying test_lib_class to ${CPP_TEST_LIB_CLASS_OUTPUT_DIR}"
)

# ---------------------------------------------------------------------------
# Test executable
# ---------------------------------------------------------------------------
set(cpp_runtime_manager_test_libs
	"doctest::doctest;Boost::filesystem;Boost::system"
)

c_cpp_exe(cpp_runtime_manager_test
	"${cpp_runtime_manager_src};${sdk_utils_src};runtime_manager_test.cpp"
	"${cpp_runtime_manager_include_dir};${sdk_utils_include_dir};${CMAKE_SOURCE_DIR}/sdk;${doctest_INCLUDE_DIRS};${Boost_INCLUDE_DIRS}"
	"${cpp_runtime_manager_test_libs}"
	"$ENV{METAFFI_HOME}/sdk/runtime_manager/cpp"
)

add_dependencies(cpp_runtime_manager_test
	cpp_runtime_manager_test_lib
	cpp_runtime_manager_test_lib_cpp
	cpp_runtime_manager_test_lib_class
)

target_compile_definitions(cpp_runtime_manager_test PRIVATE
	CPP_TEST_LIB_PATH="${CPP_TEST_LIB_PATH}"
	CPP_TEST_LIB_CPP_PATH="${CPP_TEST_LIB_CPP_PATH}"
	CPP_TEST_LIB_CLASS_PATH="${CPP_TEST_LIB_CLASS_PATH}"
)

add_test(NAME cpp_runtime_manager_test
	COMMAND "$ENV{METAFFI_HOME}/sdk/runtime_manager/cpp/cpp_runtime_manager_test${CMAKE_EXECUTABLE_SUFFIX}"
)

set(cpp_runtime_manager_test cpp_runtime_manager_test PARENT_SCOPE)
