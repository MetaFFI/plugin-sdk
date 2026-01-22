package guest;

import java.util.function.IntBinaryOperator;

public class CoreFunctions {
	public static String helloWorld() {
		return "Hello World, from Java";
	}

	public static void returnsAnError() throws Exception {
		throw new Exception("Error");
	}

	public static double divIntegers(long x, long y) {
		return ((double) x) / y;
	}

	public static String joinStrings(String[] arr) {
		return String.join(",", arr);
	}

	public static void waitABit(long millis) {
		// no-op, just verify call
	}

	public static Object returnNull() {
		return null;
	}

	public static int callCallbackAdd(IntBinaryOperator add) {
		int res = add.applyAsInt(1, 2);
		if (res != 3) {
			throw new IllegalArgumentException("expected 3, got " + res);
		}
		return res;
	}

	public static IntBinaryOperator returnCallbackAdd() {
		return (a, b) -> a + b;
	}

	public static Object[] returnMultipleReturnValues() {
		return new Object[]{1, "string", 3.0, null, new byte[]{1, 2, 3}, new SomeClass()};
	}

	public static Object[] returnsArrayWithDifferentDimensions() {
		return new Object[]{new int[]{1, 2, 3}, 4, new int[][]{{5, 6}, {7, 8}}};
	}

	public static Object[] returnsArrayOfDifferentObjects() {
		return new Object[]{1, "string", 3.0, null, new byte[]{1, 2, 3}, new SomeClass()};
	}

	public static Object returnAny(int whichType) {
		switch (whichType) {
			case 0:
				return 1;
			case 1:
				return "string";
			case 2:
				return 3.0;
			case 3:
				return new String[]{"list", "of", "strings"};
			case 4:
				return new SomeClass();
			default:
				return null;
		}
	}

	public static void acceptsAny(int whichTypeToExpect, Object value) {
		switch (whichTypeToExpect) {
			case 0:
				if (!(value instanceof Integer)) {
					throw new IllegalArgumentException("Expected Integer");
				}
				break;
			case 1:
				if (!(value instanceof String)) {
					throw new IllegalArgumentException("Expected String");
				}
				break;
			case 2:
				if (!(value instanceof Double)) {
					throw new IllegalArgumentException("Expected Double");
				}
				break;
			case 3:
				if (value != null) {
					throw new IllegalArgumentException("Expected null");
				}
				break;
			case 4:
				if (!(value instanceof byte[])) {
					throw new IllegalArgumentException("Expected byte[]");
				}
				break;
			case 5:
				if (!(value instanceof SomeClass)) {
					throw new IllegalArgumentException("Expected SomeClass");
				}
				break;
			default:
				throw new IllegalArgumentException("Unsupported type");
		}
	}
}
