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
        if (type.endsWith("_array")) {
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

    private String mapToMetaFFITypeEnum(String baseType, int dims) {
        boolean isArray = dims > 0;

        switch (baseType) {
            case "float64": return isArray ? "MetaFFIFloat64Array" : "MetaFFIFloat64";
            case "float32": return isArray ? "MetaFFIFloat32Array" : "MetaFFIFloat32";
            case "int8": return isArray ? "MetaFFIInt8Array" : "MetaFFIInt8";
            case "int16": return isArray ? "MetaFFIInt16Array" : "MetaFFIInt16";
            case "int32": return isArray ? "MetaFFIInt32Array" : "MetaFFIInt32";
            case "int64": return isArray ? "MetaFFIInt64Array" : "MetaFFIInt64";
            case "uint8": return isArray ? "MetaFFIUInt8Array" : "MetaFFIUInt8";
            case "uint16": return isArray ? "MetaFFIUInt16Array" : "MetaFFIUInt16";
            case "uint32": return isArray ? "MetaFFIUInt32Array" : "MetaFFIUInt32";
            case "uint64": return isArray ? "MetaFFIUInt64Array" : "MetaFFIUInt64";
            case "bool": return isArray ? "MetaFFIBoolArray" : "MetaFFIBool";
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
            case "string8": return isArray ? "MetaFFIString8Array" : "MetaFFIString8";
            case "string16": return isArray ? "MetaFFIString16Array" : "MetaFFIString16";
            case "string32": return isArray ? "MetaFFIString32Array" : "MetaFFIString32";
            case "handle": return isArray ? "MetaFFIHandleArray" : "MetaFFIHandle";
            case "any": return isArray ? "MetaFFIAnyArray" : "MetaFFIAny";
            case "size": return isArray ? "MetaFFISizeArray" : "MetaFFISize";
            case "callable":
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

    private String escapeJava(String value) {
        return value.replace("\\", "\\\\").replace("\"", "\\\"");
    }
}
