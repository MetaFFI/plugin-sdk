#pragma once

#include "metaffi_type.hpp"
#include <type_traits>
#include <vector>
#include <string>
#include <cstdint>
#include <utility>

namespace metaffi::idl {

/**
 * TypeMapper provides C++ type → MetaFFI type mapping
 * Uses template metaprogramming and type traits for compile-time type detection
 */
class TypeMapper {
public:
	/**
	 * Map C++ type to MetaFFI type string and dimensions
	 * @tparam T The C++ type to map
	 * @return Pair of (metaffi_type_string, dimensions)
	 */
	template<typename T>
	static std::pair<std::string, int> map_type() {
		using Base = std::remove_cv_t<std::remove_reference_t<T>>;

		// Floating point types
		if constexpr (std::is_same_v<Base, double>) {
			return {"float64", 0};
		} else if constexpr (std::is_same_v<Base, float>) {
			return {"float32", 0};
		}
		// Signed integer types
		else if constexpr (std::is_same_v<Base, int8_t>) {
			return {"int8", 0};
		} else if constexpr (std::is_same_v<Base, int16_t>) {
			return {"int16", 0};
		} else if constexpr (std::is_same_v<Base, int32_t> || std::is_same_v<Base, int>) {
			return {"int32", 0};
		} else if constexpr (std::is_same_v<Base, int64_t> || std::is_same_v<Base, long long>) {
			return {"int64", 0};
		}
		// Unsigned integer types
		else if constexpr (std::is_same_v<Base, uint8_t>) {
			return {"uint8", 0};
		} else if constexpr (std::is_same_v<Base, uint16_t>) {
			return {"uint16", 0};
		} else if constexpr (std::is_same_v<Base, uint32_t> || std::is_same_v<Base, unsigned int>) {
			return {"uint32", 0};
		} else if constexpr (std::is_same_v<Base, uint64_t> || std::is_same_v<Base, unsigned long long>) {
			return {"uint64", 0};
		}
		// Boolean
		else if constexpr (std::is_same_v<Base, bool>) {
			return {"bool", 0};
		}
		// Character types
		else if constexpr (std::is_same_v<Base, char> || std::is_same_v<Base, char8_t>) {
			return {"char8", 0};
		} else if constexpr (std::is_same_v<Base, char16_t>) {
			return {"char16", 0};
		} else if constexpr (std::is_same_v<Base, char32_t>) {
			return {"char32", 0};
		}
		// String types
		else if constexpr (std::is_same_v<Base, std::string> || std::is_same_v<Base, const char*> || std::is_same_v<Base, char*>) {
			return {"string8", 0};
		} else if constexpr (std::is_same_v<Base, std::u16string>) {
			return {"string16", 0};
		} else if constexpr (std::is_same_v<Base, std::u32string>) {
			return {"string32", 0};
		}
		// Special case: vector<uint8_t> → uint8_array
		else if constexpr (std::is_same_v<Base, std::vector<uint8_t>>) {
			return {"uint8_array", 0};
		}
		// Generic vector handling
		else if constexpr (is_vector_v<Base>) {
			return {"handle_array", 0};
		}
		// Pointers and classes
		else if constexpr (std::is_pointer_v<Base> || std::is_class_v<Base>) {
			return {"handle", 0};
		}
		// Fallback
		else {
			return {"any", 0};
		}
	}

	/**
	 * Get type alias for handle types
	 * For handle types, returns the mangled/demangled type name
	 * For other types, returns empty string
	 * @tparam T The C++ type
	 * @return Type alias string
	 */
	template<typename T>
	static std::string get_type_alias() {
		using Base = std::remove_cv_t<std::remove_reference_t<T>>;

		// For class types (excluding std::string), return type name
		if constexpr (std::is_class_v<Base> && !is_string_type_v<Base>) {
			// Use typeid for type name (may be mangled on some platforms)
			return typeid(Base).name();
		}

		// For primitive types, return standard type string
		if constexpr (std::is_same_v<Base, int8_t>) return "int8_t";
		if constexpr (std::is_same_v<Base, int16_t>) return "int16_t";
		if constexpr (std::is_same_v<Base, int32_t>) return "int32_t";
		if constexpr (std::is_same_v<Base, int64_t>) return "int64_t";
		if constexpr (std::is_same_v<Base, uint8_t>) return "uint8_t";
		if constexpr (std::is_same_v<Base, uint16_t>) return "uint16_t";
		if constexpr (std::is_same_v<Base, uint32_t>) return "uint32_t";
		if constexpr (std::is_same_v<Base, uint64_t>) return "uint64_t";
		if constexpr (std::is_same_v<Base, float>) return "float";
		if constexpr (std::is_same_v<Base, double>) return "double";
		if constexpr (std::is_same_v<Base, bool>) return "bool";
		if constexpr (std::is_same_v<Base, std::string>) return "std::string";
		if constexpr (std::is_same_v<Base, const char*>) return "const char*";
		if constexpr (std::is_same_v<Base, char8_t>) return "char8_t";
		if constexpr (std::is_same_v<Base, char16_t>) return "char16_t";
		if constexpr (std::is_same_v<Base, char32_t>) return "char32_t";

		return "";
	}

private:
	// Type trait: Check if type is std::vector
	template<typename T>
	struct is_vector : std::false_type {};

	template<typename T>
	struct is_vector<std::vector<T>> : std::true_type {};

	template<typename T>
	static constexpr bool is_vector_v = is_vector<T>::value;

	// Type trait: Check if type is a string type
	template<typename T>
	struct is_string_type : std::false_type {};

	template<>
	struct is_string_type<std::string> : std::true_type {};

	template<>
	struct is_string_type<std::u16string> : std::true_type {};

	template<>
	struct is_string_type<std::u32string> : std::true_type {};

	template<typename T>
	static constexpr bool is_string_type_v = is_string_type<T>::value;
};

} // namespace metaffi::idl
