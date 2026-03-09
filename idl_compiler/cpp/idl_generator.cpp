#include "idl_generator.h"
#include "entity_path.h"
#include "type_mapper.h"

#include <filesystem>
#include <map>
#include <set>
#include <stdexcept>


// ============================================================================
// Helpers
// ============================================================================

namespace
{
	// Serialise an entity_path map to a JSON object.
	nlohmann::json entity_path_to_json(const std::map<std::string, std::string>& ep)
	{
		nlohmann::json obj = nlohmann::json::object();
		for (const auto& [k, v] : ep)
		{
			if (!v.empty()) obj[k] = v;
		}
		return obj;
	}

	// Build an empty arg_definition JSON skeleton.
	nlohmann::json empty_arg(const std::string& name, const std::string& metaffi_type,
	                         const std::string& type_alias, int dims)
	{
		return {
			{"name",       name},
			{"type",       metaffi_type.empty() ? "handle" : metaffi_type},
			{"type_alias", type_alias},
			{"comment",    ""},
			{"tags",       nlohmann::json::object()},
			{"dimensions", dims}
		};
	}
} // anonymous namespace


// ============================================================================
// CppIDLGenerator
// ============================================================================

CppIDLGenerator::CppIDLGenerator(const CppModuleInfo& info)
	: m_info(info)
{
}

std::string CppIDLGenerator::header_stem() const
{
	// Manual stem extraction: handles both '/' and '\' as directory separators.
	std::string path = m_info.header_path;

	// Normalise separators
	for (char& c : path) { if (c == '\\') c = '/'; }

	// Strip leading directory components
	std::size_t slash = path.rfind('/');
	std::string filename = (slash == std::string::npos) ? path : path.substr(slash + 1);

	// Strip extension
	std::size_t dot = filename.rfind('.');
	if (dot != std::string::npos && dot > 0)
	{
		filename = filename.substr(0, dot);
	}

	return filename;
}

nlohmann::json CppIDLGenerator::param_to_json(const CppParamInfo& p)
{
	auto [metaffi_type, dims] = CppTypeMapper::map(p.type_str);
	return empty_arg(p.name.empty() ? "arg" : p.name, metaffi_type, p.type_str, dims);
}

nlohmann::json CppIDLGenerator::return_type_to_json(const std::string& type_str)
{
	auto [metaffi_type, dims] = CppTypeMapper::map(type_str);
	return empty_arg("return_value", metaffi_type, type_str, dims);
}

nlohmann::json CppIDLGenerator::build_function_entry(const CppFunctionInfo& fn,
                                                      int                    overload_index)
{
	nlohmann::json entry;
	entry["name"]    = fn.name;
	entry["comment"] = "";
	entry["tags"]    = nlohmann::json::object();

	// Entity path
	auto ep = CppEntityPathGenerator::free_function(fn);
	entry["entity_path"] = entity_path_to_json(ep);

	// Parameters
	nlohmann::json params = nlohmann::json::array();
	for (const auto& p : fn.params)
	{
		params.push_back(param_to_json(p));
	}
	entry["parameters"] = params;

	// Return values
	nlohmann::json retvals = nlohmann::json::array();
	for (const auto& rt : fn.return_types)
	{
		retvals.push_back(return_type_to_json(rt));
	}
	entry["return_values"]  = retvals;
	entry["overload_index"] = overload_index;

	return entry;
}

