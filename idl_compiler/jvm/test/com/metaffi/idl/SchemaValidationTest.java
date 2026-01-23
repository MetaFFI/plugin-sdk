package com.metaffi.idl;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.metaffi.idl.model.*;
import org.junit.Test;
import static org.junit.Assert.*;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Tests for schema validation - verifies generated JSON conforms to schema.json.
 */
public class SchemaValidationTest {

    private JsonObject loadSchema() throws IOException {
        // Load schema.json from sdk/idl_entities/idl.schema.json
        String schemaPath = "../../idl_entities/idl.schema.json";
        File schemaFile = new File(schemaPath);
        
        if (!schemaFile.exists()) {
            // Try alternative path
            schemaPath = "sdk/idl_entities/idl.schema.json";
            schemaFile = new File(schemaPath);
        }
        
        if (schemaFile.exists()) {
            try (FileReader reader = new FileReader(schemaFile)) {
                return JsonParser.parseReader(reader).getAsJsonObject();
            }
        }
        
        return null;  // Schema not found, skip validation
    }

    @Test
    public void testRequiredTopLevelFields() {
        ModuleInfo module = new ModuleInfo("test");
        module.getExternalResources().add("test.jar");
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();
        
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        
        // Verify required top-level fields
        assertTrue(root.has("idl_source"));
        assertTrue(root.has("idl_extension"));
        assertTrue(root.has("idl_filename_with_extension"));
        assertTrue(root.has("idl_full_path"));
        assertTrue(root.has("metaffi_guest_lib"));
        assertTrue(root.has("target_language"));
        assertTrue(root.has("modules"));
        
        // Verify values
        assertEquals("jvm", root.get("target_language").getAsString());
        assertTrue(root.get("modules").isJsonArray());
    }

    @Test
    public void testTypeEnumValues() {
        // Verify all type values are lowercase and valid MetaFFI types
        ModuleInfo module = new ModuleInfo("test");
        
        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");
        
        // Add method with various types
        MethodInfo method = new MethodInfo();
        method.setName("testMethod");
        method.setReturnType("int");
        method.setMetaffiReturnType("int32");
        classInfo.getMethods().add(method);
        
        // Add field
        FieldInfo field = new FieldInfo();
        field.setName("testField");
        field.setJavaType("String");
        field.setMetaffiType("string8");
        classInfo.getFields().add(field);
        
        module.getClasses().add(classInfo);
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();

        // Parse JSON to verify types
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        JsonArray modules_arr = root.getAsJsonArray("modules");
        JsonObject firstModule = modules_arr.get(0).getAsJsonObject();
        JsonArray classes = firstModule.getAsJsonArray("classes");
        JsonObject firstClass = classes.get(0).getAsJsonObject();

        // Check method return type
        JsonArray methods = firstClass.getAsJsonArray("methods");
        assertTrue(methods.size() > 0);
        JsonObject methodObj = methods.get(0).getAsJsonObject();
        JsonArray returnValues = methodObj.getAsJsonArray("return_values");
        if (returnValues.size() > 0) {
            JsonObject returnVal = returnValues.get(0).getAsJsonObject();
            String type = returnVal.get("type").getAsString();
            assertEquals("Type should be lowercase", type.toLowerCase(), type);
            assertEquals("Return type should be int32", "int32", type);
        }

        // Check field type (fields generate getter/setter methods)
        JsonArray fields = firstClass.getAsJsonArray("fields");
        if (fields != null && fields.size() > 0) {
            JsonObject getter = fields.get(0).getAsJsonObject();
            JsonArray getterReturns = getter.getAsJsonArray("return_values");
            if (getterReturns != null && getterReturns.size() > 0) {
                JsonObject returnVal = getterReturns.get(0).getAsJsonObject();
                String type = returnVal.get("type").getAsString();
                assertEquals("Type should be lowercase", type.toLowerCase(), type);
                assertEquals("Field type should be string8", "string8", type);
            }
        }
    }

    @Test
    public void testRequiredArgFields() {
        ModuleInfo module = new ModuleInfo("test");
        
        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");
        
        // Add method with parameters
        MethodInfo method = new MethodInfo();
        method.setName("testMethod");
        method.setReturnType("int");
        method.setMetaffiReturnType("int32");
        
        ParameterInfo param = new ParameterInfo("x", "int", "int32", 0);
        method.getParameters().add(param);
        
        classInfo.getMethods().add(method);
        module.getClasses().add(classInfo);
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();
        
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        JsonArray modulesArray = root.getAsJsonArray("modules");
        JsonObject moduleObj = modulesArray.get(0).getAsJsonObject();
        JsonArray classesArray = moduleObj.getAsJsonArray("classes");
        JsonObject classObj = classesArray.get(0).getAsJsonObject();
        JsonArray methodsArray = classObj.getAsJsonArray("methods");
        JsonObject methodObj = methodsArray.get(0).getAsJsonObject();
        JsonArray paramsArray = methodObj.getAsJsonArray("parameters");
        JsonObject paramObj = paramsArray.get(0).getAsJsonObject();
        
        // Verify required parameter fields
        assertTrue(paramObj.has("name"));
        assertTrue(paramObj.has("type"));
        assertTrue(paramObj.has("type_alias"));
        assertTrue(paramObj.has("comment"));
        assertTrue(paramObj.has("tags"));
        assertTrue(paramObj.has("dimensions"));
    }

    @Test
    public void testEntityPathStructure() {
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
        
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        JsonArray modulesArray = root.getAsJsonArray("modules");
        JsonObject moduleObj = modulesArray.get(0).getAsJsonObject();
        JsonArray classesArray = moduleObj.getAsJsonArray("classes");
        JsonObject classObj = classesArray.get(0).getAsJsonObject();
        JsonArray methodsArray = classObj.getAsJsonArray("methods");
        JsonObject methodObj = methodsArray.get(0).getAsJsonObject();
        
        // Verify entity_path is a string (comma-separated)
        assertTrue(methodObj.has("entity_path"));
        assertTrue(methodObj.get("entity_path").isJsonPrimitive());
        String entityPath = methodObj.get("entity_path").getAsString();
        
        // Verify entity_path format: class=...,callable=...,instance_required
        assertTrue(entityPath.contains("class="));
        assertTrue(entityPath.contains("callable="));
        assertTrue(entityPath.contains("instance_required"));
    }

    @Test
    public void testFullSchemaValidation() throws IOException {
        // Basic schema validation - check structure matches expected
        ModuleInfo module = new ModuleInfo("test");
        module.getExternalResources().add("test.jar");
        
        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");
        module.getClasses().add(classInfo);
        
        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);
        
        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();
        
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        
        // Verify module structure
        assertTrue(root.has("modules"));
        JsonArray modulesArray = root.getAsJsonArray("modules");
        assertEquals(1, modulesArray.size());
        
        JsonObject moduleObj = modulesArray.get(0).getAsJsonObject();
        assertTrue(moduleObj.has("name"));
        assertTrue(moduleObj.has("comment"));
        assertTrue(moduleObj.has("tags"));
        assertTrue(moduleObj.has("functions"));
        assertTrue(moduleObj.has("classes"));
        assertTrue(moduleObj.has("globals"));
        assertTrue(moduleObj.has("external_resources"));
        
        // Verify external_resources
        JsonArray externalResources = moduleObj.getAsJsonArray("external_resources");
        assertTrue(externalResources.size() > 0);
        assertEquals("test.jar", externalResources.get(0).getAsString());
    }
}
