set(JVM_HOST_COMPILER_SRC_DIR ${CMAKE_CURRENT_LIST_DIR})

set(GSON_JAR ${CMAKE_SOURCE_DIR}/sdk/idl_compiler/jvm/lib/gson-2.10.1.jar)
set(JUNIT_JAR ${CMAKE_SOURCE_DIR}/sdk/idl_compiler/jvm/lib/junit-4.13.2.jar)
set(HAMCREST_JAR ${CMAKE_SOURCE_DIR}/sdk/idl_compiler/jvm/lib/hamcrest-core-1.3.jar)
set(JVM_IDL_ENTITIES_JAR $ENV{METAFFI_HOME}/sdk/idl_entities/jvm/jvm_idl_entities.jar)

if(NOT EXISTS ${GSON_JAR})
    message(FATAL_ERROR "Missing Gson dependency: ${GSON_JAR}")
endif()
if(NOT EXISTS ${JUNIT_JAR})
    message(FATAL_ERROR "Missing JUnit dependency: ${JUNIT_JAR}")
endif()
if(NOT EXISTS ${HAMCREST_JAR})
    message(FATAL_ERROR "Missing Hamcrest dependency: ${HAMCREST_JAR}")
endif()
compile_java(jvm_host_compiler_classes
    SOURCES "src/**/*.java"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/classes"
    CLASSPATH "${JVM_IDL_ENTITIES_JAR}" "${GSON_JAR}"
    DEPENDS jvm_idl_entities
    COMMENT "Compiling JVM host compiler classes"
)

set(TEST_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/test)
file(GLOB_RECURSE TEST_SOURCES "test/**/*.java")
if(NOT TEST_SOURCES)
    message(FATAL_ERROR "No test sources found in test/")
endif()

compile_java(jvm_host_compiler_test_classes
    SOURCES ${TEST_SOURCES}
    OUTPUT_DIR "${TEST_OUTPUT_DIR}"
    CLASSPATH "${JVM_IDL_ENTITIES_JAR}" "${GSON_JAR}" "${JUNIT_JAR}" "${HAMCREST_JAR}" "${CMAKE_CURRENT_BINARY_DIR}/classes"
    DEPENDS jvm_host_compiler_classes jvm_idl_entities
    COMMENT "Compiling JVM host compiler tests"
)

add_junit_test(sdk.compiler.jvm.host.tests
    TEST_CLASSES
        com.metaffi.compiler.host.ValidationTest
        com.metaffi.compiler.host.CodeGeneratorTest
        com.metaffi.compiler.host.TypeInfoGeneratorTest
    CLASSPATH "${JVM_IDL_ENTITIES_JAR}" "${GSON_JAR}" "${JUNIT_JAR}" "${HAMCREST_JAR}" "${TEST_OUTPUT_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/classes"
    OUTPUT_DIR "${TEST_OUTPUT_DIR}"
    DEPENDS jvm_host_compiler_test_classes
    COMMENT "Running JVM host compiler tests"
)

add_custom_target(compiler_jvm_tests
    DEPENDS sdk.compiler.jvm.host.tests_build
)

set(compiler_jvm_tests compiler_jvm_tests PARENT_SCOPE)