void CppIDLGenerator::build_class_entries(const CppClassInfo& cls, nlohmann::json& module)
{
	nlohmann::json class_entry;
	class_entry["name"]    = cls.name;
	class_entry["comment"] = "";
	class_entry["tags"]    = nlohmann::json::object();

	// Constructors
	nlohmann::json ctors = nlohmann::json::array();
	for (const auto& ctor_fn : cls.constructors)
	{
		nlohmann::json ctor_entry;
		ctor_entry["name"]    = ctor_fn.name;
		ctor_entry["comment"] = "";
		ctor_entry["tags"]    = nlohmann::json::object();

		auto ep = CppEntityPathGenerator::constructor(
			cls.mangled_ctor, cls.class_size, cls.ns);
		ctor_entry["entity_path"] = entity_path_to_json(ep);

		nlohmann::json params = nlohmann::json::array();
		for (const auto& p : ctor_fn.params)
		{
			params.push_back(param_to_json(p));
		}
		ctor_entry["parameters"]   = params;
		ctor_entry["return_values"] = nlohmann::json::array({
			empty_arg("instance", "handle", cls.name + "*", 0)
		});

		ctors.push_back(ctor_entry);
	}
	class_entry["constructors"] = ctors;

	// Destructor (release)
	if (cls.has_destructor && !cls.mangled_dtor.empty())
	{
		nlohmann::json dtor_entry;
		dtor_entry["name"]              = "~" + cls.name;
		dtor_entry["comment"]           = "";
		dtor_entry["tags"]              = nlohmann::json::object();
		dtor_entry["instance_required"] = true;

		auto ep = CppEntityPathGenerator::destructor(cls.mangled_dtor, cls.ns);
		dtor_entry["entity_path"]    = entity_path_to_json(ep);
		dtor_entry["parameters"]     = nlohmann::json::array();
		dtor_entry["return_values"]  = nlohmann::json::array();

		class_entry["release"] = dtor_entry;
	}
	else
	{
		class_entry["release"] = nullptr;
	}

	// Instance methods and static methods
	nlohmann::json methods = nlohmann::json::array();

	// Count overloads for instance methods
	std::map<std::string, int> method_overload_count;
	for (const auto& m : cls.methods)
	{
		method_overload_count[m.name]++;
	}

	std::map<std::string, int> method_index;
	for (const auto& m : cls.methods)
	{
		nlohmann::json mentry;
		mentry["name"]              = m.name;
		mentry["comment"]           = "";
		mentry["tags"]              = nlohmann::json::object();
		mentry["instance_required"] = true;

		auto ep = CppEntityPathGenerator::instance_method(m);
		mentry["entity_path"] = entity_path_to_json(ep);

		nlohmann::json params = nlohmann::json::array();
		for (const auto& p : m.params)
		{
			params.push_back(param_to_json(p));
		}
		mentry["parameters"] = params;

		nlohmann::json retvals = nlohmann::json::array();
		for (const auto& rt : m.return_types)
		{
			retvals.push_back(return_type_to_json(rt));
		}
		mentry["return_values"] = retvals;

		int oi = method_overload_count[m.name] > 1 ? method_index[m.name]++ : 0;
		mentry["overload_index"] = oi;

		methods.push_back(mentry);
	}

	// Static methods are treated as free functions with instance_required=false
	for (const auto& sm : cls.static_methods)
	{
		CppFunctionInfo free_fn = sm;
		free_fn.ns = cls.ns;

		nlohmann::json smentry;
		smentry["name"]              = sm.name;
		smentry["comment"]           = "";
		smentry["tags"]              = nlohmann::json::object();
		smentry["instance_required"] = false;

		auto ep = CppEntityPathGenerator::free_function(free_fn);
		smentry["entity_path"] = entity_path_to_json(ep);

		nlohmann::json params = nlohmann::json::array();
		for (const auto& p : sm.params)
		{
			params.push_back(param_to_json(p));
		}
		smentry["parameters"] = params;

		nlohmann::json retvals = nlohmann::json::array();
		for (const auto& rt : sm.return_types)
		{
			retvals.push_back(return_type_to_json(rt));
		}
		smentry["return_values"]  = retvals;
		smentry["overload_index"] = 0;

		methods.push_back(smentry);
	}

	class_entry["methods"] = methods;

	// Fields
	nlohmann::json fields = nlohmann::json::array();
	for (const auto& f : cls.fields)
	{
		auto [metaffi_type, dims] = CppTypeMapper::map(f.type_str);

		nlohmann::json field_entry;
		field_entry["name"]       = f.name;
		field_entry["type"]       = metaffi_type.empty() ? "handle" : metaffi_type;
		field_entry["type_alias"] = f.type_str;
		field_entry["comment"]    = "";
		field_entry["tags"]       = nlohmann::json::object();
		field_entry["dimensions"] = dims;

		// Getter
		{
			nlohmann::json getter;
			getter["name"]              = f.name;
			getter["comment"]           = "";
			getter["tags"]              = nlohmann::json::object();
			getter["instance_required"] = true;

			auto ep = CppEntityPathGenerator::field_getter(f, cls.ns);
			getter["entity_path"]   = entity_path_to_json(ep);
			getter["parameters"]    = nlohmann::json::array();
			getter["return_values"] = nlohmann::json::array({
				empty_arg("return_value", metaffi_type.empty() ? "handle" : metaffi_type, f.type_str, dims)
			});
			getter["overload_index"] = 0;

			field_entry["getter"] = getter;
		}

		// Setter
		{
			nlohmann::json setter;
			setter["name"]              = f.name;
			setter["comment"]           = "";
			setter["tags"]              = nlohmann::json::object();
			setter["instance_required"] = true;

			auto ep = CppEntityPathGenerator::field_setter(f, cls.ns);
			setter["entity_path"] = entity_path_to_json(ep);
			setter["parameters"]  = nlohmann::json::array({
				empty_arg("value", metaffi_type.empty() ? "handle" : metaffi_type, f.type_str, dims)
			});
			setter["return_values"]  = nlohmann::json::array();
			setter["overload_index"] = 0;

			field_entry["setter"] = setter;
		}

		fields.push_back(field_entry);
	}

	class_entry["fields"] = fields;

	module["classes"].push_back(class_entry);
}

