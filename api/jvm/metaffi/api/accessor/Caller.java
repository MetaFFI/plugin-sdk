package metaffi.api.accessor;

import java.util.function.Function;

public class Caller
{
	private final Function<Object, Object[]> f;
	public long xcallAndContext = 0;
	public long[] parametersTypesArray = null;
	public long[] retvalsTypesArray = null;


	private Caller(Function<Object,Object[]> f)
	{
		this.f = f;
	}

	public Object[] call(Object... parameters)
	{
		return this.f.apply(parameters);
	}

	public static Caller createCaller(long xcallAndContext, MetaFFITypeInfo[] params, MetaFFITypeInfo retval)
	{
    	long[] parametersTypesArray = (params != null && params.length > 0) ? convertToLongArray(params) : new long[]{};
    	long[] retvalTypeArray = (retval != null) ? convertToLongArray(new MetaFFITypeInfo[]{retval}) : new long[]{};
		return createCaller(xcallAndContext, parametersTypesArray, retvalTypeArray);
	}

	public static Caller createCaller(long xcallAndContext, long[] parametersTypesArray, long[] retvalsTypesArray)
	{
		byte paramsCount = (parametersTypesArray == null)? 0 : (byte)parametersTypesArray.length;
		byte retvalsCount = (retvalsTypesArray == null)? 0 : (byte)retvalsTypesArray.length;

		Function<Object, Object[]> f = (Object parametersArray) ->
		{
			int actualsCount = ((Object[])parametersArray).length;

			if(paramsCount != actualsCount)
				throw new IllegalArgumentException(String.format("Expected %d parameters, received %d parameters", paramsCount, actualsCount));

			// allocate CDTS
			long xcall_params = 0;

			if(paramsCount > 0 || retvalsCount > 0)
				xcall_params = MetaFFIAccessor.alloc_cdts(paramsCount, retvalsCount);

			// get parameters CDTS and returnValue CDTS
			long parametersCDTS = 0;
			long returnValuesCDTS = 0;

			if(paramsCount > 0)
				parametersCDTS = MetaFFIAccessor.get_pcdt(xcall_params, (byte)0);

			if(retvalsCount > 0)
				returnValuesCDTS = MetaFFIAccessor.get_pcdt(xcall_params, (byte)1);

			// fill CDTS
			if(paramsCount > 0)
				MetaFFIAccessor.java_to_cdts(parametersCDTS, (Object[])parametersArray, parametersTypesArray);

			if(paramsCount > 0 && retvalsCount > 0)
				MetaFFIAccessor.xcall_params_ret(xcallAndContext, xcall_params);
			if(paramsCount > 0 && retvalsCount == 0)
				MetaFFIAccessor.xcall_params_no_ret(xcallAndContext, xcall_params);
			if(paramsCount == 0 && retvalsCount > 0)
				MetaFFIAccessor.xcall_no_params_ret(xcallAndContext, xcall_params);
			if(paramsCount == 0 && retvalsCount == 0)
				MetaFFIAccessor.xcall_no_params_no_ret(xcallAndContext);

			// fill result
			if(retvalsCount == 0)
				return null;

			Object[] res = MetaFFIAccessor.cdts_to_java(returnValuesCDTS, retvalsCount);
			return res;
		};

		var caller = new Caller(f);
		caller.xcallAndContext = xcallAndContext;
		caller.parametersTypesArray = parametersTypesArray;
		caller.retvalsTypesArray = retvalsTypesArray;
		return caller;
	}

	public static long[] convertToLongArray(MetaFFITypeInfo[] metaFFITypesArray)
	{
		long[] longArray = new long[metaFFITypesArray.length];
		for (int i = 0; i < metaFFITypesArray.length; i++) {
			longArray[i] = metaFFITypesArray[i].type.value;
		}
		return longArray;
	}
}

