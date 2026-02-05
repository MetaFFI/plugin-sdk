package metaffi.api.accessor;

public class MetaFFIAccessor
{
	public static native void load_runtime_plugin(String runtime_plugin);
	public static native void free_runtime_plugin(String runtime_plugin);
	public static native long load_function(String runtime_plugin, String module_path, String entity_path, MetaFFITypeInfo[] params_types, MetaFFITypeInfo[] retval_types);
	public static native long load_callable(String runtime_plugin, Object method, String jni_signature, MetaFFITypeInfo[] params_types, MetaFFITypeInfo[] retval_types);
	public static native void free_function(String runtime_plugin, long function_id);
	public static native void xcall_params_ret(long pxcallAndContext, long xcall_params);
	public static native void xcall_no_params_ret(long pxcallAndContext, long xcall_params);
	public static native void xcall_params_no_ret(long pxcallAndContext, long xcall_params);
	public static native void xcall_no_params_no_ret(long pxcallAndContext);
	public static native long java_to_cdts(long pcdt, Object[] params, long[] parameterTypes);
	public static native Object[] cdts_to_java(long pcdt, long length);
	public static native long alloc_cdts(byte params_count, byte retval_count);
	public static native long get_pcdt(long pcdts, byte index);
	public static native Object get_object(long phandle);
	public static native void remove_object(long phandle);

	static
	{
		String libExtension = System.mapLibraryName("metaffi.api.accessor");
		libExtension = libExtension.substring(libExtension.lastIndexOf("."));
		String metaffiHome = System.getenv("METAFFI_HOME");

		String primary = metaffiHome + "/jvm/metaffi.api.accessor" + libExtension;
		String fallback = metaffiHome + "/sdk/api/jvm/metaffi.api.accessor" + libExtension;

		java.io.File primaryFile = new java.io.File(primary);
		if (primaryFile.exists()) {
			System.load(primary);
		} else {
			System.load(fallback);
		}


	}

	//--------------------------------------------------------------------
	private MetaFFIAccessor() {}
	//--------------------------------------------------------------------
	public static long getMetaFFIType(Object o)
	{
		// TODO: get numbers directly from C++, and not hard-coded
		if(o instanceof Double) return 1;
		if(o instanceof double[] || o instanceof Double[]) return 1 | 65536;
		if(o instanceof Float) return 2;
		if(o instanceof float[] || o instanceof Float[]) return 2 | 65536;
		if(o instanceof Byte) return 4;
		if(o instanceof byte[] || o instanceof Byte[]) return 4 | 65536;
		if(o instanceof Short) return 8;
		if(o instanceof short[] || o instanceof Short[]) return 8 | 65536;
		if(o instanceof Integer) return 16;
		if(o instanceof int[] || o instanceof Integer[]) return 16 | 65536;
		if(o instanceof Long) return 32;
		if(o instanceof long[] || o instanceof Long[]) return 32 | 65536;
		if(o instanceof Boolean) return 1024;
		if(o instanceof boolean[] || o instanceof Boolean[]) return 1024 | 65536;
		if(o instanceof String) return 4096;
		if(o instanceof String[]) return 4096 | 65536;
		return 32768;
	}
	//--------------------------------------------------------------------
}
