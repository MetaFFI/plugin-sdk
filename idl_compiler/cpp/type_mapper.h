#pragma once

#include <string>
#include <utility>

/**
 * CppTypeMapper
 *
 * Maps C/C++ type strings to MetaFFI type name + dimension count.
 * Rules follow the plan's type-mapping table.
 *
 * Usage:
 *   auto [metaffi_type, dims] = CppTypeMapper::map("int");
 *   // → ("int32", 0)
 *
 *   auto [type2, dims2] = CppTypeMapper::map("int*");
 *   // → ("int32", 1)   (pointer to non-char/non-void type → array)
 */
class CppTypeMapper
{
public:
	/**
	 * Map a C/C++ type string to a MetaFFI type name and dimension count.
	 *
	 * @param cpp_type  Normalised C/C++ type string (e.g. "int", "const char*", "double[]")
	 * @return          Pair of (metaffi_type_name, dimensions).
	 *                  dimensions = 0 for scalar, 1 for pointer/array.
	 *                  Falls back to ("handle", 0) for unknown types.
	 */
	static std::pair<std::string, int> map(const std::string& cpp_type);

	/**
	 * Normalise a raw C/C++ type string before mapping:
	 *  - strips leading/trailing whitespace
	 *  - collapses multiple spaces
	 *  - normalises "unsigned X" shorthand
	 */
	static std::string normalise(const std::string& raw_type);

private:
	// No instances needed — all methods are static.
	CppTypeMapper() = delete;
};
