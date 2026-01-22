package com.metaffi.idl;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonNull;
import com.google.gson.JsonObject;
import com.metaffi.idl.model.ClassInfo;
import com.metaffi.idl.model.FieldInfo;
import com.metaffi.idl.model.MethodInfo;
import com.metaffi.idl.model.ModuleInfo;
import com.metaffi.idl.model.ParameterInfo;

import java.util.List;

/**
 * Generates MetaFFI IDL JSON from extracted module information.
 * Converts ModuleInfo objects to JSON format matching schema.json.
 */
public class IdlGenerator {
    private final List<ModuleInfo> modules;

    /**
     * Constructor.
     *
     * @param modules List of extracted modules
     */
    public IdlGenerator(List<ModuleInfo> modules) {
        this.modules = modules;
    }

    /**
     * Generate complete IDL as JSON string.
     *
     * @return JSON string
     */
    public String generateJson() {
        JsonObject root = new JsonObject();

        // Determine idl_source from first module or use default
        String idlSource = modules.isEmpty() ? "jvm" : modules.get(0).getModuleName();
        
        // Top-level metadata
        root.addProperty("idl_source", idlSource);
        root.addProperty("idl_extension", ".jar");  // Default for JARs
        root.addProperty("idl_filename_with_extension", idlSource + ".json");
        root.addProperty("idl_full_path", "");  // Will be set by runtime
        root.addProperty("metaffi_guest_lib", idlSource + "_MetaFFIGuest");
        root.addProperty("target_language", "jvm");

        // Generate modules array
        JsonArray modulesArray = new JsonArray();
        for (ModuleInfo module : modules) {
            JsonObject moduleObj = generateModule(module);
            modulesArray.add(moduleObj);
        }
        root.add("modules", modulesArray);

        // Convert to JSON string with pretty printing
        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        return gson.toJson(root);
    }

    /**
     * Generate module definition.
     */
    private JsonObject generateModule(ModuleInfo module) {
        JsonObject moduleObj = new JsonObject();

        moduleObj.addProperty("name", module.getModuleName());
        moduleObj.addProperty("comment", "Generated from " + module.getSourcePath());
        moduleObj.add("tags", new JsonObject());
        moduleObj.add("functions", new JsonArray());  // No module-level functions in Java
        moduleObj.add("globals", new JsonArray());     // No module-level globals in Java

        // Generate classes
        JsonArray classesArray = new JsonArray();
        for (ClassInfo classInfo : module.getClasses()) {
            JsonObject classObj = generateClass(classInfo);
            classesArray.add(classObj);
        }
        moduleObj.add("classes", classesArray);

        // Add external_resources
        JsonArray externalResourcesArray = new JsonArray();
        for (String resource : module.getExternalResources()) {
            externalResourcesArray.add(resource);
        }
        moduleObj.add("external_resources", externalResourcesArray);

        return moduleObj;
    }

    /**
     * Generate class definition.
     */
    private JsonObject generateClass(ClassInfo classInfo) {
        JsonObject classObj = new JsonObject();

        classObj.addProperty("name", classInfo.getSimpleName());
        classObj.addProperty("comment", classInfo.getComment() != null ? classInfo.getComment() : "");
        classObj.add("tags", new JsonObject());

        // Class-level entity_path (empty for Java - routing happens at method/field level)
        classObj.addProperty("entity_path", "");

        // Generate constructors
        JsonArray constructorsArray = new JsonArray();
        for (MethodInfo constructor : classInfo.getConstructors()) {
            JsonObject ctorObj = generateConstructor(constructor, classInfo);
            constructorsArray.add(ctorObj);
        }
        classObj.add("constructors", constructorsArray);

        // Generate release method (destructor wrapper) - Java uses GC, so null
        classObj.add("release", JsonNull.INSTANCE);

        // Generate methods
        JsonArray methodsArray = new JsonArray();
        for (MethodInfo method : classInfo.getMethods()) {
            JsonObject methodObj = generateMethod(method, classInfo);
            methodsArray.add(methodObj);
        }
        classObj.add("methods", methodsArray);

        // Generate fields (as getter/setter pairs)
        JsonArray fieldsArray = new JsonArray();
        for (FieldInfo field : classInfo.getFields()) {
            JsonObject fieldObj = generateField(field, classInfo);
            fieldsArray.add(fieldObj);
        }
        classObj.add("fields", fieldsArray);

        return classObj;
    }

    /**
     * Generate constructor definition.
     */
    private JsonObject generateConstructor(MethodInfo constructor, ClassInfo classInfo) {
        JsonObject ctorObj = new JsonObject();

        ctorObj.addProperty("comment", constructor.getComment() != null ? constructor.getComment() : "");
        ctorObj.add("tags", new JsonObject());

        // Entity path
        String entityPath = EntityPathGenerator.createConstructorPath(classInfo.getClassName());
        ctorObj.addProperty("entity_path", entityPath);

        // Parameters
        JsonArray paramsArray = new JsonArray();
        for (ParameterInfo param : constructor.getParameters()) {
            JsonObject paramObj = generateParameter(param);
            paramsArray.add(paramObj);
        }
        ctorObj.add("parameters", paramsArray);

        // Return value (the constructed object)
        JsonArray returnsArray = new JsonArray();
        JsonObject returnObj = new JsonObject();
        returnObj.addProperty("name", "ret_0");
        returnObj.addProperty("type", "handle");  // Object instance
        returnObj.addProperty("type_alias", classInfo.getClassName());
        returnObj.addProperty("comment", "");
        returnObj.add("tags", new JsonObject());
        returnObj.addProperty("dimensions", 0);
        returnsArray.add(returnObj);
        ctorObj.add("return_values", returnsArray);

        ctorObj.addProperty("overload_index", 0);  // TODO: Handle overloading

        return ctorObj;
    }

