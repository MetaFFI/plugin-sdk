from functools import singledispatch


def positional_only(a, b, /, c=3):
	return a + b + c


def keyword_only(*, named: str):
	return named


def var_positional(*args):
	return list(args)


def var_keyword(**kwargs):
	return dict(kwargs)


def var_positional_and_keyword(*args, **kwargs):
	return list(args), dict(kwargs)


def mixed_args(a, b=2, *args, c=3, **kwargs):
	return a, b, list(args), c, dict(kwargs)


def default_args(a=1, b="x", c=None):
	return a, b, c


@singledispatch
def overload(value):
	return f"unknown:{value}"


@overload.register
def _(value: int):
	return value + 1


@overload.register
def _(value: str):
	return value.upper()


@overload.register
def _(value: list):
	return len(value)


def accepts_star_only(value, *, named=1):
	return value, named


def accepts_kwargs(value="default", **kwargs):
	return value, kwargs
