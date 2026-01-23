package com.metaffi.idl.entities;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ClassDefinition {
    private String name;
    private String comment;
    private Map<String, String> tags;
    private Map<String, String> entity_path;
    private List<ConstructorDefinition> constructors;
    private ReleaseDefinition release;
    private List<MethodDefinition> methods;
    private List<FieldDefinition> fields;

    public ClassDefinition() {
        this.tags = new HashMap<>();
        this.entity_path = new HashMap<>();
        this.constructors = new ArrayList<>();
        this.methods = new ArrayList<>();
        this.fields = new ArrayList<>();
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

    public Map<String, String> getEntityPath() {
        return entity_path;
    }

    public void setEntityPath(Map<String, String> entityPath) {
        this.entity_path = entityPath;
    }

    public List<ConstructorDefinition> getConstructors() {
        return constructors;
    }

    public void setConstructors(List<ConstructorDefinition> constructors) {
        this.constructors = constructors;
    }

    public ReleaseDefinition getRelease() {
        return release;
    }

    public void setRelease(ReleaseDefinition release) {
        this.release = release;
    }

    public List<MethodDefinition> getMethods() {
        return methods;
    }

    public void setMethods(List<MethodDefinition> methods) {
        this.methods = methods;
    }

    public List<FieldDefinition> getFields() {
        return fields;
    }

    public void setFields(List<FieldDefinition> fields) {
        this.fields = fields;
    }
}
