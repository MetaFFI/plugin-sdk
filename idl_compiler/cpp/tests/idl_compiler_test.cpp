#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#ifndef DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#endif
#ifndef DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#endif
#include <doctest/doctest.h>

#include "type_mapper.h"
#include "entity_path.h"
#include "extractor.h"
#include "idl_generator.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <map>

// TEST_HEADER_PATH is injected by CMakeLists.txt.
#ifndef TEST_HEADER_PATH
#   error "TEST_HEADER_PATH not defined — configure CMake correctly"
#endif

// IDL_SCHEMA_PATH is injected by CMakeLists.txt.
#ifndef IDL_SCHEMA_PATH
#   define IDL_SCHEMA_PATH ""
#endif


// ============================================================================
// Helper: load a file to string
// ============================================================================

static std::string read_file(const std::string& path)
{
	std::ifstream f(path);
	if (!f.is_open())
	{
		throw std::runtime_error("Cannot open file: " + path);
	}
	return {std::istreambuf_iterator<char>(f), {}};
}


// ============================================================================
// Suite 1 — Type Mapping
// ============================================================================

TEST_SUITE("1. Type Mapping")
{
	TEST_CASE("1.1 int → int32")
	{
		auto [t, d] = CppTypeMapper::map("int");
		CHECK(t == "int32");
		CHECK(d == 0);
	}

	TEST_CASE("1.2 double → float64")
	{
		auto [t, d] = CppTypeMapper::map("double");
		CHECK(t == "float64");
		CHECK(d == 0);
	}

	TEST_CASE("1.3 float → float32")
	{
		auto [t, d] = CppTypeMapper::map("float");
		CHECK(t == "float32");
		CHECK(d == 0);
	}

	TEST_CASE("1.4 const char* → string8, dim=0")
	{
		auto [t, d] = CppTypeMapper::map("const char*");
		CHECK(t == "string8");
		CHECK(d == 0);
	}

	TEST_CASE("1.5 char* → string8, dim=0")
	{
		auto [t, d] = CppTypeMapper::map("char*");
		CHECK(t == "string8");
		CHECK(d == 0);
	}

	TEST_CASE("1.6 void* → handle")
	{
		auto [t, d] = CppTypeMapper::map("void*");
		CHECK(t == "handle");
		CHECK(d == 0);
	}

	TEST_CASE("1.7 bool → bool")
	{
		auto [t, d] = CppTypeMapper::map("bool");
		CHECK(t == "bool");
		CHECK(d == 0);
	}

	TEST_CASE("1.8 int* → int32, dim=1")
	{
		auto [t, d] = CppTypeMapper::map("int*");
		CHECK(t == "int32");
		CHECK(d == 1);
	}

	TEST_CASE("1.9 size_t → size")
	{
		auto [t, d] = CppTypeMapper::map("size_t");
		CHECK(t == "size");
		CHECK(d == 0);
	}

	TEST_CASE("1.10 unknown type → handle, dim=0")
	{
		auto [t, d] = CppTypeMapper::map("SomeCustomClass");
		CHECK(t == "handle");
		CHECK(d == 0);
	}
}


// ============================================================================
// Suite 2 — Entity Path Generation
// ============================================================================

