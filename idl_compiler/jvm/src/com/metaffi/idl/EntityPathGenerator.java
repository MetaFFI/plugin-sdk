package com.metaffi.idl;

import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Generates entity_path strings for JVM runtime.
 * Entity paths are comma-separated key=value pairs matching sdk/idl_entities/entity_path_specs.json.
 * 
 * Format examples:
 * - Method: "class=com.example.MyClass,callable=myMethod,instance_required"
 * - Static method: "class=com.example.MyClass,callable=staticMethod"
 * - Constructor: "class=com.example.MyClass,callable=<init>"
 * - Field getter: "class=com.example.MyClass,field=myField,getter,instance_required"
 * - Field setter: "class=com.example.MyClass,field=myField,setter,instance_required"
 */
public class EntityPathGenerator {

    /**
     * Generate entity_path for a method.
     *
     * @param className Fully qualified class name (e.g., "com.example.MyClass")
     * @param methodName Method name
     * @param instanceRequired true for instance methods, false for static methods
     * @return Comma-separated entity_path string
     */
    public static String createMethodPath(String className, String methodName, boolean instanceRequired) {
        Map<String, String> path = new LinkedHashMap<>();
        path.put("class", className);
        path.put("callable", methodName);

        if (instanceRequired) {
            path.put("instance_required", "");  // Flag with empty value
        }

        return pathToString(path);
    }

    /**
     * Generate entity_path for a constructor.
     *
     * @param className Fully qualified class name
     * @return Comma-separated entity_path string
     */
    public static String createConstructorPath(String className) {
        Map<String, String> path = new LinkedHashMap<>();
        path.put("class", className);
        path.put("callable", "<init>");
        return pathToString(path);
    }

    /**
     * Generate entity_path for a field getter.
     *
     * @param className Fully qualified class name
     * @param fieldName Field name
     * @param instanceRequired true for instance fields, false for static fields
     * @return Comma-separated entity_path string
     */
    public static String createFieldGetterPath(String className, String fieldName, boolean instanceRequired) {
        Map<String, String> path = new LinkedHashMap<>();
        path.put("class", className);
        path.put("field", fieldName);
        path.put("getter", "");  // Flag

        if (instanceRequired) {
            path.put("instance_required", "");  // Flag
        }

        return pathToString(path);
    }

    /**
     * Generate entity_path for a field setter.
     *
     * @param className Fully qualified class name
     * @param fieldName Field name
     * @param instanceRequired true for instance fields, false for static fields
     * @return Comma-separated entity_path string
     */
    public static String createFieldSetterPath(String className, String fieldName, boolean instanceRequired) {
        Map<String, String> path = new LinkedHashMap<>();
        path.put("class", className);
        path.put("field", fieldName);
        path.put("setter", "");  // Flag

        if (instanceRequired) {
            path.put("instance_required", "");  // Flag
        }

        return pathToString(path);
    }

    /**
     * Convert entity_path map to comma-separated string.
     * Format: key1=value1,key2=value2,flag1,flag2
     * Flags (empty values) are appended without = sign.
     *
     * @param path Map of key-value pairs
     * @return Comma-separated string
     */
    public static String pathToString(Map<String, String> path) {
        StringBuilder sb = new StringBuilder();
        boolean first = true;

        // First, add key=value pairs
        for (Map.Entry<String, String> entry : path.entrySet()) {
            if (!entry.getValue().isEmpty()) {
                if (!first) {
                    sb.append(',');
                }
                sb.append(entry.getKey()).append('=').append(entry.getValue());
                first = false;
            }
        }

        // Then, add flags (empty values) without = sign
        for (Map.Entry<String, String> entry : path.entrySet()) {
            if (entry.getValue().isEmpty()) {
                if (!first) {
                    sb.append(',');
                }
                sb.append(entry.getKey());
                first = false;
            }
        }

        return sb.toString();
    }
}
