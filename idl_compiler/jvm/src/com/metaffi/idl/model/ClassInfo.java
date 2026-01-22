package com.metaffi.idl.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Represents a Java class with its methods, constructors, and fields.
 * Contains all information needed to generate MetaFFI IDL JSON for the class.
 */
public class ClassInfo {
    private String className;              // Fully qualified name (e.g., "com.example.MyClass")
    private String packageName;            // Package (e.g., "com.example")
    private String simpleName;             // Simple name (e.g., "MyClass")
    private List<MethodInfo> methods;      // All public methods
    private List<MethodInfo> constructors; // Public constructors
    private List<FieldInfo> fields;        // All public fields
    private boolean isInterface;           // Interface flag
    private boolean isAbstract;            // Abstract class flag
    private String comment;                // Javadoc (if available)

    /**
     * Default constructor.
     */
    public ClassInfo() {
        this.methods = new ArrayList<>();
        this.constructors = new ArrayList<>();
        this.fields = new ArrayList<>();
        this.isInterface = false;
        this.isAbstract = false;
    }

    public String getClassName() {
        return className;
    }

    public void setClassName(String className) {
        this.className = className;
    }

    public String getPackageName() {
        return packageName;
    }

    public void setPackageName(String packageName) {
        this.packageName = packageName;
    }

    public String getSimpleName() {
        return simpleName;
    }

    public void setSimpleName(String simpleName) {
        this.simpleName = simpleName;
    }

    public List<MethodInfo> getMethods() {
        return methods;
    }

    public void setMethods(List<MethodInfo> methods) {
        this.methods = methods;
    }

    public List<MethodInfo> getConstructors() {
        return constructors;
    }

    public void setConstructors(List<MethodInfo> constructors) {
        this.constructors = constructors;
    }

    public List<FieldInfo> getFields() {
        return fields;
    }

    public void setFields(List<FieldInfo> fields) {
        this.fields = fields;
    }

    public boolean isInterface() {
        return isInterface;
    }

    public void setIsInterface(boolean isInterface) {
        this.isInterface = isInterface;
    }

    public boolean isAbstract() {
        return isAbstract;
    }

    public void setIsAbstract(boolean isAbstract) {
        this.isAbstract = isAbstract;
    }

    public String getComment() {
        return comment;
    }

    public void setComment(String comment) {
        this.comment = comment;
    }

    @Override
    public String toString() {
        return "ClassInfo{" +
                "className='" + className + '\'' +
                ", methods=" + methods.size() +
                ", constructors=" + constructors.size() +
                ", fields=" + fields.size() +
                ", isInterface=" + isInterface +
                '}';
    }
}
