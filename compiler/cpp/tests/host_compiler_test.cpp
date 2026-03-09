#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../host/type_mapper.h"
#include "../host/entity_path_converter.h"
#include "../host/code_generator.h"
#include "../host/host_compiler.h"

#include <metaffi/idl/idl_definition.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

using namespace metaffi::compiler::cpp;
using namespace metaffi::idl;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string read_fixture_idl() {
    const std::string fixtures_dir = FIXTURES_DIR;
    const std::string path = fixtures_dir + "/point_idl.json";

    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open fixture IDL: " + path);

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static IDLDefinition load_fixture_idl() {
    return IDLDefinition::load_from_json(read_fixture_idl());
}

static std::string temp_dir() {
    auto tmp = std::filesystem::temp_directory_path() / "cpp_compiler_test";
    std::filesystem::create_directories(tmp);
    return tmp.string();
}

static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Build a minimal IDL with a single module and single function.
static IDLDefinition make_minimal_idl(const std::string& module_name,
                                      const std::string& func_name,
                                      const std::vector<ArgDefinition>& params,
                                      const std::vector<ArgDefinition>& returns,
                                      const std::map<std::string,std::string>& entity_path = {{"callable","func"}}) {
    FunctionDefinition func(func_name);
    for (const auto& p : params) func.add_parameter(p);
    for (const auto& r : returns) func.add_return_value(r);
    for (const auto& [k,v] : entity_path) func.set_entity_path(k, v);

    ModuleDefinition mod(module_name);
    mod.add_function(std::move(func));

    IDLDefinition idl;
    idl.set_idl_source("test").set_target_language("cpp").set_metaffi_guest_lib("guest_test");
    idl.add_module(std::move(mod));
    return idl;
}

// ===========================================================================
// Suite 1 — CppTypeMapper: scalar primitive types
// ===========================================================================

TEST_SUITE("Suite 1 - Type Mapper Scalars") {

    TEST_CASE("int8  → int8_t") {
        CHECK(CppTypeMapper::cpp_type("int8",  0) == "int8_t");
    }

    TEST_CASE("int16 → int16_t") {
        CHECK(CppTypeMapper::cpp_type("int16", 0) == "int16_t");
    }

    TEST_CASE("int32 → int32_t") {
        CHECK(CppTypeMapper::cpp_type("int32", 0) == "int32_t");
    }

    TEST_CASE("int64 → int64_t") {
        CHECK(CppTypeMapper::cpp_type("int64", 0) == "int64_t");
    }

    TEST_CASE("uint8  → uint8_t") {
        CHECK(CppTypeMapper::cpp_type("uint8",  0) == "uint8_t");
    }

    TEST_CASE("uint16 → uint16_t") {
        CHECK(CppTypeMapper::cpp_type("uint16", 0) == "uint16_t");
    }

    TEST_CASE("uint32 → uint32_t") {
        CHECK(CppTypeMapper::cpp_type("uint32", 0) == "uint32_t");
    }

    TEST_CASE("uint64 → uint64_t") {
        CHECK(CppTypeMapper::cpp_type("uint64", 0) == "uint64_t");
    }

    TEST_CASE("float32 → float") {
        CHECK(CppTypeMapper::cpp_type("float32", 0) == "float");
    }

    TEST_CASE("float64 → double") {
        CHECK(CppTypeMapper::cpp_type("float64", 0) == "double");
    }

    TEST_CASE("bool → bool") {
        CHECK(CppTypeMapper::cpp_type("bool", 0) == "bool");
    }

    TEST_CASE("string8  → std::string") {
        CHECK(CppTypeMapper::cpp_type("string8",  0) == "std::string");
    }

    TEST_CASE("string16 → std::u16string") {
        CHECK(CppTypeMapper::cpp_type("string16", 0) == "std::u16string");
    }

    TEST_CASE("string32 → std::u32string") {
        CHECK(CppTypeMapper::cpp_type("string32", 0) == "std::u32string");
    }

    TEST_CASE("char8  → char") {
        CHECK(CppTypeMapper::cpp_type("char8",  0) == "char");
    }

    TEST_CASE("char16 → char16_t") {
        CHECK(CppTypeMapper::cpp_type("char16", 0) == "char16_t");
    }

    TEST_CASE("char32 → char32_t") {
        CHECK(CppTypeMapper::cpp_type("char32", 0) == "char32_t");
    }

    TEST_CASE("handle → metaffi_handle") {
        CHECK(CppTypeMapper::cpp_type("handle", 0) == "metaffi_handle");
    }

    TEST_CASE("size → uint64_t") {
        CHECK(CppTypeMapper::cpp_type("size", 0) == "uint64_t");
    }

    TEST_CASE("Unknown type throws std::runtime_error") {
        CHECK_THROWS_AS(CppTypeMapper::cpp_type("unknown_xyz", 0), std::runtime_error);
    }
}

// ===========================================================================
// Suite 2 — CppTypeMapper: arrays and type_info literals
// ===========================================================================

TEST_SUITE("Suite 2 - Type Mapper Arrays and TypeInfo") {

    TEST_CASE("int32 dim=1 → std::vector<int32_t>") {
        CHECK(CppTypeMapper::cpp_type("int32", 1) == "std::vector<int32_t>");
    }

    TEST_CASE("float64 dim=2 → nested vector") {
        CHECK(CppTypeMapper::cpp_type("float64", 2) == "std::vector<std::vector<double>>");
    }

    TEST_CASE("string8 dim=1 → std::vector<std::string>") {
        CHECK(CppTypeMapper::cpp_type("string8", 1) == "std::vector<std::string>");
    }

    TEST_CASE("uint8 dim=1 → std::vector<uint8_t>") {
        CHECK(CppTypeMapper::cpp_type("uint8", 1) == "std::vector<uint8_t>");
    }

    TEST_CASE("_array suffix is stripped: 'int32_array' dim=0 → std::vector<int32_t>") {
        // IDL may store type as "int32_array" with dim=0 — should still wrap in vector
        CHECK(CppTypeMapper::cpp_type("int32_array", 0) == "std::vector<int32_t>");
    }

    TEST_CASE("type_info_literal int32 scalar → metaffi_type_info(metaffi_int32_type)") {
        CHECK(CppTypeMapper::type_info_literal("int32", 0) == "metaffi_type_info(metaffi_int32_type)");
    }

    TEST_CASE("type_info_literal float64 scalar → metaffi_type_info(metaffi_float64_type)") {
        CHECK(CppTypeMapper::type_info_literal("float64", 0) == "metaffi_type_info(metaffi_float64_type)");
    }

    TEST_CASE("type_info_literal handle scalar → metaffi_type_info(metaffi_handle_type)") {
        CHECK(CppTypeMapper::type_info_literal("handle", 0) == "metaffi_type_info(metaffi_handle_type)");
    }

    TEST_CASE("type_info_literal int32 dim=1 → metaffi_type_info(metaffi_int32_type, nullptr, false, 1)") {
        CHECK(CppTypeMapper::type_info_literal("int32", 1) == "metaffi_type_info(metaffi_int32_type, nullptr, false, 1)");
    }

    TEST_CASE("type_info_literal float64 dim=2 → metaffi_type_info(metaffi_float64_type, nullptr, false, 2)") {
        CHECK(CppTypeMapper::type_info_literal("float64", 2) == "metaffi_type_info(metaffi_float64_type, nullptr, false, 2)");
    }

    TEST_CASE("type_info_literal _array suffix dim=0 → effective dim=1") {
        CHECK(CppTypeMapper::type_info_literal("int32_array", 0) == "metaffi_type_info(metaffi_int32_array_type, nullptr, false, 1)");
    }

    TEST_CASE("metaffi_constant int64 → metaffi_int64_type") {
        CHECK(CppTypeMapper::metaffi_constant("int64") == "metaffi_int64_type");
    }

    TEST_CASE("metaffi_constant string8 → metaffi_string8_type") {
        CHECK(CppTypeMapper::metaffi_constant("string8") == "metaffi_string8_type");
    }

    TEST_CASE("metaffi_constant _array suffix returns array constant") {
        CHECK(CppTypeMapper::metaffi_constant("float32_array") == "metaffi_float32_array_type");
    }
}

// ===========================================================================
// Suite 3 — CppEntityPathConverter
// ===========================================================================

TEST_SUITE("Suite 3 - Entity Path Converter") {

    TEST_CASE("Single key serialises correctly") {
        CHECK(CppEntityPathConverter::convert({{"callable", "foo"}}) == "callable=foo");
    }

    TEST_CASE("Multi-key map serialises in sorted key order (callable < namespace)") {
        CHECK(CppEntityPathConverter::convert({{"callable","bar"},{"namespace","myns"}})
              == "callable=bar,namespace=myns");
    }

    TEST_CASE("Three-key map serialises alphabetically") {
        CHECK(CppEntityPathConverter::convert({{"c","3"},{"a","1"},{"b","2"}})
              == "a=1,b=2,c=3");
    }

    TEST_CASE("Empty map produces empty string") {
        CHECK(CppEntityPathConverter::convert({}) == "");
    }

    TEST_CASE("Ordering is stable: z_key after a_key") {
        CHECK(CppEntityPathConverter::convert({{"z_key","last"},{"a_key","first"}})
              == "a_key=first,z_key=last");
    }
}

// ===========================================================================
// Suite 4 — Code Generator: Free Functions
// ===========================================================================

TEST_SUITE("Suite 4 - Code Generator: Free Functions") {

    TEST_CASE("Generated header contains int32_t add signature") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("int32_t add(int32_t a, int32_t b)") != std::string::npos);
    }

    TEST_CASE("Generated source contains bind() function") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("void bind(") != std::string::npos);
    }

    TEST_CASE("Generated source bind() calls make_unique<MetaFFIRuntime>") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("make_unique<MetaFFIRuntime>") != std::string::npos);
    }

    TEST_CASE("Generated source bind() calls load_runtime_plugin()") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("load_runtime_plugin()") != std::string::npos);
    }

    TEST_CASE("Generated source contains load_entity_with_info for add with correct entity path") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("?add@@YAHHH@Z") != std::string::npos);
    }

    TEST_CASE("Generated source contains int32 MetaFFITypeInfo for add parameters") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        // load_entity_with_info for add must include int32 type infos
        CHECK(files.source_content.find("metaffi_int32_type") != std::string::npos);
    }

    TEST_CASE("Generated code is in test_module_host namespace") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("namespace test_module_host") != std::string::npos);
        CHECK(files.source_content.find("namespace test_module_host") != std::string::npos);
    }

    TEST_CASE("Generated header contains global getter/setter declarations") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("get_g_counter") != std::string::npos);
        CHECK(files.header_content.find("set_g_counter") != std::string::npos);
    }

    TEST_CASE("Void function (no params, no returns) generates void return and .call() with no args") {
        // Function: void noop()
        auto idl = make_minimal_idl("mymod", "noop", {}, {}, {{"callable","noop"}});
        auto files = CppCodeGenerator{}.generate(idl, "out");
        CHECK(files.header_content.find("void noop()") != std::string::npos);
        // Source stub must call entity without template args and without arguments
        CHECK(files.source_content.find("_noop_entity->call()") != std::string::npos);
    }

    TEST_CASE("Multi-return function generates std::tuple return type in header") {
        // Function: (int32, string8) two_returns(int32 a)
        std::vector<ArgDefinition> params = {ArgDefinition("a","int32")};
        std::vector<ArgDefinition> rets   = {ArgDefinition("r1","int32"), ArgDefinition("r2","string8")};
        auto idl = make_minimal_idl("mymod", "two_returns", params, rets);
        auto files = CppCodeGenerator{}.generate(idl, "out");
        CHECK(files.header_content.find("std::tuple<int32_t, std::string>") != std::string::npos);
    }

    TEST_CASE("Multi-return function source uses structured binding and make_tuple") {
        std::vector<ArgDefinition> params = {ArgDefinition("a","int32")};
        std::vector<ArgDefinition> rets   = {ArgDefinition("r1","int32"), ArgDefinition("r2","string8")};
        auto idl = make_minimal_idl("mymod", "two_returns", params, rets);
        auto files = CppCodeGenerator{}.generate(idl, "out");
        CHECK(files.source_content.find("auto [r1, r2]") != std::string::npos);
        CHECK(files.source_content.find("make_tuple") != std::string::npos);
    }

    TEST_CASE("Array parameter function generates std::vector param in header") {
        // Function: void process(int32[] data)
        std::vector<ArgDefinition> params = {ArgDefinition("data","int32","","",1)};
        auto idl = make_minimal_idl("mymod", "process", params, {});
        auto files = CppCodeGenerator{}.generate(idl, "out");
        CHECK(files.header_content.find("std::vector<int32_t> data") != std::string::npos);
    }

    TEST_CASE("Array parameter generates correct type_info dim=1 in bind()") {
        std::vector<ArgDefinition> params = {ArgDefinition("data","float64","","",1)};
        auto idl = make_minimal_idl("mymod", "arr_fn", params, {});
        auto files = CppCodeGenerator{}.generate(idl, "out");
        CHECK(files.source_content.find("metaffi_type_info(metaffi_float64_type, nullptr, false, 1)") != std::string::npos);
    }
}

