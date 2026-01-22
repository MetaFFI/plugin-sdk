package guest.tests;

import guest.ArrayFunctions;
import guest.AutoCloseableResource;
import guest.Callbacks;
import guest.CollectionFunctions;
import guest.CoreFunctions;
import guest.EnumTypes;
import guest.Errors;
import guest.GenericBox;
import guest.Interfaces;
import guest.NestedTypes;
import guest.OptionalFunctions;
import guest.OverloadExamples;
import guest.PrimitiveFunctions;
import guest.SomeClass;
import guest.StaticState;
import guest.TestMap;
import guest.VarargsExamples;
import guest.sub.SubModule;

import java.io.IOException;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.function.IntBinaryOperator;

public class TestEntities {
	private static void assertTrue(boolean condition, String message) {
		if (!condition) {
			throw new AssertionError(message);
		}
	}

	private static void assertEquals(Object expected, Object actual, String message) {
		if (expected == null && actual == null) {
			return;
		}
		if (expected != null && expected.equals(actual)) {
			return;
		}
		throw new AssertionError(message + " expected=" + expected + " actual=" + actual);
	}

	public static void main(String[] args) throws Exception {
		assertTrue(CoreFunctions.helloWorld().contains("Hello"), "helloWorld");
		try {
			CoreFunctions.returnsAnError();
			throw new AssertionError("returnsAnError");
		} catch (Exception ignored) {
		}

		assertEquals(5.0, CoreFunctions.divIntegers(10, 2), "divIntegers");
		assertEquals("a,b,c", CoreFunctions.joinStrings(new String[]{"a", "b", "c"}), "joinStrings");
		assertTrue(CoreFunctions.returnNull() == null, "returnNull");

		IntBinaryOperator add = (a, b) -> a + b;
		assertEquals(3, CoreFunctions.callCallbackAdd(add), "callCallbackAdd");
		assertEquals(7, CoreFunctions.returnCallbackAdd().applyAsInt(3, 4), "returnCallbackAdd");

		Object[] multi = CoreFunctions.returnMultipleReturnValues();
		assertEquals(6, multi.length, "returnMultipleReturnValues length");
		assertTrue(multi[5] instanceof SomeClass, "returnMultipleReturnValues type");
		Object[] dims = CoreFunctions.returnsArrayWithDifferentDimensions();
		assertTrue(dims[0] instanceof int[], "returnsArrayWithDifferentDimensions 1d");
		assertTrue(dims[2] instanceof int[][], "returnsArrayWithDifferentDimensions 2d");
		Object[] diff = CoreFunctions.returnsArrayOfDifferentObjects();
		assertEquals(6, diff.length, "returnsArrayOfDifferentObjects length");

		CoreFunctions.acceptsAny(0, 1);
		CoreFunctions.acceptsAny(1, "string");
		CoreFunctions.acceptsAny(2, 3.0);
		CoreFunctions.acceptsAny(3, null);
		CoreFunctions.acceptsAny(4, new byte[]{1});
		CoreFunctions.acceptsAny(5, new SomeClass());

		byte[][] buffers = ArrayFunctions.getThreeBuffers();
		assertEquals(3, buffers.length, "getThreeBuffers");
		ArrayFunctions.expectThreeBuffers(buffers);
		SomeClass[] classes = ArrayFunctions.getSomeClasses();
		assertEquals(3, classes.length, "getSomeClasses");
		ArrayFunctions.expectThreeSomeClasses(classes);
		assertEquals(10, ArrayFunctions.sum3dArray(ArrayFunctions.make3dArray()), "sum3dArray");
		assertEquals(21, ArrayFunctions.sumRaggedArray(ArrayFunctions.makeRaggedArray()), "sumRaggedArray");

		Object[] prims = PrimitiveFunctions.acceptsPrimitives(true, (byte) 1, (short) 2, 3, 4L, 5.0f, 6.0, 'a');
		assertEquals(true, prims[0], "acceptsPrimitives");
		assertEquals('A', PrimitiveFunctions.toUpper('a'), "toUpper");

		List<String> list = CollectionFunctions.makeStringList();
		assertEquals(3, list.size(), "makeStringList");
		Map<String, Integer> map = CollectionFunctions.makeStringIntMap();
		assertEquals(2, map.size(), "makeStringIntMap");
		Set<Integer> set = CollectionFunctions.makeIntSet();
		assertEquals(3, set.size(), "makeIntSet");
		BigInteger bi = CollectionFunctions.bigIntegerValue();
		BigDecimal bd = CollectionFunctions.bigDecimalValue();
		assertTrue(bi.signum() > 0, "bigIntegerValue");
		assertTrue(bd.signum() > 0, "bigDecimalValue");

		assertEquals(EnumTypes.Color.RED, EnumTypes.getColor(0), "getColor");
		assertEquals("GREEN", EnumTypes.colorName(EnumTypes.Color.GREEN), "colorName");

		OverloadExamples ov = new OverloadExamples(2);
		assertEquals(3, ov.add(1, 2), "overload int");
		assertEquals(3.5, ov.add(1.5, 2.0), "overload double");
		assertEquals("ab", ov.add("a", "b"), "overload string");
		assertEquals(2, ov.getValue(), "overload constructor");

		NestedTypes.Inner inner = NestedTypes.makeInner(5);
		assertEquals(5, inner.getValue(), "nested inner");
		NestedTypes.Inner innerDirect = new NestedTypes.Inner(6);
		assertEquals(6, innerDirect.getValue(), "nested inner direct");

		Interfaces.Greeter greeter = new Interfaces.SimpleGreeter();
		assertEquals("Hello Bob", Interfaces.callGreeter(greeter, "Bob"), "callGreeter");

		GenericBox<String> box = new GenericBox<>("x");
		assertEquals("x", box.get(), "generic box");

		assertEquals(6, VarargsExamples.sum(1, 2, 3), "varargs sum");
		assertEquals("p:a:b", VarargsExamples.join("p", "a", "b"), "varargs join");

		Optional<String> opt = OptionalFunctions.maybeString(true);
		assertTrue(opt.isPresent(), "optional present");
		assertTrue(!OptionalFunctions.maybeString(false).isPresent(), "optional empty");

		try {
			Errors.throwRuntime();
			throw new AssertionError("throwRuntime");
		} catch (RuntimeException ignored) {
		}

		try {
			Errors.throwChecked(true);
			throw new AssertionError("throwChecked");
		} catch (IOException ignored) {
		}
		assertEquals("ok", Errors.returnErrorString(true), "returnErrorString ok");
		assertEquals("error", Errors.returnErrorString(false), "returnErrorString error");

		Callbacks.StringTransformer transformer = value -> value + "_x";
		assertEquals("a_x", Callbacks.callTransformer(transformer, "a"), "callTransformer");
		assertEquals("b_y", Callbacks.returnTransformer("_y").transform("b"), "returnTransformer");
		assertEquals(Integer.valueOf(3), Callbacks.callFunction(String::length, "abc"), "callFunction");
		try {
			Callbacks.callTransformer(value -> {
				throw new RuntimeException("callback error");
			}, "x");
			throw new AssertionError("callback error");
		} catch (RuntimeException ignored) {
		}

		StaticState.setCounter(0);
		assertEquals(5L, StaticState.FIVE_SECONDS, "FIVE_SECONDS");
		assertEquals(1, StaticState.incCounter(1), "incCounter");

		TestMap tm = new TestMap();
		tm.set("k", 1);
		assertTrue(tm.contains("k"), "TestMap contains");
		assertEquals(1, tm.get("k"), "TestMap get");
		assertEquals("name1", tm.name, "TestMap name");

		AutoCloseableResource res = new AutoCloseableResource();
		assertTrue(!res.isClosed(), "resource initial");
		res.close();
		assertTrue(res.isClosed(), "resource closed");

		assertEquals("v", SubModule.echo("v"), "sub module");
	}
}
