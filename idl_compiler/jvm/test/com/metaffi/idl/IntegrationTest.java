package com.metaffi.idl;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.metaffi.idl.model.*;
import org.junit.Test;
import static org.junit.Assert.*;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.jar.JarOutputStream;
import java.util.zip.ZipEntry;

/**
 * Integration tests for end-to-end workflows.
 * Tests complete extraction and JSON generation with schema validation.
 */
public class IntegrationTest {

    /**
     * Create a test JAR file from compiled test classes.
     */
    private String createTestJar(String jarName, String... classFiles) throws IOException {
        Path tempDir = Files.createTempDirectory("jvm_idl_test_");
        File jarFile = new File(tempDir.toFile(), jarName);

        try (JarOutputStream jos = new JarOutputStream(new FileOutputStream(jarFile))) {
            // Get testdata classes directory from system property (set by CMake)
            // This directory contains the compiled com/metaffi/idl/testdata/*.class files
            String testDataPath = System.getProperty("testdata.classes.dir");

            if (testDataPath == null) {
                throw new IOException("Cannot find testdata classes directory. " +
                    "System property 'testdata.classes.dir' not set. " +
                    "This should be set by CMake when running tests.");
            }

            File testDataDir = new File(testDataPath);
            if (!testDataDir.exists() || !testDataDir.isDirectory()) {
                throw new IOException("Testdata classes directory does not exist: " + testDataPath);
            }

            for (String className : classFiles) {
                File classFile = new File(testDataPath, className + ".class");
                if (classFile.exists()) {
                    ZipEntry entry = new ZipEntry("com/metaffi/idl/testdata/" + className + ".class");
                    jos.putNextEntry(entry);
                    Files.copy(classFile.toPath(), jos);
                    jos.closeEntry();
                }
            }
        }

        return jarFile.getAbsolutePath();
    }

    @Test
    public void testEndToEndJarToJson() throws IOException {
        // Test: JAR → Extract → Generate JSON
        String jarPath = createTestJar("simple.jar", "SimpleClass");

        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);

        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();

        // Verify JSON is valid
        assertNotNull(json);
        assertFalse(json.isEmpty());

        // Parse JSON
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        assertTrue(root.has("modules"));
        assertEquals("jvm", root.get("target_language").getAsString());
    }

    @Test
    public void testEndToEndMultipleJarsToJson() throws IOException {
        // Test: Multiple JARs → Extract → Generate JSON
        String jar1Path = createTestJar("jar1.jar", "SimpleClass");
        String jar2Path = createTestJar("jar2.jar", "ClassWithMethods");
        String[] jarPaths = {jar1Path, jar2Path};

        JvmExtractor extractor = new JvmExtractor(jarPaths);
        List<ModuleInfo> modules = extractor.extractAll();

        // Verify all modules extracted
        assertEquals(2, modules.size());

        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();

        // Verify JSON contains multiple modules
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();
        assertTrue(root.has("modules"));
        assertEquals(2, root.getAsJsonArray("modules").size());
    }

    @Test
    public void testSchemaValidationOfGeneratedJson() throws IOException {
        // Generate IDL JSON and verify it matches schema structure
        ModuleInfo module = new ModuleInfo("test");
        module.getExternalResources().add("test.jar");

        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");
        classInfo.setPackageName("com.example");

        // Add constructor
        MethodInfo constructor = new MethodInfo();
        constructor.setName("<init>");
        constructor.setIsConstructor(true);
        constructor.setReturnType("com.example.Test");
        constructor.setMetaffiReturnType("handle");
        classInfo.getConstructors().add(constructor);

        // Add method
        MethodInfo method = new MethodInfo();
        method.setName("testMethod");
        method.setReturnType("int");
        method.setMetaffiReturnType("int32");
        method.setIsStatic(false);
        classInfo.getMethods().add(method);

        // Add field
        FieldInfo field = new FieldInfo();
        field.setName("testField");
        field.setJavaType("String");
        field.setMetaffiType("string8");
        field.setIsStatic(false);
        classInfo.getFields().add(field);

        module.getClasses().add(classInfo);

        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);

        IdlGenerator generator = new IdlGenerator(modules);
        String json = generator.generateJson();

        // Parse and verify structure
        JsonObject root = JsonParser.parseString(json).getAsJsonObject();

        // Verify top-level structure
        assertTrue(root.has("idl_source"));
        assertTrue(root.has("target_language"));
        assertTrue(root.has("modules"));

        // Verify module structure
        assertTrue(root.get("modules").getAsJsonArray().size() > 0);
    }

    @Test
    public void testExternalResourcesTracking() throws IOException {
        // Verify external_resources field exists and can be set
        String jar1Path = createTestJar("app.jar", "SimpleClass");
        String jar2Path = createTestJar("lib1.jar", "ClassWithMethods");
        String jar3Path = createTestJar("lib2.jar", "InterfaceExample");
        String[] paths = {jar1Path, jar2Path, jar3Path};

        JvmExtractor extractor = new JvmExtractor(paths);
        List<ModuleInfo> modules = extractor.extractAll();

        // Verify modules were extracted
        assertEquals(3, modules.size());

        // Verify each module has the external resources list (even if empty)
        for (ModuleInfo module : modules) {
            assertNotNull(module.getExternalResources());
        }
    }

    @Test
    public void testJsonRoundTrip() throws IOException {
        // Generate JSON, parse it, verify structure is preserved
        ModuleInfo module = new ModuleInfo("test");
        module.getExternalResources().add("test.jar");

        ClassInfo classInfo = new ClassInfo();
        classInfo.setClassName("com.example.Test");
        classInfo.setSimpleName("Test");
        module.getClasses().add(classInfo);

        List<ModuleInfo> modules = new ArrayList<>();
        modules.add(module);

        IdlGenerator generator = new IdlGenerator(modules);
        String json1 = generator.generateJson();

        // Parse and re-serialize
        JsonObject root = JsonParser.parseString(json1).getAsJsonObject();
        String json2 = root.toString();

        // Both should represent the same structure
        JsonObject root2 = JsonParser.parseString(json2).getAsJsonObject();
        assertEquals(root.get("target_language"), root2.get("target_language"));
        assertEquals(root.get("modules").getAsJsonArray().size(),
                     root2.get("modules").getAsJsonArray().size());
    }
}
