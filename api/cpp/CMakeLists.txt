# C++ MetaFFI API
find_or_install_package(Boost COMPONENTS filesystem)
find_or_install_package(doctest)

set(CPP_API_SRC
	${CMAKE_CURRENT_LIST_DIR}/src/metaffi_api.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/metaffi_api_c.cpp
	${metaffi_sdk_root}/cdts_serializer/cpp/cdts_cpp_serializer.cpp
	${sdk_src}
)

set(CPP_API_INCLUDE
	${sdk_include_dir}
	${CMAKE_CURRENT_LIST_DIR}/include
	${metaffi_sdk_root}/cdts_serializer/cpp
	${Boost_INCLUDE_DIRS}
)

c_cpp_shared_lib(metaffi.api.cpp
	"${CPP_API_SRC}"
	"${CPP_API_INCLUDE}"
	"Boost::filesystem"
	"sdk/api/cpp"
)

file(GLOB_RECURSE CPP_API_HEADERS
	"${CMAKE_CURRENT_LIST_DIR}/include/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/include/*.hpp"
)

add_custom_command(TARGET metaffi.api.cpp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory $ENV{METAFFI_HOME}/sdk/api/cpp/include
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CPP_API_HEADERS} $ENV{METAFFI_HOME}/sdk/api/cpp/include
	COMMAND_EXPAND_LISTS
)

add_custom_target(api_cpp)
add_dependencies(api_cpp metaffi.api.cpp)
set(api_cpp api_cpp PARENT_SCOPE)

# --- C++ API integration test ---
c_cpp_exe(cpp_api_test
	"${CPP_API_SRC};${CMAKE_CURRENT_LIST_DIR}/tests/cpp_api_test.cpp"
	"${CPP_API_INCLUDE};${doctest_INCLUDE_DIRS}"
	"Boost::filesystem;doctest::doctest"
	"sdk/api/cpp/tests"
)
target_compile_definitions(cpp_api_test PRIVATE DOCTEST_CONFIG_NO_WINDOWS_SEH)
add_test(NAME cpp_api_test
	COMMAND "$ENV{METAFFI_HOME}/sdk/api/cpp/tests/cpp_api_test${CMAKE_EXECUTABLE_SUFFIX}")
set(cpp_api_test cpp_api_test PARENT_SCOPE)

# --- C API integration test ---
c_cpp_exe(c_api_test
	"${CPP_API_SRC};${CMAKE_CURRENT_LIST_DIR}/tests/c_api_test.cpp"
	"${CPP_API_INCLUDE};${doctest_INCLUDE_DIRS}"
	"Boost::filesystem;doctest::doctest"
	"sdk/api/cpp/tests"
)
target_compile_definitions(c_api_test PRIVATE DOCTEST_CONFIG_NO_WINDOWS_SEH)
add_test(NAME c_api_test
	COMMAND "$ENV{METAFFI_HOME}/sdk/api/cpp/tests/c_api_test${CMAKE_EXECUTABLE_SUFFIX}")
set(c_api_test c_api_test PARENT_SCOPE)
