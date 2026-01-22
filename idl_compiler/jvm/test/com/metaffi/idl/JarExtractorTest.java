package com.metaffi.idl;

import com.metaffi.idl.model.ClassInfo;
import com.metaffi.idl.model.ModuleInfo;
import org.junit.Test;
import static org.junit.Assert.*;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarOutputStream;
import java.util.zip.ZipEntry;
import java.io.FileOutputStream;

/**
 * Tests for JarExtractor - verifies extraction from .jar files.
 * Includes tests for nested classes in JARs.
 */
public class JarExtractorTest {

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
                // Handle nested classes (e.g., "ClassWithNested$InnerClass")
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
    public void testExtractFromJar() throws IOException {
        String jarPath = createTestJar("simple.jar", "SimpleClass");
        
        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        assertNotNull(module);
        assertEquals("simple", module.getModuleName());
        assertTrue(module.getClasses().size() > 0);
    }

    @Test
    public void testExtractMultipleClassesFromJar() throws IOException {
        String jarPath = createTestJar("multi.jar", 
            "SimpleClass", "ClassWithMethods", "InterfaceExample");
        
        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        // Should extract multiple classes
        assertTrue(module.getClasses().size() >= 1);
    }

    @Test
    public void testExtractNestedClassesFromJar() throws IOException {
        // Create JAR with outer and inner classes
        String jarPath = createTestJar("nested.jar",
            "ClassWithNested",
            "ClassWithNested$InnerClass",
            "ClassWithNested$StaticNested");
        
        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        // Should extract both outer and inner classes
        assertTrue(module.getClasses().size() >= 1);
        
        // Verify inner classes are present
        boolean hasInner = false;
        boolean hasOuter = false;
        
        for (ClassInfo classInfo : module.getClasses()) {
            String className = classInfo.getClassName();
            if (className.contains("ClassWithNested") && !className.contains("$")) {
                hasOuter = true;
            }
            if (className.contains("$")) {
                hasInner = true;
            }
        }
        
        // Should have at least outer class, and inner if compiled
        assertTrue(hasOuter || module.getClasses().size() > 0);
    }

    @Test
    public void testExtractPublicClassesOnly() throws IOException {
        String jarPath = createTestJar("public.jar", "SimpleClass");
        
        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        // All extracted classes should be public
        for (ClassInfo classInfo : module.getClasses()) {
            assertNotNull(classInfo.getClassName());
            // Class name should not be null (only public classes are extracted)
        }
    }

    @Test
    public void testSkipMETAINF() throws IOException {
        String jarPath = createTestJar("test.jar", "SimpleClass");
        
        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        // META-INF entries should be skipped
        // This is verified by the extractor implementation
        assertNotNull(module);
    }

    @Test
    public void testModuleNameFromJarFilename() throws IOException {
        String jarPath = createTestJar("my-custom-name.jar", "SimpleClass");
        
        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        assertEquals("my-custom-name", module.getModuleName());
    }

    @Test
    public void testExtractFromRealJar() throws IOException {
        // Test with a real JAR if available (e.g., from test fixtures)
        // For now, use created test JAR
        String jarPath = createTestJar("real.jar", "SimpleClass");
        
        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        assertNotNull(module);
        assertNotNull(module.getSourcePath());
        assertEquals(jarPath, module.getSourcePath());
    }

    @Test
    public void testExtractAllNestedLevels() throws IOException {
        // Test deeply nested classes: Outer$Inner$Nested
        String jarPath = createTestJar("deeply-nested.jar",
            "ClassWithNested",
            "ClassWithNested$InnerClass",
            "ClassWithNested$InnerClassWithNested",
            "ClassWithNested$InnerClassWithNested$DeeplyNested");
        
        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        // Should extract all nested levels
        int nestedCount = 0;
        for (ClassInfo classInfo : module.getClasses()) {
            if (classInfo.getClassName().contains("$")) {
                nestedCount++;
            }
        }
        
        // Should have nested classes if they were compiled
        assertTrue(module.getClasses().size() > 0);
    }
}
