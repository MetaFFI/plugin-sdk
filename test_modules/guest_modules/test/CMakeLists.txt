find_or_install_package(Boost COMPONENTS filesystem)

set(TEST_PLUGIN_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(TEST_PLUGIN_OUT_DIR "$ENV{METAFFI_HOME}/test")
set(TEST_PLUGIN_INCLUDE_DIR ${TEST_PLUGIN_SRC_DIR}/include)

# Collect all source files
collect_c_cpp_files("${TEST_PLUGIN_SRC_DIR}/src;${TEST_PLUGIN_INCLUDE_DIR}" test_plugin)

# Build the shared library (include sdk_src for runtime implementations)
c_cpp_shared_lib(xllr.test
	"${test_plugin_src};${sdk_src}"
	"${TEST_PLUGIN_INCLUDE_DIR};${metaffi_sdk_root}/runtime;${sdk_include_dir};${Boost_INCLUDE_DIRS}"
	"Boost::filesystem"
	"${TEST_PLUGIN_OUT_DIR}"
)

target_compile_definitions(xllr.test PRIVATE TEST_PLUGIN_EXPORTS)

# Windows: export all symbols
if(WIN32)
	set_target_properties(xllr.test PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
