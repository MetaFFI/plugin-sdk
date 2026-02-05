package com.metaffi.compiler.host;

import com.metaffi.idl.entities.ArgDefinition;
import org.junit.Assert;
import org.junit.Test;

public class TypeInfoGeneratorTest {

    @Test
    public void testArrayTypeInfoGeneration() {
        ArgDefinition arg = new ArgDefinition();
        arg.setName("arr");
        arg.setType("int64_array");
        arg.setDimensions(2);

        TypeInfoGenerator gen = new TypeInfoGenerator();
        String result = gen.generateTypeInfo(arg);

        Assert.assertEquals("new MetaFFITypeInfo(MetaFFITypes.MetaFFIInt64Array, 2)", result);
    }

    @Test
    public void testHandleAliasTypeInfoGeneration() {
        ArgDefinition arg = new ArgDefinition();
        arg.setName("h");
        arg.setType("handle");
        arg.setTypeAlias("MyType");

        TypeInfoGenerator gen = new TypeInfoGenerator();
        String result = gen.generateTypeInfo(arg);

        Assert.assertEquals("new MetaFFITypeInfo(MetaFFITypes.MetaFFIHandle, \"MyType\")", result);
    }
}
