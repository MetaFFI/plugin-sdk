import ctypes.util
from typing import Callable, Any, Tuple, List
from . import metaffi_runtime, metaffi_types, xllr_wrapper

XCallParamsRetType = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p, ctypes.POINTER(ctypes.c_char_p))
XCallNoParamsRetType = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p, ctypes.POINTER(ctypes.c_char_p))
XCallParamsNoRetType = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p, ctypes.POINTER(ctypes.c_char_p))
XCallNoParamsNoRetType = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_void_p)


def make_metaffi_callable(f: Callable) -> Callable:
	params_metaffi_type_values, retval_metaffi_type_values = metaffi_types.get_callable_types(f)
	
	# Convert type values to metaffi_type_info instances
	params_metaffi_types = tuple(metaffi_types.metaffi_type_info(metaffi_types.MetaFFITypes(t)) for t in params_metaffi_type_values)
	retval_metaffi_types = tuple(metaffi_types.metaffi_type_info(metaffi_types.MetaFFITypes(t)) for t in retval_metaffi_type_values)
	
	# Create ctypes arrays for params_metaffi_types and retval_metaffi_types
	params_array_t = metaffi_types.metaffi_type_info * len(params_metaffi_types)
	params_array = params_array_t(*params_metaffi_types)
	
	retval_array_t = metaffi_types.metaffi_type_info * len(retval_metaffi_types)
	retvals_array = retval_array_t(*retval_metaffi_types)
	
	xllr_python3_bytes = 'xllr.python3'.encode('utf-8')
	
	pxcall_and_context_array = xllr_wrapper.make_callable(xllr_python3_bytes.decode(), f, params_array, len(params_metaffi_types), retvals_array, len(retval_metaffi_types))
	
	pxcall_and_context_array = ctypes.cast(pxcall_and_context_array, ctypes.POINTER(ctypes.c_void_p * 2))
	
	pxcall = pxcall_and_context_array.contents[0]
	context = pxcall_and_context_array.contents[1]
	
	# Create lambda that calls xllr_python3.call_xcall directly (same approach as load_entity)
	# Pass type values (integers) to call_xcall, not metaffi_type_info instances
	func_lambda: Callable[..., ...] = lambda *args: xllr_wrapper.xllr_python3.call_xcall(pxcall, context, params_metaffi_type_values, retval_metaffi_type_values, None if not args else args)
	
	setattr(func_lambda, 'pxcall_and_context', ctypes.addressof(pxcall_and_context_array.contents))
	setattr(func_lambda, 'params_metaffi_types', params_metaffi_types)
	setattr(func_lambda, 'retval_metaffi_types', retval_metaffi_types)
	return func_lambda


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


class _EntityCallable:
	"""Callable wrapper that invokes xllr_python3.call_xcall on each call.
	Uses attribute lookup at call time (no closure over the function) to avoid
	closure-related issues with the PyDLL / call_xcall reference.
	"""
	def __init__(self, pxcall: ctypes.c_void_p, pcontext: ctypes.c_void_p,
	             params_metaffi_types: tuple, retval_metaffi_types: tuple):
		self.pxcall = pxcall
		self.pcontext = pcontext
		self.params_metaffi_types = params_metaffi_types
		self.retval_metaffi_types = retval_metaffi_types

	def __call__(self, *args):
		return xllr_wrapper.xllr_python3.call_xcall(
			self.pxcall,
			self.pcontext,
			self.params_metaffi_types,
			self.retval_metaffi_types,
			None if not args else args,
		)


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
		
		# Call xllr.load_function
		xcall = xllr_wrapper.load_entity('xllr.' + self.runtime.runtime_plugin, self.module_path, entity_path, params_array, len(params_metaffi_types), retval_array, len(retval_metaffi_types))
		
		xcall_casted = ctypes.cast(xcall, ctypes.POINTER(VoidPtrArray))
		
		# xcall is void*[2]. xcall[0] is the function pointer, xcall[1] is the context.
		# get them into the parameter "pxcall" and "pcontext"
		pxcall = xcall_casted.contents.first
		pcontext = xcall_casted.contents.second
		
		# TODO: to this why pxcall and pxcontext are passed seprarately check py_metaffi_callable.cpp:49
		# Use a callable object instead of a lambda to avoid closure over xllr_wrapper / call_xcall
		wrapper = _EntityCallable(pxcall, pcontext, params_metaffi_types, retval_metaffi_types)
		return MetaFFIEntity('xllr.' + self.runtime.runtime_plugin, xcall, wrapper)
