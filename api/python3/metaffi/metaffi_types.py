import ctypes.util
import inspect
from enum import IntFlag
import platform
from typing import Callable, Tuple, get_type_hints


# This should be taken from metaffi_primitives.h
class MetaFFITypes(IntFlag):
	metaffi_float64_type = 1
	metaffi_float32_type = 2
	metaffi_int8_type = 4
	metaffi_int16_type = 8
	metaffi_int32_type = 16
	metaffi_int64_type = 32
	metaffi_uint8_type = 64
	metaffi_uint16_type = 128
	metaffi_uint32_type = 256
	metaffi_uint64_type = 512
	metaffi_bool_type = 1024
	metaffi_char8_type = 524288
	metaffi_char16_type = 1048576
	metaffi_char32_type = 2097152
	metaffi_string8_type = 4096
	metaffi_string16_type = 8192
	metaffi_string32_type = 16384
	metaffi_handle_type = 32768
	metaffi_array_type = 65536
	metaffi_size_type = 262144
	metaffi_any_type = 4194304
	metaffi_null_type = 8388608
	metaffi_callable_type = 16777216
	metaffi_float64_array_type = metaffi_float64_type | metaffi_array_type
	metaffi_float32_array_type = metaffi_float32_type | metaffi_array_type
	metaffi_int8_array_type = metaffi_int8_type | metaffi_array_type
	metaffi_int16_array_type = metaffi_int16_type | metaffi_array_type
	metaffi_int32_array_type = metaffi_int32_type | metaffi_array_type
	metaffi_int64_array_type = metaffi_int64_type | metaffi_array_type
	metaffi_uint8_array_type = metaffi_uint8_type | metaffi_array_type
	metaffi_uint16_array_type = metaffi_uint16_type | metaffi_array_type
	metaffi_uint32_array_type = metaffi_uint32_type | metaffi_array_type
	metaffi_uint64_array_type = metaffi_uint64_type | metaffi_array_type
	metaffi_bool_array_type = metaffi_bool_type | metaffi_array_type
	metaffi_char8_array_type = metaffi_char8_type | metaffi_array_type
	metaffi_string8_array_type = metaffi_string8_type | metaffi_array_type
	metaffi_string16_array_type = metaffi_string16_type | metaffi_array_type
	metaffi_string32_array_type = metaffi_string32_type | metaffi_array_type
	metaffi_any_array_type = metaffi_any_type | metaffi_array_type
	metaffi_handle_array_type = metaffi_handle_type | metaffi_array_type
	metaffi_size_array_type = metaffi_size_type | metaffi_array_type
	
	def describe(self):
		return self.name, self.value


# Define the struct metaffi_type_info in Python using ctypes
class metaffi_type_info(ctypes.Structure):
	_fields_ = [("type", ctypes.c_uint64),  # metaffi_type is defined as uint64_t
	            ("alias", ctypes.c_char_p),
	            ("is_free_alias", ctypes.c_bool),
	            ("fixed_dimensions", ctypes.c_int64)]
	
	def __init__(self, metaffi_type: MetaFFITypes = MetaFFITypes.metaffi_null_type, alias: str | None = None, dims: int = 0):
		super().__init__()
		
		# Set the type
		self.type = ctypes.c_uint64(metaffi_type.value)
		self.fixed_dimensions = dims
		
		# If alias is not None, set the alias and alias_length
		if alias is not None:
			self.alias = ctypes.c_char_p(alias.encode('utf-8'))
			self.is_free_alias = True
		else:
			# If alias is None, set the alias to NULL and alias_length to 0
			self.alias = None
			self.is_free_alias = False


# Define the pointer type for metaffi_type_info
metaffi_type_info_p = ctypes.POINTER(metaffi_type_info)

pytype_to_metaffi_type_dict = {
	'str': MetaFFITypes.metaffi_string8_type.value,
	'int': MetaFFITypes.metaffi_int64_type.value,
	'float': MetaFFITypes.metaffi_float64_type.value,
	'bool': MetaFFITypes.metaffi_bool_type.value,
	'list': MetaFFITypes.metaffi_any_type.value,
	'tuple': MetaFFITypes.metaffi_any_type.value,
	'Tuple': MetaFFITypes.metaffi_any_type.value
}


def pytype_to_metaffi_type(t: type):
	global pytype_to_metaffi_type_dict
	
	if t.__name__ in pytype_to_metaffi_type_dict:
		return pytype_to_metaffi_type_dict[t.__name__]
	
	return MetaFFITypes.metaffi_handle_type.value


def get_callable_types(callable: Callable) -> Tuple[tuple[int], tuple[int]]:
	type_hints = get_type_hints(callable)
	param_metaffi_types = []
	return_metaffi_types = []
	
	if 'return' in type_hints:
		if hasattr(type_hints['return'], '__args__'):
			return_metaffi_types.extend(pytype_to_metaffi_type(t) for t in type_hints['return'].__args__)
		else:
			return_metaffi_types.append(pytype_to_metaffi_type(type_hints['return']))
	else:
		return_metaffi_types.append(pytype_to_metaffi_type(ctypes.py_object))
	
	params = inspect.signature(callable).parameters
	for param in params:
		if param in type_hints:
			param_metaffi_types.append(pytype_to_metaffi_type(type_hints[param]))
		else:
			param_metaffi_types.append(pytype_to_metaffi_type(ctypes.py_object))
	
	return tuple(param_metaffi_types), tuple(return_metaffi_types)
