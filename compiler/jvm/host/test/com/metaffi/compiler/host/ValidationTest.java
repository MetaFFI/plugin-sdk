package com.metaffi.compiler.host;

import com.metaffi.idl.entities.IDLDefinition;
import org.junit.Test;

import java.util.HashMap;

public class ValidationTest {

    @Test(expected = CompilerException.class)
    public void testMissingTargetLanguageFails() {
        IDLDefinition def = new IDLDefinition();
        def.setTargetLanguage("");
        def.setMetaffiGuestLib("guest");
        def.setModules(new java.util.ArrayList<>());

        HostCompiler compiler = new HostCompiler(CompilerContext.defaultContext());
        compiler.compile(def, "build", "Host", new HashMap<>());
    }
}
