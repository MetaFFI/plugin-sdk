package guest;

public class Interfaces {
	public interface Greeter {
		String greet(String name);
	}

	public static class SimpleGreeter implements Greeter {
		@Override
		public String greet(String name) {
			return "Hello " + name;
		}
	}

	public static String callGreeter(Greeter greeter, String name) {
		return greeter.greet(name);
	}
}
