package com.metaffi.idl;

import com.metaffi.idl.model.ModuleInfo;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Main entry point for JVM IDL extraction.
 * Handles multiple sources (JARs, directories, .class files) and orchestrates extraction.
 * All source paths are added to external_resources for dependency tracking.
 */
public class JvmExtractor {
    private final List<String> sourcePaths;

    /**
     * Create extractor for multiple sources.
     *
     * @param sourcePaths Array of paths to .jar files, .class files, or directories
     */
    public JvmExtractor(String[] sourcePaths) {
        this.sourcePaths = Arrays.asList(sourcePaths);
    }

    /**
     * Extract from all sources.
     *
     * @return List of ModuleInfo, one per source
     * @throws IOException if extraction fails
     */
    public List<ModuleInfo> extractAll() throws IOException {
        List<ModuleInfo> modules = new ArrayList<>();

        for (String path : sourcePaths) {
            File file = new File(path);

            if (!file.exists()) {
                throw new FileNotFoundException("Source not found: " + path);
            }

            Extractor extractor;

            if (file.isDirectory()) {
                extractor = new DirectoryExtractor(path);
            } else if (path.endsWith(".jar")) {
                extractor = new JarExtractor(path);
            } else if (path.endsWith(".class")) {
                // For single .class file, use filename without extension as module name
                String moduleName = file.getName();
                if (moduleName.endsWith(".class")) {
                    moduleName = moduleName.substring(0, moduleName.length() - 6);
                }
                extractor = new ClassFileExtractor(path, moduleName);
            } else {
                throw new IllegalArgumentException("Unsupported source type: " + path);
            }

            ModuleInfo module = extractor.extract();
            modules.add(module);
        }

        // Add all source paths to external_resources of each module
        // This allows runtime to load all dependencies
        for (ModuleInfo module : modules) {
            module.getExternalResources().addAll(sourcePaths);
        }

        return modules;
    }

    /**
     * Main entry point for C++ JNI bridge.
     *
     * @param sourcePaths Array of source paths (may be semicolon-separated string split)
     * @return JSON string of complete IDL
     */
    public static String extract(String[] sourcePaths) {
        try {
            JvmExtractor extractor = new JvmExtractor(sourcePaths);
            List<ModuleInfo> modules = extractor.extractAll();

            IdlGenerator generator = new IdlGenerator(modules);
            return generator.generateJson();

        } catch (Exception e) {
            // Return error in JSON format
            return String.format("{\"error\": \"%s\"}",
                e.getMessage().replace("\"", "\\\"").replace("\n", "\\n"));
        }
    }
}
