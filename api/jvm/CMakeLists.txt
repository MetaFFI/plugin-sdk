# load conan packages
find_or_install_package(Boost COMPONENTS filesystem)
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

set(JVM_API_TEST_DIR ${CMAKE_CURRENT_LIST_DIR}/unittest)
set(JVM_API_TEST_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/unittest_classes)
set(METAFFI_HOME_CMAKE "$ENV{METAFFI_HOME}")
file(TO_CMAKE_PATH "${METAFFI_HOME_CMAKE}" METAFFI_HOME_CMAKE)
set(JVM_API_JAR_PATH "${METAFFI_HOME_CMAKE}/sdk/api/jvm/metaffi.api.jar")
set(JVM_API_JUNIT_JAR ${CMAKE_CURRENT_LIST_DIR}/lib/junit-4.13.1.jar)
set(JVM_API_HAMCREST_JAR ${CMAKE_CURRENT_LIST_DIR}/lib/hamcrest-core-1.3.jar)

if(NOT EXISTS ${JVM_API_JUNIT_JAR})
	message(FATAL_ERROR "Missing JUnit dependency: ${JVM_API_JUNIT_JAR}")
endif()

if(NOT EXISTS ${JVM_API_HAMCREST_JAR})
	message(FATAL_ERROR "Missing Hamcrest dependency: ${JVM_API_HAMCREST_JAR}")
endif()

# get all java files in CMAKE_CURRENT_LIST_DIR (recursive for package subdirs)
file(GLOB_RECURSE JAVA_FILES ${CMAKE_CURRENT_LIST_DIR}/*.java)
list(FILTER JAVA_FILES EXCLUDE REGEX ".*/unittest/.*\\.java$|.*\\\\unittest\\\\.*\\.java$")

if(NOT JAVA_FILES)
	message(FATAL_ERROR "No JVM API source files found in ${CMAKE_CURRENT_LIST_DIR}")
endif()

# build metaffi.api.accessor native library
file(GLOB API_ACCESSOR_SRC ${CMAKE_CURRENT_LIST_DIR}/accessor/*.cpp)

# Use JVM runtime_manager sources (includes JNI wrappers and CDTS helpers)
collect_c_cpp_files("${metaffi_sdk_root}/runtime_manager/jvm" sdk_jvm_runtime)

set(API_JVM_SRC
	${sdk_jvm_runtime_src}
	${metaffi_sdk_root}/utils/env_utils.cpp
)

c_cpp_shared_lib(metaffi.api.accessor
		"${API_ACCESSOR_SRC};${API_JVM_SRC};${sdk_src}"
		"${sdk_include_dir};${metaffi_sdk_root};${CMAKE_CURRENT_LIST_DIR};${CMAKE_CURRENT_LIST_DIR}/accessor;${Boost_INCLUDE_DIRS};${JNI_INCLUDE_DIRS};"
		"Boost::filesystem"
		"sdk/api/jvm")

set(metaffi.api.accessor ${metaffi.api.accessor} PARENT_SCOPE)

# * ---- build metaffi.api.jar from *.java ----
build_jar(metaffi.api
	SOURCES
		${JAVA_FILES}
	OUTPUT_DIR
		"sdk/api/jvm"
	OUTPUT_NAME
		metaffi.api
)
set(metaffi.api ${metaffi.api} PARENT_SCOPE)
add_dependencies(metaffi.api metaffi.api.accessor)

add_custom_target(api_jvm
	DEPENDS metaffi.api metaffi.api.accessor
)
set(api_jvm ${api_jvm} PARENT_SCOPE)

# * ---- compile and run JVM API JUnit tests ----
file(GLOB JVM_API_TEST_SOURCES ${JVM_API_TEST_DIR}/*.java)
if(NOT JVM_API_TEST_SOURCES)
	message(FATAL_ERROR "No JVM API tests found in ${JVM_API_TEST_DIR}")
endif()

compile_java(jvm_api_test_classes
	SOURCES
		${JVM_API_TEST_SOURCES}
	OUTPUT_DIR
		${JVM_API_TEST_OUTPUT_DIR}
	CLASSPATH
		${JVM_API_JAR_PATH}
		${JVM_API_JUNIT_JAR}
		${JVM_API_HAMCREST_JAR}
	DEPENDS
		metaffi.api
	COMMENT
		"Compiling JVM API JUnit tests"
)

add_junit_test(jvm_api_test
	TEST_CLASSES
		TestJVMAPI
	CLASSPATH
		${JVM_API_JAR_PATH}
		${JVM_API_JUNIT_JAR}
		${JVM_API_HAMCREST_JAR}
		${JVM_API_TEST_OUTPUT_DIR}
	OUTPUT_DIR
		${JVM_API_TEST_OUTPUT_DIR}
	DEPENDS
		jvm_api_test_classes
		api_jvm
		xllr.test
	COMMENT
		"Running JVM API JUnit tests"
)

# Keep legacy build target name for compatibility with existing scripts.
add_custom_target(jvm_api_test
	DEPENDS jvm_api_test_build
)
