#include "type_mapper.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <regex>


// ============================================================================
// Type mapping table
// ============================================================================

namespace
{
	// Scalar type table: normalised C++ type → MetaFFI type name
	const std::map<std::string, std::string> scalar_types = {
		{"char",                "char8"},
		{"signed char",         "int8"},
		{"unsigned char",       "uint8"},
		{"short",               "int16"},
		{"short int",           "int16"},
		{"signed short",        "int16"},
		{"signed short int",    "int16"},
		{"unsigned short",      "uint16"},
		{"unsigned short int",  "uint16"},
		{"int",                 "int32"},
		{"signed",              "int32"},
		{"signed int",          "int32"},
		{"unsigned",            "uint32"},
		{"unsigned int",        "uint32"},
		{"long",                "int64"},
		{"long int",            "int64"},
		{"signed long",         "int64"},
		{"signed long int",     "int64"},
		{"long long",           "int64"},
		{"long long int",       "int64"},
		{"signed long long",    "int64"},
		{"signed long long int","int64"},
		{"unsigned long",       "uint64"},
		{"unsigned long int",   "uint64"},
		{"unsigned long long",  "uint64"},
		{"unsigned long long int","uint64"},
		{"float",               "float32"},
		{"double",              "float64"},
		{"long double",         "float64"},
		{"bool",                "bool"},
		{"size_t",              "size"},
		{"std::size_t",         "size"},
		{"void",                ""},        // void return → no return type
		{"void*",               "handle"},
		{"void *",              "handle"},
		{"char*",               "string8"},
		{"char *",              "string8"},
		{"const char*",         "string8"},
		{"const char *",        "string8"},
	};
} // anonymous namespace


// ============================================================================
// CppTypeMapper::normalise
// ============================================================================

std::string CppTypeMapper::normalise(const std::string& raw_type)
{
	// Strip leading/trailing whitespace
	auto begin = raw_type.begin();
	auto end   = raw_type.end();

	while (begin != end && std::isspace(static_cast<unsigned char>(*begin))) ++begin;
	while (begin != end && std::isspace(static_cast<unsigned char>(*std::prev(end)))) --end;

	std::string result(begin, end);

	// Collapse multiple consecutive spaces into one
	auto new_end = std::unique(result.begin(), result.end(), [](char a, char b)
	{
		return std::isspace(static_cast<unsigned char>(a)) &&
		       std::isspace(static_cast<unsigned char>(b));
	});
	result.erase(new_end, result.end());

	// Replace any remaining whitespace-chars with plain spaces
	std::replace_if(result.begin(), result.end(),
	                [](char c) { return std::isspace(static_cast<unsigned char>(c)); },
	                ' ');

	return result;
}


// ============================================================================
// CppTypeMapper::map
// ============================================================================

std::pair<std::string, int> CppTypeMapper::map(const std::string& cpp_type)
{
	std::string norm = normalise(cpp_type);

	// Direct scalar lookup (handles "char*", "const char*", "void*" etc.)
	auto it = scalar_types.find(norm);
	if (it != scalar_types.end())
	{
		const std::string& metaffi = it->second;
		if (metaffi.empty())
		{
			// void → no return type (caller treats empty as void)
			return {"", 0};
		}
		return {metaffi, 0};
	}

	// Strip "const" qualifiers for lookup
	std::string stripped = std::regex_replace(norm, std::regex(R"(\bconst\b)"), "");
	stripped = normalise(stripped);

	it = scalar_types.find(stripped);
	if (it != scalar_types.end())
	{
		const std::string& metaffi = it->second;
		if (metaffi.empty()) return {"", 0};
		return {metaffi, 0};
	}

	// Pointer type: T* or T[]
	// Check for trailing '*' or '[]' → array dimension
	bool is_ptr   = !stripped.empty() && stripped.back() == '*';
	bool is_array = stripped.size() >= 2 &&
	                stripped[stripped.size()-1] == ']' &&
	                stripped[stripped.size()-2] == '[';

	if (is_ptr || is_array)
	{
		// Strip the pointer/array suffix to get the base type
		std::string base = is_ptr ? stripped.substr(0, stripped.size() - 1)
		                          : stripped.substr(0, stripped.size() - 2);
		base = normalise(base);

		// char* / const char* are already handled above as string8; here we're
		// dealing with other pointer types.
		auto base_it = scalar_types.find(base);
		if (base_it != scalar_types.end() && !base_it->second.empty())
		{
			return {base_it->second, 1};
		}

		// Strip const from base and retry
		std::string base_stripped = normalise(std::regex_replace(base, std::regex(R"(\bconst\b)"), ""));
		auto base_it2 = scalar_types.find(base_stripped);
		if (base_it2 != scalar_types.end() && !base_it2->second.empty())
		{
			return {base_it2->second, 1};
		}

		// Unknown base type pointer → handle array
		return {"handle", 1};
	}

	// Unknown type (custom class/struct, template, etc.) → handle
	return {"handle", 0};
}
