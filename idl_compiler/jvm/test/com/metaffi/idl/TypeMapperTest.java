package com.metaffi.idl;

import org.junit.Test;
import static org.junit.Assert.*;

/**
 * Tests for TypeMapper - verifies correct mapping of Java types to MetaFFI types.
 * All MetaFFI types must be lowercase.
 */
public class TypeMapperTest {

    @Test
    public void testPrimitiveIntegerTypes() {
        // byte → int8
        TypeMapper.TypeInfo byteInfo = TypeMapper.mapType("B");
        assertEquals("int8", byteInfo.metaffiType);
        assertEquals(0, byteInfo.dimensions);

        // short → int16
        TypeMapper.TypeInfo shortInfo = TypeMapper.mapType("S");
        assertEquals("int16", shortInfo.metaffiType);
        assertEquals(0, shortInfo.dimensions);

        // int → int32
        TypeMapper.TypeInfo intInfo = TypeMapper.mapType("I");
        assertEquals("int32", intInfo.metaffiType);
        assertEquals(0, intInfo.dimensions);

        // long → int64
        TypeMapper.TypeInfo longInfo = TypeMapper.mapType("J");
        assertEquals("int64", longInfo.metaffiType);
        assertEquals(0, longInfo.dimensions);
    }

    @Test
    public void testFloatingPointTypes() {
        // float → float32
        TypeMapper.TypeInfo floatInfo = TypeMapper.mapType("F");
        assertEquals("float32", floatInfo.metaffiType);
        assertEquals(0, floatInfo.dimensions);

        // double → float64
        TypeMapper.TypeInfo doubleInfo = TypeMapper.mapType("D");
        assertEquals("float64", doubleInfo.metaffiType);
        assertEquals(0, doubleInfo.dimensions);
    }

    @Test
    public void testBooleanType() {
        TypeMapper.TypeInfo boolInfo = TypeMapper.mapType("Z");
        assertEquals("bool", boolInfo.metaffiType);
        assertEquals(0, boolInfo.dimensions);
    }

    @Test
    public void testCharType() {
        TypeMapper.TypeInfo charInfo = TypeMapper.mapType("C");
        assertEquals("char16", charInfo.metaffiType);
        assertEquals(0, charInfo.dimensions);
    }

    @Test
    public void testVoidType() {
        TypeMapper.TypeInfo voidInfo = TypeMapper.mapType("V");
        assertEquals("null", voidInfo.metaffiType);
        assertEquals(0, voidInfo.dimensions);
    }

    @Test
    public void testStringType() {
        TypeMapper.TypeInfo stringInfo = TypeMapper.mapType("Ljava/lang/String;");
        assertEquals("string8", stringInfo.metaffiType);
        assertEquals(0, stringInfo.dimensions);
    }

    @Test
    public void testArrayTypes() {
        // int[] → int32 with dimensions=1
        TypeMapper.TypeInfo intArrayInfo = TypeMapper.mapType("[I");
        assertEquals("int32", intArrayInfo.metaffiType);
        assertEquals(1, intArrayInfo.dimensions);

        // int[][] → int32 with dimensions=2
        TypeMapper.TypeInfo int2DArrayInfo = TypeMapper.mapType("[[I");
        assertEquals("int32", int2DArrayInfo.metaffiType);
        assertEquals(2, int2DArrayInfo.dimensions);

        // String[] → string8 with dimensions=1
        TypeMapper.TypeInfo stringArrayInfo = TypeMapper.mapType("[Ljava/lang/String;");
        assertEquals("string8", stringArrayInfo.metaffiType);
        assertEquals(1, stringArrayInfo.dimensions);
    }

    @Test
    public void testObjectType() {
        // Any object type → handle
        TypeMapper.TypeInfo objectInfo = TypeMapper.mapType("Ljava/util/List;");
        assertEquals("handle", objectInfo.metaffiType);
        assertEquals(0, objectInfo.dimensions);
    }

    @Test
    public void testLowercaseTypes() {
        // All MetaFFI types must be lowercase
        TypeMapper.TypeInfo intInfo = TypeMapper.mapType("I");
        assertEquals(intInfo.metaffiType, intInfo.metaffiType.toLowerCase());
        
        TypeMapper.TypeInfo stringInfo = TypeMapper.mapType("Ljava/lang/String;");
        assertEquals(stringInfo.metaffiType, stringInfo.metaffiType.toLowerCase());
    }

