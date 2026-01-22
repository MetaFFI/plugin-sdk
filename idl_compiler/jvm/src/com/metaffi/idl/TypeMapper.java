package com.metaffi.idl;

/**
 * Maps Java type descriptors to MetaFFI types.
 * Handles primitive types, arrays, String, and objects.
 * All MetaFFI types are lowercase (int32, not INT32).
 */
public class TypeMapper {

    /**
     * Map Java type descriptor to MetaFFI type and dimensions.
     *
     * @param javaType Java type descriptor (e.g., "I", "Ljava/lang/String;", "[I", "[[D")
     * @return TypeInfo with metaffi_type and dimensions
     */
    public static TypeInfo mapType(String javaType) {
        // Parse type descriptor
        int dimensions = 0;
        String baseType = javaType;

        // Count array dimensions
        while (baseType.startsWith("[")) {
            dimensions++;
            baseType = baseType.substring(1);
        }

        // Map base type
        String metaffiType;
        switch (baseType) {
            case "B": metaffiType = "int8"; break;     // byte
            case "S": metaffiType = "int16"; break;    // short
            case "I": metaffiType = "int32"; break;    // int
            case "J": metaffiType = "int64"; break;    // long
            case "F": metaffiType = "float32"; break;  // float
            case "D": metaffiType = "float64"; break;  // double
            case "Z": metaffiType = "bool"; break;     // boolean
            case "C": metaffiType = "char16"; break;   // char
            case "V": metaffiType = "null"; break;     // void
            case "Ljava/lang/String;": metaffiType = "string8"; break;
            default: metaffiType = "handle"; break;    // All objects
        }

        return new TypeInfo(metaffiType, dimensions, baseType);
    }

    /**
     * Container for type mapping results.
     */
    public static class TypeInfo {
        public final String metaffiType;
        public final int dimensions;
        public final String originalType;

        public TypeInfo(String metaffiType, int dimensions, String originalType) {
            this.metaffiType = metaffiType;
            this.dimensions = dimensions;
            this.originalType = originalType;
        }

        @Override
        public String toString() {
            return "TypeInfo{" +
                    "metaffiType='" + metaffiType + '\'' +
                    ", dimensions=" + dimensions +
                    ", originalType='" + originalType + '\'' +
                    '}';
        }
    }
}
