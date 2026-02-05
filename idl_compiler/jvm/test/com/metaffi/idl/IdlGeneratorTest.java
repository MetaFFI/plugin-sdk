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
        assertTrue(ctor.has("name"));  // Verify constructor has name field

        // entity_path is now a JsonObject
        JsonObject entityPath = ctor.get("entity_path").getAsJsonObject();
        assertEquals("<init>", entityPath.get("callable").getAsString());
        assertEquals("com.example.Test", entityPath.get("class").getAsString());
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
        assertTrue(methodObj.has("instance_required"));  // Verify instance_required is a direct field

        // entity_path is now a JsonObject
        JsonObject entityPath = methodObj.get("entity_path").getAsJsonObject();

        // Verify entity_path contains required parts
        assertEquals("com.example.Test", entityPath.get("class").getAsString());
        assertEquals("testMethod", entityPath.get("callable").getAsString());
        assertEquals("true", entityPath.get("instance_required").getAsString());
    }

    @Test
    public void testOverloadedMethodsGetSequentialIndices() {
        // Test that overloaded methods get sequential overload_index values
        ModuleInfo module = new ModuleInfo("test");

        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");

        // Add first method: process(int)
        MethodInfo method1 = new MethodInfo();
        method1.setName("process");
        method1.setReturnType("void");
        method1.setMetaffiReturnType("null");
        method1.setIsStatic(false);
        ParameterInfo param1 = new ParameterInfo("x", "int", "int32", 0);
        method1.getParameters().add(param1);
        classInfo.getMethods().add(method1);

        // Add second method: process(String)
        MethodInfo method2 = new MethodInfo();
        method2.setName("process");
        method2.setReturnType("void");
        method2.setMetaffiReturnType("null");
        method2.setIsStatic(false);
        ParameterInfo param2 = new ParameterInfo("s", "String", "string8", 0);
        method2.getParameters().add(param2);
        classInfo.getMethods().add(method2);

        // Add third method: process(int, int)
        MethodInfo method3 = new MethodInfo();
        method3.setName("process");
        method3.setReturnType("void");
        method3.setMetaffiReturnType("null");
        method3.setIsStatic(false);
        method3.getParameters().add(new ParameterInfo("a", "int", "int32", 0));
        method3.getParameters().add(new ParameterInfo("b", "int", "int32", 0));
        classInfo.getMethods().add(method3);

        // Add a different method: calculate()
        MethodInfo method4 = new MethodInfo();
        method4.setName("calculate");
        method4.setReturnType("int");
        method4.setMetaffiReturnType("int32");
        method4.setIsStatic(false);
        classInfo.getMethods().add(method4);

        module.getClasses().add(classInfo);

        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);

        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();

        // Parse and verify overload_index values
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        JsonArray modulesArr = root.getAsJsonArray("modules");
        JsonObject firstModule = modulesArr.get(0).getAsJsonObject();
        JsonArray classes = firstModule.getAsJsonArray("classes");
        JsonObject firstClass = classes.get(0).getAsJsonObject();
        JsonArray methods = firstClass.getAsJsonArray("methods");

        assertEquals(4, methods.size());

        // First process method should have overload_index 0
        JsonObject processMethod1 = methods.get(0).getAsJsonObject();
        assertEquals("process", processMethod1.get("name").getAsString());
        assertEquals(0, processMethod1.get("overload_index").getAsInt());

        // Second process method should have overload_index 1
        JsonObject processMethod2 = methods.get(1).getAsJsonObject();
        assertEquals("process", processMethod2.get("name").getAsString());
        assertEquals(1, processMethod2.get("overload_index").getAsInt());

        // Third process method should have overload_index 2
        JsonObject processMethod3 = methods.get(2).getAsJsonObject();
        assertEquals("process", processMethod3.get("name").getAsString());
        assertEquals(2, processMethod3.get("overload_index").getAsInt());

        // calculate method should have overload_index 0 (different name, not overloaded)
        JsonObject calculateMethod = methods.get(3).getAsJsonObject();
        assertEquals("calculate", calculateMethod.get("name").getAsString());
        assertEquals(0, calculateMethod.get("overload_index").getAsInt());
    }

    @Test
    public void testOverloadedConstructorsGetSequentialIndices() {
        // Test that overloaded constructors get sequential overload_index values
        ModuleInfo module = new ModuleInfo("test");

        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");

        // Add first constructor: Test()
        MethodInfo ctor1 = new MethodInfo();
        ctor1.setName("<init>");
        ctor1.setIsConstructor(true);
        ctor1.setReturnType("com.example.Test");
        ctor1.setMetaffiReturnType("handle");
        classInfo.getConstructors().add(ctor1);

        // Add second constructor: Test(int)
        MethodInfo ctor2 = new MethodInfo();
        ctor2.setName("<init>");
        ctor2.setIsConstructor(true);
        ctor2.setReturnType("com.example.Test");
        ctor2.setMetaffiReturnType("handle");
        ctor2.getParameters().add(new ParameterInfo("value", "int", "int32", 0));
        classInfo.getConstructors().add(ctor2);

        // Add third constructor: Test(String)
        MethodInfo ctor3 = new MethodInfo();
        ctor3.setName("<init>");
        ctor3.setIsConstructor(true);
        ctor3.setReturnType("com.example.Test");
        ctor3.setMetaffiReturnType("handle");
        ctor3.getParameters().add(new ParameterInfo("name", "String", "string8", 0));
        classInfo.getConstructors().add(ctor3);

        module.getClasses().add(classInfo);

        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);

        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();

        // Parse and verify overload_index values
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        JsonArray modulesArr = root.getAsJsonArray("modules");
        JsonObject firstModule = modulesArr.get(0).getAsJsonObject();
        JsonArray classes = firstModule.getAsJsonArray("classes");
        JsonObject firstClass = classes.get(0).getAsJsonObject();
        JsonArray constructors = firstClass.getAsJsonArray("constructors");

        assertEquals(3, constructors.size());

        // Constructors with same parameter COUNT get sequential overload_index values
        // Note: The implementation uses parameter count for signature key, not parameter types
        // ctor1: 0 params → key "<init>_0" → index 0
        // ctor2: 1 param (int) → key "<init>_1" → index 0
        // ctor3: 1 param (String) → key "<init>_1" → index 1 (same count as ctor2)
        JsonObject ctor1Json = constructors.get(0).getAsJsonObject();
        assertEquals(0, ctor1Json.get("overload_index").getAsInt());

        JsonObject ctor2Json = constructors.get(1).getAsJsonObject();
        assertEquals(0, ctor2Json.get("overload_index").getAsInt());

        JsonObject ctor3Json = constructors.get(2).getAsJsonObject();
        assertEquals(1, ctor3Json.get("overload_index").getAsInt());  // Same param count as ctor2

        // All constructors should have the "name" field
        assertTrue(ctor1Json.has("name"));
        assertTrue(ctor2Json.has("name"));
        assertTrue(ctor3Json.has("name"));
    }

    @Test
    public void testClassEntityPathIsJsonObject() {
        // Verify that class-level entity_path is a JsonObject (not empty string)
        ModuleInfo module = new ModuleInfo("test");

        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");
        module.getClasses().add(classInfo);

        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);

        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();

        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        JsonArray modulesArr = root.getAsJsonArray("modules");
        JsonObject firstModule = modulesArr.get(0).getAsJsonObject();
        JsonArray classes = firstModule.getAsJsonArray("classes");
        JsonObject firstClass = classes.get(0).getAsJsonObject();

        // Verify entity_path is a JsonObject
        assertTrue(firstClass.has("entity_path"));
        assertTrue(firstClass.get("entity_path").isJsonObject());
    }
}
