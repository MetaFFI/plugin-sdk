import gc
import sys
import os
import unittest

# make sure metaffi module is loaded from the local path, and not from the installed package
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
import metaffi

# Create a MetaFFIRuntime object
runtime: metaffi.metaffi_runtime.MetaFFIRuntime = metaffi.metaffi_runtime.MetaFFIRuntime('python3')
current_path = os.path.dirname(os.path.realpath(__file__))

test_runtime_module: metaffi.metaffi_module.MetaFFIModule | None = None
test_map_module: metaffi.metaffi_module.MetaFFIModule | None = None


def init():
	global runtime
	global test_runtime_module
	global test_map_module
	
	test_runtime_module = runtime.load_module(os.path.join(os.path.dirname(__file__), '..', '..', 'runtime', 'test', 'runtime_test_target.py'))
	

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
	
	def test_hello_world(self):
		global test_runtime_module
		assert test_runtime_module is not None
		
		# load hello world
		hello_world = test_runtime_module.load_entity('callable=hello_world', None, None)
		hello_world()
		del hello_world
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_returns_an_error(self):
		global test_runtime_module
		assert test_runtime_module is not None
		
		# Load the function that is expected to return an error
		returns_an_error = test_runtime_module.load_entity('callable=returns_an_error', None, None)
		try:
			returns_an_error()
			self.fail("Expected an error but none occurred.")
		except Exception as e:
			# Assuming the error is raised as an exception in Python
			pass
		
		del returns_an_error
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_div_integers(self):
		global test_runtime_module
		assert test_runtime_module is not None
		
		# Load the function for dividing integers
		div_integers = test_runtime_module.load_entity('callable=div_integers', [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type), metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)], [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_float64_type)])
		try:
			# Prepare parameters and call the function
			result = div_integers(10, 2)
			# Check if the result is as expected
			self.assertEqual(5.0, result)
		except Exception as e:
			# Handle any errors that occur
			self.fail(f"Unexpected error occurred: {str(e)}")
		
		del div_integers
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_join_strings(self):
		global test_runtime_module
		assert test_runtime_module is not None
		
		# Load the function for joining strings
		join_strings = test_runtime_module.load_entity('callable=join_strings',
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_array_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)])
		try:
			# Prepare parameters and call the function
			result = join_strings(["one", "two", "three"])
			# Check if the result is as expected
			self.assertEqual("one,two,three", result)
		except Exception as e:
			# Handle any errors that occur
			self.fail(f"Unexpected error occurred: {str(e)}")
		
		del join_strings
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_wait_a_bit(self):
		global test_runtime_module
		assert test_runtime_module is not None
		
		# Retrieve the global variable five_seconds
		five_seconds_getter = test_runtime_module.load_entity('attribute=five_seconds,getter', [], [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)])
		try:
			five_seconds = five_seconds_getter()
		except Exception as e:
			self.fail(f"Failed to get five_seconds: {str(e)}")
		
		self.assertEqual(five_seconds, 5, "Expected five_seconds to be 5")
		
		# Call wait_a_bit function with five_seconds
		wait_a_bit = test_runtime_module.load_entity('callable=wait_a_bit', [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type)], None)
		try:
			wait_a_bit(five_seconds)
		except Exception as e:
			self.fail(f"wait_a_bit function failed: {str(e)}")
		
		del five_seconds_getter
		del wait_a_bit
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_testmap_set_get_contains(self):
		global test_runtime_module
		assert test_runtime_module is not None
		
		new_testmap = test_runtime_module.load_entity('callable=testmap', [], [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type)])
		testmap_set = test_runtime_module.load_entity('callable=testmap.set,instance_required',
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_any_type)],
			[])
		testmap_contains = test_runtime_module.load_entity('callable=testmap.contains,instance_required',
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_bool_type)])
		
		testmap_get = test_runtime_module.load_entity('callable=testmap.get,instance_required',
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type),
			 metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_string8_type)],
			[metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_any_type)])
		
		# Create new testmap
		testmap_instance = new_testmap()
		self.assertIsNotNone(testmap_instance, "Failed to create testmap instance")
		
		# Set a value in the testmap
		testmap_set(testmap_instance, "key", 42)
		
		# Check if the key exists in the testmap
		contains_result = testmap_contains(testmap_instance, "key")
		self.assertTrue(contains_result, "Key does not exist in testmap")
		
		# Get the value from the testmap
		get_result = testmap_get(testmap_instance, "key")
		self.assertEqual(get_result, 42, "Retrieved value does not match expected")
		
		del new_testmap
		del testmap_set
		del testmap_contains
		del testmap_get
		
		# no need to check the "releaser" as we are calling Python from Python
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_runtime_test_target_SomeClass(self):
		global test_runtime_module
		assert test_runtime_module is not None

		pgetSomeClasses = test_runtime_module.load_entity('callable=get_some_classes', [], [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_array_type)])
		someclassList = pgetSomeClasses()
		
		self.assertEqual(3, len(someclassList))
		self.assertTrue(isinstance(someclassList, list))
		
		pSomeClassPrint = test_runtime_module.load_entity('callable=SomeClass.print', [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type)], [])
		for some_class in someclassList:
			pSomeClassPrint(some_class)
		
		pexpectThreeSomeClasses = test_runtime_module.load_entity('callable=expect_three_some_classes', [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_array_type)], [])
		pexpectThreeSomeClasses(someclassList)
		
		del pgetSomeClasses
		del pSomeClassPrint
		del pexpectThreeSomeClasses
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_runtime_test_target_ThreeBuffers(self):
		global test_runtime_module
		assert test_runtime_module is not None

		pexpectThreeBuffers = test_runtime_module.load_entity('callable=expect_three_buffers', [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_uint8_array_type, dims=2)], [])
		pgetThreeBuffers = test_runtime_module.load_entity('callable=get_three_buffers', [], [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_uint8_array_type, dims=2)])
		
		# Pass 3 buffers
		buffers_to_pass = [
			bytes([1, 2]),  # First buffer with 2 elements
			bytes([3, 4, 5]),  # Second buffer with 3 elements
			bytes([6, 7, 8, 9])  # Third buffer with 4 elements
		]
		pexpectThreeBuffers(buffers_to_pass)
		
		# Get 3 buffers
		retrieved_buffers = pgetThreeBuffers()
		
		# Assertions for retrieved buffers
		self.assertEqual(len(retrieved_buffers), 3)
		self.assertEqual(len(retrieved_buffers[0]), 4)  # First buffer length
		self.assertEqual(len(retrieved_buffers[1]), 3)  # Second buffer length
		self.assertEqual(len(retrieved_buffers[2]), 2)  # Third buffer length
		
		self.assertEqual(retrieved_buffers[0][0], 1)
		self.assertEqual(retrieved_buffers[0][1], 2)
		self.assertEqual(retrieved_buffers[0][2], 3)
		self.assertEqual(retrieved_buffers[0][3], 4)
		
		self.assertEqual(retrieved_buffers[1][0], 5)
		self.assertEqual(retrieved_buffers[1][1], 6)
		self.assertEqual(retrieved_buffers[1][2], 7)
		
		self.assertEqual(retrieved_buffers[2][0], 8)
		self.assertEqual(retrieved_buffers[2][1], 9)
		
		del pexpectThreeBuffers
		del pgetThreeBuffers
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_return_null(self):
		global test_runtime_module
		assert test_runtime_module is not None

		# Load the entity that is expected to return null
		preturn_null = test_runtime_module.load_entity('callable=return_null', [], [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type)])
		
		result = preturn_null()
		# Assert that the result is None, which is the expected equivalent of null in Python
		self.assertIsNone(result, "Expected result to be None")
		
		del preturn_null
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_returns_array_of_different_objects(self):
		global test_runtime_module
		assert test_runtime_module is not None

		# Load the entity that returns an array of different objects
		preturns_array_of_different_objects = test_runtime_module.load_entity('callable=returns_array_of_different_objects', [], [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_any_type)])
		
		# Execute the callable and get the result
		result = preturns_array_of_different_objects()
		
		# Assertions on the returned array
		self.assertIsInstance(result, list, "Result is not a list")
		self.assertEqual(len(result), 6, "Array length is not 6")
		
		# Asserting types of elements in the array
		self.assertIsInstance(result[0], int, "First element is not an int")
		self.assertEqual(result[0], 1, "First element value is not 1")
		
		self.assertIsInstance(result[1], str, "Second element is not a string")
		self.assertEqual(result[1], "string", "Second element value is not 'string'")
		
		self.assertIsInstance(result[2], float, "Third element is not a float")
		self.assertEqual(result[2], 3.0, "Third element value is not 3.0")
		
		self.assertIsNone(result[3], "Fourth element is not None")
		
		self.assertIsInstance(result[4], bytes, "Fifth element is not bytes")
		self.assertEqual(result[4], bytes([1, 2, 3]), "Fifth element value is not bytes([1, 2, 3])")
		
		# Assuming SomeClass() is represented as a dict or custom object in Python
		# Adjust this assertion based on the actual representation of SomeClass in Python
		self.assertTrue('SomeClass' in str(result[5]), "Sixth element is not a SomeClass object")
		
		del preturns_array_of_different_objects
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_call_any(self):
		global test_runtime_module
		assert test_runtime_module is not None

		# Load the callable that accepts any type
		pcall_any = test_runtime_module.load_entity('callable=accepts_any', [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_int64_type), metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_any_type)], None)
		
		# Load the SomeClass callable
		pnew_someclass = test_runtime_module.load_entity('callable=SomeClass', None, [metaffi.metaffi_type_info(metaffi.MetaFFITypes.metaffi_handle_type)])
		
		# Call with various types
		
		# Integer
		pcall_any(0, 1)
		# String
		pcall_any(1, 'string')
		# Float
		pcall_any(2, 3.0)
		# None
		pcall_any(3, None)
		# Bytes
		pcall_any(4, bytes([1, 2, 3]))
		# SomeClass instance
		some_class_instance = pnew_someclass()
		pcall_any(5, some_class_instance)
		
		del pcall_any
		del pnew_someclass
		
		# assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
