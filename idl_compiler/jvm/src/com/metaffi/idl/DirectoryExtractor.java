package com.metaffi.idl;

import com.metaffi.idl.model.ClassInfo;
import com.metaffi.idl.model.ModuleInfo;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * Extracts interface definitions from a directory containing .class files.
 * Recursively processes all .class files in the directory tree.
 * Uses ClassFileExtractor for each .class file.
 */
public class DirectoryExtractor implements Extractor {
    private final String directoryPath;
    private final String moduleName;

    /**
     * Constructor.
     *
     * @param directoryPath Path to the directory containing .class files
     */
    public DirectoryExtractor(String directoryPath) {
        this.directoryPath = directoryPath;
        // Use directory name as module name
        File file = new File(directoryPath);
        this.moduleName = file.getName();
    }

    @Override
    public ModuleInfo extract() throws IOException {
        ModuleInfo module = new ModuleInfo(moduleName);
        module.setSourcePath(directoryPath);

        Path rootPath = Paths.get(directoryPath);

        // Walk the directory tree and process all .class files
        Files.walk(rootPath)
            .filter(path -> path.toString().endsWith(".class"))
            .forEach(classFile -> {
                try {
                    ClassFileExtractor extractor = new ClassFileExtractor(
                        classFile.toString(), moduleName);
                    ModuleInfo classModule = extractor.extract();

                    // Add extracted class if it was public
                    if (!classModule.getClasses().isEmpty()) {
                        ClassInfo classInfo = classModule.getClasses().get(0);
                        module.getClasses().add(classInfo);
                    }
                } catch (Exception e) {
                    // Log error but continue processing other classes
                    System.err.println("Failed to process " + classFile + ": " + e.getMessage());
                }
            });

        return module;
    }
}
