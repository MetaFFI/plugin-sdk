#pragma once

#include <runtime/cdt.h>
#include <runtime/metaffi_primitives.h>
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>
#include <sstream>
#include <type_traits>

namespace metaffi::utils
{

/**
 * @brief CDTS C++ Serializer/Deserializer
 *
 * Provides stream-based interface for converting between C++ types and MetaFFI CDTs.
 *
 * Usage Examples:
 *
 * // Serialization (C++ → CDT)
 * cdts params(3);
 * cdts_cpp_serializer ser(params);
 * ser << int32_t(42) << std::string("hello") << 3.14;
 *
 * // Deserialization (CDT → C++)
 * cdts_cpp_serializer deser(params);
 * int32_t x; std::string s; double d;
 * deser >> x >> s >> d;
 *
 * // Nested arrays
 * std::vector<std::vector<int32_t>> matrix = {{1,2},{3,4}};
 * ser << matrix;  // Automatically handles 2D structure
 *
 * // ANY type handling
 * auto value = ser.extract_any();  // Returns std::variant
 * if (ser.peek_type() == metaffi_int32_type) {
 *     int32_t x;
 *     ser >> x;
 * }
 */
class cdts_cpp_serializer
{
public:
	// Forward declaration for recursive variant
	struct cdts_any_variant_impl;

	/**
	 * @brief Variant type for ANY values (using standard C++ types)
	 *
	 * Can hold any MetaFFI type using standard C++ representations
	 */
	using cdts_any_variant = std::variant<
		int8_t, int16_t, int32_t, int64_t,
		uint8_t, uint16_t, uint32_t, uint64_t,
		float, double, bool,
		std::string, std::u16string, std::u32string,
		metaffi_char8, metaffi_char16, metaffi_char32,
		cdt_metaffi_handle, cdt_metaffi_callable,
		std::monostate  // For null/uninitialized
	>;

private:
	cdts& data;
	metaffi_size current_index;

	// ===== Type Traits =====

	// Check if T is std::vector
	template<typename T>
	struct is_vector : std::false_type {};

	template<typename T>
	struct is_vector<std::vector<T>> : std::true_type {};

	// Array depth detection via template recursion
	template<typename T>
	struct array_depth
	{
		static constexpr int value = 0;
	};

	template<typename T>
	struct array_depth<std::vector<T>>
	{
		static constexpr int value = 1 + array_depth<T>::value;
	};

	// Get element type of vector (base case: T itself)
	template<typename T>
	struct vector_element_type
	{
		using type = T;
	};

	template<typename T>
	struct vector_element_type<std::vector<T>>
	{
		using type = typename vector_element_type<T>::type;
	};

	// ===== Helper Methods =====

	/**
	 * @brief Check if index is within bounds
	 * @throws std::out_of_range if index >= data.length
	 */
	void check_bounds(metaffi_size index) const;

	/**
	 * @brief Get MetaFFI type for C++ type (compile-time)
	 */
	template<typename T>
	static constexpr metaffi_type get_metaffi_type();

	/**
	 * @brief Validate that CDT at index matches expected type
	 * @throws std::runtime_error if type mismatch
	 */
	template<typename T>
	void validate_type_at(metaffi_size index) const;

public:
	/**
	 * @brief Construct serializer wrapping existing CDTS
	 * @param pcdts Reference to CDTS (for both serialization and deserialization)
	 */
	explicit cdts_cpp_serializer(cdts& pcdts);

	// ===== SERIALIZATION (C++ → CDT) =====

	// Primitives (standard C++ types)
	cdts_cpp_serializer& operator<<(int8_t val);
	cdts_cpp_serializer& operator<<(int16_t val);
	cdts_cpp_serializer& operator<<(int32_t val);
	cdts_cpp_serializer& operator<<(int64_t val);
	cdts_cpp_serializer& operator<<(uint8_t val);
	cdts_cpp_serializer& operator<<(uint16_t val);
	cdts_cpp_serializer& operator<<(uint32_t val);
	cdts_cpp_serializer& operator<<(uint64_t val);
	cdts_cpp_serializer& operator<<(float val);
	cdts_cpp_serializer& operator<<(double val);
	cdts_cpp_serializer& operator<<(bool val);