    /**
     * Generate method definition.
     */
    private JsonObject generateMethod(MethodInfo method, ClassInfo classInfo) {
        JsonObject methodObj = new JsonObject();

        methodObj.addProperty("name", method.getName());
        methodObj.addProperty("comment", method.getComment() != null ? method.getComment() : "");
        methodObj.add("tags", new JsonObject());

        // Entity path
        String entityPath = EntityPathGenerator.createMethodPath(
            classInfo.getClassName(), method.getName(), !method.isStatic());
        methodObj.addProperty("entity_path", entityPath);

        // Parameters
        JsonArray paramsArray = new JsonArray();
        for (ParameterInfo param : method.getParameters()) {
            JsonObject paramObj = generateParameter(param);
            paramsArray.add(paramObj);
        }
        methodObj.add("parameters", paramsArray);

        // Return values
        JsonArray returnsArray = new JsonArray();
        if (!"void".equals(method.getReturnType())) {
            JsonObject returnObj = new JsonObject();
            returnObj.addProperty("name", "ret_0");
            returnObj.addProperty("type", method.getMetaffiReturnType());
            returnObj.addProperty("type_alias", method.getReturnType());
            returnObj.addProperty("comment", "");
            returnObj.add("tags", new JsonObject());
            returnObj.addProperty("dimensions", method.getReturnDimensions());
            returnsArray.add(returnObj);
        }
        methodObj.add("return_values", returnsArray);

        methodObj.addProperty("overload_index", 0);  // TODO: Handle overloading

        return methodObj;
    }

    /**
     * Generate field definition with getter and setter.
     */
    private JsonObject generateField(FieldInfo field, ClassInfo classInfo) {
        JsonObject fieldObj = new JsonObject();

        fieldObj.addProperty("name", field.getName());
        fieldObj.addProperty("type", field.getMetaffiType());
        fieldObj.addProperty("type_alias", field.getJavaType());
        fieldObj.addProperty("comment", "");
        fieldObj.add("tags", new JsonObject());
        fieldObj.addProperty("dimensions", field.getDimensions());
        fieldObj.addProperty("is_optional", false);

        // Getter
        JsonObject getterObj = generateFieldGetter(field, classInfo);
        fieldObj.add("getter", getterObj);

        // Setter
        JsonObject setterObj = generateFieldSetter(field, classInfo);
        fieldObj.add("setter", setterObj);

        return fieldObj;
    }

    /**
     * Generate field getter method.
     */
    private JsonObject generateFieldGetter(FieldInfo field, ClassInfo classInfo) {
        JsonObject getterObj = new JsonObject();

        getterObj.addProperty("name", "get" + capitalize(field.getName()));
        getterObj.addProperty("comment", "");
        getterObj.add("tags", new JsonObject());

        // Entity path
        String entityPath = EntityPathGenerator.createFieldGetterPath(
            classInfo.getClassName(), field.getName(), !field.isStatic());
        getterObj.addProperty("entity_path", entityPath);

        // Parameters (empty for getters)
        getterObj.add("parameters", new JsonArray());

        // Return value
        JsonArray returnsArray = new JsonArray();
        JsonObject returnObj = new JsonObject();
        returnObj.addProperty("name", "ret_0");
        returnObj.addProperty("type", field.getMetaffiType());
        returnObj.addProperty("type_alias", field.getJavaType());
        returnObj.addProperty("comment", "");
        returnObj.add("tags", new JsonObject());
        returnObj.addProperty("dimensions", field.getDimensions());
        returnsArray.add(returnObj);
        getterObj.add("return_values", returnsArray);

        getterObj.addProperty("overload_index", 0);
        getterObj.addProperty("instance_required", !field.isStatic());

        return getterObj;
    }

    /**
     * Generate field setter method.
     */
    private JsonObject generateFieldSetter(FieldInfo field, ClassInfo classInfo) {
        JsonObject setterObj = new JsonObject();

        setterObj.addProperty("name", "set" + capitalize(field.getName()));
        setterObj.addProperty("comment", "");
        setterObj.add("tags", new JsonObject());

        // Entity path
        String entityPath = EntityPathGenerator.createFieldSetterPath(
            classInfo.getClassName(), field.getName(), !field.isStatic());
        setterObj.addProperty("entity_path", entityPath);

        // Parameters (value parameter)
        JsonArray paramsArray = new JsonArray();
        ParameterInfo valueParam = new ParameterInfo("value", field.getJavaType(),
            field.getMetaffiType(), field.getDimensions());
        JsonObject paramObj = generateParameter(valueParam);
        paramsArray.add(paramObj);
        setterObj.add("parameters", paramsArray);

        // No return value
        setterObj.add("return_values", new JsonArray());

        setterObj.addProperty("overload_index", 0);
        setterObj.addProperty("instance_required", !field.isStatic());

        return setterObj;
    }

    /**
     * Generate parameter definition.
     */
    private JsonObject generateParameter(ParameterInfo param) {
        JsonObject paramObj = new JsonObject();

        paramObj.addProperty("name", param.getName());
        paramObj.addProperty("type", param.getMetaffiType());
        paramObj.addProperty("type_alias", param.getJavaType());
        paramObj.addProperty("comment", "");
        paramObj.add("tags", new JsonObject());
        paramObj.addProperty("dimensions", param.getDimensions());
        paramObj.addProperty("is_optional", false);

        return paramObj;
    }

    /**
     * Capitalize first letter of string.
     */
    private String capitalize(String str) {
        if (str == null || str.isEmpty()) {
            return str;
        }
        return Character.toUpperCase(str.charAt(0)) + str.substring(1);
    }
}
