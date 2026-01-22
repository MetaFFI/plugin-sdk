package com.metaffi.idl;

import com.metaffi.idl.model.ClassInfo;
import com.metaffi.idl.model.ModuleInfo;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 * Extracts interface definitions from a .jar file.
 * Processes all public classes in the JAR, including inner classes (with $ in names).
 * Uses ClassFileExtractor for each .class file entry.
 */
public class JarExtractor implements Extractor {
    private final String jarPath;
    private final String moduleName;

    /**
     * Constructor.
     *
     * @param jarPath Path to the .jar file
     */
    public JarExtractor(String jarPath) {
        this.jarPath = jarPath;
        // Use JAR filename (without extension) as module name
        File file = new File(jarPath);
        String fileName = file.getName();
        if (fileName.endsWith(".jar")) {
            this.moduleName = fileName.substring(0, fileName.length() - 4);
        } else {
            this.moduleName = fileName;
        }
    }

    @Override
    public ModuleInfo extract() throws IOException {
        ModuleInfo module = new ModuleInfo(moduleName);
        module.setSourcePath(jarPath);

        try (JarFile jar = new JarFile(jarPath)) {
            Enumeration<JarEntry> entries = jar.entries();

            while (entries.hasMoreElements()) {
                JarEntry entry = entries.nextElement();

                // Skip directories and non-class files
                if (entry.isDirectory() || !entry.getName().endsWith(".class")) {
                    continue;
                }

                // Skip META-INF
                if (entry.getName().startsWith("META-INF/")) {
                    continue;
                }

                // Process the class file
                try (InputStream classStream = jar.getInputStream(entry)) {
                    ClassFileExtractor extractor = new ClassFileExtractor(entry.getName(), moduleName);
                    ModuleInfo classModule = extractor.extractFromStream(classStream);

                    // Add extracted class if it was public
                    if (!classModule.getClasses().isEmpty()) {
                        ClassInfo classInfo = classModule.getClasses().get(0);
                        module.getClasses().add(classInfo);
                    }
                } catch (Exception e) {
                    // Log error but continue processing other classes
                    System.err.println("Failed to process " + entry.getName() + ": " + e.getMessage());
                }
            }
        }

        return module;
    }
}
