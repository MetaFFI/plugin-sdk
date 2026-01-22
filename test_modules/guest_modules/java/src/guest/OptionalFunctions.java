package guest;

import java.util.Optional;

public class OptionalFunctions {
	public static Optional<String> maybeString(boolean present) {
		if (present) {
			return Optional.of("value");
		}
		return Optional.empty();
	}
}
