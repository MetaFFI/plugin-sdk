#include "test_runtime_plugin.h"
#include "entity_registry.h"
#include "logging.h"
#include "xllr_capi_loader.h"
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <new>

using namespace test_plugin;

//----------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------

static void set_error(char** out_err, const std::string& msg)
{
	if(out_err)
	{
		*out_err = xllr_alloc_string(msg.c_str(), msg.size());
	}
}

static void log_type_info(const metaffi_type_info* types, int8_t count, const char* label)
{
	for(int8_t i = 0; i < count; ++i)
	{
		const char* type_str;
		metaffi_type_to_str(types[i].type, type_str);

		std::cout << LOG_PREFIX << "  " << label << "[" << static_cast<int>(i) << "]: type="
		          << type_str << " (0x" << std::hex << types[i].type << std::dec << ")";

		if(types[i].alias)
		{
			std::cout << " alias=\"" << types[i].alias << "\"";
		}

		std::cout << " fixed_dims=" << types[i].fixed_dimensions << std::endl;
	}
}

//----------------------------------------------------------------------
// XCall dispatcher functions
//----------------------------------------------------------------------

static void dispatch_params_ret(void* context, cdts params_ret[2], char** out_err)
{
	auto* ctx = static_cast<EntityContext*>(context);
	log_xcall("xcall_params_ret", ctx->entity_name);

	// Log parameters
	log_cdts("PARAMS", params_ret[0]);

	// Call the handler
	ctx->handler(params_ret, out_err);

	// Log return values (only if no error)
	if(!out_err || !*out_err)
	{
		log_cdts("RETURNS", params_ret[1]);
	}
}

static void dispatch_params_no_ret(void* context, cdts data[2], char** out_err)
{
	auto* ctx = static_cast<EntityContext*>(context);
	log_xcall("xcall_params_no_ret", ctx->entity_name);

	// Log parameters (data[0] = params, data[1] = unused)
	log_cdts("PARAMS", data[0]);

	// Call the handler
	ctx->handler(data, out_err);
}

static void dispatch_no_params_ret(void* context, cdts data[2], char** out_err)
{
	auto* ctx = static_cast<EntityContext*>(context);
	log_xcall("xcall_no_params_ret", ctx->entity_name);

	// Call the handler (data[0] = unused, data[1] = returns)
	ctx->handler(data, out_err);

	// Log return values (only if no error)
	if(!out_err || !*out_err)
	{
		log_cdts("RETURNS", data[1]);
	}
}

static void dispatch_no_params_no_ret(void* context, char** out_err)
{
	auto* ctx = static_cast<EntityContext*>(context);
	log_xcall("xcall_no_params_no_ret", ctx->entity_name);

	// Call the handler
	ctx->handler(nullptr, out_err);
}

//----------------------------------------------------------------------
// Runtime Plugin API Implementation
//----------------------------------------------------------------------

