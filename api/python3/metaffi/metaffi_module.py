import ctypes.util
from typing import Callable, Any, Tuple, List
from . import metaffi_runtime, metaffi_types, xllr_wrapper

XCallParamsRetType = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p, ctypes.POINTER(ctypes.c_char_p))
XCallNoParamsRetType = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p, ctypes.POINTER(ctypes.c_char_p))
XCallParamsNoRetType = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p, ctypes.POINTER(ctypes.c_char_p))
XCallNoParamsNoRetType = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p)

def make_metaffi_callable(f: Callable) -> Callable:
	params_metaffi_types, retval_metaffi_types = metaffi_types.get_callable_types(f)
	
	params_type = ctypes.c_uint64 * len(params_metaffi_types)
	params_array = params_type(*params_metaffi_types)
	
	retvals_type = ctypes.c_uint64 * len(retval_metaffi_types)
	retvals_array = retvals_type(*retval_metaffi_types)
	
	err = ctypes.c_char_p()
	err_len = ctypes.c_uint32()
	
xllr_python3_bytes = 'xllr.python3'.encode('utf-8')
	
	pxcall_and_context_array = xllr_wrapper.make_callable(xllr_python3_bytes.decode(), f, params_array,  len(params_metaffi_types), retvals_array, len(retval_metaffi_types))
	
	pxcall_and_context_array = ctypes.cast(pxcall_and_context_array, ctypes.POINTER(ctypes.c_void_p * 2))
	
	if len(params_metaffi_types) > 0 and len(retval_metaffi_types) > 0:
		pxcall = XCallParamsRetType(pxcall_and_context_array.contents[0])
	elif len(params_metaffi_types) > 0 and len(retval_metaffi_types) == 0:
		pxcall = XCallParamsNoRetType(pxcall_and_context_array.contents[0])
	elif len(params_metaffi_types) == 0 and len(retval_metaffi_types) > 0:
		pxcall = XCallNoParamsRetType(pxcall_and_context_array.contents[0])
	else:
		pxcall = XCallNoParamsNoRetType(pxcall_and_context_array.contents[0])
	
	context = pxcall_and_context_array.contents[1]
	
	res = create_lambda(pxcall, context, params_metaffi_types, retval_metaffi_types) # type: ignore - loaded at runtime
	setattr(res, 'pxcall_and_context', ctypes.addressof(pxcall_and_context_array.contents))
	setattr(res, 'params_metaffi_types', params_metaffi_types)
	setattr(res, 'retval_metaffi_types', retval_metaffi_types)
	return res


class MetaFFIEntity:
	def __init__(self, runtime_name: str, pxcall: ctypes.c_void_p, wrapping_lambda: Callable[..., Tuple[Any, ...]]):
		self.calling_lambda = wrapping_lambda
		self.pxcall = pxcall
		self.runtime_name = runtime_name
		
	def __call__(self, *args):
		result = self.calling_lambda(*args)
		if result is not None and len(result) == 1:
			return result[0]
		else:
			return result
	
	def __del__(self):
		xllr_wrapper.free_xcall(self.runtime_name, self.pxcall)
		
class VoidPtrArray(ctypes.Structure):
    _fields_ = [("first", ctypes.c_void_p),
                ("second", ctypes.c_void_p)]

class MetaFFIModule:
	def __init__(self, runtime: metaffi_runtime.MetaFFIRuntime, module_path: str):
		self.runtime = runtime
		self.module_path = module_path
	
	def load_entity(self, entity_path: str, params_metaffi_types: Tuple[metaffi_types.metaffi_type_info, ...] | List[metaffi_types.metaffi_type_info] | None,
			retval_metaffi_types: Tuple[metaffi_types.metaffi_type_info, ...] | List[metaffi_types.metaffi_type_info] | None) -> MetaFFIEntity:
		
		if params_metaffi_types is None:
			params_metaffi_types = tuple()
		
		if retval_metaffi_types is None:
			retval_metaffi_types = tuple()

		if isinstance(params_metaffi_types, list):
			params_metaffi_types = tuple(params_metaffi_types)
		
		if not isinstance(retval_metaffi_types, tuple):
			retval_metaffi_types = tuple(retval_metaffi_types)
		
		# Create ctypes arrays for params_metaffi_types and retval_metaffi_types
		params_array_t = metaffi_types.metaffi_type_info * len(params_metaffi_types)
		params_array = params_array_t(*params_metaffi_types)
		
		retval_array_t = metaffi_types.metaffi_type_info * len(retval_metaffi_types)
		retval_array = retval_array_t(*retval_metaffi_types)
		# 
		# if parameter is a list - convert it to tuple
		if not isinstance(params_metaffi_types, tuple) and not isinstance(params_metaffi_types, list):
			raise ValueError('params_metaffi_types must be a list or tuple')
		
		if not isinstance(retval_metaffi_types, tuple) and not isinstance(retval_metaffi_types, list):
			raise ValueError('retval_metaffi_types must be a list or tuple')
		
				
		# Call xllr.load_function
		xcall = xllr_wrapper.load_entity('xllr.' + self.runtime.runtime_plugin, self.module_path, entity_path, params_array, len(params_metaffi_types), retval_array, len(retval_metaffi_types))
		
		xcall_casted = ctypes.cast(xcall, ctypes.POINTER(VoidPtrArray))

		# xcall is void*[2]. xcall[0] is the function pointer, xcall[1] is the context.
		# get them into the parameter "pxcall" and "pcontext"
		pxcall = xcall_casted.contents.first
		pcontext = xcall_casted.contents.second

		# TODO: to this why pxcall and pxcontext are passed seprarately check py_metaffi_callable.cpp:49
		func_lambda: Callable[..., ...] = lambda *args: xllr_wrapper.xllr_python3.call_xcall(pxcall, pcontext, params_metaffi_types, retval_metaffi_types, None if not args else args)
		
		return MetaFFIEntity('xllr.' + self.runtime.runtime_plugin, xcall, func_lambda)
