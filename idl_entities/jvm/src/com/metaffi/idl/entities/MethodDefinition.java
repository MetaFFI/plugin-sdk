package com.metaffi.idl.entities;

public class MethodDefinition extends FunctionDefinition {
    private boolean instance_required = true;

    public boolean isInstanceRequired() {
        return instance_required;
    }

    public void setInstanceRequired(boolean instanceRequired) {
        this.instance_required = instanceRequired;
    }
}