extern "C"
{

void load_runtime(char** err)
{
	log_api_call("load_runtime");

	// Initialize the entity registry
	EntityRegistry::instance().register_all_entities();

	// Mark runtime as loaded
	RuntimeState::instance().set_loaded(true);

	log_api_success("load_runtime");
}

void free_runtime(char** err)
{
	log_api_call("free_runtime");

	// Validate that load_runtime was called first
	if(!RuntimeState::instance().is_loaded())
	{
		std::string error_msg = "free_runtime called before load_runtime";
		log_error("free_runtime", error_msg);
		set_error(err, error_msg);
		return;
	}

	// Mark runtime as unloaded
	RuntimeState::instance().set_loaded(false);

	log_api_success("free_runtime");
}

xcall* load_entity(
	const char* module_path,
	const char* entity_path,
	metaffi_type_info* params_types,
	int8_t params_count,
	metaffi_type_info* retvals_types,
	int8_t retval_count,
	char** err)
{
	log_api_call("load_entity");

	// Log all input parameters
	std::cout << LOG_PREFIX << "  module_path: " << (module_path ? module_path : "null") << std::endl;
	std::cout << LOG_PREFIX << "  entity_path: " << (entity_path ? entity_path : "null") << std::endl;
	std::cout << LOG_PREFIX << "  params_count: " << static_cast<int>(params_count) << std::endl;
	std::cout << LOG_PREFIX << "  retval_count: " << static_cast<int>(retval_count) << std::endl;

	// Log parameter types
	if(params_count > 0 && params_types)
	{
		log_type_info(params_types, params_count, "param");
	}

	// Log return types
	if(retval_count > 0 && retvals_types)
	{
		log_type_info(retvals_types, retval_count, "retval");
	}

	// Lookup entity in registry
	const EntityDefinition* entity = EntityRegistry::instance().find_entity(entity_path ? entity_path : "");
	if(!entity)
	{
		std::string error_msg = "Entity not found: " + std::string(entity_path ? entity_path : "null");
		log_error("load_entity", error_msg);
		set_error(err, error_msg);
		return nullptr;
	}

	// Validate parameter/return types match expected
	std::string validation_error;
	if(!EntityRegistry::instance().validate_types(*entity,
	                                              params_types, params_count,
	                                              retvals_types, retval_count,
	                                              validation_error))
	{
		std::string error_msg = "Type validation failed for " + std::string(entity_path) + ": " + validation_error;
		log_error("load_entity", error_msg);
		set_error(err, error_msg);
		return nullptr;
	}

	// Create context for this xcall
	void* ctx_mem = xllr_alloc_memory(sizeof(EntityContext));
	if(!ctx_mem)
	{
		set_error(err, "Failed to allocate memory for EntityContext");
		return nullptr;
	}
	auto* ctx = new (ctx_mem) EntityContext(entity_path, entity->variant, entity->handler);

	// Create xcall structure
	void* pxcall_mem = xllr_alloc_memory(sizeof(xcall));
	if(!pxcall_mem)
	{
		ctx->~EntityContext();
		xllr_free_memory(ctx_mem);
		set_error(err, "Failed to allocate memory for xcall");
		return nullptr;
	}
	auto* pxcall = new (pxcall_mem) xcall();

	// Set function pointer based on variant
	switch(entity->variant)
	{
		case XCallVariant::PARAMS_RET:
			pxcall->pxcall_and_context[0] = reinterpret_cast<void*>(&dispatch_params_ret);
			break;
		case XCallVariant::PARAMS_NO_RET:
			pxcall->pxcall_and_context[0] = reinterpret_cast<void*>(&dispatch_params_no_ret);
			break;
		case XCallVariant::NO_PARAMS_RET:
			pxcall->pxcall_and_context[0] = reinterpret_cast<void*>(&dispatch_no_params_ret);
			break;
		case XCallVariant::NO_PARAMS_NO_RET:
			pxcall->pxcall_and_context[0] = reinterpret_cast<void*>(&dispatch_no_params_no_ret);
			break;
	}
	pxcall->pxcall_and_context[1] = ctx;

	std::cout << LOG_PREFIX << "load_entity successful: " << entity_path << std::endl;
	return pxcall;
}

xcall* make_callable(
	void* make_callable_context,
	metaffi_type_info* params_types,
	int8_t params_count,
	metaffi_type_info* retvals_types,
	int8_t retval_count,
	char** err)
{
	log_api_call("make_callable");

	std::cout << LOG_PREFIX << "  context: " << make_callable_context << std::endl;
	std::cout << LOG_PREFIX << "  params_count: " << static_cast<int>(params_count) << std::endl;
	std::cout << LOG_PREFIX << "  retval_count: " << static_cast<int>(retval_count) << std::endl;

	// Log parameter types
	if(params_count > 0 && params_types)
	{
		log_type_info(params_types, params_count, "param");
	}

	// Log return types
	if(retval_count > 0 && retvals_types)
	{
		log_type_info(retvals_types, retval_count, "retval");
	}

	// Determine variant from params/retval counts
	XCallVariant variant;
	if(params_count > 0 && retval_count > 0)
	{
		variant = XCallVariant::PARAMS_RET;
	}
	else if(params_count > 0)
	{
		variant = XCallVariant::PARAMS_NO_RET;
	}
	else if(retval_count > 0)
	{
		variant = XCallVariant::NO_PARAMS_RET;
	}
	else
	{
		variant = XCallVariant::NO_PARAMS_NO_RET;
	}

	// Create a passthrough handler that forwards to the provided context
	auto passthrough_handler = [make_callable_context](cdts* data, char** out_err)
	{
		// The context is expected to be an xcall* that we forward to
		auto* inner_xcall = static_cast<xcall*>(make_callable_context);
		if(inner_xcall && inner_xcall->is_valid())
		{
			(*inner_xcall)(data, out_err);
		}
		else
		{
			log_error("make_callable", "invalid inner xcall");
			if(out_err)
			{
				*out_err = strdup("make_callable: invalid inner xcall");
			}
		}
	};

	// Create context for this xcall
	void* ctx_mem = xllr_alloc_memory(sizeof(EntityContext));
	if(!ctx_mem)
	{
		set_error(err, "Failed to allocate memory for EntityContext");
		return nullptr;
	}
	auto* ctx = new (ctx_mem) EntityContext("__callable__", variant, passthrough_handler);

	// Create xcall structure
	void* pxcall_mem = xllr_alloc_memory(sizeof(xcall));
	if(!pxcall_mem)
	{
		ctx->~EntityContext();
		xllr_free_memory(ctx_mem);
		set_error(err, "Failed to allocate memory for xcall");
		return nullptr;
	}
	auto* pxcall = new (pxcall_mem) xcall();

	// Set function pointer based on variant
	switch(variant)
	{
		case XCallVariant::PARAMS_RET:
			pxcall->pxcall_and_context[0] = reinterpret_cast<void*>(&dispatch_params_ret);
			break;
		case XCallVariant::PARAMS_NO_RET:
			pxcall->pxcall_and_context[0] = reinterpret_cast<void*>(&dispatch_params_no_ret);
			break;
		case XCallVariant::NO_PARAMS_RET:
			pxcall->pxcall_and_context[0] = reinterpret_cast<void*>(&dispatch_no_params_ret);
			break;
		case XCallVariant::NO_PARAMS_NO_RET:
			pxcall->pxcall_and_context[0] = reinterpret_cast<void*>(&dispatch_no_params_no_ret);
			break;
	}
	pxcall->pxcall_and_context[1] = ctx;

	log_api_success("make_callable");
	return pxcall;
}

void free_xcall(xcall* pff, char** err)
{
	log_api_call("free_xcall");

	if(!pff)
	{
		log_error("free_xcall", "null xcall pointer");
		set_error(err, "free_xcall: null xcall pointer");
		return;
	}

	// Free the context
	auto* ctx = static_cast<EntityContext*>(pff->pxcall_and_context[1]);
	if(ctx)
	{
		std::cout << LOG_PREFIX << "  freeing xcall for: " << ctx->entity_name << std::endl;
		ctx->~EntityContext();
		xllr_free_memory(ctx);
	}

	// Free the xcall structure
	pff->~xcall();
	xllr_free_memory(pff);

	log_api_success("free_xcall");
}

//----------------------------------------------------------------------
// XCall variant handlers (exported, called by SDK)
//----------------------------------------------------------------------

void xcall_params_ret(void* context, cdts params_ret[2], char** out_err)
{
	dispatch_params_ret(context, params_ret, out_err);
}

void xcall_params_no_ret(void* context, cdts data[2], char** out_err)
{
	dispatch_params_no_ret(context, data, out_err);
}

void xcall_no_params_ret(void* context, cdts data[2], char** out_err)
{
	dispatch_no_params_ret(context, data, out_err);
}

void xcall_no_params_no_ret(void* context, char** out_err)
{
	dispatch_no_params_no_ret(context, out_err);
}

} // extern "C"
