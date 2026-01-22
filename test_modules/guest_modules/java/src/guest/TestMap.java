package guest;

import java.util.HashMap;
import java.util.Map;

public class TestMap {
	private final Map<String, Object> map = new HashMap<>();
	public String name = "name1";

	public void set(String key, Object value) {
		map.put(key, value);
	}

	public Object get(String key) {
		return map.get(key);
	}

	public boolean contains(String key) {
		return map.containsKey(key);
	}
}
