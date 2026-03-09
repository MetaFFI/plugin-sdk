#include "type_mapper.h"
#include <unordered_map>
#include <stdexcept>

namespace metaffi::compiler::cpp {

namespace {

// Static lookup tables for IDL base type → C++ scalar type and MetaFFI constant.
struct TypeEntry {
    const char* cpp_scalar;      // e.g. "int32_t"
    const char* metaffi_const;   // e.g. "metaffi_int32_type"
};

const std::unordered_map<std::string, TypeEntry> kTypeTable = {
    // --- Scalar types ---
    {"int8",    {"int8_t",         "metaffi_int8_type"}},
    {"int16",   {"int16_t",        "metaffi_int16_type"}},
    {"int32",   {"int32_t",        "metaffi_int32_type"}},
    {"int64",   {"int64_t",        "metaffi_int64_type"}},
    {"uint8",   {"uint8_t",        "metaffi_uint8_type"}},
    {"uint16",  {"uint16_t",       "metaffi_uint16_type"}},
    {"uint32",  {"uint32_t",       "metaffi_uint32_type"}},
    {"uint64",  {"uint64_t",       "metaffi_uint64_type"}},
    {"float32", {"float",          "metaffi_float32_type"}},
    {"float64", {"double",         "metaffi_float64_type"}},
    {"bool",    {"bool",           "metaffi_bool_type"}},
    {"string8", {"std::string",    "metaffi_string8_type"}},
    {"string16",{"std::u16string", "metaffi_string16_type"}},
    {"string32",{"std::u32string", "metaffi_string32_type"}},
    {"char8",   {"char",           "metaffi_char8_type"}},
    {"char16",  {"char16_t",       "metaffi_char16_type"}},
    {"char32",  {"char32_t",       "metaffi_char32_type"}},
    {"handle",  {"metaffi_handle", "metaffi_handle_type"}},
    {"size",    {"uint64_t",       "metaffi_size_type"}},
    {"any",     {"void*",          "metaffi_any_type"}},
    {"null",    {"void*",          "metaffi_null_type"}},

    {"callable",{"cdt_metaffi_callable*", "metaffi_callable_type"}},

    // --- Non-packed array types (idl type = base + "_array") ---
    {"int8_array",    {"int8_t",         "metaffi_int8_array_type"}},
    {"int16_array",   {"int16_t",        "metaffi_int16_array_type"}},
    {"int32_array",   {"int32_t",        "metaffi_int32_array_type"}},
    {"int64_array",   {"int64_t",        "metaffi_int64_array_type"}},
    {"uint8_array",   {"uint8_t",        "metaffi_uint8_array_type"}},
    {"uint16_array",  {"uint16_t",       "metaffi_uint16_array_type"}},
    {"uint32_array",  {"uint32_t",       "metaffi_uint32_array_type"}},
    {"uint64_array",  {"uint64_t",       "metaffi_uint64_array_type"}},
    {"float32_array", {"float",          "metaffi_float32_array_type"}},
    {"float64_array", {"double",         "metaffi_float64_array_type"}},
    {"bool_array",    {"bool",           "metaffi_bool_array_type"}},
    {"string8_array", {"std::string",    "metaffi_string8_array_type"}},
    {"char8_array",   {"char",           "metaffi_char8_array_type"}},
    {"char16_array",  {"char16_t",       "metaffi_char16_array_type"}},
    {"char32_array",  {"char32_t",       "metaffi_char32_array_type"}},
    {"handle_array",  {"metaffi_handle", "metaffi_handle_array_type"}},
    {"size_array",    {"uint64_t",       "metaffi_size_array_type"}},

    // --- Packed array base types (strip_array_suffix leaves these after removing "_array") ---
    {"int8_packed",    {"int8_t",         "metaffi_int8_packed_array_type"}},
    {"int16_packed",   {"int16_t",        "metaffi_int16_packed_array_type"}},
    {"int32_packed",   {"int32_t",        "metaffi_int32_packed_array_type"}},
    {"int64_packed",   {"int64_t",        "metaffi_int64_packed_array_type"}},
    {"uint8_packed",   {"uint8_t",        "metaffi_uint8_packed_array_type"}},
    {"uint16_packed",  {"uint16_t",       "metaffi_uint16_packed_array_type"}},
    {"uint32_packed",  {"uint32_t",       "metaffi_uint32_packed_array_type"}},
    {"uint64_packed",  {"uint64_t",       "metaffi_uint64_packed_array_type"}},
    {"float32_packed", {"float",          "metaffi_float32_packed_array_type"}},
    {"float64_packed", {"double",         "metaffi_float64_packed_array_type"}},
    {"bool_packed",    {"bool",           "metaffi_bool_packed_array_type"}},
    {"string8_packed", {"std::string",    "metaffi_string8_packed_array_type"}},
    {"char8_packed",   {"char",           "metaffi_char8_packed_array_type"}},
    {"char16_packed",  {"char16_t",       "metaffi_char16_packed_array_type"}},
    {"char32_packed",  {"char32_t",       "metaffi_char32_packed_array_type"}},
    {"handle_packed",  {"metaffi_handle", "metaffi_handle_packed_array_type"}},
    {"size_packed",    {"uint64_t",       "metaffi_size_packed_array_type"}},
};

} // namespace

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

std::string CppTypeMapper::strip_array_suffix(const std::string& idl_type) {
    const std::string suffix = "_array";
    if (idl_type.size() > suffix.size() &&
        idl_type.compare(idl_type.size() - suffix.size(), suffix.size(), suffix) == 0) {
        return idl_type.substr(0, idl_type.size() - suffix.size());
    }
    return idl_type;
}

std::string CppTypeMapper::scalar_cpp_type(const std::string& base_type) {
    auto it = kTypeTable.find(base_type);
    if (it == kTypeTable.end()) {
        throw std::runtime_error("CppTypeMapper: unknown MetaFFI type '" + base_type + "'");
    }
    return it->second.cpp_scalar;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::string CppTypeMapper::cpp_type(const std::string& idl_type, int dimensions) {
    // Strip _array suffix to get the base type
    const std::string base = strip_array_suffix(idl_type);
    const std::string scalar = scalar_cpp_type(base);

    // An _array suffix with dim=0 implies at least one dimension of wrapping
    const bool has_array_suffix = (base != idl_type);
    const int effective_dims = (has_array_suffix && dimensions == 0) ? 1 : dimensions;

    // Wrap in std::vector<> for each dimension
    if (effective_dims > 0) {
        std::string result = scalar;
        for (int d = 0; d < effective_dims; ++d) {
            result = "std::vector<" + result + ">";
        }
        return result;
    }

    return scalar;
}

std::string CppTypeMapper::metaffi_constant(const std::string& idl_type) {
    // Try the full type first (handles int64_array, int64_packed_array, etc.)
    {
        auto it = kTypeTable.find(idl_type);
        if (it != kTypeTable.end()) {
            return it->second.metaffi_const;
        }
    }

    // Fall back: strip _array suffix and look up the base scalar/packed entry
    const std::string base = strip_array_suffix(idl_type);
    auto it = kTypeTable.find(base);
    if (it == kTypeTable.end()) {
        throw std::runtime_error("CppTypeMapper: unknown MetaFFI type '" + idl_type + "'");
    }

    return it->second.metaffi_const;
}

std::string CppTypeMapper::type_info_literal(const std::string& idl_type, int dimensions) {
    // Determine effective dimensions: IDL dimension field or _array suffix implies dim ≥ 1
    const std::string base = strip_array_suffix(idl_type);
    bool has_array_suffix = (base != idl_type);

    int effective_dims = dimensions;
    if (has_array_suffix && effective_dims == 0) {
        effective_dims = 1;
    }

    const std::string constant = metaffi_constant(idl_type);

    // Use explicit constructor calls to avoid brace-init ambiguity:
    // {type, N} in a vector initializer creates N+1 elements instead of one.
    if (effective_dims > 0) {
        return "metaffi_type_info(" + constant + ", nullptr, false, " + std::to_string(effective_dims) + ")";
    }
    return "metaffi_type_info(" + constant + ")";
}

} // namespace metaffi::compiler::cpp
