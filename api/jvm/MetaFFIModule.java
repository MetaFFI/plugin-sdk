package api;

import metaffi.api.accessor.Caller;
import metaffi.api.accessor.MetaFFIAccessor;
import metaffi.api.accessor.MetaFFITypeInfo;

public class MetaFFIModule
{
    private api.MetaFFIRuntime runtime;
    private String modulePath;

    public MetaFFIModule(api.MetaFFIRuntime runtime, String modulePath)
    {
        this.runtime = runtime;
        this.modulePath = modulePath;
    }



	public Caller load(String entityPath, MetaFFITypeInfo[] parametersTypes, MetaFFITypeInfo[] retvalsTypes)
	{

		var xcallAndContext = MetaFFIAccessor.load_function(
															"xllr."+this.runtime.runtimePlugin,
															this.modulePath,
															entityPath,
															parametersTypes,
															retvalsTypes);

		// Return a Caller object that wraps a lambda that calls the foreign object
		byte paramsCount = parametersTypes != null ? (byte)parametersTypes.length : 0;
		byte retvalsCount = retvalsTypes != null ? (byte)retvalsTypes.length : 0;


		// parametersArray - Object[] with parameters
		// return value is an Object of the expected type
		// or null if calling function is void.
		return Caller.createCaller(xcallAndContext, parametersTypes, (retvalsTypes != null && retvalsTypes.length > 0) ? retvalsTypes[0] : null);
	}
}
