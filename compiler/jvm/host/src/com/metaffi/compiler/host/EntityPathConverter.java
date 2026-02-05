package com.metaffi.compiler.host;

import com.metaffi.idl.entities.FunctionDefinition;
import com.metaffi.idl.entities.IDLDefinition;
import com.metaffi.idl.entities.MethodDefinition;
import com.metaffi.idl.entities.ConstructorDefinition;
import com.metaffi.idl.entities.GlobalDefinition;
import com.metaffi.idl.entities.FieldDefinition;

import java.util.Map;
import java.util.Set;
import java.util.TreeSet;

public final class EntityPathConverter {
    private EntityPathConverter() {}

    public static String functionEntityPathToString(FunctionDefinition func, IDLDefinition idlDefinition) {
        if (func == null) {
            throw new CompilerException("FunctionDefinition is required");
        }
        return entityPathToString(func.getEntityPath(), idlDefinition, null, "function", func.getName());
    }

    public static String methodEntityPathToString(MethodDefinition method, IDLDefinition idlDefinition, Map<String, String> parentEntityPath) {
        if (method == null) {
            throw new CompilerException("MethodDefinition is required");
        }
        return entityPathToString(method.getEntityPath(), idlDefinition, parentEntityPath, "method", method.getName());
    }

    public static String constructorEntityPathToString(ConstructorDefinition ctor, IDLDefinition idlDefinition) {
        if (ctor == null) {
            throw new CompilerException("ConstructorDefinition is required");
        }
        return entityPathToString(ctor.getEntityPath(), idlDefinition, null, "constructor", ctor.getName());
    }

    public static String globalEntityPathToString(GlobalDefinition globalDef, IDLDefinition idlDefinition, boolean useGetter) {
        if (globalDef == null) {
            throw new CompilerException("GlobalDefinition is required");
        }
        if (useGetter) {
            if (globalDef.getGetter() == null) {
                throw new CompilerException("Global " + globalDef.getName() + " has no getter");
            }
            return functionEntityPathToString(globalDef.getGetter(), idlDefinition);
        }
        if (globalDef.getSetter() == null) {
            throw new CompilerException("Global " + globalDef.getName() + " has no setter");
        }
        return functionEntityPathToString(globalDef.getSetter(), idlDefinition);
    }

    public static String fieldEntityPathToString(FieldDefinition field, IDLDefinition idlDefinition, Map<String, String> parentEntityPath, boolean useGetter) {
        if (field == null) {
            throw new CompilerException("FieldDefinition is required");
        }
        if (useGetter) {
            if (field.getGetter() == null) {
                throw new CompilerException("Field " + field.getName() + " has no getter");
            }
            return methodEntityPathToString(field.getGetter(), idlDefinition, parentEntityPath);
        }
        if (field.getSetter() == null) {
            throw new CompilerException("Field " + field.getName() + " has no setter");
        }
        return methodEntityPathToString(field.getSetter(), idlDefinition, parentEntityPath);
    }

    private static String entityPathToString(
        Map<String, String> entityPath,
        IDLDefinition idlDefinition,
        Map<String, String> parentEntityPath,
        String entityKind,
        String entityName) {

        if (idlDefinition == null) {
            throw new CompilerException("IDLDefinition is required");
        }
        if (entityPath == null || entityPath.isEmpty()) {
            throw new CompilerException("" + entityKind + " " + entityName + " missing entity_path");
        }

        if (entityPath.containsKey("entity_path")) {
            String raw = entityPath.get("entity_path");
            if (raw == null || raw.isEmpty()) {
                throw new CompilerException("" + entityKind + " " + entityName + " has empty entity_path");
            }
            return raw;
        }

        Set<String> keys = new TreeSet<>();
        keys.addAll(entityPath.keySet());
        if (parentEntityPath != null) {
            keys.addAll(parentEntityPath.keySet());
        }
        keys.add("metaffi_guest_lib");

        StringBuilder sb = new StringBuilder();
        boolean first = true;
        for (String key : keys) {
            String value = entityPath.get(key);
            if (value == null && parentEntityPath != null) {
                value = parentEntityPath.get(key);
            }
            if (value == null) {
                if ("metaffi_guest_lib".equals(key)) {
                    value = idlDefinition.getMetaffiGuestLib();
                } else {
                    throw new CompilerException("Missing entity_path key " + key + " for " + entityKind + " " + entityName);
                }
            }

            if (!first) {
                sb.append(',');
            }
            sb.append(key).append('=').append(value);
            first = false;
        }

        return sb.toString();
    }
}
