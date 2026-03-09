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
		CHECK_EQ(to_string(MetaFFIType::INT64), "int64");
		CHECK_EQ(to_string(MetaFFIType::FLOAT64), "float64");
		CHECK_EQ(to_string(MetaFFIType::STRING8), "string8");
		CHECK_EQ(to_string(MetaFFIType::HANDLE), "handle");
		CHECK_EQ(to_string(MetaFFIType::INT64_ARRAY), "int64_array");
		CHECK_EQ(to_string(MetaFFIType::CALLABLE), "callable");
	}

	SUBCASE("from_string")
	{
		CHECK_EQ(from_string("int64"), MetaFFIType::INT64);
		CHECK_EQ(from_string("float64"), MetaFFIType::FLOAT64);
		CHECK_EQ(from_string("string8"), MetaFFIType::STRING8);
		CHECK_EQ(from_string("handle"), MetaFFIType::HANDLE);
		CHECK_EQ(from_string("int64_array"), MetaFFIType::INT64_ARRAY);
		CHECK_EQ(from_string("callable"), MetaFFIType::CALLABLE);
	}

	SUBCASE("Invalid type throws")
	{
		CHECK_THROWS_AS((void)from_string("invalid_type"), IDLException);
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
		CHECK_EQ(arg.name(), "param1");
		CHECK_EQ(arg.type(), "int64");
		CHECK_EQ(arg.type_alias(), "int64_t");
		CHECK_EQ(arg.comment(), "Test parameter");
		CHECK_EQ(arg.dimensions(), 0);
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

		CHECK_EQ(arg.name(), "test");
		CHECK_EQ(arg.type(), "float64");
		CHECK(arg.is_optional());
		CHECK_EQ(arg.get_tag("key"), "value");
	}

	SUBCASE("JSON serialization")
	{
		ArgDefinition arg("param1", "int64", "int64_t");
		auto json = arg.to_json();

		CHECK_EQ(json["name"], "param1");
		CHECK_EQ(json["type"], "int64");
		CHECK_EQ(json["type_alias"], "int64_t");
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
		CHECK_EQ(arg.name(), "test");
		CHECK_EQ(arg.type(), "string8");
		CHECK_EQ(arg.type_alias(), "std::string");
		CHECK(arg.is_optional());
		CHECK_EQ(arg.get_tag("key"), "value");
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

		CHECK_EQ(func.name(), "test_func");
		CHECK_EQ(func.parameters().size(), 2);
		CHECK_EQ(func.return_values().size(), 1);
		CHECK_EQ(func.get_entity_path("callable"), "test_func");
	}

	SUBCASE("JSON round-trip")
	{
		FunctionDefinition func("add");
		func.add_parameter(ArgDefinition("a", "int64", "int64_t"))
			.add_return_value(ArgDefinition("result", "int64", "int64_t"));

		auto json = func.to_json();
		auto func2 = FunctionDefinition::from_json(json);

		CHECK_EQ(func, func2);
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
		CHECK_EQ(idl.idl_source(), "xllr.test");
		CHECK_EQ(idl.target_language(), "test");
		CHECK_EQ(idl.metaffi_guest_lib(), "xllr_test_plugin");
	}

	SUBCASE("Module structure")
	{
		REQUIRE_EQ(idl.modules().size(), 1);
		const auto& module = idl.modules()[0];

		CHECK_EQ(module.name(), "test");
		CHECK_EQ(module.comment(), "Test module for MetaFFI SDK API testing");
	}

	SUBCASE("Function count")
	{
		const auto& module = idl.modules()[0];
		INFO("Number of functions: ", module.functions().size());
		CHECK_EQ(module.functions().size(), 49);
	}

	SUBCASE("Verify specific functions")
	{
		const auto& module = idl.modules()[0];
		const auto& functions = module.functions();

		// Find no_op function
		auto it = std::find_if(functions.begin(), functions.end(),
								[](const FunctionDefinition& f) { return f.name() == "no_op"; });
		REQUIRE_NE(it, functions.end());
		CHECK(it->parameters().empty());
		CHECK(it->return_values().empty());

		// Find add_int64 function
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "add_int64"; });
		REQUIRE_NE(it, functions.end());
		CHECK_EQ(it->parameters().size(), 2);
		CHECK_EQ(it->parameters()[0].type(), "int64");
		CHECK_EQ(it->parameters()[1].type(), "int64");
		CHECK_EQ(it->return_values().size(), 1);
		CHECK_EQ(it->return_values()[0].type(), "int64");

		// Find echo_string8 function
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "echo_string8"; });
		REQUIRE_NE(it, functions.end());
		CHECK_EQ(it->parameters().size(), 1);
		CHECK_EQ(it->parameters()[0].type(), "string8");
		CHECK_EQ(it->return_values()[0].type(), "string8");

		// Find return_int64_array_1d function (array test)
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "return_int64_array_1d"; });
		REQUIRE_NE(it, functions.end());
		CHECK_EQ(it->return_values().size(), 1);
		CHECK_EQ(it->return_values()[0].type(), "int64_packed_array");
		CHECK_EQ(it->return_values()[0].dimensions(), 1);

		// Find call_callback_add (callable test)
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "call_callback_add"; });
		REQUIRE_NE(it, functions.end());
		CHECK_EQ(it->parameters().size(), 1);
		CHECK_EQ(it->parameters()[0].type(), "callable");

		// Find accept_any (any type test)
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "accept_any"; });
		REQUIRE_NE(it, functions.end());
		CHECK_EQ(it->parameters().size(), 1);
		CHECK_EQ(it->parameters()[0].type(), "any");
		CHECK_EQ(it->return_values()[0].type(), "any");

		// Find return_two_values (multiple return values)
		it = std::find_if(functions.begin(), functions.end(),
						[](const FunctionDefinition& f) { return f.name() == "return_two_values"; });
		REQUIRE_NE(it, functions.end());
		CHECK_EQ(it->return_values().size(), 2);
		CHECK_EQ(it->return_values()[0].type(), "int64");
		CHECK_EQ(it->return_values()[1].type(), "string8");
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
		CHECK_EQ(idl2.idl_source(), idl.idl_source());
		CHECK_EQ(idl2.modules().size(), idl.modules().size());
		CHECK_EQ(idl2.modules()[0].functions().size(), idl.modules()[0].functions().size());

		// Cleanup
		std::filesystem::remove(temp_path);
	}
}

TEST_CASE("TypeMapper")
{
	SUBCASE("Primitive types")
	{
		auto [type, dims] = TypeMapper::map_type<int64_t>();
		CHECK_EQ(type, "int64");
		CHECK_EQ(dims, 0);

		auto [type2, dims2] = TypeMapper::map_type<double>();
		CHECK_EQ(type2, "float64");
		CHECK_EQ(dims2, 0);

		auto [type3, dims3] = TypeMapper::map_type<std::string>();
		CHECK_EQ(type3, "string8");
		CHECK_EQ(dims3, 0);
	}

	SUBCASE("Type aliases")
	{
		CHECK_EQ(TypeMapper::get_type_alias<int64_t>(), "int64_t");
		CHECK_EQ(TypeMapper::get_type_alias<double>(), "double");
		CHECK_EQ(TypeMapper::get_type_alias<std::string>(), "std::string");
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
