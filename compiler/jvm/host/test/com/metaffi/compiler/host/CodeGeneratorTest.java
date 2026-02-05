package com.metaffi.compiler.host;

import com.metaffi.idl.entities.ArgDefinition;
import com.metaffi.idl.entities.FunctionDefinition;
import com.metaffi.idl.entities.IDLDefinition;
import com.metaffi.idl.entities.ModuleDefinition;
import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

public class CodeGeneratorTest {

    @Test
    public void testMultiReturnWrapperAndOptionalOverload() {
        IDLDefinition def = new IDLDefinition();
        def.setIdlFilenameWithExtension("test.idl.json");
        def.setMetaffiGuestLib("guest_test");
        def.setTargetLanguage("cpp");

        ModuleDefinition module = new ModuleDefinition();
        module.setName("test");
        module.setFunctions(new ArrayList<>());

        FunctionDefinition multi = new FunctionDefinition();
        multi.setName("return_two_values");
        multi.setEntityPath(new HashMap<>());
        multi.getEntityPath().put("entity_path", "test::return_two_values");

        ArgDefinition rv1 = new ArgDefinition();
        rv1.setName("num");
        rv1.setType("int64");
        ArgDefinition rv2 = new ArgDefinition();
        rv2.setName("text");
        rv2.setType("string8");
        multi.setReturnValues(Arrays.asList(rv1, rv2));

        FunctionDefinition opt = new FunctionDefinition();
        opt.setName("opt");
        opt.setEntityPath(new HashMap<>());
        opt.getEntityPath().put("entity_path", "test::opt");

        ArgDefinition p1 = new ArgDefinition();
        p1.setName("a");
        p1.setType("int32");
        ArgDefinition p2 = new ArgDefinition();
        p2.setName("b");
        p2.setType("string8");
        p2.setOptional(true);
        opt.setParameters(Arrays.asList(p1, p2));

        ArgDefinition r = new ArgDefinition();
        r.setName("result");
        r.setType("int32");
        opt.setReturnValues(List.of(r));

        module.getFunctions().add(multi);
        module.getFunctions().add(opt);

        def.setModules(List.of(module));

        CodeGenerator generator = new CodeGenerator(new TypeInfoGenerator());
        String code = generator.generateModuleCode(module, def, "test", "Host");

        Assert.assertTrue(code.contains("public static class return_two_valuesResult"));
        Assert.assertTrue(code.contains("public final long num;"));
        Assert.assertTrue(code.contains("public final String text;"));

        Assert.assertTrue(code.contains("public static int opt(int a, String b)"));
        Assert.assertTrue(code.contains("public static int opt(int a)"));
        Assert.assertTrue(code.contains("return opt(a, null);"));
    }
}
