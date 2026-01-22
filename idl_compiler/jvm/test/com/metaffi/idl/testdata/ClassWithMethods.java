package com.metaffi.idl.testdata;

/**
 * Test class with various method types for comprehensive testing.
 */
public class ClassWithMethods {
    public int instanceField;
    public static String staticField;
    
    // Default constructor
    public ClassWithMethods() {
    }
    
    // Constructor with parameters
    public ClassWithMethods(int value, String name) {
        this.instanceField = value;
    }
    
    // Instance method with no parameters
    public void instanceMethodNoParams() {
    }
    
    // Instance method with parameters
    public int instanceMethodWithParams(int a, int b) {
        return a + b;
    }
    
    // Instance method with return value
    public String getStringValue() {
        return "test";
    }
    
    // Instance method returning void
    public void voidMethod() {
    }
    
    // Static method
    public static int staticMethod(int x) {
        return x * 2;
    }
    
    // Method with array parameter
    public int sumArray(int[] values) {
        int sum = 0;
        for (int v : values) {
            sum += v;
        }
        return sum;
    }
    
    // Method with multi-dimensional array
    public int sumMatrix(int[][] matrix) {
        int sum = 0;
        for (int[] row : matrix) {
            for (int v : row) {
                sum += v;
            }
        }
        return sum;
    }
    
    // Method with object parameter
    public void processObject(Object obj) {
    }
    
    // Method returning object
    public Object createObject() {
        return new Object();
    }
    
    // Overloaded methods
    public void overloaded(int x) {
    }
    
    public void overloaded(String s) {
    }
    
    public void overloaded(int x, String s) {
    }
    
    // Private method - should be filtered
    private void privateMethod() {
    }
}
