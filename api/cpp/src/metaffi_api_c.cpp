#include <metaffi/api/metaffi_api_c.h>
#include <metaffi/api/metaffi_api.h>
#include <runtime/xllr_capi_loader.h>
#include <cstring>
#include <vector>

using namespace metaffi::api;

// ---------------------------------------------------------------------------
// Internal helper
// ---------------------------------------------------------------------------

namespace
{

char* capture_err(const std::exception& e)
{
	const char* msg = e.what();
	return xllr_alloc_string(msg, static_cast<uint64_t>(strlen(msg)));
}

} // namespace

// ---------------------------------------------------------------------------
// Runtime
// ---------------------------------------------------------------------------

metaffi_runtime_h metaffi_runtime_create(const char* runtime_plugin, char** out_err)
{
	try
	{
		return new MetaFFIRuntime(runtime_plugin ? runtime_plugin : "");
	}
	catch(const std::exception& e)
	{
		if(out_err) *out_err = capture_err(e);
		return nullptr;
	}
}

void metaffi_runtime_load_plugin(metaffi_runtime_h h, char** out_err)
{
	if(!h) { if(out_err) *out_err = xllr_alloc_string("null runtime handle", 19); return; }
	try
	{
		reinterpret_cast<MetaFFIRuntime*>(h)->load_runtime_plugin();
	}
	catch(const std::exception& e)
	{
		if(out_err) *out_err = capture_err(e);
	}
}

void metaffi_runtime_release_plugin(metaffi_runtime_h h, char** out_err)
{
	if(!h) { if(out_err) *out_err = xllr_alloc_string("null runtime handle", 19); return; }
	try
	{
		reinterpret_cast<MetaFFIRuntime*>(h)->release_runtime_plugin();
	}
	catch(const std::exception& e)
	{
		if(out_err) *out_err = capture_err(e);
	}
}

void metaffi_runtime_free(metaffi_runtime_h h)
{
	delete reinterpret_cast<MetaFFIRuntime*>(h);
}

// ---------------------------------------------------------------------------
// Module
// ---------------------------------------------------------------------------

metaffi_module_h metaffi_module_load(metaffi_runtime_h runtime,
                                     const char* module_path,
                                     char** out_err)
{
	if(!runtime) { if(out_err) *out_err = xllr_alloc_string("null runtime handle", 19); return nullptr; }
	try
	{
		auto* r = reinterpret_cast<MetaFFIRuntime*>(runtime);
		MetaFFIModule m = r->load_module(module_path ? module_path : "");
		return new MetaFFIModule(std::move(m));
	}
	catch(const std::exception& e)
	{
		if(out_err) *out_err = capture_err(e);
		return nullptr;
	}
}

void metaffi_module_free(metaffi_module_h h)
{
	delete reinterpret_cast<MetaFFIModule*>(h);
}

// ---------------------------------------------------------------------------
// Entity
// ---------------------------------------------------------------------------

metaffi_entity_h metaffi_entity_load(metaffi_module_h module,
                                     const char* entity_path,
                                     const metaffi_type_info* params_types,  int8_t params_count,
                                     const metaffi_type_info* retvals_types, int8_t retvals_count,
                                     char** out_err)
{
	if(!module) { if(out_err) *out_err = xllr_alloc_string("null module handle", 18); return nullptr; }
	try
	{
		auto* m = reinterpret_cast<MetaFFIModule*>(module);

		// Build type-info vectors
		std::vector<MetaFFITypeInfo> params;
		params.reserve(params_count);
		for(int i = 0; i < params_count; ++i)
			params.emplace_back(params_types[i]);

		std::vector<MetaFFITypeInfo> retvals;
		retvals.reserve(retvals_count);
		for(int i = 0; i < retvals_count; ++i)
			retvals.emplace_back(retvals_types[i]);

		MetaFFIEntity e = m->load_entity_with_info(entity_path ? entity_path : "", params, retvals);
		return new MetaFFIEntity(std::move(e));
	}
	catch(const std::exception& e)
	{
		if(out_err) *out_err = capture_err(e);
		return nullptr;
	}
}

void metaffi_entity_call(metaffi_entity_h h, struct cdts params_ret[2], char** out_err)
{
	if(!h) { if(out_err) *out_err = xllr_alloc_string("null entity handle", 18); return; }
	try
	{
		auto* entity = reinterpret_cast<MetaFFIEntity*>(h);

		// Move the caller's params out of slot [0]
		cdts params = std::move(params_ret[0]);

		// Call entity (validates params count & types, dispatches to right xcall variant)
		cdts retvals = entity->call_raw(std::move(params));

		// Free any pre-existing content in slot [1] (e.g. from xllr_alloc_cdts_buffer)
		params_ret[1].free();

		// Move results into slot [1] for the caller to read
		params_ret[1] = std::move(retvals);
	}
	catch(const std::exception& e)
	{
		if(out_err) *out_err = capture_err(e);
	}
}

void metaffi_entity_free(metaffi_entity_h h, char** /*out_err*/)
{
	delete reinterpret_cast<MetaFFIEntity*>(h);
}
