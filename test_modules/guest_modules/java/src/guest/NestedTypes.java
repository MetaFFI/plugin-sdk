package guest;

public class NestedTypes {
	public static class Inner {
		private final int value;

		public Inner(int value) {
			this.value = value;
		}

		public int getValue() {
			return value;
		}
	}

	public static Inner makeInner(int value) {
		return new Inner(value);
	}
}
