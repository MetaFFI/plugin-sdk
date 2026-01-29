import gc
import sys
import os
import unittest

from sdk.api.python3.metaffi.xllr_wrapper import xllr_python3

# make sure metaffi module is loaded from the local path, and not from the installed package
import metaffi

# Create a MetaFFIRuntime object for test plugin
runtime: metaffi.metaffi_runtime.MetaFFIRuntime = metaffi.metaffi_runtime.MetaFFIRuntime('test')
test_module: metaffi.metaffi_module.MetaFFIModule | None = None


def init():
	global runtime
	global test_module
	
	# Test plugin doesn't use modules, so use empty string
	test_module = runtime.load_module("")


def fini():
	pass


def assert_objects_not_loaded_of_type(tc: unittest.TestCase, type_name: str):
	gc.collect()  # Force a garbage collection to update the object list
	all_objects = gc.get_objects()  # Get a list of all objects tracked by the GC
	
	# Convert type_name to lowercase for case-insensitive comparison
	type_name_lower = type_name.lower()
	
	# Find objects whose type name contains the type_name substring, case-insensitively
	specific_type_objects = [obj for obj in all_objects if type_name_lower in type(obj).__name__.lower()]
	
	if len(specific_type_objects) > 0:
		print(f"Found {len(specific_type_objects)} objects of type(s) containing '{type_name}'")
		for obj in specific_type_objects:
			print(f"Object: {obj}, Type: {type(obj).__name__}")
		tc.fail(f"Found {len(specific_type_objects)} objects of type(s) containing '{type_name}'")