	// Strings
	cdts_cpp_serializer& operator<<(const std::string& val);
	cdts_cpp_serializer& operator<<(const std::u16string& val);
	cdts_cpp_serializer& operator<<(const std::u32string& val);
	cdts_cpp_serializer& operator<<(const char* val);  // Convenience

	// Characters
	cdts_cpp_serializer& operator<<(const metaffi_char8& val);
	cdts_cpp_serializer& operator<<(const metaffi_char16& val);
	cdts_cpp_serializer& operator<<(const metaffi_char32& val);

	// Arrays - automatic multi-level nesting via template recursion
	template<typename T>
	cdts_cpp_serializer& operator<<(const std::vector<T>& vec);

	// Handles
	cdts_cpp_serializer& operator<<(const cdt_metaffi_handle& handle);

	// Callables
	cdts_cpp_serializer& operator<<(const cdt_metaffi_callable& callable);

	/**
	 * @brief Insert null value
	 */
	cdts_cpp_serializer& null();

	// ===== DESERIALIZATION (CDT → C++) =====

	// Primitives (standard C++ types)
	cdts_cpp_serializer& operator>>(int8_t& val);
	cdts_cpp_serializer& operator>>(int16_t& val);
	cdts_cpp_serializer& operator>>(int32_t& val);
	cdts_cpp_serializer& operator>>(int64_t& val);
	cdts_cpp_serializer& operator>>(uint8_t& val);
	cdts_cpp_serializer& operator>>(uint16_t& val);
	cdts_cpp_serializer& operator>>(uint32_t& val);
	cdts_cpp_serializer& operator>>(uint64_t& val);
	cdts_cpp_serializer& operator>>(float& val);
	cdts_cpp_serializer& operator>>(double& val);
	cdts_cpp_serializer& operator>>(bool& val);

	// Strings
	cdts_cpp_serializer& operator>>(std::string& val);
	cdts_cpp_serializer& operator>>(std::u16string& val);
	cdts_cpp_serializer& operator>>(std::u32string& val);

	// Characters
	cdts_cpp_serializer& operator>>(metaffi_char8& val);
	cdts_cpp_serializer& operator>>(metaffi_char16& val);
	cdts_cpp_serializer& operator>>(metaffi_char32& val);

	// Arrays - automatic multi-level nesting
	template<typename T>
	cdts_cpp_serializer& operator>>(std::vector<T>& vec);

	// Handles
	cdts_cpp_serializer& operator>>(cdt_metaffi_handle& handle);

	// Callables
	cdts_cpp_serializer& operator>>(cdt_metaffi_callable& callable);

	// ===== ANY TYPE SUPPORT =====

	/**
	 * @brief Extract value as variant (for unknown types)
	 * @return Variant containing the value
	 * @throws std::runtime_error if type is unknown
	 */
	cdts_any_variant extract_any();

	/**
	 * @brief Query type at current index without extracting
	 * @return MetaFFI type of current element
	 */
	metaffi_type peek_type() const;

	/**
	 * @brief Check if current element is null
	 */
	bool is_null() const;

	// ===== UTILITY METHODS =====

	/**
	 * @brief Reset index to 0 for re-reading
	 */
	void reset();

	/**
	 * @brief Get current index
	 */
	metaffi_size get_index() const;

	/**
	 * @brief Set current index
	 */
	void set_index(metaffi_size index);

	/**
	 * @brief Get total size of CDTS
	 */
	metaffi_size size() const;

