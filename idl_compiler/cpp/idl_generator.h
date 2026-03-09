#pragma once

#include "extractor.h"
#include <nlohmann/json.hpp>
#include <string>


/**
 * CppIDLGenerator
 *
 * Converts a CppModuleInfo (produced by CppExtractor) into a MetaFFI IDL JSON
 * document following the schema at sdk/idl_entities/idl.schema.json.
 *
 * Grouping strategy:
 *   - Entities in the global namespace → module named after the header file stem.
 *   - Entities in namespace "myns"     → module named "myns".
 *   - Entities in namespace "a::b"     → module named "a::b".
 *
 * Overloads:
 *   Multiple functions with the same name in the same module receive sequential
 *   overload_index values starting from 0.
 */
class CppIDLGenerator
{
public:
	/**
	 * @param info  Extracted C++ interface information.
	 */
	explicit CppIDLGenerator(const CppModuleInfo& info);

	/**
	 * Generate and return the MetaFFI IDL as a nlohmann::json object.
	 */
	nlohmann::json generate();

private:
	CppModuleInfo m_info;

	// Derive the stem name (without directory or extension) from the header path.
	std::string header_stem() const;

	// Build one JSON module entry for a given namespace group.
	nlohmann::json build_module(const std::string& module_name,
	                            const CppModuleInfo& grouped_info);

	// Build a function entry (free function, static method).
	nlohmann::json build_function_entry(const CppFunctionInfo& fn, int overload_index);

	// Build class entries (constructor, destructor, methods, fields).
	void build_class_entries(const CppClassInfo& cls, nlohmann::json& module);

	// Build a global variable getter + setter pair.
	void build_global_entries(const CppGlobalInfo& g, nlohmann::json& module);

	// Map a parameter to a MetaFFI type entry.
	nlohmann::json param_to_json(const CppParamInfo& p);

	// Map a return type string to a MetaFFI type entry.
	nlohmann::json return_type_to_json(const std::string& type_str);
};
