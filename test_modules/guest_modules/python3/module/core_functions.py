import typing

from .objects_and_classes import SomeClass


def hello_world() -> str:
	return "Hello World, from Python3"


def returns_an_error() -> None:
	raise Exception("Error")


def div_integers(x: int, y: int) -> float:
	return x / y


def join_strings(arr) -> str:
	return ",".join(arr)


def wait_a_bit(secs: int) -> None:
	return None


def no_op():
	pass


def return_null():
	return None


def call_callback_add(add_function):
	res = add_function(1, 2)
	if isinstance(res, (list, tuple)):
		res = res[0]
	if res != 3:
		raise ValueError(f"expected 3, got {res}")
	return res


def return_callback_add():
	def add(a: int, b: int):
		return a + b

	return add


def return_multiple_return_values():
	return 1, "string", 3.0, None, bytes([1, 2, 3]), SomeClass()


def returns_array_with_different_dimensions():
	return [[1, 2, 3], 4, [[5, 6], [7, 8]]]


def returns_array_of_different_objects():
	return [1, "string", 3.0, None, bytes([1, 2, 3]), SomeClass()]


def return_any(which_type: int) -> typing.Any:
	if which_type == 0:
		return 1
	if which_type == 1:
		return "string"
	if which_type == 2:
		return 3.0
	if which_type == 3:
		return ["list", "of", "strings"]
	if which_type == 4:
		return SomeClass()
	return None


def accepts_any(which_type_to_expect: int, val: typing.Any):
	if which_type_to_expect == 0 and not isinstance(val, int):
		raise ValueError(f"Expected int, got {type(val)}")
	if which_type_to_expect == 1 and not isinstance(val, str):
		raise ValueError(f"Expected str, got {type(val)}")
	if which_type_to_expect == 2 and not isinstance(val, float):
		raise ValueError(f"Expected float, got {type(val)}")
	if which_type_to_expect == 3 and val is not None:
		raise ValueError(f"Expected None, got {type(val)}")
	if which_type_to_expect == 4 and not isinstance(val, (bytes, bytearray)):
		raise ValueError(f"Expected bytes, got {type(val)}")
	if which_type_to_expect == 5 and not isinstance(val, SomeClass):
		raise ValueError(f"Expected SomeClass, got {type(val)}")


def echo_any(val: typing.Any) -> typing.Any:
	return val


def get_three_buffers():
	return [bytes([1, 2, 3, 4]), bytes([5, 6, 7]), bytes([8, 9])]


def expect_three_buffers(buffers):
	if len(buffers) != 3:
		raise ValueError("Buffers length is not 3")
	if buffers[0] != bytes([1, 2, 3, 4]):
		raise ValueError("1st buffer mismatch")
	if buffers[1] != bytes([5, 6, 7]):
		raise ValueError("2nd buffer mismatch")
	if buffers[2] != bytes([8, 9]):
		raise ValueError("3rd buffer mismatch")


def get_some_classes():
	return [SomeClass() for _ in range(3)]


def expect_three_some_classes(arr):
	if len(arr) != 3:
		raise ValueError("Array length is not 3")
	for el in arr:
		if not isinstance(el, SomeClass):
			raise ValueError(f"Element is not SomeClass: {type(el)}")
