from dataclasses import dataclass


class SomeClass:
	def __init__(self, name: str = "some"):
		self.name = name

	def print(self) -> str:
		return f"Hello from SomeClass {self.name}"

	def __str__(self) -> str:
		return f"SomeClass({self.name})"

	def __eq__(self, other) -> bool:
		return isinstance(other, SomeClass) and self.name == other.name


class TestMap:
	def __init__(self):
		self._dict = {}
		self.name = "name1"

	def set(self, key: str, value) -> None:
		self._dict[key] = value

	def get(self, key: str):
		return self._dict[key]

	def contains(self, key: str) -> bool:
		return key in self._dict


class BaseClass:
	def __init__(self, base_value: int):
		self.base_value = base_value

	def base_method(self) -> int:
		return self.base_value

	@classmethod
	def make_default(cls):
		return cls(7)

	@staticmethod
	def static_value() -> int:
		return 42


class DerivedClass(BaseClass):
	def __init__(self, base_value: int, extra: str):
		super().__init__(base_value)
		self.extra = extra

	def base_method(self) -> int:
		return self.base_value * 2

	def derived_method(self) -> str:
		return self.extra


class NestedContainer:
	class Inner:
		def __init__(self, value: int):
			self.value = value

		def get_value(self) -> int:
			return self.value

	def __init__(self, value: int):
		self.inner = NestedContainer.Inner(value)


class CallableClass:
	def __call__(self, x: int, y: int) -> int:
		return x + y


class IteratorClass:
	def __init__(self, n: int):
		self._n = n
		self._i = 0

	def __iter__(self):
		return self

	def __next__(self) -> int:
		if self._i >= self._n:
			raise StopIteration
		val = self._i
		self._i += 1
		return val


class ContextManager:
	def __init__(self):
		self.entered = False
		self.exited = False

	def __enter__(self):
		self.entered = True
		return self

	def __exit__(self, exc_type, exc_val, exc_tb):
		self.exited = True
		return False


@dataclass
class DataRecord:
	id: int
	name: str
