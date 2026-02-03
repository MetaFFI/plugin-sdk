#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <metaffi/idl/idl_entities.hpp>
#include <filesystem>
#include <utils/env_utils.h>

using namespace metaffi::idl;

// Helper: Get path to test IDL file
std::string get_test_idl_path()
{
	std::string src_root = get_env_var("METAFFI_SOURCE_ROOT");
	if (src_root.empty())
	{
		throw std::runtime_error("Could not find METAFFI_SOURCE_ROOT");
	}

	auto idl_path = src_root + "/sdk/test_modules/guest_modules/test/xllr.test.idl.json";

	if (!std::filesystem::exists(idl_path))
	{
		throw std::runtime_error("Could not find xllr.test.idl.json");
	}

	return idl_path;

}

TEST_CASE("MetaFFI Type Conversions")
{
	SUBCASE("to_string")
	{
		CHECK(to_string(MetaFFIType::INT64) == "int64");
		CHECK(to_string(MetaFFIType::FLOAT64) == "float64");
		CHECK(to_string(MetaFFIType::STRING8) == "string8");
		CHECK(to_string(MetaFFIType::HANDLE) == "handle");
		CHECK(to_string(MetaFFIType::INT64_ARRAY) == "int64_array");
		CHECK(to_string(MetaFFIType::CALLABLE) == "callable");
	}

	SUBCASE("from_string")
	{
		CHECK(from_string("int64") == MetaFFIType::INT64);
		CHECK(from_string("float64") == MetaFFIType::FLOAT64);
		CHECK(from_string("string8") == MetaFFIType::STRING8);
		CHECK(from_string("handle") == MetaFFIType::HANDLE);
		CHECK(from_string("int64_array") == MetaFFIType::INT64_ARRAY);
		CHECK(from_string("callable") == MetaFFIType::CALLABLE);
	}

	SUBCASE("Invalid type throws")
	{
		CHECK_THROWS_AS(from_string("invalid_type"), IDLException);
	}

	SUBCASE("Helper functions")
	{
		CHECK(is_array_type(MetaFFIType::INT64_ARRAY));
		CHECK_FALSE(is_array_type(MetaFFIType::INT64));
		CHECK(is_handle_type(MetaFFIType::HANDLE));
		CHECK(is_handle_type(MetaFFIType::HANDLE_ARRAY));
		CHECK(is_string_type(MetaFFIType::STRING8));
		CHECK(is_string_type(MetaFFIType::STRING16_ARRAY));
	}
}

TEST_CASE("ArgDefinition")
{
	SUBCASE("Construction and getters")
	{
		ArgDefinition arg("param1", "int64", "int64_t", "Test parameter", 0, false);
		CHECK(arg.name() == "param1");
		CHECK(arg.type() == "int64");
		CHECK(arg.type_alias() == "int64_t");
		CHECK(arg.comment() == "Test parameter");
		CHECK(arg.dimensions() == 0);
		CHECK_FALSE(arg.is_optional());
	}

	SUBCASE("Fluent API")
	{
		ArgDefinition arg;
		arg.set_name("test")
			.set_type("float64")
			.set_type_alias("double")
			.set_comment("Test")
			.set_dimensions(1)
			.set_optional(true)
			.set_tag("key", "value");

		CHECK(arg.name() == "test");
		CHECK(arg.type() == "float64");
		CHECK(arg.is_optional());
		CHECK(arg.get_tag("key") == "value");
	}

	SUBCASE("JSON serialization")
	{
		ArgDefinition arg("param1", "int64", "int64_t");
		auto json = arg.to_json();

		CHECK(json["name"] == "param1");
		CHECK(json["type"] == "int64");
		CHECK(json["type_alias"] == "int64_t");
	}

	SUBCASE("JSON deserialization")
	{
		nlohmann::json j = {
			{"name", "test"},
			{"type", "string8"},
			{"type_alias", "std::string"},
			{"comment", "Test comment"},
			{"tags", {{"key", "value"}}},
			{"dimensions", 0},
			{"is_optional", true}
		};

		auto arg = ArgDefinition::from_json(j);
		CHECK(arg.name() == "test");
		CHECK(arg.type() == "string8");
		CHECK(arg.type_alias() == "std::string");
		CHECK(arg.is_optional());
		CHECK(arg.get_tag("key") == "value");
	}
}

TEST_CASE("FunctionDefinition")
{
	SUBCASE("Basic construction")
	{
		FunctionDefinition func("test_func");
		func.add_parameter(ArgDefinition("a", "int64", "int64_t"))
			.add_parameter(ArgDefinition("b", "int64", "int64_t"))
			.add_return_value(ArgDefinition("result", "int64", "int64_t"))
			.set_entity_path("callable", "test_func");

		CHECK(func.name() == "test_func");
		CHECK(func.parameters().size() == 2);
		CHECK(func.return_values().size() == 1);
		CHECK(func.get_entity_path("callable") == "test_func");
	}

	SUBCASE("JSON round-trip")
	{
		FunctionDefinition func("add");
		func.add_parameter(ArgDefinition("a", "int64", "int64_t"))
			.add_return_value(ArgDefinition("result", "int64", "int64_t"));

		auto json = func.to_json();
		auto func2 = FunctionDefinition::from_json(json);

		CHECK(func == func2);
	}
}

