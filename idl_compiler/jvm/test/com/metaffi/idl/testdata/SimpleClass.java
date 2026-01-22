package com.metaffi.idl.testdata;

/**
 * Simple test class for IDL compiler testing.
 * Contains basic methods, constructors, and fields.
 */
public class SimpleClass {
    public int publicField;
    public static String staticField;
    
    public SimpleClass() {
        // Default constructor
    }
    
    public SimpleClass(int value) {
        this.publicField = value;
    }
    
    public int getValue() {
        return publicField;
    }
    
    public void setValue(int value) {
        this.publicField = value;
    }
    
    public static String getStaticValue() {
        return staticField;
    }
    
    public static void setStaticValue(String value) {
        staticField = value;
    }
}