// ===========================================================================
// Suite 5 — Code Generator: Classes
// ===========================================================================

TEST_SUITE("Suite 5 - Code Generator: Classes") {

    TEST_CASE("Generated header contains class Point declaration") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("class Point") != std::string::npos);
    }

    TEST_CASE("Generated header contains Point constructor with int32_t x, double y params") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("explicit Point(int32_t x, double y)") != std::string::npos);
    }

    TEST_CASE("Generated header contains handle-wrap constructor") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("explicit Point(metaffi_handle handle)") != std::string::npos);
    }

    TEST_CASE("Generated header contains double sum() method") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("double sum()") != std::string::npos);
    }

    TEST_CASE("Generated header contains set_x(int32_t v) method") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("set_x(int32_t v)") != std::string::npos);
    }

    TEST_CASE("Generated header contains get_x field accessor") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("get_x") != std::string::npos);
    }

    TEST_CASE("Generated header contains set_x_field field setter") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("set_x_field") != std::string::npos);
    }

    TEST_CASE("Generated header contains _handle private member") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("metaffi_handle _handle") != std::string::npos);
    }

    TEST_CASE("Generated source contains Point ctor and dtor entity statics") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("_Point_ctor_entity") != std::string::npos);
        CHECK(files.source_content.find("_Point_dtor_entity") != std::string::npos);
    }

    TEST_CASE("Generated source contains Point method entity statics") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("_Point_sum_entity") != std::string::npos);
        CHECK(files.source_content.find("_Point_set_x_entity") != std::string::npos);
    }

    TEST_CASE("Generated source bind() loads constructor with handle type_info in return") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        // Constructor entity path
        CHECK(files.source_content.find("??0Point@@QEAA@HN@Z") != std::string::npos);
        // Return value: handle
        CHECK(files.source_content.find("metaffi_handle_type") != std::string::npos);
    }

    TEST_CASE("Generated source instance method bind block loads entity with correct entity path") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        // The sum() method bind call must reference the sum entity and its mangled name
        auto pos = files.source_content.find("_Point_sum_entity = std::make_unique<MetaFFIEntity>(_module->load_entity_with_info(");
        CHECK(pos != std::string::npos);
        auto block = files.source_content.substr(pos, 300);
        CHECK(block.find("?sum@Point@@QEBANXZ") != std::string::npos);
    }

    TEST_CASE("Generated source Point::sum() calls entity with _handle as first arg") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("_Point_sum_entity->call<double>(_handle)") != std::string::npos);
    }

    TEST_CASE("Generated source Point constructor sets _owns_handle = true") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("_owns_handle = true") != std::string::npos);
    }

    TEST_CASE("Generated source destructor has RAII guard (if _owns_handle && _handle != nullptr)") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("_owns_handle && _handle != nullptr") != std::string::npos);
    }

    TEST_CASE("Generated source destructor nulls out _handle after call") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("_handle = nullptr") != std::string::npos);
    }

    TEST_CASE("Generated source field setter calls entity with _handle and value") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("_Point_set_x_field_entity") != std::string::npos);
    }
}

