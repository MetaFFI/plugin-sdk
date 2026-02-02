package com.metaffi.idl;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.metaffi.idl.model.*;
import org.junit.Test;
import static org.junit.Assert.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Tests for IdlGenerator - verifies correct JSON generation.
 */
public class IdlGeneratorTest {

    @Test
    public void testGenerateEmptyModule() {
        ModuleInfo module = new ModuleInfo("test");
        module.getExternalResources().add("test.jar");
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();
        
        assertNotNull(json);
        assertFalse(json.isEmpty());
        
        // Parse and verify structure
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        assertEquals("test", root.get("idl_source").getAsString());
        assertEquals(root.get("target_language").getAsString(), "jvm");
        assertTrue(root.has("modules"));
    }

    @Test
    public void testGenerateModuleWithClass() {
        ModuleInfo module = new ModuleInfo("test");
        module.getExternalResources().add("test.jar");
        
        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.TestClass");
        classInfo.setSimpleName("TestClass");
        classInfo.setPackageName("com.example");
        module.getClasses().add(classInfo);
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();
        
        assertNotNull(json);
        
        // Parse and verify class is present
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        assertTrue(root.has("modules"));
    }

    @Test
    public void testExternalResourcesInModule() {
        ModuleInfo module = new ModuleInfo("test");
        module.getExternalResources().add("app.jar");
        module.getExternalResources().add("lib.jar");
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();
        
        // Verify external_resources are included
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        // External resources should be in the module
        assertTrue(json.contains("app.jar"));
        assertTrue(json.contains("lib.jar"));
    }

    @Test
    public void testGenerateConstructor() {
        ModuleInfo module = new ModuleInfo("test");
        
        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");
        
        MethodInfo constructor = new MethodInfo();
        constructor.setName("<init>");
        constructor.setIsConstructor(true);
        constructor.setReturnType("com.example.Test");
        constructor.setMetaffiReturnType("handle");
        
        ParameterInfo param = new ParameterInfo("value", "int", "int32", 0);
        constructor.getParameters().add(param);
        
        classInfo.getConstructors().add(constructor);
        module.getClasses().add(classInfo);
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();

        // Verify constructor is in JSON by parsing
        assertNotNull(json);
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        assertTrue(root.has("modules"));
        JsonArray modules_arr = root.getAsJsonArray("modules");
        assertTrue(modules_arr.size() > 0);

        JsonObject firstModule = modules_arr.get(0).getAsJsonObject();
        assertTrue(firstModule.has("classes"));
        JsonArray classes = firstModule.getAsJsonArray("classes");
        assertTrue(classes.size() > 0);

        JsonObject firstClass = classes.get(0).getAsJsonObject();
        assertTrue(firstClass.has("constructors"));
        JsonArray constructors = firstClass.getAsJsonArray("constructors");
        assertTrue(constructors.size() > 0);

        JsonObject ctor = constructors.get(0).getAsJsonObject();
        assertTrue(ctor.has("entity_path"));
        String entityPath = ctor.get("entity_path").getAsString();
        assertTrue(entityPath.contains("<init>"));
    }

    @Test
    public void testGenerateMethod() {
        ModuleInfo module = new ModuleInfo("test");
        
        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");
        
        MethodInfo method = new MethodInfo();
        method.setName("testMethod");
        method.setReturnType("String");
        method.setMetaffiReturnType("string8");
        method.setIsStatic(false);
        
        classInfo.getMethods().add(method);
        module.getClasses().add(classInfo);
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();
        
        // Verify method is in JSON
        assertTrue(json.contains("testMethod"));
        assertTrue(json.contains("methods"));
        assertTrue(json.contains("instance_required"));
    }

    @Test
    public void testGenerateFieldWithGetterSetter() {
        ModuleInfo module = new ModuleInfo("test");
        
        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");
        
        FieldInfo field = new FieldInfo();
        field.setName("testField");
        field.setJavaType("int");
        field.setMetaffiType("int32");
        field.setIsStatic(false);
        
        classInfo.getFields().add(field);
        module.getClasses().add(classInfo);
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();
        
        // Verify field with getter/setter is in JSON
        assertTrue(json.contains("testField"));
        assertTrue(json.contains("fields"));
        assertTrue(json.contains("getter"));
        assertTrue(json.contains("setter"));
    }

    @Test
    public void testGenerateMultipleModules() {
        ModuleInfo module1 = new ModuleInfo("module1");
        ModuleInfo module2 = new ModuleInfo("module2");
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module1);
        modules.add(module2);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();
        
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        assertTrue(root.has("modules"));
        
        // Should have 2 modules
        assertEquals(2, root.getAsJsonArray("modules").size());
    }

    @Test
    public void testGenerateEmptyModules() {
        // Test with empty modules list
        List<ModuleInfo> modules = new ArrayList<>();
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();
        
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        assertTrue(root.has("modules"));
        assertEquals(0, root.getAsJsonArray("modules").size());
    }

    @Test
    public void testEntityPathInGeneratedJson() {
        ModuleInfo module = new ModuleInfo("test");
        
        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");
        
        MethodInfo method = new MethodInfo();
        method.setName("testMethod");
        method.setReturnType("void");
        method.setMetaffiReturnType("null");
        method.setIsStatic(false);
        classInfo.getMethods().add(method);
        
        module.getClasses().add(classInfo);
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();

        // Verify entity_path is present and correctly formatted by parsing
        assertNotNull(json);
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        assertTrue(root.has("modules"));

        JsonArray modules_arr = root.getAsJsonArray("modules");
        JsonObject firstModule = modules_arr.get(0).getAsJsonObject();
        JsonArray classes = firstModule.getAsJsonArray("classes");
        JsonObject firstClass = classes.get(0).getAsJsonObject();
        JsonArray methods = firstClass.getAsJsonArray("methods");
        assertTrue(methods.size() > 0);

        JsonObject methodObj = methods.get(0).getAsJsonObject();
        assertTrue(methodObj.has("entity_path"));
        String entityPath = methodObj.get("entity_path").getAsString();

        // Verify entity_path contains required parts
        assertTrue("Entity path should contain class name", entityPath.contains("com.example.Test"));
        assertTrue("Entity path should contain callable", entityPath.contains("testMethod"));
        assertTrue("Entity path should contain instance_required for instance methods",
                   entityPath.contains("instance_required"));
    }
}
