#include "entity_registry.h"
#include "entity_handlers.h"
#include "logging.h"

namespace test_plugin
{

//----------------------------------------------------------------------
// RuntimeState singleton
//----------------------------------------------------------------------
RuntimeState& RuntimeState::instance()
{
	static RuntimeState s_instance;
	return s_instance;
}

//----------------------------------------------------------------------
// EntityRegistry singleton
//----------------------------------------------------------------------
EntityRegistry& EntityRegistry::instance()
{
	static EntityRegistry s_instance;
	return s_instance;
}

void EntityRegistry::register_entity(EntityDefinition def)
{
	m_entities[def.name] = std::move(def);
}

const EntityDefinition* EntityRegistry::find_entity(const std::string& entity_path) const
{
	auto it = m_entities.find(entity_path);
	if(it != m_entities.end())
	{
		return &it->second;
	}
	return nullptr;
}

bool EntityRegistry::validate_types(const EntityDefinition& entity,
                                    const metaffi_type_info* params_types, int8_t params_count,
                                    const metaffi_type_info* retval_types, int8_t retval_count,
                                    std::string& error_message) const
{
	// Check parameter count
	if(static_cast<size_t>(params_count) != entity.params_types.size())
	{
		error_message = "Parameter count mismatch: expected " +
		                std::to_string(entity.params_types.size()) +
		                ", got " + std::to_string(params_count);
		return false;
	}

	// Check return value count
	if(static_cast<size_t>(retval_count) != entity.retval_types.size())
	{
		error_message = "Return value count mismatch: expected " +
		                std::to_string(entity.retval_types.size()) +
		                ", got " + std::to_string(retval_count);
		return false;
	}

	// Check parameter types
	for(int8_t i = 0; i < params_count; ++i)
	{
		if(params_types[i].type != entity.params_types[i].type)
		{
			const char* expected_str;
			const char* got_str;
			metaffi_type_to_str(entity.params_types[i].type, expected_str);
			metaffi_type_to_str(params_types[i].type, got_str);

			error_message = "Parameter " + std::to_string(i) + " type mismatch: expected " +
			                expected_str + ", got " + got_str;
			return false;
		}
	}

	// Check return types
	for(int8_t i = 0; i < retval_count; ++i)
	{
		if(retval_types[i].type != entity.retval_types[i].type)
		{
			const char* expected_str;
			const char* got_str;
			metaffi_type_to_str(entity.retval_types[i].type, expected_str);
			metaffi_type_to_str(retval_types[i].type, got_str);

			error_message = "Return value " + std::to_string(i) + " type mismatch: expected " +
			                expected_str + ", got " + got_str;
			return false;
		}
	}

	return true;
}

void EntityRegistry::register_all_entities()
{
	if(m_initialized) return;
	m_initialized = true;

	//------------------------------------------------------------------
	// NO_PARAMS_NO_RET entities
	//------------------------------------------------------------------

	register_entity({
		"test::no_op",
		{}, {},
		XCallVariant::NO_PARAMS_NO_RET,
		handler_no_op
	});

	register_entity({
		"test::print_hello",
		{}, {},
		XCallVariant::NO_PARAMS_NO_RET,
		handler_print_hello
	});

	//------------------------------------------------------------------
	// NO_PARAMS_RET entities - Return primitives
	//------------------------------------------------------------------

	register_entity({
		"test::return_int8",
		{},
		{metaffi_type_info(metaffi_int8_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_int8
	});

	register_entity({
		"test::return_int16",
		{},
		{metaffi_type_info(metaffi_int16_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_int16
	});

	register_entity({
		"test::return_int32",
		{},
		{metaffi_type_info(metaffi_int32_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_int32
	});

	register_entity({
		"test::return_int64",
		{},
		{metaffi_type_info(metaffi_int64_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_int64
	});

	register_entity({
		"test::return_uint8",
		{},
		{metaffi_type_info(metaffi_uint8_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_uint8
	});

	register_entity({
		"test::return_uint16",
		{},
		{metaffi_type_info(metaffi_uint16_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_uint16
	});

	register_entity({
		"test::return_uint32",
		{},
		{metaffi_type_info(metaffi_uint32_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_uint32
	});

	register_entity({
		"test::return_uint64",
		{},
		{metaffi_type_info(metaffi_uint64_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_uint64
	});

	register_entity({
		"test::return_float32",
		{},
		{metaffi_type_info(metaffi_float32_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_float32
	});

	register_entity({
		"test::return_float64",
		{},
		{metaffi_type_info(metaffi_float64_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_float64
	});

	register_entity({
		"test::return_bool_true",
		{},
		{metaffi_type_info(metaffi_bool_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_bool_true
	});

	register_entity({
		"test::return_bool_false",
		{},
		{metaffi_type_info(metaffi_bool_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_bool_false
	});

	register_entity({
		"test::return_string8",
		{},
		{metaffi_type_info(metaffi_string8_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_string8
	});

	register_entity({
		"test::return_null",
		{},
		{metaffi_type_info(metaffi_null_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_null
	});

	//------------------------------------------------------------------
	// PARAMS_NO_RET entities - Accept primitives
	//------------------------------------------------------------------

	register_entity({
		"test::accept_int8",
		{metaffi_type_info(metaffi_int8_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_accept_int8
	});

	register_entity({
		"test::accept_int16",
		{metaffi_type_info(metaffi_int16_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_accept_int16
	});

	register_entity({
		"test::accept_int32",
		{metaffi_type_info(metaffi_int32_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_accept_int32
	});

	register_entity({
		"test::accept_int64",
		{metaffi_type_info(metaffi_int64_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_accept_int64
	});

	register_entity({
		"test::accept_float32",
		{metaffi_type_info(metaffi_float32_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_accept_float32
	});

	register_entity({
		"test::accept_float64",
		{metaffi_type_info(metaffi_float64_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_accept_float64
	});

	register_entity({
		"test::accept_bool",
		{metaffi_type_info(metaffi_bool_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_accept_bool
	});

	register_entity({
		"test::accept_string8",
		{metaffi_type_info(metaffi_string8_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_accept_string8
	});

	//------------------------------------------------------------------
	// PARAMS_RET entities - Echo functions
	//------------------------------------------------------------------

	register_entity({
		"test::echo_int64",
		{metaffi_type_info(metaffi_int64_type)},
		{metaffi_type_info(metaffi_int64_type)},
		XCallVariant::PARAMS_RET,
		handler_echo_int64
	});

	register_entity({
		"test::echo_float64",
		{metaffi_type_info(metaffi_float64_type)},
		{metaffi_type_info(metaffi_float64_type)},
		XCallVariant::PARAMS_RET,
		handler_echo_float64
	});

	register_entity({
		"test::echo_string8",
		{metaffi_type_info(metaffi_string8_type)},
		{metaffi_type_info(metaffi_string8_type)},
		XCallVariant::PARAMS_RET,
		handler_echo_string8
	});

	register_entity({
		"test::echo_bool",
		{metaffi_type_info(metaffi_bool_type)},
		{metaffi_type_info(metaffi_bool_type)},
		XCallVariant::PARAMS_RET,
		handler_echo_bool
	});

	//------------------------------------------------------------------
	// PARAMS_RET entities - Arithmetic
	//------------------------------------------------------------------

	register_entity({
		"test::add_int64",
		{metaffi_type_info(metaffi_int64_type), metaffi_type_info(metaffi_int64_type)},
		{metaffi_type_info(metaffi_int64_type)},
		XCallVariant::PARAMS_RET,
		handler_add_int64
	});

	register_entity({
		"test::add_float64",
		{metaffi_type_info(metaffi_float64_type), metaffi_type_info(metaffi_float64_type)},
		{metaffi_type_info(metaffi_float64_type)},
		XCallVariant::PARAMS_RET,
		handler_add_float64
	});

	register_entity({
		"test::concat_strings",
		{metaffi_type_info(metaffi_string8_type), metaffi_type_info(metaffi_string8_type)},
		{metaffi_type_info(metaffi_string8_type)},
		XCallVariant::PARAMS_RET,
		handler_concat_strings
	});

	//------------------------------------------------------------------
	// Array entities - NO_PARAMS_RET
	//------------------------------------------------------------------

	register_entity({
		"test::return_int64_array_1d",
		{},
		{metaffi_type_info(metaffi_int64_array_type, nullptr, false, 1)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_int64_array_1d
	});

	register_entity({
		"test::return_int64_array_2d",
		{},
		{metaffi_type_info(metaffi_int64_array_type, nullptr, false, 2)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_int64_array_2d
	});

	register_entity({
		"test::return_int64_array_3d",
		{},
		{metaffi_type_info(metaffi_int64_array_type, nullptr, false, 3)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_int64_array_3d
	});

	register_entity({
		"test::return_ragged_array",
		{},
		{metaffi_type_info(metaffi_int64_array_type, nullptr, false, 2)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_ragged_array
	});

	register_entity({
		"test::return_string_array",
		{},
		{metaffi_type_info(metaffi_string8_array_type, nullptr, false, 1)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_string_array
	});

	//------------------------------------------------------------------
	// Array entities - PARAMS_RET
	//------------------------------------------------------------------

	register_entity({
		"test::sum_int64_array",
		{metaffi_type_info(metaffi_int64_array_type, nullptr, false, 1)},
		{metaffi_type_info(metaffi_int64_type)},
		XCallVariant::PARAMS_RET,
		handler_sum_int64_array
	});

	register_entity({
		"test::echo_int64_array",
		{metaffi_type_info(metaffi_int64_array_type, nullptr, false, 1)},
		{metaffi_type_info(metaffi_int64_array_type, nullptr, false, 1)},
		XCallVariant::PARAMS_RET,
		handler_echo_int64_array
	});

	register_entity({
		"test::join_strings",
		{metaffi_type_info(metaffi_string8_array_type, nullptr, false, 1)},
		{metaffi_type_info(metaffi_string8_type)},
		XCallVariant::PARAMS_RET,
		handler_join_strings
	});

	//------------------------------------------------------------------
	// Handle entities
	//------------------------------------------------------------------

	register_entity({
		"test::create_handle",
		{},
		{metaffi_type_info(metaffi_handle_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_create_handle
	});

	register_entity({
		"test::get_handle_data",
		{metaffi_type_info(metaffi_handle_type)},
		{metaffi_type_info(metaffi_string8_type)},
		XCallVariant::PARAMS_RET,
		handler_get_handle_data
	});

	register_entity({
		"test::set_handle_data",
		{metaffi_type_info(metaffi_handle_type), metaffi_type_info(metaffi_string8_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_set_handle_data
	});

	register_entity({
		"test::release_handle",
		{metaffi_type_info(metaffi_handle_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_release_handle
	});

	//------------------------------------------------------------------
	// Callable entities
	//------------------------------------------------------------------

	register_entity({
		"test::call_callback_add",
		{metaffi_type_info(metaffi_callable_type)},
		{metaffi_type_info(metaffi_int64_type)},
		XCallVariant::PARAMS_RET,
		handler_call_callback_add
	});

	register_entity({
		"test::call_callback_string",
		{metaffi_type_info(metaffi_callable_type)},
		{metaffi_type_info(metaffi_string8_type)},
		XCallVariant::PARAMS_RET,
		handler_call_callback_string
	});

	register_entity({
		"test::return_adder_callback",
		{},
		{metaffi_type_info(metaffi_callable_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_adder_callback
	});

	//------------------------------------------------------------------
	// Error handling entities
	//------------------------------------------------------------------

	register_entity({
		"test::throw_error",
		{},
		{},
		XCallVariant::NO_PARAMS_NO_RET,
		handler_throw_error
	});

	register_entity({
		"test::throw_with_message",
		{metaffi_type_info(metaffi_string8_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_throw_with_message
	});

	register_entity({
		"test::error_if_negative",
		{metaffi_type_info(metaffi_int64_type)},
		{},
		XCallVariant::PARAMS_NO_RET,
		handler_error_if_negative
	});

	//------------------------------------------------------------------
	// Any type entity
	//------------------------------------------------------------------

	register_entity({
		"test::accept_any",
		{metaffi_type_info(metaffi_any_type)},
		{metaffi_type_info(metaffi_any_type)},
		XCallVariant::PARAMS_RET,
		handler_accept_any
	});

	//------------------------------------------------------------------
	// Multiple return values entities
	//------------------------------------------------------------------

	register_entity({
		"test::return_two_values",
		{},
		{metaffi_type_info(metaffi_int64_type), metaffi_type_info(metaffi_string8_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_two_values
	});

	register_entity({
		"test::return_three_values",
		{},
		{metaffi_type_info(metaffi_int64_type), metaffi_type_info(metaffi_float64_type), metaffi_type_info(metaffi_bool_type)},
		XCallVariant::NO_PARAMS_RET,
		handler_return_three_values
	});

	register_entity({
		"test::swap_values",
		{metaffi_type_info(metaffi_int64_type), metaffi_type_info(metaffi_string8_type)},
		{metaffi_type_info(metaffi_string8_type), metaffi_type_info(metaffi_int64_type)},
		XCallVariant::PARAMS_RET,
		handler_swap_values
	});

	std::cout << LOG_PREFIX << "Registered " << m_entities.size() << " test entities" << std::endl;
}

} // namespace test_plugin
