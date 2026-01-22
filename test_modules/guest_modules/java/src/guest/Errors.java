package guest;

import java.io.IOException;

public class Errors {
	public static void throwRuntime() {
		throw new RuntimeException("Error");
	}

	public static void throwChecked(boolean doThrow) throws IOException {
		if (doThrow) {
			throw new IOException("IO error");
		}
	}

	public static String returnErrorString(boolean ok) {
		return ok ? "ok" : "error";
	}
}
