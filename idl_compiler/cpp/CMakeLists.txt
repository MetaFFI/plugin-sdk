# C++ IDL Compiler
# Requires: libclang (LLVM), nlohmann_json, doctest

# ---------------------------------------------------------------------------
# Dependency: nlohmann_json
# ---------------------------------------------------------------------------
find_or_install_package(nlohmann_json)

if(NOT TARGET nlohmann_json::nlohmann_json AND NOT DEFINED nlohmann_json_INCLUDE_DIRS)
	message(FATAL_ERROR
		"[cpp_idl_compiler] nlohmann_json not found.\n"
		"  Linux:   sudo apt install nlohmann-json3-dev\n"
		"  Windows: winget install nlohmann.json  (or use vcpkg / conan)\n"
		"  Or set nlohmann_json_DIR to the cmake config directory."
	)
endif()

# ---------------------------------------------------------------------------
# Dependency: doctest
# ---------------------------------------------------------------------------
find_or_install_package(doctest)

if(NOT TARGET doctest::doctest)
	message(FATAL_ERROR
		"[cpp_idl_compiler] doctest not found.\n"
		"  Linux:   sudo apt install doctest-dev\n"
		"  Windows: winget install doctest  (or use vcpkg / conan)\n"
		"  Or set doctest_DIR to the cmake config directory."
	)
endif()

# ---------------------------------------------------------------------------
# Dependency: libclang (LLVM stable C API)
# ---------------------------------------------------------------------------