// ===========================================================================
// Suite 6 — Code Generator: Namespaces / Multi-module
// ===========================================================================

TEST_SUITE("Suite 6 - Code Generator: Namespaces") {

    TEST_CASE("Module 'math' generates math_host namespace in header") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("namespace math_host") != std::string::npos);
    }

    TEST_CASE("Module 'math' generates math_host namespace in source") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("namespace math_host") != std::string::npos);
    }

    TEST_CASE("Two separate bind() functions are emitted (one per module)") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        // Count occurrences of "void bind(" — should be at least 2
        size_t count = 0;
        size_t pos = 0;
        while ((pos = files.source_content.find("void bind(", pos)) != std::string::npos) {
            ++count;
            ++pos;
        }
        CHECK(count >= 2);
    }

    TEST_CASE("multiply function present in math_host header") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("int32_t multiply(") != std::string::npos);
    }

    TEST_CASE("g_scale getter/setter present in math_host header") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("get_g_scale") != std::string::npos);
        CHECK(files.header_content.find("set_g_scale") != std::string::npos);
    }
}

// ===========================================================================
// Suite 7 — Generated file structure
// ===========================================================================

TEST_SUITE("Suite 7 - Generated File Structure") {

    TEST_CASE("Header begins with #pragma once") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.substr(0, 12) == "#pragma once");
    }

    TEST_CASE("Header includes metaffi_api.h") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("#include <metaffi/api/metaffi_api.h>") != std::string::npos);
    }

    TEST_CASE("Header includes <cstdint>") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("#include <cstdint>") != std::string::npos);
    }

    TEST_CASE("Header includes <vector> for array support") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.header_content.find("#include <vector>") != std::string::npos);
    }

    TEST_CASE("Source begins with DO NOT EDIT comment") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("DO NOT EDIT") != std::string::npos);
    }

    TEST_CASE("Source includes the generated header") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("test_MetaFFIHost.hpp") != std::string::npos);
    }

    TEST_CASE("Source includes <memory> for unique_ptr") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("#include <memory>") != std::string::npos);
    }

    TEST_CASE("Source declares static MetaFFIRuntime and MetaFFIModule") {
        auto idl = load_fixture_idl();
        auto files = CppCodeGenerator{}.generate(idl, "test_MetaFFIHost");
        CHECK(files.source_content.find("static std::unique_ptr<MetaFFIRuntime>") != std::string::npos);
        CHECK(files.source_content.find("static std::unique_ptr<MetaFFIModule>")  != std::string::npos);
    }
}

