package com.metaffi.idl.entities;

import java.util.HashMap;
import java.util.Map;

/**
 * Maps between MetaFFI types and Java types.
 *
 * Used by the JVM host compiler to:
 * - Generate Java type annotations from MetaFFI IDL (metaffiTypeToJavaType)
 * - Generate MetaFFI IDL from Java source code (javaTypeToMetaffiType)
 * - Generate JNI signatures (metaffiTypeToJniSignature)
 *
 * Enforces fail-fast policy: throws IllegalArgumentException for unknown types.
 */
public class TypeMapper {

    // MetaFFI type -> Java type mapping (for deserialization / code generation)
    private static final Map<String, String> METAFFI_TO_JAVA = new HashMap<>();

    // Java type -> MetaFFI type mapping (for serialization / IDL generation)
    private static final Map<String, String> JAVA_TO_METAFFI = new HashMap<>();

    // MetaFFI type -> JNI signature mapping
    private static final Map<String, String> METAFFI_TO_JNI = new HashMap<>();

    static {
        // Signed integers
        METAFFI_TO_JAVA.put("int8", "byte");
        METAFFI_TO_JAVA.put("int16", "short");
        METAFFI_TO_JAVA.put("int32", "int");
        METAFFI_TO_JAVA.put("int64", "long");

        // Unsigned integers - use larger signed types to preserve range
        METAFFI_TO_JAVA.put("uint8", "short");      // 0-255 fits in short
        METAFFI_TO_JAVA.put("uint16", "int");       // 0-65535 fits in int
        METAFFI_TO_JAVA.put("uint32", "long");      // 0-4B fits in long
        METAFFI_TO_JAVA.put("uint64", "BigInteger"); // Use BigInteger for full range

        // Floating point
        METAFFI_TO_JAVA.put("float32", "float");
        METAFFI_TO_JAVA.put("float64", "double");

        // Boolean
        METAFFI_TO_JAVA.put("bool", "boolean");

        // Characters - all map to char (char32 will fail-fast at runtime if > BMP)
        METAFFI_TO_JAVA.put("char8", "char");
        METAFFI_TO_JAVA.put("char16", "char");
        METAFFI_TO_JAVA.put("char32", "char");

        // Strings - all map to String
        METAFFI_TO_JAVA.put("string8", "String");
        METAFFI_TO_JAVA.put("string16", "String");
        METAFFI_TO_JAVA.put("string32", "String");

        // Handle and special types
        METAFFI_TO_JAVA.put("handle", "Object");
        METAFFI_TO_JAVA.put("callable", "metaffi.Caller");
        METAFFI_TO_JAVA.put("any", "Object");
        METAFFI_TO_JAVA.put("null", "void");
        METAFFI_TO_JAVA.put("size", "long");

        // Java -> MetaFFI (reverse mapping for IDL generation)
        JAVA_TO_METAFFI.put("byte", "int8");
        JAVA_TO_METAFFI.put("short", "int16");
        JAVA_TO_METAFFI.put("int", "int32");
        JAVA_TO_METAFFI.put("long", "int64");
        JAVA_TO_METAFFI.put("float", "float32");
        JAVA_TO_METAFFI.put("double", "float64");
        JAVA_TO_METAFFI.put("boolean", "bool");
        JAVA_TO_METAFFI.put("char", "char16");
        JAVA_TO_METAFFI.put("String", "string8");
        JAVA_TO_METAFFI.put("java.lang.String", "string8");
        JAVA_TO_METAFFI.put("void", "null");
        JAVA_TO_METAFFI.put("BigInteger", "uint64");
        JAVA_TO_METAFFI.put("java.math.BigInteger", "uint64");
        JAVA_TO_METAFFI.put("metaffi.Caller", "callable");
        JAVA_TO_METAFFI.put("Object", "handle");
        JAVA_TO_METAFFI.put("java.lang.Object", "handle");

        // Boxed types -> MetaFFI
        JAVA_TO_METAFFI.put("Byte", "int8");
        JAVA_TO_METAFFI.put("java.lang.Byte", "int8");
        JAVA_TO_METAFFI.put("Short", "int16");
        JAVA_TO_METAFFI.put("java.lang.Short", "int16");
        JAVA_TO_METAFFI.put("Integer", "int32");
        JAVA_TO_METAFFI.put("java.lang.Integer", "int32");
        JAVA_TO_METAFFI.put("Long", "int64");
        JAVA_TO_METAFFI.put("java.lang.Long", "int64");
        JAVA_TO_METAFFI.put("Float", "float32");
        JAVA_TO_METAFFI.put("java.lang.Float", "float32");
        JAVA_TO_METAFFI.put("Double", "float64");
        JAVA_TO_METAFFI.put("java.lang.Double", "float64");
        JAVA_TO_METAFFI.put("Boolean", "bool");
        JAVA_TO_METAFFI.put("java.lang.Boolean", "bool");
        JAVA_TO_METAFFI.put("Character", "char16");
        JAVA_TO_METAFFI.put("java.lang.Character", "char16");

        // JNI signatures
        METAFFI_TO_JNI.put("int8", "B");
        METAFFI_TO_JNI.put("int16", "S");
        METAFFI_TO_JNI.put("int32", "I");
        METAFFI_TO_JNI.put("int64", "J");
        METAFFI_TO_JNI.put("uint8", "S");      // short for uint8
        METAFFI_TO_JNI.put("uint16", "I");     // int for uint16
        METAFFI_TO_JNI.put("uint32", "J");     // long for uint32
        METAFFI_TO_JNI.put("uint64", "Ljava/math/BigInteger;");
        METAFFI_TO_JNI.put("float32", "F");
        METAFFI_TO_JNI.put("float64", "D");
        METAFFI_TO_JNI.put("bool", "Z");
        METAFFI_TO_JNI.put("char8", "C");
        METAFFI_TO_JNI.put("char16", "C");
        METAFFI_TO_JNI.put("char32", "C");
        METAFFI_TO_JNI.put("string8", "Ljava/lang/String;");
        METAFFI_TO_JNI.put("string16", "Ljava/lang/String;");
        METAFFI_TO_JNI.put("string32", "Ljava/lang/String;");
        METAFFI_TO_JNI.put("handle", "Ljava/lang/Object;");
        METAFFI_TO_JNI.put("callable", "Lmetaffi/Caller;");
        METAFFI_TO_JNI.put("any", "Ljava/lang/Object;");
        METAFFI_TO_JNI.put("null", "V");
        METAFFI_TO_JNI.put("size", "J");
    }

