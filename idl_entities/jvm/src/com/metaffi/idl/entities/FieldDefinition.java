package com.metaffi.idl.entities;

public class FieldDefinition extends ArgDefinition {
    private MethodDefinition getter;
    private MethodDefinition setter;

    public MethodDefinition getGetter() {
        return getter;
    }

    public void setGetter(MethodDefinition getter) {
        this.getter = getter;
    }

    public MethodDefinition getSetter() {
        return setter;
    }

    public void setSetter(MethodDefinition setter) {
        this.setter = setter;
    }
}
