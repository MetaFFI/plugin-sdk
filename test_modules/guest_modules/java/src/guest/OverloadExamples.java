package guest;

public class OverloadExamples {
	private final int value;

	public OverloadExamples() {
		this(0);
	}

	public OverloadExamples(int value) {
		this.value = value;
	}

	public int add(int a, int b) {
		return a + b;
	}

	public double add(double a, double b) {
		return a + b;
	}

	public String add(String a, String b) {
		return a + b;
	}

	public int getValue() {
		return value;
	}
}
