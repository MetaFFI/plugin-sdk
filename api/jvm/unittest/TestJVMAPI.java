import api.MetaFFIModule;
import api.MetaFFIRuntime;
import metaffi.api.accessor.Caller;
import metaffi.api.accessor.MetaFFIHandle;
import metaffi.api.accessor.MetaFFITypeInfo;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import java.math.BigInteger;

import static org.junit.Assert.*;

public class TestJVMAPI
{
	private static MetaFFIRuntime runtime;
	private static MetaFFIModule testModule;

	@BeforeClass
	public static void setUpClass()
	{
		String metaffiHome = System.getenv("METAFFI_HOME");
		assertNotNull("METAFFI_HOME must be set for JVM API tests", metaffiHome);

		runtime = new MetaFFIRuntime("test");
		runtime.loadRuntimePlugin();
		testModule = runtime.loadModule("");
		assertNotNull("Failed to load xllr.test module", testModule);
	}

	@AfterClass
	public static void tearDownClass()
	{
		try
		{
			if(runtime != null)
			{
				runtime.releaseRuntimePlugin();
			}
		}
		finally
		{
			testModule = null;
			runtime = null;
		}
	}

	private static MetaFFITypeInfo type(MetaFFITypeInfo.MetaFFITypes t)
	{
		return new MetaFFITypeInfo(t);
	}

	private static MetaFFITypeInfo arrayType(MetaFFITypeInfo.MetaFFITypes t, int dims)
	{
		return new MetaFFITypeInfo(t, dims);
	}

	private static Caller load(String entityPath, MetaFFITypeInfo[] params, MetaFFITypeInfo[] retvals)
	{
		assertNotNull("testModule is not initialized", testModule);
		Caller c = testModule.load(entityPath, params, retvals);
		assertNotNull("Failed to load entity: " + entityPath, c);
		return c;
	}

	private static Object[] callAndAssertResults(String entityPath, Caller caller, Object... args)
	{
		Object[] result = caller.call(args);
		assertNotNull("Expected return values for " + entityPath + " but got null", result);
		return result;
	}

	private static String asUnsignedString(Object value)
	{
		if(value instanceof BigInteger)
		{
			return ((BigInteger)value).toString();
		}
		if(value instanceof Byte)
		{
			return Integer.toString(Byte.toUnsignedInt((Byte)value));
		}
		if(value instanceof Short)
		{
			return Integer.toString(Short.toUnsignedInt((Short)value));
		}
		if(value instanceof Integer)
		{
			return Long.toString(Integer.toUnsignedLong((Integer)value));
		}
		if(value instanceof Long)
		{
			return Long.toUnsignedString((Long)value);
		}
		fail("Unsupported unsigned representation type: " + (value == null ? "null" : value.getClass().getName()));
		return "";
	}

	@Test
	public void testPrimitivesNoParamsNoRet()
	{
		load("test::no_op", null, null).call();
		load("test::print_hello", null, null).call();
	}

