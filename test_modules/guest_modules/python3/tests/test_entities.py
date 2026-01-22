import asyncio
import os
import sys
import unittest

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.abspath(os.path.join(CURRENT_DIR, ".."))
sys.path.insert(0, ROOT_DIR)

from module import (
	hello_world,
	returns_an_error,
	div_integers,
	join_strings,
	wait_a_bit,
	return_null,
	call_callback_add,
	return_callback_add,
	return_multiple_return_values,
	returns_array_with_different_dimensions,
	returns_array_of_different_objects,
	return_any,
	accepts_any,
	get_three_buffers,
	expect_three_buffers,
	get_some_classes,
	expect_three_some_classes,
	SomeClass,
	TestMap,
	BaseClass,
	DerivedClass,
	NestedContainer,
	CallableClass,
	IteratorClass,
	ContextManager,
	DataRecord,
	Color,
	make_1d_array,
	make_2d_array,
	make_3d_array,
	make_ragged_array,
	accepts_3d_array,
	accepts_ragged_array,
	accepts_primitives,
	accepts_collections,
	returns_bytes_buffer,
	returns_bytearray,
	returns_memoryview,
	returns_nested_dict,
	returns_list_of_objects,
	returns_optional,
	generator_count,
	async_add,
	CustomError,
	raise_custom_error,
	return_error_tuple,
	callback_raises_error,
	positional_only,
	keyword_only,
	var_positional,
	var_keyword,
	var_positional_and_keyword,
	mixed_args,
	default_args,
	overload,
	accepts_star_only,
	accepts_kwargs,
	CONSTANT_FIVE_SECONDS,
	get_counter,
	inc_counter,
	set_counter,
	get_global_value,
	set_global_value,
	closure_factory,
)


class TestEntities(unittest.TestCase):
	def test_core_functions(self):
		self.assertIn("Hello World", hello_world())
		with self.assertRaises(Exception):
			returns_an_error()
		self.assertEqual(5.0, div_integers(10, 2))
		self.assertEqual("a,b,c", join_strings(["a", "b", "c"]))
		self.assertIsNone(wait_a_bit(1))
		self.assertIsNone(return_null())

	def test_callbacks(self):
		def add(a, b):
			return a + b
		self.assertEqual(3, call_callback_add(add))
		cb = return_callback_add()
		self.assertEqual(7, cb(3, 4))

	def test_multiple_returns_and_any(self):
		ret = return_multiple_return_values()
		self.assertEqual(6, len(ret))
		self.assertIsInstance(ret[5], SomeClass)
		self.assertIsInstance(returns_array_of_different_objects()[5], SomeClass)
		self.assertIsInstance(return_any(4), SomeClass)
		accepts_any(0, 1)
		accepts_any(1, "string")
		accepts_any(2, 3.0)
		accepts_any(3, None)
		accepts_any(4, bytes([1]))
		accepts_any(5, SomeClass())

	def test_buffers(self):
		buffers = get_three_buffers()
		self.assertEqual(3, len(buffers))
		expect_three_buffers(buffers)

	def test_objects(self):
		obj = SomeClass("x")
		self.assertIn("x", obj.print())
		tm = TestMap()
		tm.set("k", 1)
		self.assertTrue(tm.contains("k"))
		self.assertEqual(1, tm.get("k"))
		base = BaseClass.make_default()
		self.assertEqual(7, base.base_method())
		derived = DerivedClass(2, "extra")
		self.assertEqual(4, derived.base_method())
		self.assertEqual("extra", derived.derived_method())
		self.assertEqual(42, BaseClass.static_value())
		nested = NestedContainer(5)
		self.assertEqual(5, nested.inner.get_value())
		callable_obj = CallableClass()
		self.assertEqual(5, callable_obj(2, 3))
		it = IteratorClass(3)
		self.assertEqual([0, 1, 2], list(it))
		with ContextManager() as cm:
			self.assertTrue(cm.entered)
		self.assertTrue(cm.exited)
		record = DataRecord(1, "name")
		self.assertEqual(1, record.id)
		classes = get_some_classes()
		self.assertEqual(3, len(classes))
		expect_three_some_classes(classes)

	def test_types_and_arrays(self):
		self.assertEqual([1, 2, 3], make_1d_array())
		self.assertEqual([[1, 2], [3, 4]], make_2d_array())
		self.assertEqual([[[1], [2]], [[3], [4]]], make_3d_array())
		self.assertEqual([[1, 2, 3], [4], [5, 6]], make_ragged_array())
		self.assertEqual(10, accepts_3d_array(make_3d_array()))
		self.assertEqual(21, accepts_ragged_array([[1, 2, 3], [4], [5, 6]]))
		self.assertEqual(Color.RED, Color(1))
		prims = accepts_primitives(True, 1, 2.0, 3j, 255, "s", b"x", bytearray(b"y"))
		self.assertEqual(True, prims[0])
		colls = accepts_collections([1], (2,), {3}, frozenset({4}), {"k": 5})
		self.assertEqual([1], colls[0])
		self.assertEqual(b"\x01\x02\x03", returns_bytes_buffer())
		self.assertEqual(bytearray([4, 5, 6]), returns_bytearray())
		self.assertEqual(bytes([7, 8, 9]), returns_memoryview().tobytes())
		self.assertIn("a", returns_nested_dict())
		self.assertEqual(3, len(returns_list_of_objects()))
		self.assertEqual(123, returns_optional(True))
		self.assertIsNone(returns_optional(False))
		self.assertEqual([0, 1, 2], list(generator_count(3)))
		self.assertEqual(5, asyncio.run(async_add(2, 3)))

	def test_errors(self):
		with self.assertRaises(CustomError):
			raise_custom_error("boom")
		self.assertEqual((True, None), return_error_tuple(True))
		self.assertEqual((False, "error"), return_error_tuple(False))

		def cb():
			raise RuntimeError("callback error")
		self.assertIn("callback error", callback_raises_error(cb))

	def test_args_and_signatures(self):
		self.assertEqual(6, positional_only(1, 2, 3))
		self.assertEqual("x", keyword_only(named="x"))
		self.assertEqual([1, 2], var_positional(1, 2))
		self.assertEqual({"a": 1}, var_keyword(a=1))
		self.assertEqual(([1, 2], {"a": 3}), var_positional_and_keyword(1, 2, a=3))
		self.assertEqual((1, 2, [3], 4, {"x": 5}), mixed_args(1, 2, 3, c=4, x=5))
		self.assertEqual((1, "x", None), default_args())
		self.assertEqual(2, overload(1))
		self.assertEqual("ABC", overload("abc"))
		self.assertEqual(3, overload([1, 2, 3]))
		self.assertEqual((1, 2), accepts_star_only(1, named=2))
		self.assertEqual(("v", {"k": "v"}), accepts_kwargs("v", k="v"))

	def test_module_state(self):
		self.assertEqual(5, CONSTANT_FIVE_SECONDS)
		set_counter(0)
		self.assertEqual(1, inc_counter())
		set_global_value("k", "v")
		self.assertEqual("v", get_global_value("k"))
		adder = closure_factory(10)
		self.assertEqual(12, adder(2))


if __name__ == "__main__":
	unittest.main()
