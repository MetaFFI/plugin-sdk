package guest;

public class EnumTypes {
	public enum Color {
		RED,
		GREEN,
		BLUE
	}

	public static Color getColor(int idx) {
		switch (idx) {
			case 0:
				return Color.RED;
			case 1:
				return Color.GREEN;
			default:
				return Color.BLUE;
		}
	}

	public static String colorName(Color color) {
		return color.name();
	}
}