    /**
     * Convert MetaFFI type string to Java type string.
     *
     * @param metaffiType MetaFFI type string (e.g., "int32", "string8", "handle")
     * @param dimensions Number of array dimensions (0 = not an array)
     * @param typeAlias Optional type alias for handle types (can be null)
     * @return Java type string (e.g., "int", "String", "int[][]", "MyClass")
     * @throws IllegalArgumentException if metaffiType is unknown
     */
    public static String metaffiTypeToJavaType(String metaffiType, int dimensions, String typeAlias) {
        if (metaffiType == null || metaffiType.isEmpty()) {
            throw new IllegalArgumentException("metaffiType cannot be null or empty");
        }
        if (dimensions < 0) {
            throw new IllegalArgumentException("dimensions cannot be negative: " + dimensions);
        }

        // Handle array suffix in type name (e.g., "int32_array")
        String baseType = metaffiType;
        if (metaffiType.endsWith("_array")) {
            baseType = metaffiType.substring(0, metaffiType.length() - 6);
            if (dimensions == 0) {
                dimensions = 1; // _array suffix implies at least 1 dimension
            }
        }

        // Get Java type
        String javaType;
        if (baseType.equals("handle") && typeAlias != null && !typeAlias.isEmpty()) {
            javaType = typeAlias;
        } else {
            javaType = METAFFI_TO_JAVA.get(baseType);
            if (javaType == null) {
                throw new IllegalArgumentException("Unknown MetaFFI type: " + metaffiType);
            }
        }

        // Append array brackets
        if (dimensions > 0) {
            StringBuilder sb = new StringBuilder(javaType);
            for (int i = 0; i < dimensions; i++) {
                sb.append("[]");
            }
            return sb.toString();
        }

        return javaType;
    }

