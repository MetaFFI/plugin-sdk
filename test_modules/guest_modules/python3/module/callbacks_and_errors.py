class CustomError(Exception):
	pass


def raise_custom_error(msg: str) -> None:
	raise CustomError(msg)


def return_error_tuple(ok: bool):
	if ok:
		return True, None
	return False, "error"


def callback_raises_error(cb):
	try:
		cb()
	except Exception as exc:
		return str(exc)
	return None
