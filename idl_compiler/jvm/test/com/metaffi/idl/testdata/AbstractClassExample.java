package com.metaffi.idl.testdata;

/**
 * Test abstract class for IDL compiler testing.
 */
public abstract class AbstractClassExample {
    public int concreteField;
    
    public AbstractClassExample() {
        // Abstract class constructor
    }
    
    // Concrete method
    public void concreteMethod() {
    }
    
    // Abstract method
    public abstract void abstractMethod();
    
    // Abstract method with parameters
    public abstract int abstractMethodWithParams(int x, int y);
}
