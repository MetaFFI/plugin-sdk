package guest;

public class SomeClass {
	private final String name;

	public SomeClass() {
		this("some");
	}

	public SomeClass(String name) {
		this.name = name;
	}

	public String print() {
		return "Hello from SomeClass " + name;
	}

	public String getName() {
		return name;
	}
}
