package api;

import metaffi.api.accessor.Caller;
import metaffi.api.accessor.MetaFFIAccessor;
import metaffi.api.accessor.MetaFFITypeInfo;

import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.List;

public class MetaFFIRuntime
{
	public final String runtimePlugin;

	public MetaFFIRuntime(String runtimePlugin)
	{
		String metaFFIHome = System.getenv("METAFFI_HOME");
		if (metaFFIHome == null) {
			throw new IllegalStateException("METAFFI_HOME environment variable is not set");
		}

		this.runtimePlugin = runtimePlugin;
	}

	public void loadRuntimePlugin()
	{
		MetaFFIAccessor.load_runtime_plugin("xllr."+this.runtimePlugin);
	}

	public void releaseRuntimePlugin()
	{
		MetaFFIAccessor.free_runtime_plugin("xllr."+this.runtimePlugin);
	}

	public api.MetaFFIModule loadModule(String modulePath)
	{
		return new api.MetaFFIModule(this, modulePath);
	}

	public static Caller makeMetaFFICallable(Method m) throws NoSuchMethodException
	{
		ArrayList<MetaFFITypeInfo> outParameters = new ArrayList<>();
		ArrayList<MetaFFITypeInfo> outRetvals = new ArrayList<>();
		String jniSignature = MetaFFIRuntime.getJNISignature(m, outParameters, outRetvals);

		var params = outParameters.toArray(new MetaFFITypeInfo[]{});
		var retvals = outRetvals.toArray(new MetaFFITypeInfo[]{});
		long xcall_and_context = MetaFFIAccessor.load_callable("xllr.jvm", m, jniSignature, params, retvals);

		return Caller.createCaller(xcall_and_context, params, retvals);
	}

	private static String getJNISignature(Method method, List<MetaFFITypeInfo> outParameters, List<MetaFFITypeInfo> outRetvals) throws NoSuchMethodException
	{
		// Get the parameter types
		Type[] paramTypes = method.getGenericParameterTypes();

		// Get the return type
		Type returnType = method.getGenericReturnType();

		// Convert the types to JNI signatures
		StringBuilder signature = new StringBuilder("(");
		for (Type paramType : paramTypes)
		{
			String jniSig = getJNISignature(paramType);
			outParameters.add(new MetaFFITypeInfo(jniSig));
			signature.append(jniSig);
		}
		signature.append(")");

		String jniSig = getJNISignature(returnType);
		if(!"V".equals(jniSig))
		{
			outRetvals.add(new MetaFFITypeInfo(jniSig));
		}
		signature.append(jniSig);

		return signature.toString();
	}

	private static String getJNISignature(Type type)
	{
		String typeName = type.getTypeName();
		StringBuilder signature = new StringBuilder();
		while (typeName.endsWith("[]"))
		{
			// It's an array type
			signature.append("[");
			typeName = typeName.substring(0, typeName.length() - 2);
		}

		switch (typeName)
		{
			case "void": signature.append("V"); break;
			case "boolean": signature.append("Z"); break;
			case "byte": signature.append("B"); break;
			case "char": signature.append("C"); break;
			case "short": signature.append("S"); break;
			case "int": signature.append("I"); break;
			case "long": signature.append("J"); break;
			case "float": signature.append("F"); break;
			case "double": signature.append("D"); break;
			default: signature.append("L").append(typeName.replace('.', '/')).append(";"); break;
		}
		return signature.toString();
	}


}

