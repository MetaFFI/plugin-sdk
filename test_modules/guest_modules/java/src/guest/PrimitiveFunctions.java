package guest;

public class PrimitiveFunctions {
	public static Object[] acceptsPrimitives(
		boolean boolVal,
		byte byteVal,
		short shortVal,
		int intVal,
		long longVal,
		float floatVal,
		double doubleVal,
		char charVal
	) {
		return new Object[]{boolVal, byteVal, shortVal, intVal, longVal, floatVal, doubleVal, charVal};
	}

	public static byte[] echoBytes(byte[] data) {
		return data;
	}

	public static char toUpper(char c) {
		return Character.toUpperCase(c);
	}
}
