set(JVM_HOST_COMPILER_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})

set(GSON_JAR ${CMAKE_SOURCE_DIR}/sdk/idl_compiler/jvm/lib/gson-2.10.1.jar)
set(JUNIT_JAR ${CMAKE_SOURCE_DIR}/sdk/idl_compiler/jvm/lib/junit-4.13.2.jar)
set(HAMCREST_JAR ${CMAKE_SOURCE_DIR}/sdk/idl_compiler/jvm/lib/hamcrest-core-1.3.jar)
set(JVM_IDL_ENTITIES_JAR $ENV{METAFFI_HOME}/sdk/idl_entities/jvm/jvm_idl_entities.jar)
set(JVM_HOST_COMPILER_JAR_NAME "metaffi.jvm.compiler.host")
set(JVM_HOST_COMPILER_JAR_PATH "$ENV{METAFFI_HOME}/jvm/compiler/${JVM_HOST_COMPILER_JAR_NAME}.jar")

if(NOT EXISTS ${GSON_JAR})
    message(FATAL_ERROR "Missing Gson dependency: ${GSON_JAR}")
endif()
if(NOT EXISTS ${JUNIT_JAR})
    message(FATAL_ERROR "Missing JUnit dependency: ${JUNIT_JAR}")
endif()
if(NOT EXISTS ${HAMCREST_JAR})
    message(FATAL_ERROR "Missing Hamcrest dependency: ${HAMCREST_JAR}")
endif()

if(NOT JVM_HOST_COMPILER_JAR_PATH)
    message(FATAL_ERROR "METAFFI_HOME environment variable not set. JVM host compiler jar will not be created.")
endif()

build_jar(jvm_host_compiler
    SOURCES "src/**/*.java"
    OUTPUT_DIR "jvm/compiler"
    OUTPUT_NAME "${JVM_HOST_COMPILER_JAR_NAME}"
    INCLUDE_JARS "${JVM_IDL_ENTITIES_JAR}" "${GSON_JAR}"
)

add_dependencies(jvm_host_compiler jvm_idl_entities)

set(TEST_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/test)
file(GLOB_RECURSE TEST_SOURCES "test/**/*.java")
if(NOT TEST_SOURCES)
    message(FATAL_ERROR "No test sources found in test/")
endif()

compile_java(jvm_host_compiler_test_classes
    SOURCES ${TEST_SOURCES}
    OUTPUT_DIR "${TEST_OUTPUT_DIR}"
    CLASSPATH "${JVM_IDL_ENTITIES_JAR}" "${GSON_JAR}" "${JUNIT_JAR}" "${HAMCREST_JAR}" "${JVM_HOST_COMPILER_JAR_PATH}"
    DEPENDS jvm_host_compiler jvm_idl_entities
    COMMENT "Compiling JVM host compiler tests"
)

add_junit_test(sdk.compiler.jvm.host.tests
    TEST_CLASSES
        com.metaffi.compiler.host.ValidationTest
        com.metaffi.compiler.host.CodeGeneratorTest
        com.metaffi.compiler.host.TypeInfoGeneratorTest
    CLASSPATH "${JVM_IDL_ENTITIES_JAR}" "${GSON_JAR}" "${JUNIT_JAR}" "${HAMCREST_JAR}" "${TEST_OUTPUT_DIR}" "${JVM_HOST_COMPILER_JAR_PATH}"
    OUTPUT_DIR "${TEST_OUTPUT_DIR}"
    DEPENDS jvm_host_compiler_test_classes
    COMMENT "Running JVM host compiler tests"
)

add_custom_target(compiler_jvm_tests
    DEPENDS sdk.compiler.jvm.host.tests_build
)

set(compiler_jvm_tests compiler_jvm_tests PARENT_SCOPE)
