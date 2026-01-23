package com.metaffi.idl.entities;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class FunctionDefinition {
    private String name;
    private String comment;
    private Map<String, String> tags;
    private Map<String, String> entity_path;
    private List<ArgDefinition> parameters;
    private List<ArgDefinition> return_values;
    private int overload_index;

    public FunctionDefinition() {
        this.tags = new HashMap<>();
        this.entity_path = new HashMap<>();
        this.parameters = new ArrayList<>();
        this.return_values = new ArrayList<>();
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

    public List<ArgDefinition> getParameters() {
        return parameters;
    }

    public void setParameters(List<ArgDefinition> parameters) {
        this.parameters = parameters;
    }

    public List<ArgDefinition> getReturnValues() {
        return return_values;
    }

    public void setReturnValues(List<ArgDefinition> returnValues) {
        this.return_values = returnValues;
    }

    public int getOverloadIndex() {
        return overload_index;
    }

    public void setOverloadIndex(int overloadIndex) {
        this.overload_index = overloadIndex;
    }
}
