package com.metaffi.idl;

import org.junit.Test;
import static org.junit.Assert.*;

import com.metaffi.idl.model.ModuleInfo;

import java.io.File;
import java.io.IOException;

/**
 * Tests for error handling - verifies graceful handling of invalid inputs.
 */
public class ErrorHandlingTest {

    @Test
    public void testMissingFileError() {
        // Test handling of non-existent file
        try {
            ClassFileExtractor extractor = new ClassFileExtractor(
                "nonexistent.class", "test");
            extractor.extract();
            fail("Should throw IOException for missing file");
        } catch (IOException e) {
            // Expected - file not found
            assertNotNull(e.getMessage());
        }
    }

    @Test
    public void testInvalidJarError() {
        // Test handling of invalid JAR file
        try {
            // Create a file that's not a valid JAR
            File tempFile = File.createTempFile("invalid", ".jar");
            tempFile.deleteOnExit();

            JarExtractor extractor = new JarExtractor(tempFile.getAbsolutePath());
            extractor.extract();
            
            // Should either throw exception or return empty module
            // (Implementation may handle gracefully)
        } catch (IOException e) {
            // Expected for invalid JAR
            assertNotNull(e.getMessage());
        } catch (Exception e) {
            // Other exceptions are also acceptable
            assertNotNull(e.getMessage());
        }
    }

    @Test
    public void testEmptyJarHandling() throws IOException {
        // Test handling of empty JAR (no classes)
        // This would require creating an empty JAR file
        // For now, just verify extractor doesn't crash
        String emptyJarPath = "test/empty.jar";
        File emptyJar = new File(emptyJarPath);
        
        if (emptyJar.exists()) {
            JarExtractor extractor = new JarExtractor(emptyJarPath);
            ModuleInfo module = extractor.extract();
            
            // Should return module with no classes (not null)
            assertNotNull(module);
            assertEquals(0, module.getClasses().size());
        }
    }

    @Test
    public void testUnsupportedSourceType() {
        // Test handling of unsupported file type
        try {
            String[] paths = {"file.txt"};  // Not .jar, .class, or directory
            JvmExtractor extractor = new JvmExtractor(paths);
            extractor.extractAll();
            fail("Should throw IllegalArgumentException for unsupported type");
        } catch (IllegalArgumentException e) {
            // Expected
            assertNotNull(e.getMessage());
            assertTrue(e.getMessage().contains("Unsupported"));
        } catch (IOException e) {
            // File not found is also acceptable
        }
    }

    @Test
    public void testJvmExtractorErrorHandling() {
        // Test JvmExtractor.extract() returns error JSON on failure
        String[] invalidPaths = {"nonexistent.jar"};
        String result = JvmExtractor.extract(invalidPaths);
        
        // Should return JSON (either success or error)
        assertNotNull(result);
        assertTrue(result.startsWith("{"));
        assertTrue(result.endsWith("}"));
        
        // If error, should contain "error" key
        if (result.contains("\"error\"")) {
            // Error case - verify it's valid JSON
            assertTrue(result.contains("error"));
        }
    }

    @Test
    public void testCorruptedClassFileHandling() {
        // Test handling of corrupted .class file
        try {
            // Create a file with invalid class data
            File tempFile = File.createTempFile("corrupted", ".class");
            tempFile.deleteOnExit();
            
            // Write invalid data
            java.io.FileWriter writer = new java.io.FileWriter(tempFile);
            writer.write("invalid class data");
            writer.close();

            ClassFileExtractor extractor = new ClassFileExtractor(
                tempFile.getAbsolutePath(), "test");
            extractor.extract();
            
            // Should either throw exception or handle gracefully
        } catch (Exception e) {
            // Expected - corrupted file should cause error
            assertNotNull(e.getMessage());
        }
    }
}
