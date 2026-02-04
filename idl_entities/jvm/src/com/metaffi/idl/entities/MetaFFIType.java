package com.metaffi.idl.entities;

public enum MetaFFIType {
    FLOAT64("float64"),
    FLOAT32("float32"),
    INT8("int8"),
    INT16("int16"),
    INT32("int32"),
    INT64("int64"),
    UINT8("uint8"),
    UINT16("uint16"),
    UINT32("uint32"),
    UINT64("uint64"),
    BOOL("bool"),
    CHAR8("char8"),
    CHAR16("char16"),
    CHAR32("char32"),
    STRING8("string8"),
    STRING16("string16"),
    STRING32("string32"),
    HANDLE("handle"),
    CALLABLE("callable"),
    ARRAY("array"),
    ANY("any"),
    SIZE("size"),
    NULL("null"),
    FLOAT64_ARRAY("float64_array"),
    FLOAT32_ARRAY("float32_array"),
    INT8_ARRAY("int8_array"),
    INT16_ARRAY("int16_array"),
    INT32_ARRAY("int32_array"),
    INT64_ARRAY("int64_array"),
    UINT8_ARRAY("uint8_array"),
    UINT16_ARRAY("uint16_array"),
    UINT32_ARRAY("uint32_array"),
    UINT64_ARRAY("uint64_array"),
    BOOL_ARRAY("bool_array"),
    CHAR8_ARRAY("char8_array"),
    CHAR16_ARRAY("char16_array"),
    CHAR32_ARRAY("char32_array"),
    STRING8_ARRAY("string8_array"),
    STRING16_ARRAY("string16_array"),
    STRING32_ARRAY("string32_array"),
    HANDLE_ARRAY("handle_array"),
    CALLABLE_ARRAY("callable_array"),
    ANY_ARRAY("any_array"),
    SIZE_ARRAY("size_array");

    private final String jsonName;

    MetaFFIType(String jsonName) {
        this.jsonName = jsonName;
    }

    public String getJsonName() {
        return jsonName;
    }

    @Override
    public String toString() {
        return jsonName;
    }
}
