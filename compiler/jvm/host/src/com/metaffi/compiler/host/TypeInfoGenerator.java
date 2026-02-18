package com.metaffi.compiler.host;

import com.metaffi.idl.entities.ArgDefinition;

import java.util.ArrayList;
import java.util.List;

public class TypeInfoGenerator {

    public String generateTypeInfoArray(List<ArgDefinition> args) {
        if (args == null || args.isEmpty()) {
            return "new MetaFFITypeInfo[]{}";
        }

        List<String> parts = new ArrayList<>();
        for (ArgDefinition arg : args) {
            parts.add(generateTypeInfo(arg));
        }

        return "new MetaFFITypeInfo[]{" + String.join(", ", parts) + "}";
    }

    public String generateTypeInfo(ArgDefinition arg) {
        if (arg == null) {
            throw new CompilerException("ArgDefinition is required");
        }

        String type = arg.getType();
        if (type == null || type.isEmpty()) {
            throw new CompilerException("ArgDefinition missing type");
        }

        int dims = arg.getDimensions();
        if (dims < 0) {
            throw new CompilerException("ArgDefinition has negative dimensions: " + dims);
        }

        String baseType = type;
        if (type.endsWith("_packed_array")) {
            // Explicit packed array from IDL (e.g. "int64_packed_array")
            baseType = type.substring(0, type.length() - 13); // strip "_packed_array"
            dims = 1; // packed arrays are always 1D
        } else if (type.endsWith("_array")) {
            baseType = type.substring(0, type.length() - 6);
            if (dims == 0) {
                dims = 1;
            }
        }

        String enumName = mapToMetaFFITypeEnum(baseType, dims);
        String alias = arg.getTypeAlias();
        boolean hasAlias = alias != null && !alias.isEmpty();

        if (hasAlias && dims > 0) {
            return "new MetaFFITypeInfo(MetaFFITypes." + enumName + ", \"" + escapeJava(alias) + "\", " + dims + ")";
        }
        if (hasAlias) {
            return "new MetaFFITypeInfo(MetaFFITypes." + enumName + ", \"" + escapeJava(alias) + "\")";
        }
        if (dims > 0) {
            return "new MetaFFITypeInfo(MetaFFITypes." + enumName + ", " + dims + ")";
        }
        return "new MetaFFITypeInfo(MetaFFITypes." + enumName + ")";
    }

    /**
     * Maps a base MetaFFI type and dimension count to a Java MetaFFITypes enum name.
     * For 1D arrays of packable primitives, automatically upgrades to packed array types
     * for performance optimization.
     */
    private String mapToMetaFFITypeEnum(String baseType, int dims) {
        boolean isArray = dims > 0;
        boolean isPacked = dims == 1 && isPackablePrimitive(baseType);

        switch (baseType) {
            case "float64": return isPacked ? "MetaFFIFloat64PackedArray" : isArray ? "MetaFFIFloat64Array" : "MetaFFIFloat64";
            case "float32": return isPacked ? "MetaFFIFloat32PackedArray" : isArray ? "MetaFFIFloat32Array" : "MetaFFIFloat32";
            case "int8": return isPacked ? "MetaFFIInt8PackedArray" : isArray ? "MetaFFIInt8Array" : "MetaFFIInt8";
            case "int16": return isPacked ? "MetaFFIInt16PackedArray" : isArray ? "MetaFFIInt16Array" : "MetaFFIInt16";
            case "int32": return isPacked ? "MetaFFIInt32PackedArray" : isArray ? "MetaFFIInt32Array" : "MetaFFIInt32";
            case "int64": return isPacked ? "MetaFFIInt64PackedArray" : isArray ? "MetaFFIInt64Array" : "MetaFFIInt64";
            case "uint8": return isPacked ? "MetaFFIUInt8PackedArray" : isArray ? "MetaFFIUInt8Array" : "MetaFFIUInt8";
            case "uint16": return isPacked ? "MetaFFIUInt16PackedArray" : isArray ? "MetaFFIUInt16Array" : "MetaFFIUInt16";
            case "uint32": return isPacked ? "MetaFFIUInt32PackedArray" : isArray ? "MetaFFIUInt32Array" : "MetaFFIUInt32";
            case "uint64": return isPacked ? "MetaFFIUInt64PackedArray" : isArray ? "MetaFFIUInt64Array" : "MetaFFIUInt64";
            case "bool": return isPacked ? "MetaFFIBoolPackedArray" : isArray ? "MetaFFIBoolArray" : "MetaFFIBool";
            case "char8": return isArray ? "MetaFFIChar8Array" : "MetaFFIChar8";
            case "char16":
                if (isArray) {
                    throw new CompilerException("char16_array is not supported in JVM MetaFFITypeInfo");
                }
                return "MetaFFIChar16";
            case "char32":
                if (isArray) {
                    throw new CompilerException("char32_array is not supported in JVM MetaFFITypeInfo");
                }
                return "MetaFFIChar32";
            case "string8": return isPacked ? "MetaFFIString8PackedArray" : isArray ? "MetaFFIString8Array" : "MetaFFIString8";
            case "string16": return isArray ? "MetaFFIString16Array" : "MetaFFIString16";
            case "string32": return isArray ? "MetaFFIString32Array" : "MetaFFIString32";
            case "handle": return isPacked ? "MetaFFIHandlePackedArray" : isArray ? "MetaFFIHandleArray" : "MetaFFIHandle";
            case "any": return isArray ? "MetaFFIAnyArray" : "MetaFFIAny";
            case "size": return isArray ? "MetaFFISizeArray" : "MetaFFISize";
            case "callable":
                if (isPacked) {
                    return "MetaFFICallablePackedArray";
                }
                if (isArray) {
                    throw new CompilerException("callable arrays are not supported in JVM MetaFFITypeInfo");
                }
                return "MetaFFICallable";
            case "null":
                if (isArray) {
                    throw new CompilerException("null arrays are not supported in JVM MetaFFITypeInfo");
                }
                return "MetaFFINull";
            default:
                throw new CompilerException("Unsupported MetaFFI type: " + baseType);
        }
    }

    /**
     * Returns true if the given base type has a packed array variant.
     * Packed arrays are supported for numeric types, bool, string8, handle, and callable.
     */
    private boolean isPackablePrimitive(String baseType) {
        switch (baseType) {
            case "float64":
            case "float32":
            case "int8":
            case "int16":
            case "int32":
            case "int64":
            case "uint8":
            case "uint16":
            case "uint32":
            case "uint64":
            case "bool":
            case "string8":
            case "handle":
            case "callable":
                return true;
            default:
                return false;
        }
    }

    private String escapeJava(String value) {
        return value.replace("\\", "\\\\").replace("\"", "\\\"");
    }
}