	@Test
	public void testPrimitivesReturnOnly()
	{
		Object[] retInt8 = callAndAssertResults("test::return_int8",
				load("test::return_int8", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt8)}));
		assertEquals((byte)42, retInt8[0]);

		Object[] retInt16 = callAndAssertResults("test::return_int16",
				load("test::return_int16", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt16)}));
		assertEquals((short)1000, retInt16[0]);

		Object[] retInt32 = callAndAssertResults("test::return_int32",
				load("test::return_int32", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt32)}));
		assertEquals(100000, retInt32[0]);

		Object[] retInt64 = callAndAssertResults("test::return_int64",
				load("test::return_int64", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)}));
		assertEquals(9223372036854775807L, retInt64[0]);

		Object[] retUInt8 = callAndAssertResults("test::return_uint8",
				load("test::return_uint8", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIUInt8)}));
		assertEquals("255", asUnsignedString(retUInt8[0]));

		Object[] retUInt16 = callAndAssertResults("test::return_uint16",
				load("test::return_uint16", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIUInt16)}));
		assertEquals("65535", asUnsignedString(retUInt16[0]));

		Object[] retUInt32 = callAndAssertResults("test::return_uint32",
				load("test::return_uint32", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIUInt32)}));
		assertEquals("4294967295", asUnsignedString(retUInt32[0]));

		Object[] retUInt64 = callAndAssertResults("test::return_uint64",
				load("test::return_uint64", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIUInt64)}));
		assertEquals("18446744073709551615", asUnsignedString(retUInt64[0]));

		Object[] retFloat32 = callAndAssertResults("test::return_float32",
				load("test::return_float32", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat32)}));
		assertEquals(3.14159f, ((Float)retFloat32[0]), 0.00001f);

		Object[] retFloat64 = callAndAssertResults("test::return_float64",
				load("test::return_float64", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64)}));
		assertEquals(3.141592653589793, ((Double)retFloat64[0]), 0.000000000000001);

		Object[] retBoolTrue = callAndAssertResults("test::return_bool_true",
				load("test::return_bool_true", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIBool)}));
		assertEquals(true, retBoolTrue[0]);

		Object[] retBoolFalse = callAndAssertResults("test::return_bool_false",
				load("test::return_bool_false", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIBool)}));
		assertEquals(false, retBoolFalse[0]);

		Object[] retString8 = callAndAssertResults("test::return_string8",
				load("test::return_string8", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)}));
		assertEquals("Hello from test plugin", retString8[0]);

		Object[] retNull = callAndAssertResults("test::return_null",
				load("test::return_null", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFINull)}));
		assertNull(retNull[0]);
	}

	@Test
	public void testPrimitivesAcceptOnly()
	{
		load("test::accept_int8", new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt8)}, null).call((byte)42);
		load("test::accept_int16", new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt16)}, null).call((short)1000);
		load("test::accept_int32", new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt32)}, null).call(100000);
		load("test::accept_int64", new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)}, null).call(9223372036854775807L);
		load("test::accept_float32", new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat32)}, null).call(3.14159f);
		load("test::accept_float64", new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64)}, null).call(3.141592653589793);
		load("test::accept_bool", new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIBool)}, null).call(true);
		load("test::accept_string8", new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)}, null).call("test string");
	}

	@Test
	public void testEchoFunctions()
	{
		Object[] echoInt64 = callAndAssertResults("test::echo_int64",
				load("test::echo_int64",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)}),
				42L);
		assertEquals(42L, echoInt64[0]);

		Object[] echoFloat64 = callAndAssertResults("test::echo_float64",
				load("test::echo_float64",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64)}),
				3.14);
		assertEquals(3.14, (Double)echoFloat64[0], 0.000000001);

		Object[] echoString8 = callAndAssertResults("test::echo_string8",
				load("test::echo_string8",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)}),
				"hello");
		assertEquals("hello", echoString8[0]);

		Object[] echoBool = callAndAssertResults("test::echo_bool",
				load("test::echo_bool",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIBool)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIBool)}),
				true);
		assertEquals(true, echoBool[0]);
	}

	@Test
	public void testArithmeticFunctions()
	{
		Object[] addInt64 = callAndAssertResults("test::add_int64",
				load("test::add_int64",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)}),
				10L, 20L);
		assertEquals(30L, addInt64[0]);

		Object[] addFloat64 = callAndAssertResults("test::add_float64",
				load("test::add_float64",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64)}),
				1.5, 2.5);
		assertEquals(4.0, (Double)addFloat64[0], 0.000000001);

		Object[] concatStrings = callAndAssertResults("test::concat_strings",
				load("test::concat_strings",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)}),
				"hello", "world");
		assertEquals("helloworld", concatStrings[0]);
	}

	@Test
	public void testArrays()
	{
		Object[] arr1dResult = callAndAssertResults("test::return_int64_array_1d",
				load("test::return_int64_array_1d", null, new MetaFFITypeInfo[]{arrayType(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64Array, 1)}));
		assertArrayEquals(new long[]{1L, 2L, 3L}, (long[])arr1dResult[0]);

		Object[] arr2dResult = callAndAssertResults("test::return_int64_array_2d",
				load("test::return_int64_array_2d", null, new MetaFFITypeInfo[]{arrayType(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64Array, 2)}));
		assertArrayEquals(new long[]{1L, 2L}, ((long[][])arr2dResult[0])[0]);
		assertArrayEquals(new long[]{3L, 4L}, ((long[][])arr2dResult[0])[1]);

		Object[] arr3dResult = callAndAssertResults("test::return_int64_array_3d",
				load("test::return_int64_array_3d", null, new MetaFFITypeInfo[]{arrayType(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64Array, 3)}));
		long[][][] arr3d = (long[][][])arr3dResult[0];
		assertArrayEquals(new long[]{1L, 2L}, arr3d[0][0]);
		assertArrayEquals(new long[]{3L, 4L}, arr3d[0][1]);
		assertArrayEquals(new long[]{5L, 6L}, arr3d[1][0]);
		assertArrayEquals(new long[]{7L, 8L}, arr3d[1][1]);

		Object[] raggedResult = callAndAssertResults("test::return_ragged_array",
				load("test::return_ragged_array", null, new MetaFFITypeInfo[]{arrayType(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64Array, 2)}));
		long[][] ragged = (long[][])raggedResult[0];
		assertArrayEquals(new long[]{1L, 2L, 3L}, ragged[0]);
		assertArrayEquals(new long[]{4L}, ragged[1]);
		assertArrayEquals(new long[]{5L, 6L}, ragged[2]);

		Object[] stringArrayResult = callAndAssertResults("test::return_string_array",
				load("test::return_string_array", null, new MetaFFITypeInfo[]{arrayType(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8Array, 1)}));
		assertArrayEquals(new String[]{"one", "two", "three"}, (String[])stringArrayResult[0]);

		Object[] sumResult = callAndAssertResults("test::sum_int64_array",
				load("test::sum_int64_array",
						new MetaFFITypeInfo[]{arrayType(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64Array, 1)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)}),
				(Object)new long[]{1L, 2L, 3L, 4L, 5L});
		assertEquals(15L, sumResult[0]);

		Object[] echoArrayResult = callAndAssertResults("test::echo_int64_array",
				load("test::echo_int64_array",
						new MetaFFITypeInfo[]{arrayType(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64Array, 1)},
						new MetaFFITypeInfo[]{arrayType(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64Array, 1)}),
				(Object)new long[]{10L, 20L, 30L});
		assertArrayEquals(new long[]{10L, 20L, 30L}, (long[])echoArrayResult[0]);

		Object[] joined = callAndAssertResults("test::join_strings",
				load("test::join_strings",
						new MetaFFITypeInfo[]{arrayType(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8Array, 1)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)}),
				(Object)new String[]{"one", "two", "three"});
		assertEquals("one, two, three", joined[0]);
	}

	@Test
	public void testHandles()
	{
		Object[] createResult = callAndAssertResults("test::create_handle",
				load("test::create_handle", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)}));
		assertTrue(createResult[0] instanceof MetaFFIHandle);
		MetaFFIHandle handle = (MetaFFIHandle)createResult[0];

		Object[] dataResult = callAndAssertResults("test::get_handle_data",
				load("test::get_handle_data",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)}),
				handle);
		assertEquals("test_data", dataResult[0]);

		load("test::set_handle_data",
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				null).call(handle, "new_data");

		Object[] dataAfterSet = callAndAssertResults("test::get_handle_data",
				load("test::get_handle_data",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)}),
				handle);
		assertEquals("new_data", dataAfterSet[0]);

		load("test::release_handle",
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				null).call(handle);
	}

	@Test
	public void testHandleClassMethods()
	{
		Object[] createResult = callAndAssertResults("test::create_handle",
				load("test::create_handle", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)}));
		MetaFFIHandle handle = (MetaFFIHandle)createResult[0];

		Object[] idResult = callAndAssertResults("test::TestHandle.get_id",
				load("test::TestHandle.get_id",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)}),
				handle);
		assertTrue(((Long)idResult[0]) > 0L);

		load("test::TestHandle.append_to_data",
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				null).call(handle, "_suffix");

		Object[] dataResult = callAndAssertResults("test::get_handle_data",
				load("test::get_handle_data",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)}),
				handle);
		assertEquals("test_data_suffix", dataResult[0]);

		load("test::release_handle",
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				null).call(handle);
	}

	@Test
	public void testCallables()
	{
		Caller addCallable = load("test::add_int64",
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)},
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)});

		Object[] callbackAddResult = callAndAssertResults("test::call_callback_add",
				load("test::call_callback_add",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFICallable)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)}),
				addCallable);
		assertEquals(7L, callbackAddResult[0]);

		Caller stringCallable = load("test::echo_string8",
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)});

		Object[] callbackStringResult = callAndAssertResults("test::call_callback_string",
				load("test::call_callback_string",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFICallable)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)}),
				stringCallable);
		assertEquals("test", callbackStringResult[0]);

		Object[] returnedCallableResult = callAndAssertResults("test::return_adder_callback",
				load("test::return_adder_callback", null, new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFICallable)}));
		assertEquals(1, returnedCallableResult.length);
		assertTrue(returnedCallableResult[0] instanceof Caller);

		Caller returnedAdder = (Caller)returnedCallableResult[0];
		Object[] addResult = callAndAssertResults("returned adder callback", returnedAdder, 10L, 20L);
		assertEquals(30L, addResult[0]);
	}

	@Test
	public void testErrorHandling()
	{
		Caller throwError = load("test::throw_error", null, null);
		Throwable err1 = assertThrows(Throwable.class, () -> throwError.call());
		assertNotNull(err1.getMessage());
		assertTrue(err1.getMessage().contains("Test error thrown intentionally"));

		Caller throwWithMessage = load("test::throw_with_message",
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				null);
		Throwable err2 = assertThrows(Throwable.class, () -> throwWithMessage.call("Custom error message"));
		assertNotNull(err2.getMessage());
		assertTrue(err2.getMessage().contains("Custom error message"));

		Caller errorIfNegative = load("test::error_if_negative",
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)},
				null);
		errorIfNegative.call(42L);
		Throwable err3 = assertThrows(Throwable.class, () -> errorIfNegative.call(-1L));
		assertNotNull(err3.getMessage());
		assertTrue(err3.getMessage().toLowerCase().contains("negative"));
	}

	@Test
	public void testAnyType()
	{
		Caller acceptAny = load("test::accept_any",
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)},
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)});

		Object[] intResult = callAndAssertResults("test::accept_any(int64)", acceptAny, 42L);
		assertEquals(142L, ((Number)intResult[0]).longValue());

		Object[] floatResult = callAndAssertResults("test::accept_any(float64)", acceptAny, 3.14);
		assertEquals(6.28, ((Number)floatResult[0]).doubleValue(), 0.000000001);

		Object[] stringResult = callAndAssertResults("test::accept_any(string8)", acceptAny, "hello");
		assertEquals("echoed: hello", stringResult[0]);

		Object[] int32ArrayResult = callAndAssertResults("test::accept_any(int32[])", acceptAny, (Object)new int[]{1, 2, 3});
		assertArrayEquals(new int[]{4, 5, 6}, (int[])int32ArrayResult[0]);

		Object[] int64ArrayResult = callAndAssertResults("test::accept_any(int64[])", acceptAny, (Object)new long[]{1L, 2L, 3L});
		assertArrayEquals(new long[]{10L, 20L, 30L}, (long[])int64ArrayResult[0]);
	}

	@Test
	public void testMultipleReturnValues()
	{
		Caller returnTwoValuesCaller = load("test::return_two_values",
				null,
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)});
		assertNotNull(returnTwoValuesCaller.retvalsTypesArray);
		assertEquals(2, returnTwoValuesCaller.retvalsTypesArray.length);
		Object[] twoValues = callAndAssertResults("test::return_two_values",
				returnTwoValuesCaller);
		assertEquals(2, twoValues.length);
		assertEquals(42L, twoValues[0]);
		assertEquals("answer", twoValues[1]);

		Caller returnThreeValuesCaller = load("test::return_three_values",
				null,
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIBool)});
		assertNotNull(returnThreeValuesCaller.retvalsTypesArray);
		assertEquals(3, returnThreeValuesCaller.retvalsTypesArray.length);
		Object[] threeValues = callAndAssertResults("test::return_three_values",
				returnThreeValuesCaller);
		assertEquals(3, threeValues.length);
		assertEquals(1L, threeValues[0]);
		assertEquals(2.5, ((Double)threeValues[1]), 0.000000001);
		assertEquals(true, threeValues[2]);

		Object[] swapped = callAndAssertResults("test::swap_values",
				load("test::swap_values",
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
						new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8), type(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)}),
				42L, "hello");
		assertEquals(2, swapped.length);
		assertEquals("hello", swapped[0]);
		assertEquals(42L, swapped[1]);
	}

	@Test
	public void testGlobalVariable()
	{
		Caller getGName = load("test::get_g_name",
				null,
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)});
		Caller setGName = load("test::set_g_name",
				new MetaFFITypeInfo[]{type(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				null);

		Object[] original = callAndAssertResults("test::get_g_name", getGName);
		String oldValue = (String)original[0];
		String newValue = "jvm_api_test_value";

		try
		{
			setGName.call(newValue);
			Object[] afterSet = callAndAssertResults("test::get_g_name", getGName);
			assertEquals(newValue, afterSet[0]);
		}
		finally
		{
			setGName.call(oldValue);
		}
	}
}