class TestAPI(unittest.TestCase):
	
	@classmethod
	def setUpClass(cls):
		init()
	
	@classmethod
	def tearDownClass(cls):
		fini()
	
	# ========================================================================
	# Primitives - No Params, No Return
	# ========================================================================
	
	def test_primitives_no_params_no_ret(self):
		"""Test entities that take no parameters and return nothing"""
		global test_module
		assert test_module is not None
		
		# test::no_op - Does nothing, just logs
		no_op = test_module.load_entity("test::no_op", None, None)
		no_op()
		del no_op
		
		# test::print_hello - Prints "Hello from test plugin!" to STDOUT
		print_hello = test_module.load_entity("test::print_hello", None, None)
		print_hello()
		del print_hello
	
	# ========================================================================
	# Primitives - Return Values Only
	# ========================================================================
	
	def test_primitives_return_only(self):
		"""Test entities that return specific primitive values"""
		global test_module
		assert test_module is not None
		
		# Integer types
		return_int8 = test_module.load_entity("test::return_int8", None, 
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int8_type)])
		self.assertEqual(return_int8(), 42)
		del return_int8
		
		return_int16 = test_module.load_entity("test::return_int16", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int16_type)])
		self.assertEqual(return_int16(), 1000)
		del return_int16
		
		return_int32 = test_module.load_entity("test::return_int32", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int32_type)])
		self.assertEqual(return_int32(), 100000)
		del return_int32
		
		return_int64 = test_module.load_entity("test::return_int64", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)])
		self.assertEqual(return_int64(), 9223372036854775807)
		del return_int64
		
		# Unsigned integer types
		return_uint8 = test_module.load_entity("test::return_uint8", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_uint8_type)])
		self.assertEqual(return_uint8(), 255)
		del return_uint8
		
		return_uint16 = test_module.load_entity("test::return_uint16", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_uint16_type)])
		self.assertEqual(return_uint16(), 65535)
		del return_uint16
		
		return_uint32 = test_module.load_entity("test::return_uint32", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_uint32_type)])
		self.assertEqual(return_uint32(), 4294967295)
		del return_uint32
		
		return_uint64 = test_module.load_entity("test::return_uint64", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_uint64_type)])
		self.assertEqual(return_uint64(), 18446744073709551615)
		del return_uint64
		
		# Float types
		return_float32 = test_module.load_entity("test::return_float32", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float32_type)])
		self.assertAlmostEqual(return_float32(), 3.14159, places=5)
		del return_float32
		
		return_float64 = test_module.load_entity("test::return_float64", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float64_type)])
		self.assertAlmostEqual(return_float64(), 3.141592653589793, places=15)
		del return_float64
		
		# Boolean types
		return_bool_true = test_module.load_entity("test::return_bool_true", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_bool_type)])
		self.assertEqual(return_bool_true(), True)
		del return_bool_true
		
		return_bool_false = test_module.load_entity("test::return_bool_false", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_bool_type)])
		self.assertEqual(return_bool_false(), False)
		del return_bool_false
		
		# String type
		return_string8 = test_module.load_entity("test::return_string8", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)])
		self.assertEqual(return_string8(), "Hello from test plugin")
		del return_string8
		
		# Null type
		return_null = test_module.load_entity("test::return_null", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_null_type)])
		self.assertIsNone(return_null())
		del return_null
	
	# ========================================================================
	# Primitives - Accept Values Only
	# ========================================================================
	
	def test_primitives_accept_only(self):
		"""Test entities that accept primitive values and log them"""
		global test_module
		assert test_module is not None
		
		# Integer types
		accept_int8 = test_module.load_entity("test::accept_int8",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int8_type)], None)
		accept_int8(42)
		del accept_int8
		
		accept_int16 = test_module.load_entity("test::accept_int16",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int16_type)], None)
		accept_int16(1000)
		del accept_int16
		
		accept_int32 = test_module.load_entity("test::accept_int32",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int32_type)], None)
		accept_int32(100000)
		del accept_int32
		
		accept_int64 = test_module.load_entity("test::accept_int64",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)], None)
		accept_int64(9223372036854775807)
		del accept_int64
		
		# Float types
		accept_float32 = test_module.load_entity("test::accept_float32",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float32_type)], None)
		accept_float32(3.14159)
		del accept_float32
		
		accept_float64 = test_module.load_entity("test::accept_float64",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float64_type)], None)
		accept_float64(3.141592653589793)
		del accept_float64
		
		# Boolean type
		accept_bool = test_module.load_entity("test::accept_bool",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_bool_type)], None)
		accept_bool(True)
		accept_bool(False)
		del accept_bool
		
		# String type
		accept_string8 = test_module.load_entity("test::accept_string8",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)], None)
		accept_string8("test string")
		del accept_string8
	
	# ========================================================================
	# Echo Functions
	# ========================================================================
	
	def test_echo_functions(self):
		"""Test entities that echo (return) their input unchanged"""
		global test_module
		assert test_module is not None
		
		# Echo int64
		echo_int64 = test_module.load_entity("test::echo_int64",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)])
		self.assertEqual(echo_int64(42), 42)
		self.assertEqual(echo_int64(-100), -100)
		del echo_int64
		
		# Echo float64
		echo_float64 = test_module.load_entity("test::echo_float64",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float64_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float64_type)])
		self.assertAlmostEqual(echo_float64(3.14), 3.14, places=10)
		del echo_float64
		
		# Echo string8
		echo_string8 = test_module.load_entity("test::echo_string8",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)])
		self.assertEqual(echo_string8("hello"), "hello")
		self.assertIsNone(echo_string8(None))  # null input returns null
		del echo_string8
		
		# Echo bool
		echo_bool = test_module.load_entity("test::echo_bool",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_bool_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_bool_type)])
		self.assertEqual(echo_bool(True), True)
		self.assertEqual(echo_bool(False), False)
		del echo_bool
	
	# ========================================================================
	# Arithmetic Functions
	# ========================================================================
	
	def test_arithmetic_functions(self):
		"""Test arithmetic operations"""
		global test_module
		assert test_module is not None
		
		# Add int64
		add_int64 = test_module.load_entity("test::add_int64",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)])
		self.assertEqual(add_int64(10, 20), 30)
		self.assertEqual(add_int64(-5, 5), 0)
		del add_int64
		
		# Add float64
		add_float64 = test_module.load_entity("test::add_float64",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float64_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float64_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float64_type)])
		self.assertAlmostEqual(add_float64(1.5, 2.5), 4.0, places=10)
		del add_float64
		
		# Concat strings
		concat_strings = test_module.load_entity("test::concat_strings",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)])
		self.assertEqual(concat_strings("hello", "world"), "helloworld")
		del concat_strings
	
	# ========================================================================
	# Arrays
	# ========================================================================
	
	def test_arrays(self):
		"""Test array operations - 1D, 2D, 3D, ragged, and string arrays"""
		global test_module
		assert test_module is not None
		
		# Return 1D int64 array
		return_int64_array_1d = test_module.load_entity("test::return_int64_array_1d", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_array_type, dims=1)])
		result_1d = return_int64_array_1d()
		self.assertEqual(result_1d, [1, 2, 3])
		del return_int64_array_1d
		
		# Return 2D int64 array
		return_int64_array_2d = test_module.load_entity("test::return_int64_array_2d", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_array_type, dims=2)])
		result_2d = return_int64_array_2d()
		self.assertEqual(result_2d, [[1, 2], [3, 4]])
		del return_int64_array_2d
		
		# Return 3D int64 array
		return_int64_array_3d = test_module.load_entity("test::return_int64_array_3d", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_array_type, dims=3)])
		result_3d = return_int64_array_3d()
		self.assertEqual(result_3d, [[[1, 2], [3, 4]], [[5, 6], [7, 8]]])
		del return_int64_array_3d
		
		# Return ragged array
		return_ragged_array = test_module.load_entity("test::return_ragged_array", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_array_type, dims=2)])
		result_ragged = return_ragged_array()
		self.assertEqual(result_ragged, [[1, 2, 3], [4], [5, 6]])
		del return_ragged_array
		
		# Return string array
		return_string_array = test_module.load_entity("test::return_string_array", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_array_type, dims=1)])
		result_strings = return_string_array()
		self.assertEqual(result_strings, ["one", "two", "three"])
		del return_string_array
		
		# Sum int64 array
		sum_int64_array = test_module.load_entity("test::sum_int64_array",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_array_type, dims=1)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)])
		self.assertEqual(sum_int64_array([1, 2, 3, 4, 5]), 15)
		del sum_int64_array
		
		# Echo int64 array
		echo_int64_array = test_module.load_entity("test::echo_int64_array",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_array_type, dims=1)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_array_type, dims=1)])
		input_arr = [10, 20, 30]
		result = echo_int64_array(input_arr)
		self.assertEqual(result, input_arr)
		del echo_int64_array
		
		# Join strings
		join_strings = test_module.load_entity("test::join_strings",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_array_type, dims=1)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)])
		result = join_strings(["one", "two", "three"])
		self.assertEqual(result, "one, two, three")
		del join_strings
	
	# ========================================================================
	# Handles (Opaque Objects)
	# ========================================================================
	
	def test_handles(self):
		"""Test handle (opaque object) operations"""
		global test_module
		assert test_module is not None
		
		# Create handle
		create_handle = test_module.load_entity("test::create_handle", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type)])
		handle = create_handle()
		self.assertIsNotNone(handle)
		del create_handle
		
		# Get handle data
		get_handle_data = test_module.load_entity("test::get_handle_data",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)])
		data = get_handle_data(handle)
		self.assertEqual(data, "test_data")
		del get_handle_data
		
		# Set handle data
		set_handle_data = test_module.load_entity("test::set_handle_data",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)],
			None)
		set_handle_data(handle, "new_data")
		del set_handle_data
		
		# Verify data was updated
		get_handle_data2 = test_module.load_entity("test::get_handle_data",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)])
		data2 = get_handle_data2(handle)
		self.assertEqual(data2, "new_data")
		del get_handle_data2
		
		# Release handle (logs release request)
		release_handle = test_module.load_entity("test::release_handle",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type)],
			None)
		release_handle(handle)
		del release_handle
		del handle
	
	# ========================================================================
	# Callables (Callbacks)
	# ========================================================================
	
	def test_callables(self):
		"""Test callable (callback) operations"""
		global test_module
		assert test_module is not None
		
		# sys.path.insert(0, os.path.join(os.getenv('METAFFI_SOURCE_ROOT'), 'sdk', 'api', 'python3'))
		from metaffi import xllr_wrapper
		
		
		print(f'1 - type(metaffi.xllr_wrapper.xllr_python3.call_xcall) {type(xllr_wrapper.xllr_python3.call_xcall)}')
		print(f'1 - id(metaffi.xllr_wrapper.xllr_python3) {id(xllr_wrapper.xllr_python3)}')

		# Define a Python function to pass as callback
		def adder(a: int, b: int) -> int:
			return a + b

		# Call callback add
		call_callback_add = test_module.load_entity("test::call_callback_add",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_callable_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)])

		metaffi_callable = metaffi.make_metaffi_callable(adder)
		result = call_callback_add(metaffi_callable)
		self.assertEqual(result, 7)  # 3 + 4 = 7
		del call_callback_add
		del metaffi_callable
		
		print(f'2 - type(metaffi.xllr_wrapper.xllr_python3.call_xcall) {type(xllr_wrapper.xllr_python3.call_xcall)}')
		print(f'2 - id(metaffi.xllr_wrapper.xllr_python3) {id(xllr_wrapper.xllr_python3)}')

		# Call callback string
		def string_processor(s: str) -> str:
			return s.upper()

		call_callback_string = test_module.load_entity("test::call_callback_string",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_callable_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)])

		metaffi_callable2 = metaffi.make_metaffi_callable(string_processor)
		result2 = call_callback_string(metaffi_callable2)
		self.assertEqual(result2, "TEST")
		del call_callback_string
		del metaffi_callable2
		
		print(f'3 - type(metaffi.xllr_wrapper.xllr_python3.call_xcall) {type(xllr_wrapper.xllr_python3.call_xcall)}')
		print(f'3 - id(metaffi.xllr_wrapper.xllr_python3) {id(xllr_wrapper.xllr_python3)}')
		
		
		# Return adder callback
		return_adder_callback = test_module.load_entity("test::return_adder_callback", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_callable_type)])
		adder_callback = return_adder_callback()
		result3 = adder_callback(10, 20)
		self.assertEqual(result3[0], 30)
		del return_adder_callback
		del adder_callback
	
	# ========================================================================
	# Error Handling
	# ========================================================================
	
	def test_error_handling(self):
		"""Test error handling scenarios"""
		global test_module
		assert test_module is not None
		
		# Throw error - always returns error
		throw_error = test_module.load_entity("test::throw_error", None, None)
		with self.assertRaises(RuntimeError) as context:
			throw_error()
		self.assertIn("Test error thrown intentionally", str(context.exception))
		del throw_error
		
		# Throw with message
		throw_with_message = test_module.load_entity("test::throw_with_message",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)],
			None)
		with self.assertRaises(RuntimeError) as context:
			throw_with_message("Custom error message")
		self.assertIn("Custom error message", str(context.exception))
		del throw_with_message
		
		# Error if negative - succeeds for positive
		error_if_negative = test_module.load_entity("test::error_if_negative",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)],
			None)
		error_if_negative(42)  # Should succeed
		
		# Error if negative - fails for negative
		with self.assertRaises(RuntimeError) as context:
			error_if_negative(-1)
		self.assertIn("negative", str(context.exception).lower())
		del error_if_negative
	
	# ========================================================================
	# Any Type (Dynamic Type)
	# ========================================================================
	
	def test_any_type(self):
		"""Test accept_any entity that accepts any type at runtime"""
		global test_module
		assert test_module is not None
		
		accept_any = test_module.load_entity("test::accept_any",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_any_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_any_type)])
		
		# Test with int64 - returns int64 with different value (input + 100)
		result1 = accept_any(42)
		self.assertEqual(142, result1)  # 42 + 100 = 142
		
		# Test with float64 - returns float64 with different value (input * 2.0)
		result2 = accept_any(3.14)
		self.assertAlmostEqual(6.28, result2, places=10)  # 3.14 * 2.0 = 6.28
		
		# Test with string8 - returns string8 with different value
		result3 = accept_any("hello")
		self.assertEqual("echoed: hello", result3)
		
		# Test with int64[] array (Python ints serialize as int64) - returns different array
		result4 = accept_any([1, 2, 3])
		self.assertEqual([10, 20, 30], result4)
		
		del accept_any
	
	# ========================================================================
	# Multiple Return Values
	# ========================================================================
	
	def test_multiple_return_values(self):
		"""Test entities that return multiple values (as tuples)"""
		global test_module
		assert test_module is not None
		
		# Return two values
		return_two_values = test_module.load_entity("test::return_two_values", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)])
		result = return_two_values()
		self.assertEqual(result, (42, "answer"))
		del return_two_values
		
		# Return three values
		return_three_values = test_module.load_entity("test::return_three_values", None,
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float64_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_bool_type)])
		result = return_three_values()
		self.assertEqual(result, (1, 2.5, True))
		del return_three_values
		
		# Swap values
		swap_values = test_module.load_entity("test::swap_values",
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)])
		result = swap_values(42, "hello")
		self.assertEqual(result, ("hello", 42))
		del swap_values
