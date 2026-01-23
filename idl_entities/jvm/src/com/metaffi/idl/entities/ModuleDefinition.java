package com.metaffi.idl.entities;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ModuleDefinition {
    private String name;
    private String comment;
    private Map<String, String> tags;
    private List<FunctionDefinition> functions;
    private List<ClassDefinition> classes;
    private List<GlobalDefinition> globals;
    private List<String> external_resources;

    public ModuleDefinition() {
        this.tags = new HashMap<>();
        this.functions = new ArrayList<>();
        this.classes = new ArrayList<>();
        this.globals = new ArrayList<>();
        this.external_resources = new ArrayList<>();
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getComment() {
        return comment;
    }

    public void setComment(String comment) {
        this.comment = comment;
    }

    public Map<String, String> getTags() {
        return tags;
    }

    public void setTags(Map<String, String> tags) {
        this.tags = tags;
    }

    public List<FunctionDefinition> getFunctions() {
        return functions;
    }

    public void setFunctions(List<FunctionDefinition> functions) {
        this.functions = functions;
    }

    public List<ClassDefinition> getClasses() {
        return classes;
    }

    public void setClasses(List<ClassDefinition> classes) {
        this.classes = classes;
    }

    public List<GlobalDefinition> getGlobals() {
        return globals;
    }

    public void setGlobals(List<GlobalDefinition> globals) {
        this.globals = globals;
    }

    public List<String> getExternalResources() {
        return external_resources;
    }

    public void setExternalResources(List<String> externalResources) {
        this.external_resources = externalResources;
    }
}
