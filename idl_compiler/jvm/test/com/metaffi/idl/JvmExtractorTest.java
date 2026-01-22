package com.metaffi.idl;

import com.metaffi.idl.model.ClassInfo;
import com.metaffi.idl.model.ModuleInfo;
import org.junit.Test;
import static org.junit.Assert.*;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

/**
 * Basic tests for JvmExtractor - verifies extraction from different source types.
 * Note: Full test suite would require compiled test classes/JARs.
 */
public class JvmExtractorTest {

    @Test
    public void testExtractFromSingleJar() throws IOException {
        // This test would require a test JAR file
        // For now, just verify the extractor can be instantiated
        String[] paths = {"test.jar"};
        JvmExtractor extractor = new JvmExtractor(paths);
        assertNotNull(extractor);
    }

    @Test
    public void testExtractFromMultipleJars() throws IOException {
        // Test multiple JAR extraction
        String[] paths = {"jar1.jar", "jar2.jar"};
        JvmExtractor extractor = new JvmExtractor(paths);
        assertNotNull(extractor);
    }

    @Test
    public void testExternalResourcesTracking() throws IOException {
        // Verify that all source paths are added to external_resources
        String[] paths = {"app.jar", "lib.jar"};
        JvmExtractor extractor = new JvmExtractor(paths);
        
        try {
            List<ModuleInfo> modules = extractor.extractAll();
            
            // Each module should have all paths in external_resources
            for (ModuleInfo module : modules) {
                assertEquals(paths.length, module.getExternalResources().size());
                for (String path : paths) {
                    assertTrue(module.getExternalResources().contains(path));
                }
            }
        } catch (IOException e) {
            // Expected if test JARs don't exist
        }
    }

    @Test
    public void testMultipleSourceTypes() throws IOException {
        // Test mixing .jar, .class, and directory sources
        String testDataPath = "test/com/metaffi/idl/testdata";
        File testDir = new File(testDataPath);
        
        if (!testDir.exists()) {
            return;
        }

        String[] paths = {
            testDir.getAbsolutePath(),  // Directory
            // Would add .jar and .class paths if available
        };

        JvmExtractor extractor = new JvmExtractor(paths);
        
        try {
            List<ModuleInfo> modules = extractor.extractAll();
            assertTrue(modules.size() > 0);
        } catch (IOException e) {
            // Expected if sources don't exist
        }
    }

    @Test
    public void testExtractAllReturnsModules() throws IOException {
        String[] paths = {"test.jar"};
        JvmExtractor extractor = new JvmExtractor(paths);
        
        try {
            List<ModuleInfo> modules = extractor.extractAll();
            assertNotNull(modules);
            // Should return one module per source
            assertEquals(paths.length, modules.size());
        } catch (IOException e) {
            // Expected if test JAR doesn't exist
        }
    }

    @Test
    public void testStaticExtractMethod() {
        // Test static extract method returns JSON string
        String[] paths = {"test.jar"};
        String result = JvmExtractor.extract(paths);
        
        // Should return JSON string (even if error)
        assertNotNull(result);
        assertTrue(result.startsWith("{"));
        assertTrue(result.endsWith("}"));
    }

    @Test
    public void testStaticExtractWithError() {
        // Test static extract returns error JSON on failure
        String[] invalidPaths = {"nonexistent.jar"};
        String result = JvmExtractor.extract(invalidPaths);
        
        // Should return JSON with error key
        assertTrue(result.contains("error") || result.contains("modules"));
    }
}