// ===========================================================================
// Suite 8 — Full File Output (CppHostCompiler)
// ===========================================================================

TEST_SUITE("Suite 8 - Full File Output") {

    TEST_CASE("compile() writes .hpp and .cpp to output directory") {
        const std::string out = temp_dir();
        CppHostCompiler{}.compile(read_fixture_idl(), out, "point");

        CHECK(std::filesystem::exists(std::filesystem::path(out) / "point_MetaFFIHost.hpp"));
        CHECK(std::filesystem::exists(std::filesystem::path(out) / "point_MetaFFIHost.cpp"));
    }

    TEST_CASE("Written .hpp starts with #pragma once") {
        const std::string out = temp_dir();
        CppHostCompiler{}.compile(read_fixture_idl(), out, "point");
        const auto content = read_file((std::filesystem::path(out) / "point_MetaFFIHost.hpp").string());
        CHECK(content.substr(0, 12) == "#pragma once");
    }

    TEST_CASE("Written .cpp includes the generated header by stem") {
        const std::string out = temp_dir();
        CppHostCompiler{}.compile(read_fixture_idl(), out, "point");
        const auto content = read_file((std::filesystem::path(out) / "point_MetaFFIHost.cpp").string());
        CHECK(content.find("point_MetaFFIHost.hpp") != std::string::npos);
    }

    TEST_CASE("compile() creates the output directory if it does not exist") {
        auto new_dir = std::filesystem::temp_directory_path() / "cpp_compiler_new_dir_test";
        std::filesystem::remove_all(new_dir);

        CppHostCompiler{}.compile(read_fixture_idl(), new_dir.string(), "stem");
        CHECK(std::filesystem::exists(new_dir));

        std::filesystem::remove_all(new_dir);
    }
}