TEST_SUITE("2. Entity Path Generation")
{
	TEST_CASE("2.1 Free function entity path")
	{
		CppFunctionInfo fn;
		fn.name         = "add";
		fn.mangled_name = "add";
		fn.ns           = "";

		auto ep = CppEntityPathGenerator::free_function(fn);
		CHECK(ep["callable"] == "add");
		CHECK(ep.count("namespace") == 0);  // no namespace key when empty
	}

	TEST_CASE("2.2 Namespaced function entity path includes namespace key")
	{
		CppFunctionInfo fn;
		fn.name         = "multiply";
		fn.mangled_name = "?multiply@math@@YAHHH@Z";
		fn.ns           = "math";

		auto ep = CppEntityPathGenerator::free_function(fn);
		CHECK(ep["callable"] == "?multiply@math@@YAHHH@Z");
		CHECK(ep["namespace"] == "math");
	}

	TEST_CASE("2.3 Global getter entity path")
	{
		CppGlobalInfo g;
		g.name         = "g_counter";
		g.mangled_name = "g_counter";
		g.ns           = "";

		auto ep = CppEntityPathGenerator::global_getter(g);
		CHECK(ep["global"] == "g_counter");
		CHECK(ep["getter"] == "true");
		CHECK(ep.count("namespace") == 0);
	}

	TEST_CASE("2.4 Global setter entity path")
	{
		CppGlobalInfo g;
		g.name         = "g_counter";
		g.mangled_name = "g_counter";
		g.ns           = "";

		auto ep = CppEntityPathGenerator::global_setter(g);
		CHECK(ep["global"] == "g_counter");
		CHECK(ep["setter"] == "true");
	}

	TEST_CASE("2.5 Instance method entity path has instance_required=true")
	{
		CppFunctionInfo fn;
		fn.name         = "sum";
		fn.mangled_name = "?sum@Point@@QEBANXZ";
		fn.ns           = "";

		auto ep = CppEntityPathGenerator::instance_method(fn);
		CHECK(ep["callable"] == "?sum@Point@@QEBANXZ");
		CHECK(ep["instance_required"] == "true");
	}

	TEST_CASE("2.6 Constructor entity path embeds class_size")
	{
		auto ep = CppEntityPathGenerator::constructor("??0Point@@QEAA@HN@Z", 16, "");
		CHECK(ep["callable"]    == "??0Point@@QEAA@HN@Z");
		CHECK(ep["constructor"] == "true");
		CHECK(ep["class_size"]  == "16");
		CHECK(ep.count("namespace") == 0);
	}

	TEST_CASE("2.7 Destructor entity path")
	{
		auto ep = CppEntityPathGenerator::destructor("??1Point@@QEAA@XZ", "");
		CHECK(ep["callable"]   == "??1Point@@QEAA@XZ");
		CHECK(ep["destructor"] == "true");
	}

	TEST_CASE("2.8 Field getter and setter entity paths")
	{
		CppFieldInfo f;
		f.name         = "x";
		f.type_str     = "int";
		f.offset_bytes = 0;
		f.size_bytes   = 4;

		auto gep = CppEntityPathGenerator::field_getter(f, "");
		CHECK(gep["field"]             == "x");
		CHECK(gep["getter"]            == "true");
		CHECK(gep["instance_required"] == "true");
		CHECK(gep["field_offset"]      == "0");

		auto sep = CppEntityPathGenerator::field_setter(f, "");
		CHECK(sep["field"]             == "x");
		CHECK(sep["setter"]            == "true");
		CHECK(sep["instance_required"] == "true");
		CHECK(sep["field_offset"]      == "0");
		CHECK(sep["field_size"]        == "4");
	}
}


// ============================================================================
// Suite 3 — Function Extraction
// ============================================================================

TEST_SUITE("3. Function Extraction")
{
	TEST_CASE("3.1 Extract 'add' from test_header.h")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		// Find the 'add' function (global namespace)
		bool found = false;
		for (const auto& fn : info.functions)
		{
			if (fn.name == "add" && fn.ns.empty())
			{
				found = true;
				CHECK(fn.params.size() == 2);
				CHECK(fn.return_types.size() == 1);
				// Mangled name should be non-empty
				CHECK(!fn.mangled_name.empty());
			}
		}
		CHECK(found);
	}

	TEST_CASE("3.2 Extract 'math::multiply' as namespaced function")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		bool found = false;
		for (const auto& fn : info.functions)
		{
			if (fn.name == "multiply" && fn.ns == "math")
			{
				found = true;
				CHECK(fn.params.size() == 2);
				CHECK(!fn.mangled_name.empty());
			}
		}
		CHECK(found);
	}

	TEST_CASE("3.3 Function parameter types mapped correctly for 'add'")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		for (const auto& fn : info.functions)
		{
			if (fn.name == "add" && fn.ns.empty())
			{
				// Both params should be 'int'
				for (const auto& p : fn.params)
				{
					auto [t, d] = CppTypeMapper::map(p.type_str);
					CHECK(t == "int32");
					CHECK(d == 0);
				}
			}
		}
	}

	TEST_CASE("3.4 Return type of 'add' is int")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		for (const auto& fn : info.functions)
		{
			if (fn.name == "add" && fn.ns.empty())
			{
				REQUIRE(!fn.return_types.empty());
				auto [t, d] = CppTypeMapper::map(fn.return_types[0]);
				CHECK(t == "int32");
			}
		}
	}

	TEST_CASE("3.5 Global namespace and math namespace functions extracted separately")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		int global_fns = 0;
		int math_fns   = 0;

		for (const auto& fn : info.functions)
		{
			if (fn.ns.empty())      global_fns++;
			else if (fn.ns == "math") math_fns++;
		}

		CHECK(global_fns >= 1);  // at least 'add'
		CHECK(math_fns   >= 1);  // at least 'multiply'
	}
}


