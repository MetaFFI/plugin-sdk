package guest;

public class VarargsExamples {
	public static int sum(int... values) {
		int sum = 0;
		for (int v : values) {
			sum += v;
		}
		return sum;
	}

	public static String join(String prefix, String... values) {
		StringBuilder builder = new StringBuilder(prefix);
		for (String v : values) {
			builder.append(":").append(v);
		}
		return builder.toString();
	}
}
