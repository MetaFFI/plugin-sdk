package com.metaffi.idl.entities;

/**
 * Holds MetaFFI type information including type, dimensions, and type alias.
 * Used by TypeMapper to return results from type mapping operations.
 */
public class MetaffiTypeInfo {

    private final String type;
    private final int dimensions;
    private final String typeAlias;

    public MetaffiTypeInfo(String type, int dimensions, String typeAlias) {
        if (type == null || type.isEmpty()) {
            throw new IllegalArgumentException("type cannot be null or empty");
        }
        if (dimensions < 0) {
            throw new IllegalArgumentException("dimensions cannot be negative: " + dimensions);
        }
        this.type = type;
        this.dimensions = dimensions;
        this.typeAlias = typeAlias;
    }

    public MetaffiTypeInfo(String type, int dimensions) {
        this(type, dimensions, null);
    }

    public MetaffiTypeInfo(String type) {
        this(type, 0, null);
    }

    public String getType() {
        return type;
    }

    public int getDimensions() {
        return dimensions;
    }

    public String getTypeAlias() {
        return typeAlias;
    }

    public boolean isArray() {
        return dimensions > 0;
    }

    public boolean hasTypeAlias() {
        return typeAlias != null && !typeAlias.isEmpty();
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("MetaffiTypeInfo{type='").append(type).append("'");
        if (dimensions > 0) {
            sb.append(", dimensions=").append(dimensions);
        }
        if (typeAlias != null) {
            sb.append(", typeAlias='").append(typeAlias).append("'");
        }
        sb.append("}");
        return sb.toString();
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        MetaffiTypeInfo other = (MetaffiTypeInfo) obj;
        return dimensions == other.dimensions &&
               type.equals(other.type) &&
               (typeAlias == null ? other.typeAlias == null : typeAlias.equals(other.typeAlias));
    }

    @Override
    public int hashCode() {
        int result = type.hashCode();
        result = 31 * result + dimensions;
        result = 31 * result + (typeAlias != null ? typeAlias.hashCode() : 0);
        return result;
    }
}
