#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

namespace metaffi::idl {

/**
 * Custom exception for IDL-related errors
 * Follows fail-fast policy with detailed error information
 */
class IDLException : public std::runtime_error {
	std::string error_code_;
	std::string context_;

public:
	IDLException(const std::string& error_code, const std::string& message, const std::string& context = "")
		: std::runtime_error(error_code + ": " + message + (context.empty() ? "" : " [Context: " + context + "]"))
		, error_code_(error_code)
		, context_(context)
	{}

	[[nodiscard]] const std::string& error_code() const noexcept { return error_code_; }
	[[nodiscard]] const std::string& context() const noexcept { return context_; }
};

/**
 * MetaFFI type enumeration
 * Aligned with sdk/runtime/metaffi_primitives.h types
 * Used in IDL JSON files as lowercase strings
 */
enum class MetaFFIType {
	// Scalar types
	FLOAT64,
	FLOAT32,
	INT8,
	INT16,
	INT32,
	INT64,
	UINT8,
	UINT16,
	UINT32,
	UINT64,
	BOOL,

	// Character types
	CHAR8,
	CHAR16,
	CHAR32,

	// String types
	STRING8,
	STRING16,
	STRING32,

	// Special types
	HANDLE,
	ARRAY,
	ANY,
	SIZE,
	NULL_TYPE,
	CALLABLE,

