# JVM IDL entities

build_jar(jvm_idl_entities
    SOURCES "src/**/*.java"
    OUTPUT_DIR "sdk/idl_entities/jvm"
    OUTPUT_NAME "jvm_idl_entities"
)

set(jvm_idl_entities jvm_idl_entities PARENT_SCOPE)
