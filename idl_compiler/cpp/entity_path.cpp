#include "entity_path.h"
#include "extractor.h"

#include <sstream>


// ============================================================================
// CppEntityPathGenerator
// ============================================================================

std::map<std::string, std::string>
CppEntityPathGenerator::free_function(const CppFunctionInfo& fn)
{
	std::map<std::string, std::string> path;
	path["callable"] = fn.mangled_name;
	if (!fn.ns.empty())
	{
		path["namespace"] = fn.ns;
	}
	return path;
}

std::map<std::string, std::string>
CppEntityPathGenerator::instance_method(const CppFunctionInfo& fn)
{
	std::map<std::string, std::string> path;
	path["callable"]          = fn.mangled_name;
	path["instance_required"] = "true";
	if (!fn.ns.empty())
	{
		path["namespace"] = fn.ns;
	}
	return path;
}

std::map<std::string, std::string>
CppEntityPathGenerator::constructor(const std::string& mangled_ctor,
                                    std::size_t        class_size,
                                    const std::string& ns)
{
	std::map<std::string, std::string> path;
	path["callable"]    = mangled_ctor;
	path["constructor"] = "true";
	path["class_size"]  = std::to_string(class_size);
	if (!ns.empty())
	{
		path["namespace"] = ns;
	}
	return path;
}

std::map<std::string, std::string>
CppEntityPathGenerator::destructor(const std::string& mangled_dtor,
                                   const std::string& ns)
{
	std::map<std::string, std::string> path;
	path["callable"]   = mangled_dtor;
	path["destructor"] = "true";
	if (!ns.empty())
	{
		path["namespace"] = ns;
	}
	return path;
}

std::map<std::string, std::string>
CppEntityPathGenerator::global_getter(const CppGlobalInfo& g)
{
	std::map<std::string, std::string> path;
	path["global"] = g.mangled_name;
	path["getter"] = "true";
	if (!g.ns.empty())
	{
		path["namespace"] = g.ns;
	}
	return path;
}

std::map<std::string, std::string>
CppEntityPathGenerator::global_setter(const CppGlobalInfo& g)
{
	std::map<std::string, std::string> path;
	path["global"] = g.mangled_name;
	path["setter"] = "true";
	if (!g.ns.empty())
	{
		path["namespace"] = g.ns;
	}
	return path;
}

std::map<std::string, std::string>
CppEntityPathGenerator::field_getter(const CppFieldInfo& field, const std::string& ns)
{
	std::map<std::string, std::string> path;
	path["field"]             = field.name;
	path["getter"]            = "true";
	path["instance_required"] = "true";
	path["field_offset"]      = std::to_string(field.offset_bytes);
	if (!ns.empty())
	{
		path["namespace"] = ns;
	}
	return path;
}

std::map<std::string, std::string>
CppEntityPathGenerator::field_setter(const CppFieldInfo& field, const std::string& ns)
{
	std::map<std::string, std::string> path;
	path["field"]             = field.name;
	path["setter"]            = "true";
	path["instance_required"] = "true";
	path["field_offset"]      = std::to_string(field.offset_bytes);
	path["field_size"]        = std::to_string(field.size_bytes);
	if (!ns.empty())
	{
		path["namespace"] = ns;
	}
	return path;
}

std::string
CppEntityPathGenerator::serialise(const std::map<std::string, std::string>& path_map)
{
	std::ostringstream oss;
	bool first = true;

	for (const auto& [key, value] : path_map)
	{
		if (value.empty()) continue;

		if (!first) oss << ',';
		oss << key << '=' << value;
		first = false;
	}

	return oss.str();
}
