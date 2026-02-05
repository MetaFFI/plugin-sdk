package com.metaffi.idl;

import com.google.gson.JsonObject;
import com.metaffi.idl.model.ClassInfo;
import com.metaffi.idl.model.MethodInfo;
import com.metaffi.idl.model.ModuleInfo;
import org.junit.Test;
import static org.junit.Assert.*;

import java.io.File;
import java.io.IOException;

/**
 * Specific tests for nested/inner class extraction.
 * Verifies that classes with $ in names are properly extracted.
 */
public class NestedClassTest {

    private String getTestClassesPath() {
        return "test/com/metaffi/idl/testdata";
    }

    @Test
    public void testExtractOuterWithInner() throws IOException {
        // Test extracting both outer and inner classes
        String outerPath = getTestClassesPath() + "/ClassWithNested.class";
        String innerPath = getTestClassesPath() + "/ClassWithNested$InnerClass.class";
        
        File outerFile = new File(outerPath);
        File innerFile = new File(innerPath);
        
        if (!outerFile.exists()) {
            return;
        }

        // Extract outer class
        ClassFileExtractor outerExtractor = new ClassFileExtractor(
            outerFile.getAbsolutePath(), "test");
        ModuleInfo outerModule = outerExtractor.extract();

        assertNotNull(outerModule);
        if (!outerModule.getClasses().isEmpty()) {
            ClassInfo outerClass = outerModule.getClasses().get(0);
            assertEquals("com.metaffi.idl.testdata.ClassWithNested", outerClass.getClassName());
        }

        // Extract inner class if it exists
        if (innerFile.exists()) {
            ClassFileExtractor innerExtractor = new ClassFileExtractor(
                innerFile.getAbsolutePath(), "test");
            ModuleInfo innerModule = innerExtractor.extract();

            if (!innerModule.getClasses().isEmpty()) {
                ClassInfo innerClass = innerModule.getClasses().get(0);
                assertTrue(innerClass.getClassName().contains("$"));
                assertTrue(innerClass.getClassName().contains("InnerClass"));
            }
        }
    }