# Allow override via LLVM_DIR environment variable or CMake variable.
if(NOT DEFINED LLVM_INSTALL_DIR)
	if(DEFINED ENV{LLVM_DIR})
		set(LLVM_INSTALL_DIR "$ENV{LLVM_DIR}")
	elseif(WIN32)
		set(LLVM_INSTALL_DIR "C:/Program Files/LLVM")
	else()
		# Try unversioned llvm-config first, then common versioned names.
		find_program(LLVM_CONFIG NAMES llvm-config llvm-config-18 llvm-config-17 llvm-config-16)
		if(LLVM_CONFIG)
			execute_process(
				COMMAND "${LLVM_CONFIG}" --prefix
				OUTPUT_VARIABLE LLVM_INSTALL_DIR
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
		else()
			set(LLVM_INSTALL_DIR "/usr")
		endif()
	endif()
endif()

message(STATUS "[cpp_idl_compiler] Looking for libclang in: ${LLVM_INSTALL_DIR}")

# On Ubuntu/Debian, versioned packages install under /usr/lib/llvm-XX/.
# Build candidate hint lists covering both standard and versioned layouts.
set(_LIBCLANG_INCLUDE_HINTS "${LLVM_INSTALL_DIR}/include")
set(_LIBCLANG_LIB_HINTS     "${LLVM_INSTALL_DIR}/lib")
if(UNIX AND NOT APPLE)
	foreach(_ver 20 19 18 17 16 15 14)
		list(APPEND _LIBCLANG_INCLUDE_HINTS "/usr/lib/llvm-${_ver}/include")
		list(APPEND _LIBCLANG_LIB_HINTS
			"/usr/lib/llvm-${_ver}/lib"
			"/usr/lib/x86_64-linux-gnu"
			"/lib/x86_64-linux-gnu"
		)
	endforeach()
endif()

find_path(LIBCLANG_INCLUDE_DIR
	NAMES "clang-c/Index.h"
	HINTS ${_LIBCLANG_INCLUDE_HINTS}
)

find_library(LIBCLANG_LIB
	NAMES libclang clang libclang-18 libclang.so.18 libclang-18.so.18
	HINTS ${_LIBCLANG_LIB_HINTS}
)

# Check header
if(NOT LIBCLANG_INCLUDE_DIR)
	message(FATAL_ERROR
		"[cpp_idl_compiler] libclang header 'clang-c/Index.h' not found.\n"
		"  Searched hints: ${_LIBCLANG_INCLUDE_HINTS}\n\n"
		"  To fix:\n"
		"    Ubuntu/Debian: sudo apt install libclang-dev\n"
		"                   (or: sudo apt install libclang-18-dev)\n"
		"    Windows:       winget install LLVM.LLVM\n"
		"  Then re-run CMake.  To point CMake at a custom LLVM installation:\n"
		"    cmake -DLLVM_INSTALL_DIR=/path/to/llvm ...\n"
		"    or: export LLVM_DIR=/path/to/llvm && cmake ..."
	)
endif()

# Check library
if(NOT LIBCLANG_LIB)
	message(FATAL_ERROR
		"[cpp_idl_compiler] libclang library not found.\n"
		"  Searched hints: ${_LIBCLANG_LIB_HINTS}\n\n"
		"  To fix:\n"
		"    Ubuntu/Debian: sudo apt install libclang-dev\n"
		"                   (or: sudo apt install libclang-18-dev)\n"
		"    Windows:       winget install LLVM.LLVM\n"
		"  Then re-run CMake.  To point CMake at a custom LLVM installation:\n"
		"    cmake -DLLVM_INSTALL_DIR=/path/to/llvm ...\n"
		"    or: export LLVM_DIR=/path/to/llvm && cmake ..."
	)
endif()

message(STATUS "[cpp_idl_compiler] libclang include: ${LIBCLANG_INCLUDE_DIR}")
message(STATUS "[cpp_idl_compiler] libclang lib:     ${LIBCLANG_LIB}")

# ---------------------------------------------------------------------------
# IDL compiler sources
# ---------------------------------------------------------------------------
set(CPP_IDL_COMPILER_SRCS
	type_mapper.cpp
	entity_path.cpp
	extractor.cpp
	idl_generator.cpp
)

# ---------------------------------------------------------------------------
# Test executable
# ---------------------------------------------------------------------------
set(TEST_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_header.h")
string(REPLACE "\\" "/" TEST_HEADER_PATH "${TEST_HEADER_PATH}")

set(IDL_SCHEMA_PATH "${CMAKE_SOURCE_DIR}/sdk/idl_entities/idl.schema.json")
string(REPLACE "\\" "/" IDL_SCHEMA_PATH "${IDL_SCHEMA_PATH}")

add_executable(cpp_idl_compiler_test
	${CPP_IDL_COMPILER_SRCS}
	tests/idl_compiler_test.cpp
)

target_include_directories(cpp_idl_compiler_test PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${LIBCLANG_INCLUDE_DIR}
	${doctest_INCLUDE_DIRS}
)

target_link_libraries(cpp_idl_compiler_test PRIVATE
	${LIBCLANG_LIB}
	doctest::doctest
)

if(TARGET nlohmann_json::nlohmann_json)
	target_link_libraries(cpp_idl_compiler_test PRIVATE nlohmann_json::nlohmann_json)
else()
	target_include_directories(cpp_idl_compiler_test PRIVATE ${nlohmann_json_INCLUDE_DIRS})
endif()

target_compile_definitions(cpp_idl_compiler_test PRIVATE
	TEST_HEADER_PATH="${TEST_HEADER_PATH}"
	IDL_SCHEMA_PATH="${IDL_SCHEMA_PATH}"
	DOCTEST_CONFIG_NO_WINDOWS_SEH
)

set_target_properties(cpp_idl_compiler_test PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "$ENV{METAFFI_HOME}/sdk/idl_compiler/cpp"
)

# ---------------------------------------------------------------------------
# On Windows: copy libclang.dll next to the test executable
# ---------------------------------------------------------------------------
if(WIN32)
	find_file(LIBCLANG_DLL
		NAMES libclang.dll
		HINTS "${LLVM_INSTALL_DIR}/bin"
	)

	if(LIBCLANG_DLL)
		add_custom_command(TARGET cpp_idl_compiler_test POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${LIBCLANG_DLL}"
				"$<TARGET_FILE_DIR:cpp_idl_compiler_test>/libclang.dll"
			COMMENT "Copying libclang.dll next to test executable"
		)
	elseif(WIN32)
		message(WARNING
			"[cpp_idl_compiler] libclang.dll not found in ${LLVM_INSTALL_DIR}/bin.\n"
			"  The test executable may fail to run without it.\n"
			"  Ensure LLVM is installed: winget install LLVM.LLVM"
		)
	endif()
endif()

# ---------------------------------------------------------------------------
# CTest registration
# ---------------------------------------------------------------------------
add_test(NAME cpp_idl_compiler_test
	COMMAND "$ENV{METAFFI_HOME}/sdk/idl_compiler/cpp/cpp_idl_compiler_test${CMAKE_EXECUTABLE_SUFFIX}"
)

set(cpp_idl_compiler_test cpp_idl_compiler_test PARENT_SCOPE)
