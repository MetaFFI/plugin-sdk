package com.metaffi.jdk;

public class TestModule {
	public static int static_field = 42;
	public int instance_field;

	public TestModule(int value) {
		this.instance_field = value;
	}

	public static int add_ints(int a, int b) {
		return a + b;
	}

	public static double add_ints(double a, double b) {
		return a + b;
	}

	public int add_to_instance(int value) {
		return instance_field + value;
	}

	public static String concat(String a, String b) {
		return a + b;
	}

	public static void static_void() {
	}

	public void instance_void(int value) {
		this.instance_field = value;
	}

	public static class Inner {
		public String inner_method() {
			return "inner";
		}
	}
}
