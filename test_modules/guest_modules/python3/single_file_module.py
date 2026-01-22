class SomeClass:
	def __init__(self, name: str = "single"):
		self.name = name

	def print(self) -> str:
		return f"Hello from SomeClass {self.name}"


class TestMap:
	def __init__(self):
		self._dict = {}
		self.name = "name1"

	def set(self, key: str, value):
		self._dict[key] = value

	def get(self, key: str):
		return self._dict[key]

	def contains(self, key: str) -> bool:
		return key in self._dict


def hello_world() -> str:
	return "Hello World, from Python3 (single file)"


def return_null():
	return None


def get_three_buffers():
	return [bytes([1, 2, 3, 4]), bytes([5, 6, 7]), bytes([8, 9])]


def positional_only(a, b, /, c=0):
	return a + b + c


def keyword_only(*, named: str):
	return named


def var_args(*args):
	return list(args)


def var_kwargs(**kwargs):
	return dict(kwargs)