// ============================================================================
// Suite 4 — Class Extraction
// ============================================================================

TEST_SUITE("4. Class Extraction")
{
	TEST_CASE("4.1 Extract 'Point' class from test_header.h")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		bool found = false;
		for (const auto& cls : info.classes)
		{
			if (cls.name == "Point")
			{
				found = true;
			}
		}
		CHECK(found);
	}

	TEST_CASE("4.2 Point has constructor and destructor")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		for (const auto& cls : info.classes)
		{
			if (cls.name == "Point")
			{
				CHECK(!cls.constructors.empty());
				CHECK(cls.has_destructor);
				CHECK(!cls.mangled_ctor.empty());
				CHECK(!cls.mangled_dtor.empty());
			}
		}
	}

	TEST_CASE("4.3 Point has public fields x and y")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		for (const auto& cls : info.classes)
		{
			if (cls.name == "Point")
			{
				bool has_x = false, has_y = false;

				for (const auto& f : cls.fields)
				{
					if (f.name == "x") has_x = true;
					if (f.name == "y") has_y = true;
				}

				CHECK(has_x);
				CHECK(has_y);
			}
		}
	}

	TEST_CASE("4.4 Point::class_size > 0")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		for (const auto& cls : info.classes)
		{
			if (cls.name == "Point")
			{
				CHECK(cls.class_size > 0);
			}
		}
	}

	TEST_CASE("4.5 Point has instance methods sum and set_x")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		for (const auto& cls : info.classes)
		{
			if (cls.name == "Point")
			{
				bool has_sum   = false;
				bool has_set_x = false;

				for (const auto& m : cls.methods)
				{
					if (m.name == "sum")   has_sum   = true;
					if (m.name == "set_x") has_set_x = true;
					CHECK(!m.is_static);
				}

				CHECK(has_sum);
				CHECK(has_set_x);
			}
		}
	}

	TEST_CASE("4.6 Point has static method static_add")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		for (const auto& cls : info.classes)
		{
			if (cls.name == "Point")
			{
				bool found = false;
				for (const auto& sm : cls.static_methods)
				{
					if (sm.name == "static_add")
					{
						found = true;
						CHECK(sm.is_static);
					}
				}
				CHECK(found);
			}
		}
	}
}


// ============================================================================
// Suite 5 — Global Extraction
// ============================================================================

TEST_SUITE("5. Global Extraction")
{
	TEST_CASE("5.1 Extract 'g_counter' global from global namespace")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		bool found = false;
		for (const auto& g : info.globals)
		{
			if (g.name == "g_counter" && g.ns.empty())
			{
				found = true;
				CHECK(!g.mangled_name.empty());
			}
		}
		CHECK(found);
	}

	TEST_CASE("5.2 g_counter getter and setter entity paths are correct")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		for (const auto& g : info.globals)
		{
			if (g.name == "g_counter")
			{
				auto gep = CppEntityPathGenerator::global_getter(g);
				auto sep = CppEntityPathGenerator::global_setter(g);

				CHECK(gep["global"] == g.mangled_name);
				CHECK(gep["getter"] == "true");

				CHECK(sep["global"] == g.mangled_name);
				CHECK(sep["setter"] == "true");
			}
		}
	}

	TEST_CASE("5.3 Extract 'math::g_scale' with namespace key")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		bool found = false;
		for (const auto& g : info.globals)
		{
			if (g.name == "g_scale" && g.ns == "math")
			{
				found = true;
				auto ep = CppEntityPathGenerator::global_getter(g);
				CHECK(ep["namespace"] == "math");
			}
		}
		CHECK(found);
	}

	TEST_CASE("5.4 Type of g_counter is int32")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppModuleInfo info = ex.extract();

		for (const auto& g : info.globals)
		{
			if (g.name == "g_counter")
			{
				auto [t, d] = CppTypeMapper::map(g.type_str);
				CHECK(t == "int32");
			}
		}
	}
}