void CppIDLGenerator::build_global_entries(const CppGlobalInfo& g, nlohmann::json& module)
{
	auto [metaffi_type, dims] = CppTypeMapper::map(g.type_str);
	const std::string mt = metaffi_type.empty() ? "handle" : metaffi_type;

	nlohmann::json global_entry;
	global_entry["name"]       = g.name;
	global_entry["type"]       = mt;
	global_entry["type_alias"] = g.type_str;
	global_entry["comment"]    = "";
	global_entry["tags"]       = nlohmann::json::object();
	global_entry["dimensions"] = dims;

	// Getter function
	{
		nlohmann::json getter;
		getter["name"]    = g.name + "_getter";
		getter["comment"] = "";
		getter["tags"]    = nlohmann::json::object();

		auto ep = CppEntityPathGenerator::global_getter(g);
		getter["entity_path"]    = entity_path_to_json(ep);
		getter["parameters"]     = nlohmann::json::array();
		getter["return_values"]  = nlohmann::json::array({
			empty_arg("return_value", mt, g.type_str, dims)
		});
		getter["overload_index"] = 0;

		global_entry["getter"] = getter;
	}

	// Setter function
	{
		nlohmann::json setter;
		setter["name"]    = g.name + "_setter";
		setter["comment"] = "";
		setter["tags"]    = nlohmann::json::object();

		auto ep = CppEntityPathGenerator::global_setter(g);
		setter["entity_path"]   = entity_path_to_json(ep);
		setter["parameters"]    = nlohmann::json::array({
			empty_arg("value", mt, g.type_str, dims)
		});
		setter["return_values"]  = nlohmann::json::array();
		setter["overload_index"] = 0;

		global_entry["setter"] = setter;
	}

	module["globals"].push_back(global_entry);
}

nlohmann::json CppIDLGenerator::generate()
{
	std::string stem = header_stem();

	// Build normalised path components without relying on filesystem::path::stem()
	// which can behave unexpectedly with mixed separators on Windows.
	std::string norm_path = m_info.header_path;
	for (char& c : norm_path) { if (c == '\\') c = '/'; }

	// Extract extension and filename-with-extension via string ops
	std::size_t slash  = norm_path.rfind('/');
	std::string fname  = (slash == std::string::npos) ? norm_path : norm_path.substr(slash + 1);
	std::size_t dot    = fname.rfind('.');
	std::string ext    = (dot != std::string::npos) ? fname.substr(dot) : "";

	std::string abs_path;
	try
	{
		abs_path = std::filesystem::absolute(m_info.header_path).string();
		for (char& c : abs_path) { if (c == '\\') c = '/'; }
	}
	catch (...)
	{
		abs_path = norm_path;
	}

	nlohmann::json root;
	root["idl_source"]                  = stem;
	root["idl_extension"]               = ext;
	root["idl_filename_with_extension"] = fname;
	root["idl_full_path"]               = abs_path;
	root["metaffi_guest_lib"]           = stem;
	root["target_language"]             = "cpp";

	// Collect all namespace names (plus "" for global)
	std::set<std::string> namespaces;
	namespaces.insert("");  // global namespace always present

	for (const auto& fn : m_info.functions) namespaces.insert(fn.ns);
	for (const auto& g  : m_info.globals)   namespaces.insert(g.ns);
	for (const auto& cls : m_info.classes)  namespaces.insert(cls.ns);

	nlohmann::json modules = nlohmann::json::array();

	for (const std::string& ns : namespaces)
	{
		// Module name: "" → header stem, otherwise → namespace path
		std::string module_name = ns.empty() ? stem : ns;

		// Collect entities that belong to this namespace
		CppModuleInfo grouped;
		grouped.header_path = m_info.header_path;

		for (const auto& fn : m_info.functions)
		{
			if (fn.ns == ns) grouped.functions.push_back(fn);
		}
		for (const auto& g : m_info.globals)
		{
			if (g.ns == ns) grouped.globals.push_back(g);
		}
		for (const auto& cls : m_info.classes)
		{
			if (cls.ns == ns) grouped.classes.push_back(cls);
		}

		// Skip completely empty modules (e.g. no entities in global namespace)
		if (grouped.functions.empty() && grouped.globals.empty() && grouped.classes.empty())
		{
			continue;
		}

		nlohmann::json module_json = build_module(module_name, grouped);
		modules.push_back(module_json);
	}

	root["modules"] = modules;
	return root;
}

nlohmann::json CppIDLGenerator::build_module(const std::string& module_name,
                                              const CppModuleInfo& grouped)
{
	nlohmann::json module;
	module["name"]               = module_name;
	module["comment"]            = "";
	module["tags"]               = nlohmann::json::object();
	module["functions"]          = nlohmann::json::array();
	module["classes"]            = nlohmann::json::array();
	module["globals"]            = nlohmann::json::array();
	module["external_resources"] = nlohmann::json::array();

	// Count overloads for free functions
	std::map<std::string, int> fn_overload_count;
	for (const auto& fn : grouped.functions)
	{
		fn_overload_count[fn.name]++;
	}

	std::map<std::string, int> fn_index;
	for (const auto& fn : grouped.functions)
	{
		int oi = fn_overload_count[fn.name] > 1 ? fn_index[fn.name]++ : 0;
		module["functions"].push_back(build_function_entry(fn, oi));
	}

	// Globals
	for (const auto& g : grouped.globals)
	{
		build_global_entries(g, module);
	}

	// Classes
	for (const auto& cls : grouped.classes)
	{
		build_class_entries(cls, module);
	}

	return module;
}
