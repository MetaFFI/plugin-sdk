package com.metaffi.idl;

import org.junit.Test;
import static org.junit.Assert.*;

/**
 * Tests for EntityPathGenerator - verifies correct entity_path generation
 * matching sdk/entity_path_specs.json for JVM.
 */
public class EntityPathGeneratorTest {

    @Test
    public void testMethodPathInstance() {
        String path = EntityPathGenerator.createMethodPath(
            "com.example.MyClass", "myMethod", true);
        
        assertEquals("class=com.example.MyClass,callable=myMethod,instance_required", path);
    }

    @Test
    public void testMethodPathStatic() {
        String path = EntityPathGenerator.createMethodPath(
            "com.example.MyClass", "staticMethod", false);
        
        assertEquals("class=com.example.MyClass,callable=staticMethod", path);
    }

    @Test
    public void testConstructorPath() {
        String path = EntityPathGenerator.createConstructorPath("com.example.MyClass");
        
        assertEquals("class=com.example.MyClass,callable=<init>", path);
    }

    @Test
    public void testFieldGetterPathInstance() {
        String path = EntityPathGenerator.createFieldGetterPath(
            "com.example.MyClass", "myField", true);
        
        assertEquals("class=com.example.MyClass,field=myField,getter,instance_required", path);
    }

    @Test
    public void testFieldGetterPathStatic() {
        String path = EntityPathGenerator.createFieldGetterPath(
            "com.example.MyClass", "STATIC_FIELD", false);
        
        assertEquals("class=com.example.MyClass,field=STATIC_FIELD,getter", path);
    }

    @Test
    public void testFieldSetterPathInstance() {
        String path = EntityPathGenerator.createFieldSetterPath(
            "com.example.MyClass", "myField", true);
        
        assertEquals("class=com.example.MyClass,field=myField,setter,instance_required", path);
    }

    @Test
    public void testFieldSetterPathStatic() {
        String path = EntityPathGenerator.createFieldSetterPath(
            "com.example.MyClass", "STATIC_FIELD", false);
        
        assertEquals("class=com.example.MyClass,field=STATIC_FIELD,setter", path);
    }

    @Test
    public void testInnerClassPath() {
        // Inner classes use $ in names
        String path = EntityPathGenerator.createMethodPath(
            "com.example.Outer$Inner", "method", true);
        
        assertEquals("class=com.example.Outer$Inner,callable=method,instance_required", path);
    }

    @Test
    public void testFlagOrdering() {
        // Verify flags come after key=value pairs
        String path = EntityPathGenerator.createFieldGetterPath(
            "com.example.MyClass", "myField", true);
        
        // Should be: class=...,field=...,getter,instance_required
        // Not: class=...,getter,field=...,instance_required
        assertTrue(path.startsWith("class=com.example.MyClass,field=myField"));
        assertTrue(path.contains("getter"));
        assertTrue(path.contains("instance_required"));
        
        // Verify getter and instance_required are flags (no = sign)
        String[] parts = path.split(",");
        for (String part : parts) {
            if (part.equals("getter") || part.equals("instance_required")) {
                assertFalse("Flag should not have = sign: " + part, part.contains("="));
            }
        }
    }

    @Test
    public void testConstructorPathFormat() {
        // Constructor should use <init>
        String path = EntityPathGenerator.createConstructorPath("com.example.MyClass");
        assertEquals("class=com.example.MyClass,callable=<init>", path);
    }

    @Test
    public void testStaticFieldPaths() {
        // Static field getter - no instance_required
        String getterPath = EntityPathGenerator.createFieldGetterPath(
            "com.example.MyClass", "STATIC_FIELD", false);
        assertFalse(getterPath.contains("instance_required"));
        assertTrue(getterPath.contains("getter"));
        
        // Static field setter - no instance_required
        String setterPath = EntityPathGenerator.createFieldSetterPath(
            "com.example.MyClass", "STATIC_FIELD", false);
        assertFalse(setterPath.contains("instance_required"));
        assertTrue(setterPath.contains("setter"));
    }

    @Test
    public void testInstanceFieldPaths() {
        // Instance field getter - has instance_required
        String getterPath = EntityPathGenerator.createFieldGetterPath(
            "com.example.MyClass", "instanceField", true);
        assertTrue(getterPath.contains("instance_required"));
        assertTrue(getterPath.contains("getter"));
        
        // Instance field setter - has instance_required
        String setterPath = EntityPathGenerator.createFieldSetterPath(
            "com.example.MyClass", "instanceField", true);
        assertTrue(setterPath.contains("instance_required"));
        assertTrue(setterPath.contains("setter"));
    }

    @Test
    public void testMethodPathWithoutInstanceRequired() {
        // Static method - no instance_required flag
        String path = EntityPathGenerator.createMethodPath(
            "com.example.MyClass", "staticMethod", false);
        assertFalse(path.contains("instance_required"));
        assertEquals("class=com.example.MyClass,callable=staticMethod", path);
    }

    @Test
    public void testMethodPathWithInstanceRequired() {
        // Instance method - has instance_required flag
        String path = EntityPathGenerator.createMethodPath(
            "com.example.MyClass", "instanceMethod", true);
        assertTrue(path.contains("instance_required"));
        assertTrue(path.startsWith("class=com.example.MyClass,callable=instanceMethod"));
    }
}
