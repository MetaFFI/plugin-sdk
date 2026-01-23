package com.metaffi.idl.entities;

public class GlobalDefinition extends ArgDefinition {
    private FunctionDefinition getter;
    private FunctionDefinition setter;

    public FunctionDefinition getGetter() {
        return getter;
    }

    public void setGetter(FunctionDefinition getter) {
        this.getter = getter;
    }

    public FunctionDefinition getSetter() {
        return setter;
    }

    public void setSetter(FunctionDefinition setter) {
        this.setter = setter;
    }
}
