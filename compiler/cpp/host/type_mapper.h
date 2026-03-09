#pragma once

#include <string>
#include <stdexcept>

namespace metaffi::compiler::cpp {

/**
 * Maps MetaFFI IDL type strings and dimensions to C++ type names
 * and MetaFFI runtime type constant names.
 *
 * Scalar types (dimensions == 0) → C++ primitive or std::string.
 * Array types  (dimensions >  0) → std::vector<T>.
 */
class CppTypeMapper {
public:
    // Returns the C++ type string for the given IDL base type and dimensions.
    // e.g. ("int32", 0) → "int32_t", ("int32", 1) → "std::vector<int32_t>"
    static std::string cpp_type(const std::string& idl_type, int dimensions);

    // Returns the MetaFFI runtime constant name for the given IDL base type.
    // e.g. "int32" → "metaffi_int32_type"
    static std::string metaffi_constant(const std::string& idl_type);

    // Returns a MetaFFITypeInfo initialiser literal: "{metaffi_int32_type, 0}"
    // dimensions is the array depth stored in the IDL.
    static std::string type_info_literal(const std::string& idl_type, int dimensions);

private:
    // Returns C++ scalar type for a base IDL type (no _array suffix, no dimensions).
    static std::string scalar_cpp_type(const std::string& base_type);

    // Strips trailing "_array" suffix if present, returning the base type name.
    static std::string strip_array_suffix(const std::string& idl_type);
};

} // namespace metaffi::compiler::cpp
