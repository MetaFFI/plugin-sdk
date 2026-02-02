# C++ IDL Entities Library (header-only)

# Find dependencies
find_or_install_package(nlohmann_json)
find_or_install_package(doctest)

# Set paths
set(CPP_IDL_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CPP_IDL_OUT_DIR "$ENV{METAFFI_HOME}/sdk/idl_entities/cpp")

# Create custom target to copy header-only library to output directory
add_custom_target(cpp_idl_entities
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CPP_IDL_OUT_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CPP_IDL_SRC_DIR}/include ${CPP_IDL_OUT_DIR}/include
	COMMAND ${CMAKE_COMMAND} -E copy ${CPP_IDL_SRC_DIR}/type_mapping.md ${CPP_IDL_OUT_DIR}/type_mapping.md
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	COMMENT "Copying C++ IDL entities headers to ${CPP_IDL_OUT_DIR}"
)

# Bubble up target to parent scope
set(cpp_idl_entities cpp_idl_entities PARENT_SCOPE)

# Build unit test executable using c_cpp_exe macro
c_cpp_exe(cpp_idl_entities_test
	"${CMAKE_CURRENT_LIST_DIR}/tests/idl_entities_test.cpp"
	"${CPP_IDL_SRC_DIR}/include;${CMAKE_SOURCE_DIR}/sdk;${nlohmann_json_INCLUDE_DIRS};${doctest_INCLUDE_DIRS}"
	"doctest::doctest"
	"${CPP_IDL_OUT_DIR}/tests")

# Add test
add_test(NAME cpp_idl_entities_test
	COMMAND ${CPP_IDL_OUT_DIR}/tests/cpp_idl_entities_test${CMAKE_EXECUTABLE_SUFFIX}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/sdk/test_modules/guest_modules/test)

# Bubble up test target
set(cpp_idl_entities_test cpp_idl_entities_test PARENT_SCOPE)
