# C++ Host Compiler Plugin

find_or_install_package(nlohmann_json)
find_or_install_package(doctest)

# Locate the MetaFFI IDL C++ headers
find_path(METAFFI_IDL_INCLUDE_DIR
    NAMES "metaffi/idl/idl_definition.hpp"
    HINTS
        "$ENV{METAFFI_HOME}/sdk/idl_entities/cpp/include"
        "${CMAKE_SOURCE_DIR}/sdk/idl_entities/cpp/include"
    NO_DEFAULT_PATH
)

if(NOT METAFFI_IDL_INCLUDE_DIR)
    message(FATAL_ERROR
        "[cpp_compiler] MetaFFI IDL C++ headers not found.\n"
        "  Expected: sdk/idl_entities/cpp/include/metaffi/idl/idl_definition.hpp\n"
        "  Set METAFFI_HOME or ensure sdk/idl_entities/cpp/ is built first.")
endif()

set(CPP_COMPILER_HOST_SRCS
    host/host_compiler.cpp
    host/code_generator.cpp
    host/type_mapper.cpp
    host/entity_path_converter.cpp
)

set(CPP_COMPILER_GUEST_SRCS
    guest/guest_compiler.cpp
    guest/xcall_generator.cpp
)

set(CPP_COMPILER_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/host
    ${CMAKE_CURRENT_SOURCE_DIR}/guest
    ${METAFFI_IDL_INCLUDE_DIR}
    ${nlohmann_json_INCLUDE_DIRS}
    ${sdk_include_dir}
)

# --- Plugin shared library (SDK-internal test/demo only; real plugin is in lang-plugin-cpp) ---
c_cpp_shared_lib(xllr.cpp.compiler
    "${CPP_COMPILER_HOST_SRCS};plugin/cpp_compiler_plugin.cpp"
    "${CPP_COMPILER_INCLUDE_DIRS};${CMAKE_SOURCE_DIR}/sdk/compiler"
    "nlohmann_json::nlohmann_json"
    "cpp/compiler"
)

add_custom_target(cpp_compiler)
add_dependencies(cpp_compiler xllr.cpp.compiler)
set(cpp_compiler cpp_compiler PARENT_SCOPE)

# --- Test executable ---
c_cpp_exe(cpp_compiler_test
    "${CPP_COMPILER_HOST_SRCS};${CPP_COMPILER_GUEST_SRCS};tests/host_compiler_test.cpp"
    "${CPP_COMPILER_INCLUDE_DIRS};${doctest_INCLUDE_DIRS}"
    "nlohmann_json::nlohmann_json;doctest::doctest"
    "sdk/compiler/cpp"
)

target_compile_definitions(cpp_compiler_test PRIVATE
    FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
    DOCTEST_CONFIG_NO_WINDOWS_SEH
)

add_test(NAME cpp_compiler_test
    COMMAND "$ENV{METAFFI_HOME}/sdk/compiler/cpp/cpp_compiler_test${CMAKE_EXECUTABLE_SUFFIX}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(cpp_compiler_test cpp_compiler_test PARENT_SCOPE)