    @Test
    public void testMultiDimensionalArrays() {
        // int[][]
        TypeMapper.TypeInfo int2D = TypeMapper.mapType("[[I");
        assertEquals("int32", int2D.metaffiType);
        assertEquals(2, int2D.dimensions);

        // String[][][]
        TypeMapper.TypeInfo string3D = TypeMapper.mapType("[[[Ljava/lang/String;");
        assertEquals("string8", string3D.metaffiType);
        assertEquals(3, string3D.dimensions);

        // double[][]
        TypeMapper.TypeInfo double2D = TypeMapper.mapType("[[D");
        assertEquals("float64", double2D.metaffiType);
        assertEquals(2, double2D.dimensions);
    }

    @Test
    public void testGenericTypes() {
        // Map<String, Integer> → handle
        TypeMapper.TypeInfo mapType = TypeMapper.mapType("Ljava/util/Map;");
        assertEquals("handle", mapType.metaffiType);
        assertEquals(0, mapType.dimensions);

        // List<String> → handle
        TypeMapper.TypeInfo listType = TypeMapper.mapType("Ljava/util/List;");
        assertEquals("handle", listType.metaffiType);
        assertEquals(0, listType.dimensions);

        // Custom class → handle
        TypeMapper.TypeInfo customType = TypeMapper.mapType("Lcom/example/CustomClass;");
        assertEquals("handle", customType.metaffiType);
        assertEquals(0, customType.dimensions);
    }

    @Test
    public void testJavaLangObject() {
        // java.lang.Object → handle
        TypeMapper.TypeInfo objectType = TypeMapper.mapType("Ljava/lang/Object;");
        assertEquals("handle", objectType.metaffiType);
        assertEquals(0, objectType.dimensions);
    }

    @Test
    public void testArrayOfObjects() {
        // Object[] → handle with dimensions=1
        TypeMapper.TypeInfo objectArray = TypeMapper.mapType("[Ljava/lang/Object;");
        assertEquals("handle", objectArray.metaffiType);
        assertEquals(1, objectArray.dimensions);

        // CustomClass[] → handle with dimensions=1
        TypeMapper.TypeInfo customArray = TypeMapper.mapType("[Lcom/example/CustomClass;");
        assertEquals("handle", customArray.metaffiType);
        assertEquals(1, customArray.dimensions);
    }

    @Test
    public void testTypeAliasPreserved() {
        // Verify originalType is preserved in TypeInfo
        TypeMapper.TypeInfo intInfo = TypeMapper.mapType("I");
        assertNotNull(intInfo.originalType);
        assertEquals("I", intInfo.originalType);

        TypeMapper.TypeInfo stringInfo = TypeMapper.mapType("Ljava/lang/String;");
        assertNotNull(stringInfo.originalType);
        assertEquals("Ljava/lang/String;", stringInfo.originalType);
    }

    @Test
    public void testUnknownTypes() {
        // Any unrecognized type should map to handle without exception
        TypeMapper.TypeInfo unknown = TypeMapper.mapType("Lcom/unknown/UnknownType;");
        assertEquals("handle", unknown.metaffiType);
        assertEquals(0, unknown.dimensions);
    }

    @Test
    public void testAllPrimitiveTypes() {
        // Verify all 8 primitives are covered
        assertEquals("int8", TypeMapper.mapType("B").metaffiType);    // byte
        assertEquals("int16", TypeMapper.mapType("S").metaffiType);   // short
        assertEquals("int32", TypeMapper.mapType("I").metaffiType);    // int
        assertEquals("int64", TypeMapper.mapType("J").metaffiType);   // long
        assertEquals("float32", TypeMapper.mapType("F").metaffiType); // float
        assertEquals("float64", TypeMapper.mapType("D").metaffiType); // double
        assertEquals("bool", TypeMapper.mapType("Z").metaffiType);     // boolean
        assertEquals("char16", TypeMapper.mapType("C").metaffiType);  // char
    }
}
