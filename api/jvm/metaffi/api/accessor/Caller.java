package metaffi.api.accessor;

import java.util.function.Function;
import java.util.concurrent.atomic.LongAdder;

public class Caller
{
	private static final class CallProfiler
	{
		private static final boolean ENABLED = isEnabled();
		private static final LongAdder calls = new LongAdder();
		private static final LongAdder allocNs = new LongAdder();
		private static final LongAdder serializeNs = new LongAdder();
		private static final LongAdder invokeNs = new LongAdder();
		private static final LongAdder deserializeNs = new LongAdder();
		private static final LongAdder freeNs = new LongAdder();

		static
		{
			if (ENABLED)
			{
				Runtime.getRuntime().addShutdownHook(new Thread(CallProfiler::printSummary, "metaffi-caller-profiler"));
			}
		}

		private static boolean isEnabled()
		{
			String raw = System.getenv("METAFFI_PROFILE_CALLER");
			if (raw == null)
			{
				return false;
			}
			raw = raw.trim().toLowerCase();
			return "1".equals(raw) || "true".equals(raw) || "yes".equals(raw) || "on".equals(raw);
		}

		private static void printSummary()
		{
			long n = calls.sum();
			if (n == 0)
			{
				System.err.println("[metaffi][caller-profiler] no calls recorded");
				return;
			}

			double inv = 1.0 / n;
			System.err.printf(
				"[metaffi][caller-profiler] calls=%d mean_ns: alloc=%.1f serialize=%.1f invoke=%.1f deserialize=%.1f free=%.1f total=%.1f%n",
				n,
				allocNs.sum() * inv,
				serializeNs.sum() * inv,
				invokeNs.sum() * inv,
				deserializeNs.sum() * inv,
				freeNs.sum() * inv,
				(allocNs.sum() + serializeNs.sum() + invokeNs.sum() + deserializeNs.sum() + freeNs.sum()) * inv
			);
		}
	}

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

	public static Caller createCaller(long xcallAndContext, MetaFFITypeInfo[] params, MetaFFITypeInfo[] retvals)
	{
		long[] parametersTypesArray = (params != null && params.length > 0) ? convertToLongArray(params) : new long[]{};
		long[] retvalTypeArray = (retvals != null && retvals.length > 0) ? convertToLongArray(retvals) : new long[]{};
		return createCaller(xcallAndContext, parametersTypesArray, retvalTypeArray);
	}

	public static Caller createCaller(long xcallAndContext, MetaFFITypeInfo[] params, MetaFFITypeInfo retval)
	{
		MetaFFITypeInfo[] retvals = (retval != null) ? new MetaFFITypeInfo[]{retval} : new MetaFFITypeInfo[]{};
		return createCaller(xcallAndContext, params, retvals);
	}

