import ctypes
from ctypes import py_object
from typing import List, Union, Tuple
import os
import platform
from . import metaffi_types, xllr_wrapper
import sys

python_plugin_dir = 'python3'

if platform.system() == 'Windows':
	os.add_dll_directory(os.environ['METAFFI_HOME'])
	os.add_dll_directory(os.environ['METAFFI_HOME'] + f'\\{python_plugin_dir}\\')

xllr_python3 = ctypes.cdll.LoadLibrary(xllr_wrapper.get_dynamic_lib_path_from_metaffi_home(python_plugin_dir))

# Set argtypes and restype for convert_host_params_to_cdts
xllr_python3.convert_host_params_to_cdts.argtypes = [py_object, ctypes.POINTER(metaffi_types.metaffi_type_info), ctypes.c_uint64, ctypes.c_uint64]
xllr_python3.convert_host_params_to_cdts.restype = ctypes.c_void_p

# Set argtypes and restype for convert_host_return_values_from_cdts
xllr_python3.convert_host_return_values_from_cdts.argtypes = [ctypes.c_void_p, ctypes.c_uint64]
xllr_python3.convert_host_return_values_from_cdts.restype = py_object


def convert_to_metaffi_type_info_ptr(input_types: Union[Tuple[metaffi_types.metaffi_type_info], List[metaffi_types.metaffi_type_info]]) -> ctypes.POINTER(metaffi_types.metaffi_type_info):
	if not isinstance(input_types, (tuple, list)):
		raise ValueError("Input must be a tuple or list of metaffi_type_info objects")
	
	metaffi_type_info_Array = metaffi_types.metaffi_type_info * len(input_types)
	metaffi_type_info_array_instance = metaffi_type_info_Array(*input_types)
	
	return ctypes.pointer(metaffi_type_info_array_instance)


def convert_host_params_to_cdts(params_names: py_object, params_types: tuple | list, params_size: ctypes.c_uint64, return_values_size: ctypes.c_uint64) -> ctypes.c_void_p:
	pparams_types = convert_to_metaffi_type_info_ptr(params_types)
	res = xllr_python3.convert_host_params_to_cdts(params_names, pparams_types, params_size, return_values_size)
	return res


def convert_host_return_values_from_cdts(pcdts: ctypes.c_void_p, index: int) -> py_object:
	res = xllr_python3.convert_host_return_values_from_cdts(pcdts, index)
	return res
