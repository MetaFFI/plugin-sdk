set(JAVA_GUEST_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(JAVA_GUEST_OUT_DIR ${CMAKE_CURRENT_LIST_DIR}/test_bin)
set(JAVA_SRC_DIR ${JAVA_GUEST_SRC_DIR}/src)
set(JAVA_TEST_SRC_DIR ${JAVA_GUEST_SRC_DIR}/tests)
set(JAVA_CLASSES_DIR ${JAVA_GUEST_OUT_DIR}/classes)
set(JAVA_TEST_CLASSES_DIR ${JAVA_GUEST_OUT_DIR}/tests)
set(JAVA_JAR_NAME guest_java)

file(GLOB_RECURSE JAVA_SOURCES ${JAVA_SRC_DIR}/*.java)
file(GLOB_RECURSE JAVA_TEST_SOURCES ${JAVA_TEST_SRC_DIR}/*.java)

compile_java(java_guest_classes
	SOURCES ${JAVA_SOURCES}
	OUTPUT_DIR ${JAVA_CLASSES_DIR}
	COMMENT "Compiling Java guest module classes"
)

build_jar(java_guest_module
	SOURCES ${JAVA_SRC_DIR}/*.java
	OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/test_bin
	OUTPUT_NAME ${JAVA_JAR_NAME}
)

add_custom_command(
	TARGET java_guest_module
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${JAVA_GUEST_OUT_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${JAR_OUTPUT_PATH} ${JAVA_GUEST_OUT_DIR}/${JAVA_JAR_NAME}.jar
)

compile_java(java_guest_tests_classes
	SOURCES ${JAVA_TEST_SOURCES}
	OUTPUT_DIR ${JAVA_TEST_CLASSES_DIR}
	CLASSPATH ${JAVA_CLASSES_DIR}
	DEPENDS java_guest_classes
	COMMENT "Compiling Java guest module tests"
)

if(WIN32)
	set(JAVA_CP_SEP ";")
else()
	set(JAVA_CP_SEP ":")
endif()

set(JAVA_TEST_CLASSPATH "${JAVA_CLASSES_DIR}${JAVA_CP_SEP}${JAVA_TEST_CLASSES_DIR}")

add_java_main_test(java_guest_module_tests
	TEST_CLASS guest.tests.TestEntities
	CLASSPATH "${JAVA_TEST_CLASSPATH}"
	OUTPUT_DIR "${JAVA_GUEST_OUT_DIR}"
	DEPENDS java_guest_tests_classes java_guest_module
	WORKING_DIRECTORY ${JAVA_GUEST_SRC_DIR}
	COMMENT "Running Java guest module tests"
)

add_custom_target(java_guest_module_tests
	DEPENDS java_guest_module_tests_build
)
