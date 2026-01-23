import ctypes
from ctypes import cdll
from .metaffi_types import *
import platform
import os
import sys

metaffi_home = os.getenv('METAFFI_HOME')
if metaffi_home is None:
	raise RuntimeError('METAFFI_HOME environment variable is not set')

python_plugin_dir = 'python3'

assert isinstance(metaffi_home, str)

def get_dynamic_lib_path_from_metaffi_home(fname: str):
	global metaffi_home
	assert isinstance(metaffi_home, str)
	assert isinstance(fname, str)

	if fname != 'xllr':
		fname = f'/{fname}/xllr.{fname}'

	osname = platform.system()
	
	if osname == 'Windows':
		return metaffi_home + '\\' + fname + '.dll'
	elif osname == 'Darwin':
		return metaffi_home + '/' + fname + '.dylib'
	else:
		return metaffi_home + '/' + fname + '.so'  # for everything that is not windows or mac, return .so


if platform.system() == 'Windows':
	os.add_dll_directory(metaffi_home)
	os.add_dll_directory(metaffi_home + f'\\{python_plugin_dir}\\')

xllr_python3 = ctypes.cdll.LoadLibrary(get_dynamic_lib_path_from_metaffi_home(python_plugin_dir))
xllr_python3.call_xcall.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.py_object, ctypes.py_object, ctypes.py_object]
xllr_python3.call_xcall.restype = ctypes.py_object

_xllr = cdll.LoadLibrary(get_dynamic_lib_path_from_metaffi_home('xllr'))

# Set argtypes and restype
_xllr.load_runtime_plugin.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_char_p)]
_xllr.load_runtime_plugin.restype = None

_xllr.free_runtime_plugin.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_char_p)]
_xllr.free_runtime_plugin.restype = None

_xllr.load_entity.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.POINTER(metaffi_type_info), ctypes.c_int8, ctypes.POINTER(metaffi_type_info), ctypes.c_int8, ctypes.POINTER(ctypes.c_char_p)]
_xllr.load_entity.restype = ctypes.c_void_p

_xllr.free_xcall.argtypes = [ctypes.c_char_p, ctypes.c_void_p, ctypes.POINTER(ctypes.c_char_p)]
_xllr.free_xcall.restype = None

_xllr.make_callable.argtypes = [ctypes.c_char_p, ctypes.py_object, ctypes.POINTER(metaffi_type_info), ctypes.c_int8, ctypes.POINTER(metaffi_type_info), ctypes.c_int8, ctypes.POINTER(ctypes.c_char_p)]
_xllr.make_callable.restype = ctypes.c_void_p

_xllr.alloc_cdts_buffer.argtypes = [ctypes.c_uint64, ctypes.c_uint64]
_xllr.alloc_cdts_buffer.restype = ctypes.c_void_p

_xllr.free_cdts_buffer.argtypes = [ctypes.c_void_p]
_xllr.free_cdts_buffer.restype = None

_xllr.free_string.argtypes = [ctypes.c_char_p]
_xllr.free_string.restype = None


def load_runtime_plugin(runtime_plugin: str) -> None:
	global _xllr
	err = ctypes.c_char_p()
	_xllr.load_runtime_plugin(runtime_plugin.encode('utf-8'), ctypes.byref(err))
	
	# check if err is not NULL
	if err.value is not None:
		msg = err.value.decode('utf-8')
		_xllr.free_string(err)  # call xllr.free_string to free the memory allocated by xllr
		raise RuntimeError(msg)


def free_runtime_plugin(runtime_plugin: str) -> None:
	err = ctypes.c_char_p()
	_xllr.free_runtime_plugin(runtime_plugin.encode('utf-8'), ctypes.byref(err))
	
	# check if err is not NULL
	if err.value is not None:
		msg = err.value.decode('utf-8')
		_xllr.free_string(err)  # call xllr.free_string to free the memory allocated by xllr
		raise RuntimeError(msg)


def load_entity(runtime_plugin_name: str, module_path: str, entity_path: str,
		params_types: ctypes.POINTER(metaffi_type_info), params_count: int,
		retvals_types: ctypes.POINTER(metaffi_type_info), retval_count: int) -> ctypes.c_void_p:
	err = ctypes.c_char_p()
	result = _xllr.load_entity(runtime_plugin_name.encode('utf-8'), module_path.encode('utf-8'),
		entity_path.encode('utf-8'), params_types, params_count, retvals_types,
		retval_count, ctypes.byref(err))
	
	# check if err is not NULL
	if err.value is not None:
		msg = err.value.decode('utf-8')
		_xllr.free_string(err)  # call xllr.free_string to free the memory allocated by xllr
		raise RuntimeError(msg)
	
	return result


def free_xcall(runtime_plugin_name: str, pxcall: ctypes.c_void_p) -> None:
	err = ctypes.c_char_p()
	_xllr.free_xcall(ctypes.c_char_p(runtime_plugin_name.encode('utf-8')), pxcall, ctypes.byref(err))
	# check if err is not NULL
	if err.value is not None:
		msg = err.value.decode('utf-8')
		_xllr.free_string(err)  # call xllr.free_string to free the memory allocated by xllr
		raise RuntimeError(msg)


def make_callable(runtime_plugin_name: str, f: Callable, params_types: ctypes.POINTER(metaffi_type_info), params_count: int, retvals_types: ctypes.POINTER(metaffi_type_info), retval_count: int) -> ctypes.c_void_p:
	err = ctypes.c_char_p()
	result = _xllr.make_callable(runtime_plugin_name.encode('utf-8'), f, params_types, params_count, retvals_types, retval_count, ctypes.byref(err))
	
	# check if err is not NULL
	if err.value is not None:
		msg = err.value.decode('utf-8')
		_xllr.free_string(err)  # call xllr.free_string to free the memory allocated by xllr
		raise RuntimeError(msg)
	
	return result


def alloc_cdts_buffer(params_count: int, ret_count: int) -> ctypes.c_void_p:
	return _xllr.alloc_cdts_buffer(params_count, ret_count)


def free_cdts_buffer(buffer: ctypes.c_void_p) -> None:
	_xllr.free_cdts_buffer(buffer)
