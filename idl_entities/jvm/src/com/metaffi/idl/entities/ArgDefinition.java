package com.metaffi.idl.entities;

import java.util.HashMap;
import java.util.Map;

public class ArgDefinition {
    private String name;
    private String type;
    private String type_alias;
    private String comment;
    private Map<String, String> tags;
    private int dimensions;
    private boolean is_optional;

    public ArgDefinition() {
        this.tags = new HashMap<>();
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getTypeAlias() {
        return type_alias;
    }

    public void setTypeAlias(String typeAlias) {
        this.type_alias = typeAlias;
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

    public int getDimensions() {
        return dimensions;
    }

    public void setDimensions(int dimensions) {
        this.dimensions = dimensions;
    }

    public boolean isOptional() {
        return is_optional;
    }

    public void setOptional(boolean optional) {
        this.is_optional = optional;
    }
}