    /**
     * Convert Java type to MetaFFI type and dimensions.
     *
     * @param javaType Java type string (e.g., "int", "int[]", "String[][]", "MyClass")
     * @return MetaffiTypeInfo containing (metaffiType, dimensions, typeAlias)
     * @throws IllegalArgumentException if javaType is null or empty
     */
    public static MetaffiTypeInfo javaTypeToMetaffiType(String javaType) {
        if (javaType == null || javaType.isEmpty()) {
            throw new IllegalArgumentException("javaType cannot be null or empty");
        }

        // Count and remove array brackets
        int dimensions = 0;
        String baseType = javaType.trim();
        while (baseType.endsWith("[]")) {
            dimensions++;
            baseType = baseType.substring(0, baseType.length() - 2).trim();
        }

        // Map base type
        String metaffiType = JAVA_TO_METAFFI.get(baseType);
        String typeAlias = null;

        if (metaffiType == null) {
            // Unknown type becomes handle with type alias
            metaffiType = "handle";
            typeAlias = baseType;
        }

        return new MetaffiTypeInfo(metaffiType, dimensions, typeAlias);
    }

    /**
     * Get JNI signature for a MetaFFI type.
     *
     * @param metaffiType MetaFFI type string
     * @param dimensions Number of array dimensions
     * @return JNI signature string (e.g., "I", "[I", "[[Ljava/lang/String;")
     * @throws IllegalArgumentException if metaffiType is unknown
     */
    public static String metaffiTypeToJniSignature(String metaffiType, int dimensions) {
        if (metaffiType == null || metaffiType.isEmpty()) {
            throw new IllegalArgumentException("metaffiType cannot be null or empty");
        }
        if (dimensions < 0) {
            throw new IllegalArgumentException("dimensions cannot be negative: " + dimensions);
        }

        // Handle array suffix in type name
        String baseType = metaffiType;
        if (metaffiType.endsWith("_array")) {
            baseType = metaffiType.substring(0, metaffiType.length() - 6);
            if (dimensions == 0) {
                dimensions = 1;
            }
        }

        String jniSig = METAFFI_TO_JNI.get(baseType);
        if (jniSig == null) {
            throw new IllegalArgumentException("Unknown MetaFFI type for JNI signature: " + metaffiType);
        }

        // Prepend array brackets
        if (dimensions > 0) {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < dimensions; i++) {
                sb.append("[");
            }
            sb.append(jniSig);
            return sb.toString();
        }

        return jniSig;
    }

    /**
     * Get type alias for handle types.
     *
     * @param javaType Original Java type string
     * @param metaffiType The mapped MetaFFI type
     * @return Type alias string (the original Java type for handles, null for primitives)
     */
    public static String getTypeAlias(String javaType, String metaffiType) {
        if ("handle".equals(metaffiType)) {
            // Extract base type if it's an array
            String baseType = javaType;
            while (baseType.endsWith("[]")) {
                baseType = baseType.substring(0, baseType.length() - 2).trim();
            }
            // Only return alias if it's not a standard type
            if (!JAVA_TO_METAFFI.containsKey(baseType)) {
                return baseType;
            }
        }
        return null;
    }

    /**
     * Check if a MetaFFI type is a primitive type.
     *
     * @param metaffiType MetaFFI type string
     * @return true if primitive, false if object type
     */
    public static boolean isPrimitiveType(String metaffiType) {
        if (metaffiType == null) {
            return false;
        }
        String baseType = metaffiType.endsWith("_array")
            ? metaffiType.substring(0, metaffiType.length() - 6)
            : metaffiType;

        switch (baseType) {
            case "int8":
            case "int16":
            case "int32":
            case "int64":
            case "uint8":
            case "uint16":
            case "uint32":
            case "float32":
            case "float64":
            case "bool":
            case "char8":
            case "char16":
            case "char32":
                return true;
            default:
                return false;
        }
    }

    /**
     * Check if a MetaFFI type requires special handling for unsigned values.
     *
     * @param metaffiType MetaFFI type string
     * @return true if unsigned type that needs special handling
     */
    public static boolean isUnsignedType(String metaffiType) {
        if (metaffiType == null) {
            return false;
        }
        String baseType = metaffiType.endsWith("_array")
            ? metaffiType.substring(0, metaffiType.length() - 6)
            : metaffiType;

        return baseType.startsWith("uint");
    }
}
