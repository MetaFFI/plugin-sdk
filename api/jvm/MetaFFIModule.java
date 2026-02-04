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
		return Caller.createCaller(xcallAndContext, parametersTypes, retvalsTypes);
	}
}
