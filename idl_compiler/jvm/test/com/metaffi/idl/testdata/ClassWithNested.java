package com.metaffi.idl.testdata;

/**
 * Test class with nested/inner classes.
 * Tests extraction of Outer$Inner class structure.
 */
public class ClassWithNested {
    public int outerField;
    
    public ClassWithNested() {
        // Outer class constructor
    }
    
    public void outerMethod() {
        // Outer class method
    }
    
    /**
     * Public inner class - should be extracted.
     */
    public class InnerClass {
        public String innerField;
        
        public InnerClass() {
            // Inner class constructor
        }
        
        public void innerMethod() {
            // Inner class method
        }
    }
    
    /**
     * Private inner class - should be filtered out.
     */
    private class PrivateInner {
        // Should not be extracted
    }
    
    /**
     * Static nested class - should be extracted.
     */
    public static class StaticNested {
        public int nestedField;
        
        public StaticNested() {
            // Static nested constructor
        }
        
        public static void staticNestedMethod() {
            // Static nested method
        }
    }
    
    /**
     * Deeply nested class - Outer$Inner$Nested
     */
    public class InnerClassWithNested {
        public class DeeplyNested {
            public void deeplyNestedMethod() {
                // Deeply nested method
            }
        }
    }
}
