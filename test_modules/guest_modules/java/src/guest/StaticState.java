package guest;

public class StaticState {
	public static final long FIVE_SECONDS = 5L;
	private static int counter = 0;

	public static int getCounter() {
		return counter;
	}

	public static int incCounter(int delta) {
		counter += delta;
		return counter;
	}

	public static void setCounter(int value) {
		counter = value;
	}
}