// ===========================================================================
// Suite 9 — Error Handling and Validation
// ===========================================================================

TEST_SUITE("Suite 9 - Error Handling") {

    TEST_CASE("Invalid JSON throws std::runtime_error") {
        CHECK_THROWS_AS(
            CppHostCompiler{}.compile("{this is not json}", temp_dir(), "bad"),
            std::runtime_error);
    }

    TEST_CASE("Unwritable output path throws std::runtime_error") {
        // Null byte in path → always invalid on Win32 and POSIX
        CHECK_THROWS_AS(
            CppHostCompiler{}.compile(read_fixture_idl(), std::string("/dev/null/\0bad", 14), "stem"),
            std::runtime_error);
    }

    TEST_CASE("IDL with empty modules list generates valid (empty namespace-less) files") {
        // Should not throw — just produce minimal content
        IDLDefinition idl;
        idl.set_idl_source("empty").set_target_language("cpp").set_metaffi_guest_lib("g");
        // No modules added

        CppCodeGenerator gen;
        auto files = gen.generate(idl, "empty_MetaFFIHost");

        // Files are valid even with zero modules
        CHECK(files.header_content.find("#pragma once") != std::string::npos);
        CHECK(files.source_content.find("DO NOT EDIT") != std::string::npos);
    }

    TEST_CASE("IDL with module but no functions/classes/globals generates bind() only") {
        IDLDefinition idl;
        idl.set_idl_source("bare").set_target_language("cpp").set_metaffi_guest_lib("g");
        idl.add_module(ModuleDefinition("bare_module"));

        auto files = CppCodeGenerator{}.generate(idl, "bare_MetaFFIHost");

        CHECK(files.header_content.find("namespace bare_module_host") != std::string::npos);
        CHECK(files.source_content.find("void bind(") != std::string::npos);
    }
}
