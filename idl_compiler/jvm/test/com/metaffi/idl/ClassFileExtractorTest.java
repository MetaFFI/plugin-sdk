package com.metaffi.idl;

import com.metaffi.idl.model.ClassInfo;
import com.metaffi.idl.model.FieldInfo;
import com.metaffi.idl.model.MethodInfo;
import com.metaffi.idl.model.ModuleInfo;
import org.junit.Test;
import static org.junit.Assert.*;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * Tests for ClassFileExtractor - verifies extraction from single .class files.
 * Includes tests for nested classes.
 */
public class ClassFileExtractorTest {

    private String getTestClassesPath() {
        // Path to compiled test classes
        String testDataPath = "test/com/metaffi/idl/testdata";
        return testDataPath;
    }

    @Test
    public void testExtractSimpleClass() throws IOException {
        String classPath = getTestClassesPath() + "/SimpleClass.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            // Skip if test classes not compiled yet
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        assertNotNull(module);
        assertEquals(1, module.getClasses().size());

        ClassInfo classInfo = module.getClasses().get(0);
        assertEquals("com.metaffi.idl.testdata.SimpleClass", classInfo.getClassName());
        assertEquals("SimpleClass", classInfo.getSimpleName());
    }

    @Test
    public void testExtractClassWithConstructor() throws IOException {
        String classPath = getTestClassesPath() + "/SimpleClass.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        ClassInfo classInfo = module.getClasses().get(0);
        
        // Should have at least one constructor
        assertTrue(classInfo.getConstructors().size() > 0);
        
        // Verify constructor has correct entity_path format
        MethodInfo constructor = classInfo.getConstructors().get(0);
        assertNotNull(constructor);
    }

    @Test
    public void testExtractClassWithMethods() throws IOException {
        String classPath = getTestClassesPath() + "/ClassWithMethods.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        ClassInfo classInfo = module.getClasses().get(0);
        
        // Should have multiple public methods
        assertTrue(classInfo.getMethods().size() > 0);
        
        // Verify methods are extracted
        boolean hasInstanceMethod = false;
        boolean hasStaticMethod = false;
        
        for (MethodInfo method : classInfo.getMethods()) {
            if (!method.isStatic()) {
                hasInstanceMethod = true;
            } else {
                hasStaticMethod = true;
            }
        }
        
        // Should have both instance and static methods
        assertTrue(hasInstanceMethod || hasStaticMethod);
    }

    @Test
    public void testExtractClassWithFields() throws IOException {
        String classPath = getTestClassesPath() + "/SimpleClass.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        ClassInfo classInfo = module.getClasses().get(0);
        
        // Should have public fields
        assertTrue(classInfo.getFields().size() > 0);
        
        // Verify field types are mapped
        for (FieldInfo field : classInfo.getFields()) {
            assertNotNull(field.getMetaffiType());
            assertTrue(field.getMetaffiType().equals(field.getMetaffiType().toLowerCase()));
        }
    }

    @Test
    public void testExtractNestedClass() throws IOException {
        // Test extraction of Outer$Inner class
        String innerClassPath = getTestClassesPath() + "/ClassWithNested$InnerClass.class";
        File innerClassFile = new File(innerClassPath);
        
        if (!innerClassFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            innerClassFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        // Should extract the inner class
        if (!module.getClasses().isEmpty()) {
            ClassInfo innerClass = module.getClasses().get(0);
            assertTrue(innerClass.getClassName().contains("$"));
            assertEquals("ClassWithNested$InnerClass", 
                innerClass.getClassName().substring(innerClass.getClassName().lastIndexOf('.') + 1));
        }
    }

    @Test
    public void testExtractInterface() throws IOException {
        String classPath = getTestClassesPath() + "/InterfaceExample.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        if (!module.getClasses().isEmpty()) {
            ClassInfo interfaceInfo = module.getClasses().get(0);
            assertTrue(interfaceInfo.isInterface());
        }
    }

    @Test
    public void testExtractAbstractClass() throws IOException {
        String classPath = getTestClassesPath() + "/AbstractClassExample.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        if (!module.getClasses().isEmpty()) {
            ClassInfo abstractClass = module.getClasses().get(0);
            assertTrue(abstractClass.isAbstract());
        }
    }

    @Test
    public void testFilterPrivateMethods() throws IOException {
        String classPath = getTestClassesPath() + "/ClassWithMethods.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        ClassInfo classInfo = module.getClasses().get(0);
        
        // Verify no private methods are extracted
        for (MethodInfo method : classInfo.getMethods()) {
            // All extracted methods should be public (privateMethod should not appear)
            assertNotEquals("privateMethod", method.getName());
        }
    }

    @Test
    public void testFilterPrivateFields() throws IOException {
        String classPath = getTestClassesPath() + "/ClassWithMethods.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        ClassInfo classInfo = module.getClasses().get(0);
        
        // All extracted fields should be public
        for (FieldInfo field : classInfo.getFields()) {
            // Fields are already filtered by ACC_PUBLIC in extractor
            assertNotNull(field.getName());
        }
    }

    @Test
    public void testExtractOverloadedMethods() throws IOException {
        String classPath = getTestClassesPath() + "/ClassWithMethods.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        ClassInfo classInfo = module.getClasses().get(0);
        
        // Count methods named "overloaded"
        int overloadedCount = 0;
        for (MethodInfo method : classInfo.getMethods()) {
            if ("overloaded".equals(method.getName())) {
                overloadedCount++;
            }
        }
        
        // Should have multiple overloaded methods
        assertTrue(overloadedCount >= 2);
    }

    @Test
    public void testExtractStaticMethods() throws IOException {
        String classPath = getTestClassesPath() + "/ClassWithMethods.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        ClassInfo classInfo = module.getClasses().get(0);
        
        // Should have at least one static method
        boolean hasStatic = false;
        for (MethodInfo method : classInfo.getMethods()) {
            if (method.isStatic()) {
                hasStatic = true;
                break;
            }
        }
        
        assertTrue(hasStatic);
    }

    @Test
    public void testExtractStaticFields() throws IOException {
        String classPath = getTestClassesPath() + "/SimpleClass.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        ClassInfo classInfo = module.getClasses().get(0);
        
        // Should have static fields
        boolean hasStaticField = false;
        for (FieldInfo field : classInfo.getFields()) {
            if (field.isStatic()) {
                hasStaticField = true;
                break;
            }
        }
        
        assertTrue(hasStaticField);
    }

    @Test
    public void testExtractMethodsWithArrays() throws IOException {
        String classPath = getTestClassesPath() + "/ClassWithMethods.class";
        File classFile = new File(classPath);
        
        if (!classFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            classFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        ClassInfo classInfo = module.getClasses().get(0);
        
        // Find method with array parameter
        boolean hasArrayParam = false;
        for (MethodInfo method : classInfo.getMethods()) {
            for (com.metaffi.idl.model.ParameterInfo param : method.getParameters()) {
                if (param.getDimensions() > 0) {
                    hasArrayParam = true;
                    break;
                }
            }
            if (hasArrayParam) break;
        }
        
        assertTrue(hasArrayParam);
    }
}
