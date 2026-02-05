package com.metaffi.idl;

import com.google.gson.JsonObject;
import org.junit.Test;
import static org.junit.Assert.*;

/**
 * Tests for EntityPathGenerator - verifies correct entity_path generation
 * matching sdk/idl_entities/entity_path_specs.json for JVM.
 * Entity paths are now JsonObjects per the IDL schema.
 */
public class EntityPathGeneratorTest {

    @Test
    public void testMethodPathInstance() {
        JsonObject path = EntityPathGenerator.createMethodPath(
            "com.example.MyClass", "myMethod", true);

        assertEquals("com.example.MyClass", path.get("class").getAsString());
        assertEquals("myMethod", path.get("callable").getAsString());
        assertEquals("true", path.get("instance_required").getAsString());
    }

    @Test
    public void testMethodPathStatic() {
        JsonObject path = EntityPathGenerator.createMethodPath(
            "com.example.MyClass", "staticMethod", false);

        assertEquals("com.example.MyClass", path.get("class").getAsString());
        assertEquals("staticMethod", path.get("callable").getAsString());
        assertNull(path.get("instance_required"));
    }

    @Test
    public void testConstructorPath() {
        JsonObject path = EntityPathGenerator.createConstructorPath("com.example.MyClass");

        assertEquals("com.example.MyClass", path.get("class").getAsString());
        assertEquals("<init>", path.get("callable").getAsString());
    }

    @Test
    public void testFieldGetterPathInstance() {
        JsonObject path = EntityPathGenerator.createFieldGetterPath(
            "com.example.MyClass", "myField", true);

        assertEquals("com.example.MyClass", path.get("class").getAsString());
        assertEquals("myField", path.get("field").getAsString());
        assertEquals("true", path.get("getter").getAsString());
        assertEquals("true", path.get("instance_required").getAsString());
    }

    @Test
    public void testFieldGetterPathStatic() {
        JsonObject path = EntityPathGenerator.createFieldGetterPath(
            "com.example.MyClass", "STATIC_FIELD", false);

        assertEquals("com.example.MyClass", path.get("class").getAsString());
        assertEquals("STATIC_FIELD", path.get("field").getAsString());
        assertEquals("true", path.get("getter").getAsString());
        assertNull(path.get("instance_required"));
    }

    @Test
    public void testFieldSetterPathInstance() {
        JsonObject path = EntityPathGenerator.createFieldSetterPath(
            "com.example.MyClass", "myField", true);

        assertEquals("com.example.MyClass", path.get("class").getAsString());
        assertEquals("myField", path.get("field").getAsString());
        assertEquals("true", path.get("setter").getAsString());
        assertEquals("true", path.get("instance_required").getAsString());
    }

    @Test
    public void testFieldSetterPathStatic() {
        JsonObject path = EntityPathGenerator.createFieldSetterPath(
            "com.example.MyClass", "STATIC_FIELD", false);

        assertEquals("com.example.MyClass", path.get("class").getAsString());
        assertEquals("STATIC_FIELD", path.get("field").getAsString());
        assertEquals("true", path.get("setter").getAsString());
        assertNull(path.get("instance_required"));
    }

    @Test
    public void testInnerClassPath() {
        // Inner classes use $ in names
        JsonObject path = EntityPathGenerator.createMethodPath(
            "com.example.Outer$Inner", "method", true);

        assertEquals("com.example.Outer$Inner", path.get("class").getAsString());
        assertEquals("method", path.get("callable").getAsString());
        assertEquals("true", path.get("instance_required").getAsString());
    }

    @Test
    public void testConstructorPathFormat() {
        // Constructor should use <init>
        JsonObject path = EntityPathGenerator.createConstructorPath("com.example.MyClass");

        assertEquals("com.example.MyClass", path.get("class").getAsString());
        assertEquals("<init>", path.get("callable").getAsString());
    }

    @Test
    public void testStaticFieldPaths() {
        // Static field getter - no instance_required
        JsonObject getterPath = EntityPathGenerator.createFieldGetterPath(
            "com.example.MyClass", "STATIC_FIELD", false);
        assertNull(getterPath.get("instance_required"));
        assertEquals("true", getterPath.get("getter").getAsString());

        // Static field setter - no instance_required
        JsonObject setterPath = EntityPathGenerator.createFieldSetterPath(
            "com.example.MyClass", "STATIC_FIELD", false);
        assertNull(setterPath.get("instance_required"));
        assertEquals("true", setterPath.get("setter").getAsString());
    }

    @Test
    public void testInstanceFieldPaths() {
        // Instance field getter - has instance_required
        JsonObject getterPath = EntityPathGenerator.createFieldGetterPath(
            "com.example.MyClass", "instanceField", true);
        assertEquals("true", getterPath.get("instance_required").getAsString());
        assertEquals("true", getterPath.get("getter").getAsString());

        // Instance field setter - has instance_required
        JsonObject setterPath = EntityPathGenerator.createFieldSetterPath(
            "com.example.MyClass", "instanceField", true);
        assertEquals("true", setterPath.get("instance_required").getAsString());
        assertEquals("true", setterPath.get("setter").getAsString());
    }

    @Test
    public void testMethodPathWithoutInstanceRequired() {
        // Static method - no instance_required flag
        JsonObject path = EntityPathGenerator.createMethodPath(
            "com.example.MyClass", "staticMethod", false);
        assertNull(path.get("instance_required"));
        assertEquals("com.example.MyClass", path.get("class").getAsString());
        assertEquals("staticMethod", path.get("callable").getAsString());
    }

    @Test
    public void testMethodPathWithInstanceRequired() {
        // Instance method - has instance_required flag
        JsonObject path = EntityPathGenerator.createMethodPath(
            "com.example.MyClass", "instanceMethod", true);
        assertEquals("true", path.get("instance_required").getAsString());
        assertEquals("com.example.MyClass", path.get("class").getAsString());
        assertEquals("instanceMethod", path.get("callable").getAsString());
    }

    @Test
    public void testEntityPathIsJsonObject() {
        // Verify that all methods return JsonObject instances
        JsonObject methodPath = EntityPathGenerator.createMethodPath("Test", "method", true);
        JsonObject ctorPath = EntityPathGenerator.createConstructorPath("Test");
        JsonObject getterPath = EntityPathGenerator.createFieldGetterPath("Test", "field", true);
        JsonObject setterPath = EntityPathGenerator.createFieldSetterPath("Test", "field", true);

        assertNotNull(methodPath);
        assertNotNull(ctorPath);
        assertNotNull(getterPath);
        assertNotNull(setterPath);

        // All should have "class" property
        assertTrue(methodPath.has("class"));
        assertTrue(ctorPath.has("class"));
        assertTrue(getterPath.has("class"));
        assertTrue(setterPath.has("class"));
    }
}