// ============================================================================
// Suite 6 — IDL Generation & Schema Structure
// ============================================================================

TEST_SUITE("6. IDL Generation")
{
	TEST_CASE("6.1 Generated IDL has required top-level fields")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppIDLGenerator gen(ex.extract());
		nlohmann::json idl = gen.generate();

		CHECK(idl.contains("idl_source"));
		CHECK(idl.contains("idl_extension"));
		CHECK(idl.contains("idl_filename_with_extension"));
		CHECK(idl.contains("idl_full_path"));
		CHECK(idl.contains("metaffi_guest_lib"));
		CHECK(idl.contains("target_language"));
		CHECK(idl.contains("modules"));

		CHECK(idl["target_language"] == "cpp");
		CHECK(idl["idl_source"]      == "test_header");
		CHECK(idl["modules"].is_array());
	}

	TEST_CASE("6.2 Modules contain functions, classes, and globals arrays")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppIDLGenerator gen(ex.extract());
		nlohmann::json idl = gen.generate();

		for (const auto& m : idl["modules"])
		{
			CHECK(m.contains("name"));
			CHECK(m.contains("functions"));
			CHECK(m.contains("classes"));
			CHECK(m.contains("globals"));
			CHECK(m.contains("external_resources"));

			CHECK(m["functions"].is_array());
			CHECK(m["classes"].is_array());
			CHECK(m["globals"].is_array());
		}
	}

	TEST_CASE("6.3 Global namespace module contains 'add' function")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppIDLGenerator gen(ex.extract());
		nlohmann::json idl = gen.generate();

		bool found_add = false;
		for (const auto& m : idl["modules"])
		{
			if (m["name"].get<std::string>() == "test_header")
			{
				for (const auto& fn : m["functions"])
				{
					if (fn["name"].get<std::string>() == "add") found_add = true;
				}
			}
		}
		CHECK(found_add);
	}

	TEST_CASE("6.4 'math' module contains 'multiply' function")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppIDLGenerator gen(ex.extract());
		nlohmann::json idl = gen.generate();

		bool found = false;
		for (const auto& m : idl["modules"])
		{
			if (m["name"] == "math")
			{
				for (const auto& fn : m["functions"])
				{
					if (fn["name"] == "multiply") found = true;
				}
			}
		}
		CHECK(found);
	}

	TEST_CASE("6.5 Point class present in global module with constructor and destructor")
	{
		CppExtractor ex(TEST_HEADER_PATH);
		CppIDLGenerator gen(ex.extract());
		nlohmann::json idl = gen.generate();

		for (const auto& m : idl["modules"])
		{
			for (const auto& cls : m["classes"])
			{
				if (cls["name"] == "Point")
				{
					CHECK(!cls["constructors"].empty());
					CHECK(!cls["release"].is_null());
					CHECK(cls.contains("methods"));
					CHECK(cls.contains("fields"));
					CHECK(!cls["fields"].empty());
				}
			}
		}
	}
}


// ============================================================================
// Suite 7 — Error Handling
// ============================================================================

TEST_SUITE("7. Error Handling")
{
	TEST_CASE("7.1 CppExtractor throws on nonexistent header")
	{
		CHECK_THROWS_AS(
			CppExtractor("/nonexistent/path/does_not_exist.h"),
			std::runtime_error);
	}

	TEST_CASE("7.2 Unknown type falls back to handle")
	{
		auto [t, d] = CppTypeMapper::map("CompletelyUnknownType_XYZ");
		CHECK(t == "handle");
		CHECK(d == 0);
	}

	TEST_CASE("7.3 Pointer to unknown type is handle with dim=1")
	{
		auto [t, d] = CppTypeMapper::map("CompletelyUnknownType_XYZ*");
		CHECK(t == "handle");
		CHECK(d == 1);
	}
}
