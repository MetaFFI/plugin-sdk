import ctypes


class CDTMetaFFIHandle(ctypes.Structure):
	_fields_ = [
		('handle', ctypes.c_void_p),
		('runtime_id', ctypes.c_uint64),
		('releaser', ctypes.c_void_p)
	]


ReleaserFuncType = ctypes.CFUNCTYPE(None, CDTMetaFFIHandle)


class MetaFFIHandle(ctypes.Structure):
	def __init__(self, h, runtime_id, releaser, *args, **kw):
		super().__init__(*args, **kw)
		self.handle = int(h)
		self.runtime_id = int(runtime_id)
		self.releaser = int(releaser)
	
	def release(self):
		if self.releaser:
			release_func = ctypes.CFUNCTYPE(None, ctypes.POINTER(CDTMetaFFIHandle))(self.releaser)
			handle_instance = CDTMetaFFIHandle(ctypes.c_void_p(self.val), ctypes.c_uint64(self.runtime_id), ctypes.cast(self.releaser, ctypes.c_void_p))
			release_func(ctypes.byref(handle_instance))
	
	def detach(self):
		self.releaser = None
	
	def __exit__(self, exc_type, exc_val, exc_tb):
		self.release()
	
	def __del__(self):
		self.release()
	
	def __str__(self):
		return f'MetaFFIHandle({self.handle}, {self.runtime_id}, {self.releaser})'
