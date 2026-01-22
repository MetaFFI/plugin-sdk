package guest;

import java.util.function.Function;

public class Callbacks {
	public interface StringTransformer {
		String transform(String value);
	}

	public static String callTransformer(StringTransformer transformer, String value) {
		return transformer.transform(value);
	}

	public static StringTransformer returnTransformer(String suffix) {
		return value -> value + suffix;
	}

	public static Integer callFunction(Function<String, Integer> function, String value) {
		return function.apply(value);
	}
}
