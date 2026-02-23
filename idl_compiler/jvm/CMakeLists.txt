# JVM IDL Compiler

# Find Java
find_package(Java REQUIRED)
include(UseJava)

# External dependencies
set(ASM_VERSION "9.6")
set(GSON_VERSION "2.10.1")

# Create lib directory for dependencies
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
file(MAKE_DIRECTORY ${LIB_DIR})

# Download ASM if not present
if_exists_or_download(ASM_JAR "${LIB_DIR}/asm-${ASM_VERSION}.jar" "https://repo1.maven.org/maven2/org/ow2/asm/asm/${ASM_VERSION}/asm-${ASM_VERSION}.jar")

# Download Gson if not present
if_exists_or_download(GSON_JAR "${LIB_DIR}/gson-${GSON_VERSION}.jar" "https://repo1.maven.org/maven2/com/google/code/gson/gson/${GSON_VERSION}/gson-${GSON_VERSION}.jar")

# Build JAR using build_jar macro
build_jar(jvm_idl_compiler
    SOURCES "src/**/*.java"
    OUTPUT_DIR "sdk/idl_compiler/jvm"
    OUTPUT_NAME "jvm_idl_compiler"
    INCLUDE_JARS "${ASM_JAR};${GSON_JAR}"
)

# Make target available for dependencies
set(jvm_idl_compiler_classes jvm_idl_compiler PARENT_SCOPE)

# Test dependencies
set(JUNIT_VERSION "4.13.2")
set(HAMCREST_VERSION "1.3")

# Download JUnit if not present
if_exists_or_download(JUNIT_JAR "${LIB_DIR}/junit-${JUNIT_VERSION}.jar" "https://repo1.maven.org/maven2/junit/junit/${JUNIT_VERSION}/junit-${JUNIT_VERSION}.jar")

# Download Hamcrest if not present
if_exists_or_download(HAMCREST_JAR "${LIB_DIR}/hamcrest-core-${HAMCREST_VERSION}.jar" "https://repo1.maven.org/maven2/org/hamcrest/hamcrest-core/${HAMCREST_VERSION}/hamcrest-core-${HAMCREST_VERSION}.jar")

# Compile test data classes
# Note: javac -d creates package directories automatically
compile_java(testdata_classes
    SOURCES "test/com/metaffi/idl/testdata/*.java"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/test"
)

# Compile test classes
# Note: javac -d creates package directories automatically, so don't include package structure in output dir
set(TEST_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/test)

# Get the JAR file path for classpath (built by build_jar macro)
set(METAFFI_HOME_CMAKE "$ENV{METAFFI_HOME}")
file(TO_CMAKE_PATH "${METAFFI_HOME_CMAKE}" METAFFI_HOME_CMAKE)
set(JVM_IDL_COMPILER_JAR "${METAFFI_HOME_CMAKE}/sdk/idl_compiler/jvm/jvm_idl_compiler.jar")
set(TESTDATA_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/test)

# Get testdata sources to exclude them from test sources
file(GLOB_RECURSE TESTDATA_SOURCES "test/com/metaffi/idl/testdata/*.java")

# Compile test classes (excluding testdata)
file(GLOB_RECURSE TEST_SOURCES "test/com/metaffi/idl/*.java")
if(TESTDATA_SOURCES)
    list(REMOVE_ITEM TEST_SOURCES ${TESTDATA_SOURCES})
endif()

# Fail if no test sources found
if(NOT TEST_SOURCES)
    message(FATAL_ERROR "No test sources found in test/com/metaffi/idl/")
endif()

compile_java(test_classes
    SOURCES ${TEST_SOURCES}
    OUTPUT_DIR "${TEST_OUTPUT_DIR}"
    CLASSPATH "${JVM_IDL_COMPILER_JAR}" "${ASM_JAR}" "${GSON_JAR}" "${JUNIT_JAR}" "${HAMCREST_JAR}" "${TESTDATA_OUTPUT_DIR}"
    DEPENDS jvm_idl_compiler
    COMMENT "Compiling test classes"
)

# Add dependency on testdata if it exists
if(TARGET testdata_classes)
    add_dependencies(test_classes testdata_classes)
endif()

# Run tests
add_junit_test(sdk.idl_compiler.jvm.compiler_test
    TEST_CLASSES
        com.metaffi.idl.TypeMapperTest
        com.metaffi.idl.EntityPathGeneratorTest
        com.metaffi.idl.ClassFileExtractorTest
        com.metaffi.idl.JarExtractorTest
        com.metaffi.idl.DirectoryExtractorTest
        com.metaffi.idl.JvmExtractorTest
        com.metaffi.idl.IdlGeneratorTest
        com.metaffi.idl.SchemaValidationTest
        com.metaffi.idl.IntegrationTest
        com.metaffi.idl.ErrorHandlingTest
        com.metaffi.idl.NestedClassTest
    CLASSPATH "${JVM_IDL_COMPILER_JAR}" "${ASM_JAR}" "${GSON_JAR}" "${JUNIT_JAR}" "${HAMCREST_JAR}" "${TEST_OUTPUT_DIR}" "${TESTDATA_OUTPUT_DIR}"
    OUTPUT_DIR "${TEST_OUTPUT_DIR}"
    DEPENDS test_classes
    COMMENT "Running JUnit tests"
)

# Add test target (optional - can be run manually)
add_custom_target(jvm_idl_compiler_tests
    DEPENDS run_tests
)
