package guest;

public class ArrayFunctions {
	public static byte[][] getThreeBuffers() {
		return new byte[][]{
			{1, 2, 3, 4},
			{5, 6, 7},
			{8, 9}
		};
	}

	public static void expectThreeBuffers(byte[][] buffers) {
		if (buffers.length != 3) {
			throw new IllegalArgumentException("Buffers length is not 3");
		}
		if (buffers[0].length != 4 || buffers[1].length != 3 || buffers[2].length != 2) {
			throw new IllegalArgumentException("Buffer sizes mismatch");
		}
	}

	public static SomeClass[] getSomeClasses() {
		return new SomeClass[]{new SomeClass(), new SomeClass(), new SomeClass()};
	}

	public static void expectThreeSomeClasses(SomeClass[] arr) {
		if (arr.length != 3) {
			throw new IllegalArgumentException("Array length is not 3");
		}
		for (SomeClass sc : arr) {
			if (sc == null) {
				throw new IllegalArgumentException("SomeClass element is null");
			}
		}
	}

	public static int[][] make2dArray() {
		return new int[][]{{1, 2}, {3, 4}};
	}

	public static int[][][] make3dArray() {
		return new int[][][]{{{1}, {2}}, {{3}, {4}}};
	}

	public static int[][] makeRaggedArray() {
		return new int[][]{{1, 2, 3}, {4}, {5, 6}};
	}

	public static int sum3dArray(int[][][] arr) {
		int sum = 0;
		for (int[][] plane : arr) {
			for (int[] row : plane) {
				for (int val : row) {
					sum += val;
				}
			}
		}
		return sum;
	}

	public static int sum3dArrayFromFactory() {
		return sum3dArray(make3dArray());
	}

	public static int sumRaggedArray(int[][] arr) {
		int sum = 0;
		for (int[] row : arr) {
			for (int val : row) {
				sum += val;
			}
		}
		return sum;
	}
}
