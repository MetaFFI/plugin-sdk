package com.metaffi.idl.model;

import java.util.ArrayList;
import java.util.List;

/**
 * Represents a method or constructor in a Java class.
 * Contains signature information including parameters, return type, and modifiers.
 */
public class MethodInfo {
    private String name;                    // Method name or "<init>" for constructors
    private List<ParameterInfo> parameters; // Method parameters
    private String returnType;              // Return type (Java, e.g., "int", "java.lang.String")
    private String metaffiReturnType;       // Return type (MetaFFI, e.g., "int32", "handle")
    private int returnDimensions;           // Array dimensions for return type
    private boolean isStatic;               // Static method flag
    private boolean isConstructor;          // Constructor flag
    private String comment;                 // Javadoc (if available)

    /**
     * Default constructor.
     */
    public MethodInfo() {
        this.parameters = new ArrayList<>();
        this.returnDimensions = 0;
        this.isStatic = false;
        this.isConstructor = false;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public List<ParameterInfo> getParameters() {
        return parameters;
    }

    public void setParameters(List<ParameterInfo> parameters) {
        this.parameters = parameters;
    }

    public String getReturnType() {
        return returnType;
    }

    public void setReturnType(String returnType) {
        this.returnType = returnType;
    }

    public String getMetaffiReturnType() {
        return metaffiReturnType;
    }

    public void setMetaffiReturnType(String metaffiReturnType) {
        this.metaffiReturnType = metaffiReturnType;
    }

    public int getReturnDimensions() {
        return returnDimensions;
    }

    public void setReturnDimensions(int returnDimensions) {
        this.returnDimensions = returnDimensions;
    }

    public boolean isStatic() {
        return isStatic;
    }

    public void setIsStatic(boolean isStatic) {
        this.isStatic = isStatic;
    }

    public boolean isConstructor() {
        return isConstructor;
    }

    public void setIsConstructor(boolean isConstructor) {
        this.isConstructor = isConstructor;
    }

    public String getComment() {
        return comment;
    }

    public void setComment(String comment) {
        this.comment = comment;
    }

    @Override
    public String toString() {
        return "MethodInfo{" +
                "name='" + name + '\'' +
                ", parameters=" + parameters.size() +
                ", returnType='" + returnType + '\'' +
                ", isStatic=" + isStatic +
                ", isConstructor=" + isConstructor +
                '}';
    }
}
