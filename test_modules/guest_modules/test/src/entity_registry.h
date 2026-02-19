#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>
#include "cdt.h"
#include "metaffi_primitives.h"

// Define metaffi_any_array_type if not defined (missing from SDK)
#ifndef metaffi_any_array_type
#define metaffi_any_array_type (metaffi_any_type | metaffi_array_type)
#endif

namespace test_plugin
{

//----------------------------------------------------------------------
// Runtime State - Tracks if load_runtime was called
//----------------------------------------------------------------------
class RuntimeState
{
public:
	static RuntimeState& instance();

	bool is_loaded() const { return m_loaded.load(); }
	void set_loaded(bool loaded) { m_loaded.store(loaded); }

private:
	RuntimeState() = default;
	std::atomic<bool> m_loaded{false};
};

//----------------------------------------------------------------------
// XCall Variant - Determines which xcall signature to use
//----------------------------------------------------------------------
enum class XCallVariant
{
	PARAMS_RET,        // void xcall_params_ret(void* context, cdts params_ret[2], char** out_err)
	PARAMS_NO_RET,     // void xcall_params_no_ret(void* context, cdts params_ret[2], char** out_err)
	NO_PARAMS_RET,     // void xcall_no_params_ret(void* context, cdts params_ret[2], char** out_err)
	NO_PARAMS_NO_RET   // void xcall_no_params_no_ret(void* context, char** out_err)
};

//----------------------------------------------------------------------
// Handler function type
// For PARAMS_RET: data[0] = params, data[1] = returns
// For PARAMS_NO_RET: data[0] = params
// For NO_PARAMS_RET: data[0] = returns
// For NO_PARAMS_NO_RET: data is nullptr
//----------------------------------------------------------------------
using HandlerFunc = std::function<void(cdts* data, char** out_err)>;

//----------------------------------------------------------------------
// Entity Definition - Defines a test entity
//----------------------------------------------------------------------
struct EntityDefinition
{
	std::string name;
	std::vector<metaffi_type_info> params_types;
	std::vector<metaffi_type_info> retval_types;
	XCallVariant variant;
	HandlerFunc handler;

	EntityDefinition() = default;
	EntityDefinition(std::string n, std::vector<metaffi_type_info> p, std::vector<metaffi_type_info> r,
	                 XCallVariant v, HandlerFunc h)
		: name(std::move(n)), params_types(std::move(p)), retval_types(std::move(r)),
		  variant(v), handler(std::move(h)) {}
};

//----------------------------------------------------------------------
// Entity Context - Stored in xcall->pxcall_and_context[1]
//----------------------------------------------------------------------
struct EntityContext
{
	std::string entity_name;
	XCallVariant variant;
	HandlerFunc handler;

	EntityContext() = default;
	EntityContext(std::string name, XCallVariant v, HandlerFunc h)
		: entity_name(std::move(name)), variant(v), handler(std::move(h)) {}
};

//----------------------------------------------------------------------
// Entity Registry - Singleton that holds all entity definitions
//----------------------------------------------------------------------
class EntityRegistry
{
public:
	static EntityRegistry& instance();

	// Register all predefined entities
	void register_all_entities();

	// Find an entity by path
	const EntityDefinition* find_entity(const std::string& entity_path) const;

	// Validate that given types match expected types
	bool validate_types(const EntityDefinition& entity,
	                    const metaffi_type_info* params_types, int8_t params_count,
	                    const metaffi_type_info* retval_types, int8_t retval_count,
	                    std::string& error_message) const;

private:
	EntityRegistry() = default;

	void register_entity(EntityDefinition def);

	std::unordered_map<std::string, EntityDefinition> m_entities;
	bool m_initialized = false;
};

} // namespace test_plugin