	// Array types (scalar + array)
	FLOAT64_ARRAY,
	FLOAT32_ARRAY,
	INT8_ARRAY,
	INT16_ARRAY,
	INT32_ARRAY,
	INT64_ARRAY,
	UINT8_ARRAY,
	UINT16_ARRAY,
	UINT32_ARRAY,
	UINT64_ARRAY,
	BOOL_ARRAY,
	CHAR8_ARRAY,
	CHAR16_ARRAY,
	CHAR32_ARRAY,
	STRING8_ARRAY,
	STRING16_ARRAY,
	STRING32_ARRAY,
	HANDLE_ARRAY,
	ANY_ARRAY,
	SIZE_ARRAY
};

/**
 * Convert MetaFFIType enum to lowercase string for JSON serialization
 * @param type The MetaFFI type enum value
 * @return Lowercase string representation (e.g., "int64", "float32_array")
 */
[[nodiscard]] inline std::string to_string(MetaFFIType type) {
	static const std::unordered_map<MetaFFIType, std::string> type_map = {
		{MetaFFIType::FLOAT64, "float64"},
		{MetaFFIType::FLOAT32, "float32"},
		{MetaFFIType::INT8, "int8"},
		{MetaFFIType::INT16, "int16"},
		{MetaFFIType::INT32, "int32"},
		{MetaFFIType::INT64, "int64"},
		{MetaFFIType::UINT8, "uint8"},
		{MetaFFIType::UINT16, "uint16"},
		{MetaFFIType::UINT32, "uint32"},
		{MetaFFIType::UINT64, "uint64"},
		{MetaFFIType::BOOL, "bool"},
		{MetaFFIType::CHAR8, "char8"},
		{MetaFFIType::CHAR16, "char16"},
		{MetaFFIType::CHAR32, "char32"},
		{MetaFFIType::STRING8, "string8"},
		{MetaFFIType::STRING16, "string16"},
		{MetaFFIType::STRING32, "string32"},
		{MetaFFIType::HANDLE, "handle"},
		{MetaFFIType::ARRAY, "array"},
		{MetaFFIType::ANY, "any"},
		{MetaFFIType::SIZE, "size"},
		{MetaFFIType::NULL_TYPE, "null"},
		{MetaFFIType::CALLABLE, "callable"},
		{MetaFFIType::FLOAT64_ARRAY, "float64_array"},
		{MetaFFIType::FLOAT32_ARRAY, "float32_array"},
		{MetaFFIType::INT8_ARRAY, "int8_array"},
		{MetaFFIType::INT16_ARRAY, "int16_array"},
		{MetaFFIType::INT32_ARRAY, "int32_array"},
		{MetaFFIType::INT64_ARRAY, "int64_array"},
		{MetaFFIType::UINT8_ARRAY, "uint8_array"},
		{MetaFFIType::UINT16_ARRAY, "uint16_array"},
		{MetaFFIType::UINT32_ARRAY, "uint32_array"},
		{MetaFFIType::UINT64_ARRAY, "uint64_array"},
		{MetaFFIType::BOOL_ARRAY, "bool_array"},
		{MetaFFIType::CHAR8_ARRAY, "char8_array"},
		{MetaFFIType::CHAR16_ARRAY, "char16_array"},
		{MetaFFIType::CHAR32_ARRAY, "char32_array"},
		{MetaFFIType::STRING8_ARRAY, "string8_array"},
		{MetaFFIType::STRING16_ARRAY, "string16_array"},
		{MetaFFIType::STRING32_ARRAY, "string32_array"},
		{MetaFFIType::HANDLE_ARRAY, "handle_array"},
		{MetaFFIType::ANY_ARRAY, "any_array"},
		{MetaFFIType::SIZE_ARRAY, "size_array"}
	};

	auto it = type_map.find(type);
	if (it == type_map.end()) {
		throw IDLException("INVALID_TYPE_ENUM", "Unknown MetaFFI type enum value");
	}
	return it->second;
}

/**
 * Convert lowercase string to MetaFFIType enum from JSON deserialization
 * @param str Lowercase string representation (e.g., "int64", "float32_array")
 * @return Corresponding MetaFFIType enum value
 * @throws IDLException if string is not a valid MetaFFI type
 */
[[nodiscard]] inline MetaFFIType from_string(const std::string& str) {
	static const std::unordered_map<std::string, MetaFFIType> string_map = {
		{"float64", MetaFFIType::FLOAT64},
		{"float32", MetaFFIType::FLOAT32},
		{"int8", MetaFFIType::INT8},
		{"int16", MetaFFIType::INT16},
		{"int32", MetaFFIType::INT32},
		{"int64", MetaFFIType::INT64},
		{"uint8", MetaFFIType::UINT8},
		{"uint16", MetaFFIType::UINT16},
		{"uint32", MetaFFIType::UINT32},
		{"uint64", MetaFFIType::UINT64},
		{"bool", MetaFFIType::BOOL},
		{"char8", MetaFFIType::CHAR8},
		{"char16", MetaFFIType::CHAR16},
		{"char32", MetaFFIType::CHAR32},
		{"string8", MetaFFIType::STRING8},
		{"string16", MetaFFIType::STRING16},
		{"string32", MetaFFIType::STRING32},
		{"handle", MetaFFIType::HANDLE},
		{"array", MetaFFIType::ARRAY},
		{"any", MetaFFIType::ANY},
		{"size", MetaFFIType::SIZE},
		{"null", MetaFFIType::NULL_TYPE},
		{"callable", MetaFFIType::CALLABLE},
		{"float64_array", MetaFFIType::FLOAT64_ARRAY},
		{"float32_array", MetaFFIType::FLOAT32_ARRAY},
		{"int8_array", MetaFFIType::INT8_ARRAY},
		{"int16_array", MetaFFIType::INT16_ARRAY},
		{"int32_array", MetaFFIType::INT32_ARRAY},
		{"int64_array", MetaFFIType::INT64_ARRAY},
		{"uint8_array", MetaFFIType::UINT8_ARRAY},
		{"uint16_array", MetaFFIType::UINT16_ARRAY},
		{"uint32_array", MetaFFIType::UINT32_ARRAY},
		{"uint64_array", MetaFFIType::UINT64_ARRAY},
		{"bool_array", MetaFFIType::BOOL_ARRAY},
		{"char8_array", MetaFFIType::CHAR8_ARRAY},
		{"char16_array", MetaFFIType::CHAR16_ARRAY},
		{"char32_array", MetaFFIType::CHAR32_ARRAY},
		{"string8_array", MetaFFIType::STRING8_ARRAY},
		{"string16_array", MetaFFIType::STRING16_ARRAY},
		{"string32_array", MetaFFIType::STRING32_ARRAY},
		{"handle_array", MetaFFIType::HANDLE_ARRAY},
		{"any_array", MetaFFIType::ANY_ARRAY},
		{"size_array", MetaFFIType::SIZE_ARRAY}
	};

	auto it = string_map.find(str);
	if (it == string_map.end()) {
		throw IDLException("INVALID_TYPE_STRING", "Unknown MetaFFI type string: '" + str + "'");
	}
	return it->second;
}

/**
 * Check if a MetaFFI type is an array type
 * @param type The MetaFFI type to check
 * @return true if the type is an array type, false otherwise
 */
[[nodiscard]] inline bool is_array_type(MetaFFIType type) {
	return type >= MetaFFIType::FLOAT64_ARRAY && type <= MetaFFIType::SIZE_ARRAY;
}

/**
 * Check if a MetaFFI type is a handle or handle array
 * @param type The MetaFFI type to check
 * @return true if the type is handle or handle_array, false otherwise
 */
[[nodiscard]] inline bool is_handle_type(MetaFFIType type) {
	return type == MetaFFIType::HANDLE || type == MetaFFIType::HANDLE_ARRAY;
}

/**
 * Check if a MetaFFI type is a string type
 * @param type The MetaFFI type to check
 * @return true if the type is a string type, false otherwise
 */
[[nodiscard]] inline bool is_string_type(MetaFFIType type) {
	return type == MetaFFIType::STRING8 || type == MetaFFIType::STRING16 || type == MetaFFIType::STRING32 ||
	       type == MetaFFIType::STRING8_ARRAY || type == MetaFFIType::STRING16_ARRAY || type == MetaFFIType::STRING32_ARRAY;
}

} // namespace metaffi::idl
