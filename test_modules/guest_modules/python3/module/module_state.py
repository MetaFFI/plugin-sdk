CONSTANT_FIVE_SECONDS = 5

_mutable_state = {"counter": 0, "values": {}}


def get_counter() -> int:
	return _mutable_state["counter"]


def inc_counter(delta: int = 1) -> int:
	_mutable_state["counter"] += delta
	return _mutable_state["counter"]


def set_counter(value: int) -> None:
	_mutable_state["counter"] = value


def set_global_value(name: str, value) -> None:
	_mutable_state["values"][name] = value


def get_global_value(name: str):
	return _mutable_state["values"].get(name)


def closure_factory(start: int):
	value = start

	def add(delta: int) -> int:
		nonlocal value
		value += delta
		return value

	return add