TEST_CASE("Load xllr.test.idl.json")
{
	std::string idl_path = get_test_idl_path();
	INFO("Loading IDL from: ", idl_path);

	IDLDefinition idl;
	REQUIRE_NOTHROW(idl = IDLDefinition::load_from_file(idl_path));

	SUBCASE("IDL metadata")
	{
		CHECK(idl.idl_source() == "xllr.test");
		CHECK(idl.target_language() == "cpp");
		CHECK(idl.metaffi_guest_lib() == "xllr_test_plugin");
	}

	SUBCASE("Module structure")
	{
		REQUIRE(idl.modules().size() == 1);
		const auto& module = idl.modules()[0];

		CHECK(module.name() == "test");
		CHECK(module.comment() == "Test module for MetaFFI SDK API testing");
	}

	SUBCASE("Function count")
	{
		const auto& module = idl.modules()[0];
		INFO("Number of functions: ", module.functions().size());
		CHECK(module.functions().size() == 49);
	}

	SUBCASE("Verify specific functions")
	{
		const auto& module = idl.modules()[0];
		const auto& functions = module.functions();

		// Find no_op function
		auto it = std::find_if(functions.begin(), functions.end(),
								[](const FunctionDefinition& f) { return f.name() == "no_op"; });
		REQUIRE(it != functions.end());
		CHECK(it->parameters().empty());
		CHECK(it->return_values().empty());

		// Find add_int64 function
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "add_int64"; });
		REQUIRE(it != functions.end());
		CHECK(it->parameters().size() == 2);
		CHECK(it->parameters()[0].type() == "int64");
		CHECK(it->parameters()[1].type() == "int64");
		CHECK(it->return_values().size() == 1);
		CHECK(it->return_values()[0].type() == "int64");

		// Find echo_string8 function
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "echo_string8"; });
		REQUIRE(it != functions.end());
		CHECK(it->parameters().size() == 1);
		CHECK(it->parameters()[0].type() == "string8");
		CHECK(it->return_values()[0].type() == "string8");

		// Find return_int64_array_1d function (array test)
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "return_int64_array_1d"; });
		REQUIRE(it != functions.end());
		CHECK(it->return_values().size() == 1);
		CHECK(it->return_values()[0].type() == "int64_array");
		CHECK(it->return_values()[0].dimensions() == 1);

		// Find call_callback_add (callable test)
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "call_callback_add"; });
		REQUIRE(it != functions.end());
		CHECK(it->parameters().size() == 1);
		CHECK(it->parameters()[0].type() == "callable");

		// Find accept_any (any type test)
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "accept_any"; });
		REQUIRE(it != functions.end());
		CHECK(it->parameters().size() == 1);
		CHECK(it->parameters()[0].type() == "any");
		CHECK(it->return_values()[0].type() == "any");

		// Find return_two_values (multiple return values)
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "return_two_values"; });
		REQUIRE(it != functions.end());
		CHECK(it->return_values().size() == 2);
		CHECK(it->return_values()[0].type() == "int64");
		CHECK(it->return_values()[1].type() == "string8");
	}

	SUBCASE("Round-trip serialization")
	{
		// Save to temp file
		std::string temp_path = "temp_test.idl.json";
		idl.save_to_file(temp_path);

		// Load it back
		IDLDefinition idl2;
		REQUIRE_NOTHROW(idl2 = IDLDefinition::load_from_file(temp_path));

		// Compare
		CHECK(idl2.idl_source() == idl.idl_source());
		CHECK(idl2.modules().size() == idl.modules().size());
		CHECK(idl2.modules()[0].functions().size() == idl.modules()[0].functions().size());

		// Cleanup
		std::filesystem::remove(temp_path);
	}
}

TEST_CASE("TypeMapper")
{
	SUBCASE("Primitive types")
	{
		auto [type, dims] = TypeMapper::map_type<int64_t>();
		CHECK(type == "int64");
		CHECK(dims == 0);

		auto [type2, dims2] = TypeMapper::map_type<double>();
		CHECK(type2 == "float64");
		CHECK(dims2 == 0);

		auto [type3, dims3] = TypeMapper::map_type<std::string>();
		CHECK(type3 == "string8");
		CHECK(dims3 == 0);
	}

	SUBCASE("Type aliases")
	{
		CHECK(TypeMapper::get_type_alias<int64_t>() == "int64_t");
		CHECK(TypeMapper::get_type_alias<double>() == "double");
		CHECK(TypeMapper::get_type_alias<std::string>() == "std::string");
	}
}

TEST_CASE("Error handling")
{
	SUBCASE("Missing file")
	{
		CHECK_THROWS_AS(IDLDefinition::load_from_file("nonexistent.json"), IDLException);
	}

	SUBCASE("Invalid JSON")
	{
		CHECK_THROWS_AS(IDLDefinition::load_from_json("invalid json {"), IDLException);
	}

	SUBCASE("Missing required field")
	{
		nlohmann::json j = {
			{"type", "int64"}
			// Missing "name" field
		};
		CHECK_THROWS_AS(ArgDefinition::from_json(j), IDLException);
	}
}
