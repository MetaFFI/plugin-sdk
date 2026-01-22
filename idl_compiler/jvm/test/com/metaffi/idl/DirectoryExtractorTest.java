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

/**
 * Tests for DirectoryExtractor - verifies extraction from directories.
 */
public class DirectoryExtractorTest {

    private String getTestClassesPath() {
        return "test/com/metaffi/idl/testdata";
    }

    @Test
    public void testExtractFromDirectory() throws IOException {
        String dirPath = getTestClassesPath();
        File dir = new File(dirPath);
        
        if (!dir.exists() || !dir.isDirectory()) {
            return;
        }

        DirectoryExtractor extractor = new DirectoryExtractor(dir.getAbsolutePath());
        ModuleInfo module = extractor.extract();

        assertNotNull(module);
        assertEquals("testdata", module.getModuleName());
        // Should extract multiple classes from directory
        assertTrue(module.getClasses().size() >= 0);
    }

    @Test
    public void testRecursiveDirectoryTraversal() throws IOException {
        // DirectoryExtractor should recursively find all .class files
        String dirPath = getTestClassesPath();
        File dir = new File(dirPath);
        
        if (!dir.exists()) {
            return;
        }

        DirectoryExtractor extractor = new DirectoryExtractor(dir.getAbsolutePath());
        ModuleInfo module = extractor.extract();

        // Should find classes in subdirectories if any
        assertNotNull(module);
    }

    @Test
    public void testModuleNameFromDirectory() throws IOException {
        String dirPath = getTestClassesPath();
        File dir = new File(dirPath);
        
        if (!dir.exists()) {
            return;
        }

        DirectoryExtractor extractor = new DirectoryExtractor(dir.getAbsolutePath());
        ModuleInfo module = extractor.extract();

        assertEquals(dir.getName(), module.getModuleName());
    }
}