	public static Caller createCaller(long xcallAndContext, long[] parametersTypesArray, long[] retvalsTypesArray)
	{
		byte paramsCount = (parametersTypesArray == null)? 0 : (byte)parametersTypesArray.length;
		byte retvalsCount = (retvalsTypesArray == null)? 0 : (byte)retvalsTypesArray.length;
		final boolean fastPathSingleInt64NoRet =
			paramsCount == 1 &&
			retvalsCount == 0 &&
			parametersTypesArray != null &&
			parametersTypesArray.length == 1 &&
			parametersTypesArray[0] == MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64.value;

		Function<Object, Object[]> f = (Object parametersArray) ->
		{
			final boolean profile = CallProfiler.ENABLED;
			long t0 = 0;
			long t1 = 0;
			Object[] actuals = (Object[])parametersArray;
			int actualsCount = actuals.length;

			if(paramsCount != actualsCount)
				throw new IllegalArgumentException(String.format("Expected %d parameters, received %d parameters", paramsCount, actualsCount));

			// allocate CDTS
			long xcall_params = 0;

			try
			{
				if(profile) t0 = System.nanoTime();
				if(paramsCount > 0 || retvalsCount > 0)
					xcall_params = MetaFFIAccessor.alloc_cdts(paramsCount, retvalsCount);
				if(profile)
				{
					t1 = System.nanoTime();
					CallProfiler.allocNs.add(t1 - t0);
				}

				// get parameters CDTS and returnValue CDTS
				long parametersCDTS = 0;
				long returnValuesCDTS = 0;

				if(paramsCount > 0)
					parametersCDTS = MetaFFIAccessor.get_pcdt(xcall_params, (byte)0);

				if(retvalsCount > 0)
					returnValuesCDTS = MetaFFIAccessor.get_pcdt(xcall_params, (byte)1);

				// fill CDTS
				if(profile) t0 = System.nanoTime();
				if(paramsCount > 0)
				{
					if(fastPathSingleInt64NoRet)
					{
						Object p0 = actuals[0];
						long value;
						if(p0 instanceof Long)
						{
							value = ((Long)p0).longValue();
						}
						else if(p0 instanceof Number)
						{
							value = ((Number)p0).longValue();
						}
						else
						{
							throw new IllegalArgumentException("Expected numeric parameter for int64 fast-path");
						}
						MetaFFIAccessor.set_cdt_int64(parametersCDTS, 0, value);
					}
					else
					{
						MetaFFIAccessor.java_to_cdts(parametersCDTS, actuals, parametersTypesArray);
					}
				}
				if(profile)
				{
					t1 = System.nanoTime();
					CallProfiler.serializeNs.add(t1 - t0);
				}

				if(profile) t0 = System.nanoTime();
				if(paramsCount > 0 && retvalsCount > 0)
					MetaFFIAccessor.xcall_params_ret(xcallAndContext, xcall_params);
				else if(paramsCount > 0)
					MetaFFIAccessor.xcall_params_no_ret(xcallAndContext, parametersCDTS);
				else if(retvalsCount > 0)
					MetaFFIAccessor.xcall_no_params_ret(xcallAndContext, returnValuesCDTS);
				else
					MetaFFIAccessor.xcall_no_params_no_ret(xcallAndContext);
				if(profile)
				{
					t1 = System.nanoTime();
					CallProfiler.invokeNs.add(t1 - t0);
				}

				// fill result
				if(retvalsCount == 0)
					return null;

				if(profile) t0 = System.nanoTime();
				Object[] result = MetaFFIAccessor.cdts_to_java(returnValuesCDTS, retvalsCount);
				if(profile)
				{
					t1 = System.nanoTime();
					CallProfiler.deserializeNs.add(t1 - t0);
				}
				return result;
			}
			finally
			{
				long tf0 = 0;
				if(profile) tf0 = System.nanoTime();
				if(xcall_params != 0)
					MetaFFIAccessor.free_cdts(xcall_params);
				if(profile)
				{
					CallProfiler.freeNs.add(System.nanoTime() - tf0);
					CallProfiler.calls.increment();
					long seen = CallProfiler.calls.sum();
					if(seen % 5000 == 0)
					{
						double inv = 1.0 / seen;
						System.err.printf(
							"[metaffi][caller-profiler] progress calls=%d mean_ns alloc=%.1f serialize=%.1f invoke=%.1f deserialize=%.1f free=%.1f total=%.1f%n",
							seen,
							CallProfiler.allocNs.sum() * inv,
							CallProfiler.serializeNs.sum() * inv,
							CallProfiler.invokeNs.sum() * inv,
							CallProfiler.deserializeNs.sum() * inv,
							CallProfiler.freeNs.sum() * inv,
							(CallProfiler.allocNs.sum() + CallProfiler.serializeNs.sum() + CallProfiler.invokeNs.sum() + CallProfiler.deserializeNs.sum() + CallProfiler.freeNs.sum()) * inv
						);
					}
				}
			}
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

