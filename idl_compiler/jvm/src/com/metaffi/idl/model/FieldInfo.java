package com.metaffi.idl.model;

/**
 * Represents a field in a Java class.
 * Contains both the original Java type information and the mapped MetaFFI type.
 */
public class FieldInfo {
    private String name;           // Field name
    private String javaType;       // Original Java type descriptor
    private String metaffiType;    // Mapped MetaFFI type
    private int dimensions;        // Array dimensions (0 for scalar)
    private boolean isStatic;      // Static field flag

    /**
     * Default constructor.
     */
    public FieldInfo() {
        this.dimensions = 0;
        this.isStatic = false;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getJavaType() {
        return javaType;
    }

    public void setJavaType(String javaType) {
        this.javaType = javaType;
    }

    public String getMetaffiType() {
        return metaffiType;
    }

    public void setMetaffiType(String metaffiType) {
        this.metaffiType = metaffiType;
    }

    public int getDimensions() {
        return dimensions;
    }

    public void setDimensions(int dimensions) {
        this.dimensions = dimensions;
    }

    public boolean isStatic() {
        return isStatic;
    }

    public void setIsStatic(boolean isStatic) {
        this.isStatic = isStatic;
    }

    @Override
    public String toString() {
        return "FieldInfo{" +
                "name='" + name + '\'' +
                ", javaType='" + javaType + '\'' +
                ", metaffiType='" + metaffiType + '\'' +
                ", dimensions=" + dimensions +
                ", isStatic=" + isStatic +
                '}';
    }
}