	/**
	 * @brief Check if more elements available
	 */
	bool has_more() const;
};

// ===== Template Implementations =====

template<typename T>
constexpr metaffi_type cdts_cpp_serializer::get_metaffi_type()
{
	if constexpr (std::is_same_v<T, metaffi_int8>) return metaffi_int8_type;
	else if constexpr (std::is_same_v<T, metaffi_int16>) return metaffi_int16_type;
	else if constexpr (std::is_same_v<T, metaffi_int32>) return metaffi_int32_type;
	else if constexpr (std::is_same_v<T, metaffi_int64>) return metaffi_int64_type;
	else if constexpr (std::is_same_v<T, metaffi_uint8>) return metaffi_uint8_type;
	else if constexpr (std::is_same_v<T, metaffi_uint16>) return metaffi_uint16_type;
	else if constexpr (std::is_same_v<T, metaffi_uint32>) return metaffi_uint32_type;
	else if constexpr (std::is_same_v<T, metaffi_uint64>) return metaffi_uint64_type;
	else if constexpr (std::is_same_v<T, metaffi_float32>) return metaffi_float32_type;
	else if constexpr (std::is_same_v<T, metaffi_float64>) return metaffi_float64_type;
	else if constexpr (std::is_same_v<T, bool>) return metaffi_bool_type;
	else if constexpr (std::is_same_v<T, std::string>) return metaffi_string8_type;
	else if constexpr (std::is_same_v<T, std::u16string>) return metaffi_string16_type;
	else if constexpr (std::is_same_v<T, std::u32string>) return metaffi_string32_type;
	else if constexpr (std::is_same_v<T, metaffi_char8>) return metaffi_char8_type;
	else if constexpr (std::is_same_v<T, metaffi_char16>) return metaffi_char16_type;
	else if constexpr (std::is_same_v<T, metaffi_char32>) return metaffi_char32_type;
	else if constexpr (std::is_same_v<T, cdt_metaffi_handle>) return metaffi_handle_type;
	else if constexpr (std::is_same_v<T, cdt_metaffi_callable>) return metaffi_callable_type;
	else return metaffi_any_type;
}

template<typename T>
void cdts_cpp_serializer::validate_type_at(metaffi_size index) const
{
	check_bounds(index);

	metaffi_type expected = get_metaffi_type<T>();
	metaffi_type actual = data[index].type;

	if (actual != expected)
	{
		std::stringstream ss;
		ss << "Type mismatch at index " << index << ": expected type "
		   << expected << ", got " << actual;
		throw std::runtime_error(ss.str());
	}
}

template<typename T>
cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const std::vector<T>& vec)
{
	check_bounds(current_index);

	// Determine array depth and element type
	constexpr int depth = array_depth<std::vector<T>>::value;
	using ElementType = typename vector_element_type<std::vector<T>>::type;
	constexpr metaffi_type common_type = get_metaffi_type<ElementType>();

	// Create nested CDTS structure
	data[current_index].set_new_array(vec.size(), depth, static_cast<metaffi_types>(common_type));
	cdts& arr = static_cast<cdts&>(data[current_index]);

	// Fill array elements
	for (size_t i = 0; i < vec.size(); ++i)
	{
		if constexpr (is_vector<T>::value)
		{
			// Nested vector - recurse
			cdts_cpp_serializer nested(arr);
			nested.set_index(i);
			nested << vec[i];
		}
		else
		{
			// Base type - use assignment operator
			arr[i] = vec[i];
		}
	}

	current_index++;
	return *this;
}

template<typename T>
cdts_cpp_serializer& cdts_cpp_serializer::operator>>(std::vector<T>& vec)
{
	check_bounds(current_index);

	// Verify it's an array type
	if (!(data[current_index].type & metaffi_array_type))
	{
		std::stringstream ss;
		ss << "Type mismatch at index " << current_index << ": expected array, got type "
		   << data[current_index].type;
		throw std::runtime_error(ss.str());
	}

	cdts& arr = static_cast<cdts&>(data[current_index]);
	vec.resize(arr.length);

	// Extract array elements
	for (size_t i = 0; i < arr.length; ++i)
	{
		if constexpr (is_vector<T>::value)
		{
			// Nested vector - recurse
			cdts_cpp_serializer nested(arr);
			nested.set_index(i);
			nested >> vec[i];
		}
		else
		{
			// Base type - direct extraction
			vec[i] = static_cast<T>(arr[i]);
		}
	}

	current_index++;
	return *this;
}

} // namespace metaffi::utils
