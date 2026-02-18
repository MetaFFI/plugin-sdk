import typing
from enum import Enum

from .objects_and_classes import SomeClass


class Color(Enum):
	RED = 1
	GREEN = 2
	BLUE = 3


def make_1d_array() -> list[int]:
	return [1, 2, 3]


def make_2d_array() -> list[list[int]]:
	return [[1, 2], [3, 4]]


def make_3d_array() -> list[list[list[int]]]:
	return [[[1], [2]], [[3], [4]]]


def make_ragged_array() -> list[list[int]]:
	return [[1, 2, 3], [4], [5, 6]]


def accepts_3d_array(arr: list[list[list[int]]]) -> int:
	return sum(sum(sum(inner) for inner in outer) for outer in arr)


def accepts_ragged_array(arr: list[list[int]]) -> int:
	return sum(sum(inner) for inner in arr)


def accepts_primitives(
	boolean: bool,
	integer: int,
	floating: float,
	complex_number: complex,
	byte_val: int,
	text: str,
	binary: bytes,
	byte_arr: bytearray,
) -> tuple:
	return boolean, integer, floating, complex_number, byte_val, text, binary, byte_arr


def accepts_collections(
	items_list,
	items_tuple,
	items_set,
	items_frozenset,
	items_dict,
) -> tuple:
	# Validate types
	if not isinstance(items_list, list):
		raise TypeError(f"Expected list, got {type(items_list)}")
	if not isinstance(items_tuple, tuple):
		raise TypeError(f"Expected tuple, got {type(items_tuple)}")
	if not isinstance(items_set, set):
		raise TypeError(f"Expected set, got {type(items_set)}")
	if not isinstance(items_frozenset, frozenset):
		raise TypeError(f"Expected frozenset, got {type(items_frozenset)}")
	if not isinstance(items_dict, dict):
		raise TypeError(f"Expected dict, got {type(items_dict)}")

	return items_list, items_tuple, items_set, items_frozenset, items_dict


def returns_bytes_buffer() -> bytes:
	return bytes([1, 2, 3])


def returns_bytearray() -> bytearray:
	return bytearray([4, 5, 6])


def returns_memoryview() -> memoryview:
	return memoryview(bytes([7, 8, 9]))


def returns_nested_dict() -> dict:
	return {"a": {"b": [1, 2, 3], "c": {"d": "e"}}}


def returns_list_of_objects() -> list[SomeClass]:
	return [SomeClass("a"), SomeClass("b"), SomeClass("c")]


def returns_optional(flag: bool) -> typing.Optional[int]:
	if flag:
		return 123
	return None


# --- Packed array test functions (1D primitive arrays) ---

def sum_1d_int_array(arr: list[int]) -> int:
	return sum(arr)


def echo_1d_int_array(arr: list[int]) -> list[int]:
	return list(arr)


def echo_1d_float_array(arr: list[float]) -> list[float]:
	return list(arr)


def make_1d_int_array() -> list[int]:
	return [10, 20, 30, 40, 50]


def generator_count(n: int):
	for i in range(n):
		yield i


async def async_add(x: int, y: int) -> int:
	return x + y
