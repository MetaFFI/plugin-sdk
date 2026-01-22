package com.metaffi.idl.model;

/**
 * Represents a parameter in a method or constructor.
 * Contains both the original Java type information and the mapped MetaFFI type.
 */
public class ParameterInfo {
    private String name;          // Parameter name (may be p0, p1 from bytecode)
    private String javaType;      // Original Java type (e.g., "java.lang.String")
    private String metaffiType;   // Mapped MetaFFI type (e.g., "handle")
    private int dimensions;       // Array dimensions (0 for scalar)

    /**
     * Default constructor.
     */
    public ParameterInfo() {
        this.dimensions = 0;
    }

    /**
     * Full constructor.
     *
     * @param name Parameter name
     * @param javaType Original Java type
     * @param metaffiType Mapped MetaFFI type
     * @param dimensions Array dimensions
     */
    public ParameterInfo(String name, String javaType, String metaffiType, int dimensions) {
        this.name = name;
        this.javaType = javaType;
        this.metaffiType = metaffiType;
        this.dimensions = dimensions;
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

    @Override
    public String toString() {
        return "ParameterInfo{" +
                "name='" + name + '\'' +
                ", javaType='" + javaType + '\'' +
                ", metaffiType='" + metaffiType + '\'' +
                ", dimensions=" + dimensions +
                '}';
    }
}
