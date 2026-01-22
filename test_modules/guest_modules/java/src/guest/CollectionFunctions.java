package guest;

import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class CollectionFunctions {
	public static List<String> makeStringList() {
		return Arrays.asList("a", "b", "c");
	}

	public static Map<String, Integer> makeStringIntMap() {
		Map<String, Integer> map = new HashMap<>();
		map.put("a", 1);
		map.put("b", 2);
		return map;
	}

	public static Set<Integer> makeIntSet() {
		Set<Integer> set = new HashSet<>();
		set.add(1);
		set.add(2);
		set.add(3);
		return set;
	}

	public static Map<String, List<Integer>> makeNestedMap() {
		Map<String, List<Integer>> map = new HashMap<>();
		map.put("nums", Arrays.asList(1, 2, 3));
		return map;
	}

	public static List<SomeClass> makeSomeClassList() {
		return Arrays.asList(new SomeClass("a"), new SomeClass("b"), new SomeClass("c"));
	}

	public static BigInteger bigIntegerValue() {
		return new BigInteger("12345678901234567890");
	}

	public static BigDecimal bigDecimalValue() {
		return new BigDecimal("12345.6789");
	}
}
