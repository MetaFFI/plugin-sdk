# C++ MetaFFI API
find_or_install_package(Boost COMPONENTS filesystem)

set(CPP_API_SRC
	${CMAKE_CURRENT_LIST_DIR}/src/metaffi_api.cpp
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

add_custom_target(api_cpp
	DEPENDS metaffi.api.cpp
)
set(api_cpp api_cpp PARENT_SCOPE)
