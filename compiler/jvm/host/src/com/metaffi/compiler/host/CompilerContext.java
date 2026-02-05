package com.metaffi.compiler.host;

import com.google.gson.Gson;

public class CompilerContext {
    private final Gson gson;

    public CompilerContext(Gson gson) {
        if (gson == null) {
            throw new CompilerException("Gson is required and cannot be null");
        }
        this.gson = gson;
    }

    public Gson getGson() {
        return gson;
    }

    public static CompilerContext defaultContext() {
        return new CompilerContext(new Gson());
    }
}