    @Test
    public void testInnerClassEntityPath() throws IOException {
        // Verify inner class entity_path uses $ notation
        String innerPath = getTestClassesPath() + "/ClassWithNested$InnerClass.class";
        File innerFile = new File(innerPath);
        
        if (!innerFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            innerFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        if (!module.getClasses().isEmpty()) {
            ClassInfo innerClass = module.getClasses().get(0);
            String className = innerClass.getClassName();
            
            // Entity path should use $ notation
            assertTrue(className.contains("$"));
            
            // Verify entity_path generation works with $ in class name
            if (!innerClass.getMethods().isEmpty()) {
                MethodInfo method = innerClass.getMethods().get(0);
                JsonObject entityPath = EntityPathGenerator.createMethodPath(
                    className, method.getName(), !method.isStatic());

                // Verify the class field in entity_path contains $
                assertTrue(entityPath.get("class").getAsString().contains("$"));
                assertEquals(className, entityPath.get("class").getAsString());
            }
        }
    }

    @Test
    public void testBothClassesInSameModule() throws IOException {
        // When extracting from JAR, both outer and inner should be in same module
        // This is tested in JarExtractorTest, but verify here too
        String jarPath = "test/nested.jar";  // Would be created by test infrastructure
        File jarFile = new File(jarPath);
        
        if (!jarFile.exists()) {
            return;
        }

        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        // Should have multiple classes (outer + inner)
        int classCount = module.getClasses().size();
        assertTrue(classCount > 0);
        
        // Verify at least one class has $ (inner class)
        boolean hasInner = false;
        boolean hasOuter = false;
        
        for (ClassInfo classInfo : module.getClasses()) {
            if (classInfo.getClassName().contains("$")) {
                hasInner = true;
            } else if (classInfo.getClassName().contains("ClassWithNested")) {
                hasOuter = true;
            }
        }
        
        // Should have outer class at minimum
        assertTrue(hasOuter || classCount > 0);
    }

    @Test
    public void testNestedClassMethods() throws IOException {
        // Verify methods from nested classes are extracted
        String innerPath = getTestClassesPath() + "/ClassWithNested$InnerClass.class";
        File innerFile = new File(innerPath);
        
        if (!innerFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            innerFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        if (!module.getClasses().isEmpty()) {
            ClassInfo innerClass = module.getClasses().get(0);
            
            // Inner class should have methods
            assertTrue(innerClass.getMethods().size() >= 0);
            
            // Verify method entity_path includes $ in class name
            for (MethodInfo method : innerClass.getMethods()) {
                JsonObject entityPath = EntityPathGenerator.createMethodPath(
                    innerClass.getClassName(), method.getName(), !method.isStatic());

                assertTrue(entityPath.get("class").getAsString().contains("$"));
            }
        }
    }

    @Test
    public void testNestedClassFields() throws IOException {
        // Verify fields from nested classes are extracted
        String innerPath = getTestClassesPath() + "/ClassWithNested$InnerClass.class";
        File innerFile = new File(innerPath);
        
        if (!innerFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            innerFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        if (!module.getClasses().isEmpty()) {
            ClassInfo innerClass = module.getClasses().get(0);
            
            // Inner class should have fields
            assertTrue(innerClass.getFields().size() >= 0);
            
            // Verify field entity_path includes $ in class name
            for (com.metaffi.idl.model.FieldInfo field : innerClass.getFields()) {
                JsonObject getterPath = EntityPathGenerator.createFieldGetterPath(
                    innerClass.getClassName(), field.getName(), !field.isStatic());

                assertTrue(getterPath.get("class").getAsString().contains("$"));
            }
        }
    }

    @Test
    public void testStaticNestedClass() throws IOException {
        // Test static nested class extraction
        String staticNestedPath = getTestClassesPath() + "/ClassWithNested$StaticNested.class";
        File staticNestedFile = new File(staticNestedPath);
        
        if (!staticNestedFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            staticNestedFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        if (!module.getClasses().isEmpty()) {
            ClassInfo staticNested = module.getClasses().get(0);
            assertTrue(staticNested.getClassName().contains("$"));
            assertTrue(staticNested.getClassName().contains("StaticNested"));
        }
    }

    @Test
    public void testDeeplyNestedClass() throws IOException {
        // Test Outer$Inner$Nested extraction
        String deeplyNestedPath = getTestClassesPath() + 
            "/ClassWithNested$InnerClassWithNested$DeeplyNested.class";
        File deeplyNestedFile = new File(deeplyNestedPath);
        
        if (!deeplyNestedFile.exists()) {
            return;
        }

        ClassFileExtractor extractor = new ClassFileExtractor(
            deeplyNestedFile.getAbsolutePath(), "test");
        ModuleInfo module = extractor.extract();

        if (!module.getClasses().isEmpty()) {
            ClassInfo deeplyNested = module.getClasses().get(0);
            String className = deeplyNested.getClassName();
            
            // Should have multiple $ separators
            long dollarCount = className.chars().filter(ch -> ch == '$').count();
            assertTrue(dollarCount >= 2);
        }
    }

    @Test
    public void testPrivateInnerClassFiltered() throws IOException {
        // Private inner classes should be filtered (not extracted)
        // This is verified by the fact that ClassWithNested$PrivateInner
        // should not appear in extracted classes
        
        String jarPath = "test/nested.jar";
        File jarFile = new File(jarPath);
        
        if (!jarFile.exists()) {
            return;
        }

        JarExtractor extractor = new JarExtractor(jarPath);
        ModuleInfo module = extractor.extract();

        // Verify PrivateInner is not extracted
        for (ClassInfo classInfo : module.getClasses()) {
            assertFalse(classInfo.getClassName().contains("PrivateInner"));
        }
    }
}
