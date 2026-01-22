package com.metaffi.idl.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Represents a module (JAR file, directory, or single .class file) with all its classes.
 * Each source becomes a separate module in the MetaFFI IDL JSON.
 */
public class ModuleInfo {
    private String moduleName;        // Module name (JAR filename or directory name)
    private String sourcePath;        // Original source path
    private List<ClassInfo> classes;  // All extracted classes
    private List<String> externalResources;  // External dependencies (JAR paths, directories)

    /**
     * Default constructor.
     */
    public ModuleInfo() {
        this.classes = new ArrayList<>();
        this.externalResources = new ArrayList<>();
    }

    /**
     * Constructor with module name.
     *
     * @param moduleName Module name
     */
    public ModuleInfo(String moduleName) {
        this.moduleName = moduleName;
        this.classes = new ArrayList<>();
        this.externalResources = new ArrayList<>();
    }

    public String getModuleName() {
        return moduleName;
    }

    public void setModuleName(String moduleName) {
        this.moduleName = moduleName;
    }

    public String getSourcePath() {
        return sourcePath;
    }

    public void setSourcePath(String sourcePath) {
        this.sourcePath = sourcePath;
    }

    public List<ClassInfo> getClasses() {
        return classes;
    }

    public void setClasses(List<ClassInfo> classes) {
        this.classes = classes;
    }

    public List<String> getExternalResources() {
        return externalResources;
    }

    public void setExternalResources(List<String> externalResources) {
        this.externalResources = externalResources;
    }

    @Override
    public String toString() {
        return "ModuleInfo{" +
                "moduleName='" + moduleName + '\'' +
                ", sourcePath='" + sourcePath + '\'' +
                ", classes=" + classes.size() +
                ", externalResources=" + externalResources.size() +
                '}';
    }
}
