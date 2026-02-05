package com.metaffi.idl;

import com.google.gson.JsonObject;

/**
 * Generates entity_path JSON objects for JVM runtime.
 * Entity paths are JSON objects matching sdk/idl_entities/entity_path_specs.json schema.
 *
 * Format examples:
 * - Method: {"class": "com.example.MyClass", "callable": "myMethod", "instance_required": "true"}
 * - Static method: {"class": "com.example.MyClass", "callable": "staticMethod"}
 * - Constructor: {"class": "com.example.MyClass", "callable": "<init>"}
 * - Field getter: {"class": "com.example.MyClass", "field": "myField", "getter": "true", "instance_required": "true"}
 * - Field setter: {"class": "com.example.MyClass", "field": "myField", "setter": "true", "instance_required": "true"}
 */
public class EntityPathGenerator {

    /**
     * Generate entity_path for a method.
     *
     * @param className Fully qualified class name (e.g., "com.example.MyClass")
     * @param methodName Method name
     * @param instanceRequired true for instance methods, false for static methods
     * @return JsonObject representing entity_path
     */
    public static JsonObject createMethodPath(String className, String methodName, boolean instanceRequired) {
        JsonObject path = new JsonObject();
        path.addProperty("class", className);
        path.addProperty("callable", methodName);

        if (instanceRequired) {
            path.addProperty("instance_required", "true");
        }

        return path;
    }

    /**
     * Generate entity_path for a constructor.
     *
     * @param className Fully qualified class name
     * @return JsonObject representing entity_path
     */
    public static JsonObject createConstructorPath(String className) {
        JsonObject path = new JsonObject();
        path.addProperty("class", className);
        path.addProperty("callable", "<init>");
        return path;
    }

    /**
     * Generate entity_path for a field getter.
     *
     * @param className Fully qualified class name
     * @param fieldName Field name
     * @param instanceRequired true for instance fields, false for static fields
     * @return JsonObject representing entity_path
     */
    public static JsonObject createFieldGetterPath(String className, String fieldName, boolean instanceRequired) {
        JsonObject path = new JsonObject();
        path.addProperty("class", className);
        path.addProperty("field", fieldName);
        path.addProperty("getter", "true");

        if (instanceRequired) {
            path.addProperty("instance_required", "true");
        }

        return path;
    }

    /**
     * Generate entity_path for a field setter.
     *
     * @param className Fully qualified class name
     * @param fieldName Field name
     * @param instanceRequired true for instance fields, false for static fields
     * @return JsonObject representing entity_path
     */
    public static JsonObject createFieldSetterPath(String className, String fieldName, boolean instanceRequired) {
        JsonObject path = new JsonObject();
        path.addProperty("class", className);
        path.addProperty("field", fieldName);
        path.addProperty("setter", "true");

        if (instanceRequired) {
            path.addProperty("instance_required", "true");
        }

        return path;
    }
}
