#pragma once

#include <runtime/cdt.h>
#include <runtime/metaffi_primitives.h>
#include <runtime/xllr_capi_loader.h>
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
	/**
	 * @brief Variant type for ANY values (MetaFFI primitive types)
	 */
	using cdts_any_variant = metaffi_variant;

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

	template<typename T>
	using clean_t = std::remove_cv_t<std::remove_reference_t<T>>;

	template<typename T>
	static constexpr bool is_fixed_signed_integral_v =
		std::is_same_v<clean_t<T>, int8_t> ||
		std::is_same_v<clean_t<T>, int16_t> ||
		std::is_same_v<clean_t<T>, int32_t> ||
		std::is_same_v<clean_t<T>, int64_t>;

	template<typename T>
	static constexpr bool is_fixed_unsigned_integral_v =
		std::is_same_v<clean_t<T>, uint8_t> ||
		std::is_same_v<clean_t<T>, uint16_t> ||
		std::is_same_v<clean_t<T>, uint32_t> ||
		std::is_same_v<clean_t<T>, uint64_t>;

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

	// Fallback integrals for platform-dependent native types (e.g., long long on Linux)
	template<typename T, typename std::enable_if_t<
		std::is_integral_v<clean_t<T>> &&
		std::is_signed_v<clean_t<T>> &&
		!std::is_same_v<clean_t<T>, bool> &&
		!is_fixed_signed_integral_v<T>, int> = 0>
	cdts_cpp_serializer& operator<<(T val);

	template<typename T, typename std::enable_if_t<
		std::is_integral_v<clean_t<T>> &&
		std::is_unsigned_v<clean_t<T>> &&
		!is_fixed_unsigned_integral_v<T>, int> = 0>
	cdts_cpp_serializer& operator<<(T val);

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
	cdts_cpp_serializer& operator<<(metaffi_handle val);
	cdts_cpp_serializer& operator<<(const cdt_metaffi_handle& handle);

	// Callables
	cdts_cpp_serializer& operator<<(const cdt_metaffi_callable& callable);

	// Null
	cdts_cpp_serializer& operator<<(std::nullptr_t);

	// Any (variant)
	cdts_cpp_serializer& operator<<(const metaffi_variant& val);

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

	template<typename T, typename std::enable_if_t<
		std::is_integral_v<clean_t<T>> &&
		std::is_signed_v<clean_t<T>> &&
		!std::is_same_v<clean_t<T>, bool> &&
		!is_fixed_signed_integral_v<T>, int> = 0>
	cdts_cpp_serializer& operator>>(T& val);

	template<typename T, typename std::enable_if_t<
		std::is_integral_v<clean_t<T>> &&
		std::is_unsigned_v<clean_t<T>> &&
		!is_fixed_unsigned_integral_v<T>, int> = 0>
	cdts_cpp_serializer& operator>>(T& val);

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
	cdts_cpp_serializer& operator>>(metaffi_handle& val);
	cdts_cpp_serializer& operator>>(cdt_metaffi_handle& handle);
	cdts_cpp_serializer& operator>>(cdt_metaffi_handle*& handle);
	template<typename T>
	cdts_cpp_serializer& operator>>(T*& val);

	// Callables
	cdts_cpp_serializer& operator>>(cdt_metaffi_callable& callable);
	cdts_cpp_serializer& operator>>(cdt_metaffi_callable*& callable);

	// Any (variant)
	cdts_cpp_serializer& operator>>(metaffi_variant& val);

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

template<typename T, typename std::enable_if_t<
	std::is_integral_v<cdts_cpp_serializer::clean_t<T>> &&
	std::is_signed_v<cdts_cpp_serializer::clean_t<T>> &&
	!std::is_same_v<cdts_cpp_serializer::clean_t<T>, bool> &&
	!cdts_cpp_serializer::is_fixed_signed_integral_v<T>, int>>
cdts_cpp_serializer& cdts_cpp_serializer::operator<<(T val)
{
	using ValueT = clean_t<T>;
	static_assert(sizeof(ValueT) <= sizeof(int64_t), "Unsupported signed integral width");

	if constexpr (sizeof(ValueT) <= sizeof(int32_t))
	{
		return (*this) << static_cast<int32_t>(val);
	}

	return (*this) << static_cast<int64_t>(val);
}

template<typename T, typename std::enable_if_t<
	std::is_integral_v<cdts_cpp_serializer::clean_t<T>> &&
	std::is_unsigned_v<cdts_cpp_serializer::clean_t<T>> &&
	!cdts_cpp_serializer::is_fixed_unsigned_integral_v<T>, int>>
cdts_cpp_serializer& cdts_cpp_serializer::operator<<(T val)
{
	using ValueT = clean_t<T>;
	static_assert(sizeof(ValueT) <= sizeof(uint64_t), "Unsupported unsigned integral width");

	if constexpr (sizeof(ValueT) <= sizeof(uint32_t))
	{
		return (*this) << static_cast<uint32_t>(val);
	}

	return (*this) << static_cast<uint64_t>(val);
}

template<typename T, typename std::enable_if_t<
	std::is_integral_v<cdts_cpp_serializer::clean_t<T>> &&
	std::is_signed_v<cdts_cpp_serializer::clean_t<T>> &&
	!std::is_same_v<cdts_cpp_serializer::clean_t<T>, bool> &&
	!cdts_cpp_serializer::is_fixed_signed_integral_v<T>, int>>
cdts_cpp_serializer& cdts_cpp_serializer::operator>>(T& val)
{
	using ValueT = clean_t<T>;
	static_assert(sizeof(ValueT) <= sizeof(int64_t), "Unsupported signed integral width");

	if constexpr (sizeof(ValueT) <= sizeof(int32_t))
	{
		int32_t tmp{};
		(*this) >> tmp;
		val = static_cast<ValueT>(tmp);
	}
	else
	{
		int64_t tmp{};
		(*this) >> tmp;
		val = static_cast<ValueT>(tmp);
	}

	return *this;
}

template<typename T, typename std::enable_if_t<
	std::is_integral_v<cdts_cpp_serializer::clean_t<T>> &&
	std::is_unsigned_v<cdts_cpp_serializer::clean_t<T>> &&
	!cdts_cpp_serializer::is_fixed_unsigned_integral_v<T>, int>>
cdts_cpp_serializer& cdts_cpp_serializer::operator>>(T& val)
{
	using ValueT = clean_t<T>;
	static_assert(sizeof(ValueT) <= sizeof(uint64_t), "Unsupported unsigned integral width");

	if constexpr (sizeof(ValueT) <= sizeof(uint32_t))
	{
		uint32_t tmp{};
		(*this) >> tmp;
		val = static_cast<ValueT>(tmp);
	}
	else
	{
		uint64_t tmp{};
		(*this) >> tmp;
		val = static_cast<ValueT>(tmp);
	}

	return *this;
}

template<typename T>
cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const std::vector<T>& vec)
{
	check_bounds(current_index);

	// Determine array depth and element type
	constexpr int depth = array_depth<std::vector<T>>::value;
	using ElementType = typename vector_element_type<std::vector<T>>::type;
	constexpr metaffi_type common_type = get_metaffi_type<ElementType>();

	// Special-case: vector<vector<uint8_t/int8_t>> -> array of bytes buffers (list of bytes)
	if constexpr (depth == 2 && (std::is_same_v<ElementType, metaffi_uint8> || std::is_same_v<ElementType, metaffi_int8>))
	{
		data[current_index].set_new_array(vec.size(), 1, metaffi_any_type);
		cdts& arr = static_cast<cdts&>(data[current_index]);
		for(size_t i = 0; i < vec.size(); ++i)
		{
			const auto& inner = vec[i];
			const metaffi_types inner_type = std::is_same_v<ElementType, metaffi_uint8> ? metaffi_uint8_type : metaffi_int8_type;
			arr[i].set_new_array(inner.size(), 1, inner_type);
			cdts& inner_arr = static_cast<cdts&>(arr[i]);
			for(size_t j = 0; j < inner.size(); ++j)
			{
				inner_arr[j] = inner[j];
			}
		}

		current_index++;
		return *this;
	}

	// For top-level 1D vectors of primitive types (NOT metaffi_variant or handles): use packed arrays.
	// Nested vectors (e.g. rows in vector<vector<int32_t>>) stay as regular CDTS arrays because some
	// runtimes expect explicit nested array objects there.
	if constexpr (depth == 1 && !std::is_same_v<T, metaffi_variant> &&
	              !std::is_same_v<T, cdt_metaffi_handle> && !std::is_same_v<T, cdt_metaffi_handle*>)
	{
		if(data.fixed_dimensions != MIXED_OR_UNKNOWN_DIMENSIONS)
		{
			data[current_index].set_new_array(vec.size(), 1, static_cast<metaffi_types>(common_type));
			cdts& arr = static_cast<cdts&>(data[current_index]);
			cdts_cpp_serializer nested(arr);
			for(size_t i = 0; i < vec.size(); ++i)
			{
				nested.set_index(i);
				nested << vec[i];
			}

			current_index++;
			return *this;
		}

		// Allocate packed array struct via xllr_alloc_memory (matching xllr_free_memory in cdt::free_packed_array)
		cdt_packed_array* packed = static_cast<cdt_packed_array*>(xllr_alloc_memory(sizeof(cdt_packed_array)));
		packed->length = static_cast<metaffi_size>(vec.size());

		if (vec.empty())
		{
			packed->data = nullptr;
			data[current_index].set_packed_array(packed, static_cast<metaffi_types>(common_type));
			current_index++;
			return *this;
		}

		// Allocate and copy data based on element type.
		// All data buffers use xllr_alloc_memory to match xllr_free_memory in free_packed_array.
		if constexpr (std::is_same_v<T, metaffi_int8>)
		{
			auto* buf = static_cast<metaffi_int8*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_int8)));
			std::memcpy(buf, vec.data(), vec.size() * sizeof(metaffi_int8));
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, metaffi_uint8>)
		{
			auto* buf = static_cast<metaffi_uint8*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_uint8)));
			std::memcpy(buf, vec.data(), vec.size() * sizeof(metaffi_uint8));
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, metaffi_int16>)
		{
			auto* buf = static_cast<metaffi_int16*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_int16)));
			std::memcpy(buf, vec.data(), vec.size() * sizeof(metaffi_int16));
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, metaffi_uint16>)
		{
			auto* buf = static_cast<metaffi_uint16*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_uint16)));
			std::memcpy(buf, vec.data(), vec.size() * sizeof(metaffi_uint16));
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, metaffi_int32>)
		{
			auto* buf = static_cast<metaffi_int32*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_int32)));
			std::memcpy(buf, vec.data(), vec.size() * sizeof(metaffi_int32));
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, metaffi_uint32>)
		{
			auto* buf = static_cast<metaffi_uint32*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_uint32)));
			std::memcpy(buf, vec.data(), vec.size() * sizeof(metaffi_uint32));
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, metaffi_int64>)
		{
			auto* buf = static_cast<metaffi_int64*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_int64)));
			std::memcpy(buf, vec.data(), vec.size() * sizeof(metaffi_int64));
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, metaffi_uint64>)
		{
			auto* buf = static_cast<metaffi_uint64*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_uint64)));
			std::memcpy(buf, vec.data(), vec.size() * sizeof(metaffi_uint64));
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, metaffi_float32>)
		{
			auto* buf = static_cast<metaffi_float32*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_float32)));
			std::memcpy(buf, vec.data(), vec.size() * sizeof(metaffi_float32));
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, metaffi_float64>)
		{
			auto* buf = static_cast<metaffi_float64*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_float64)));
			std::memcpy(buf, vec.data(), vec.size() * sizeof(metaffi_float64));
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			auto* buf = static_cast<metaffi_bool*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_bool)));
			for (size_t i = 0; i < vec.size(); ++i)
			{
				buf[i] = vec[i] ? 1 : 0;
			}
			packed->data = buf;
		}
		else if constexpr (std::is_same_v<T, std::string>)
		{
			auto* buf = static_cast<metaffi_string8*>(xllr_alloc_memory(vec.size() * sizeof(metaffi_string8)));
			for (size_t i = 0; i < vec.size(); ++i)
			{
				buf[i] = xllr_alloc_string8(reinterpret_cast<const char8_t*>(vec[i].c_str()), vec[i].length());
			}
			packed->data = buf;
		}
		else
		{
			// Unsupported type for packed array - clean up and throw
			xllr_free_memory(packed);
			throw std::runtime_error("Unsupported type for packed array serialization");
		}

		data[current_index].set_packed_array(packed, static_cast<metaffi_types>(common_type));
		current_index++;
		return *this;
	}

	// For multi-dimensional arrays or vector<metaffi_variant>: use regular CDTS arrays
	data[current_index].set_new_array(vec.size(), depth, static_cast<metaffi_types>(common_type));
	cdts& arr = static_cast<cdts&>(data[current_index]);

	// Fill array elements
	cdts_cpp_serializer nested(arr);
	for (size_t i = 0; i < vec.size(); ++i)
	{
		nested.set_index(i);
		nested << vec[i];
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

	// Check if it's a packed array
	if (metaffi_is_packed_array(data[current_index].type))
	{
		const cdt_packed_array* packed = data[current_index].cdt_val.packed_array_val;
		if (!packed)
		{
			vec.clear();
			current_index++;
			return *this;
		}

		vec.resize(packed->length);
		metaffi_type elem_type = metaffi_packed_element_type(data[current_index].type);

		// Extract based on element type
		if constexpr (std::is_same_v<T, metaffi_int8>)
		{
			std::memcpy(vec.data(), packed->data, packed->length * sizeof(metaffi_int8));
		}
		else if constexpr (std::is_same_v<T, metaffi_uint8>)
		{
			std::memcpy(vec.data(), packed->data, packed->length * sizeof(metaffi_uint8));
		}
		else if constexpr (std::is_same_v<T, metaffi_int16>)
		{
			std::memcpy(vec.data(), packed->data, packed->length * sizeof(metaffi_int16));
		}
		else if constexpr (std::is_same_v<T, metaffi_uint16>)
		{
			std::memcpy(vec.data(), packed->data, packed->length * sizeof(metaffi_uint16));
		}
		else if constexpr (std::is_same_v<T, metaffi_int32>)
		{
			std::memcpy(vec.data(), packed->data, packed->length * sizeof(metaffi_int32));
		}
		else if constexpr (std::is_same_v<T, metaffi_uint32>)
		{
			std::memcpy(vec.data(), packed->data, packed->length * sizeof(metaffi_uint32));
		}
		else if constexpr (std::is_same_v<T, metaffi_int64>)
		{
			std::memcpy(vec.data(), packed->data, packed->length * sizeof(metaffi_int64));
		}
		else if constexpr (std::is_same_v<T, metaffi_uint64>)
		{
			std::memcpy(vec.data(), packed->data, packed->length * sizeof(metaffi_uint64));
		}
		else if constexpr (std::is_same_v<T, metaffi_float32>)
		{
			std::memcpy(vec.data(), packed->data, packed->length * sizeof(metaffi_float32));
		}
		else if constexpr (std::is_same_v<T, metaffi_float64>)
		{
			std::memcpy(vec.data(), packed->data, packed->length * sizeof(metaffi_float64));
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			const metaffi_bool* buf = static_cast<const metaffi_bool*>(packed->data);
			for (size_t i = 0; i < packed->length; ++i)
			{
				vec[i] = (buf[i] != 0);
			}
		}
		else if constexpr (std::is_same_v<T, std::string>)
		{
			const metaffi_string8* buf = static_cast<const metaffi_string8*>(packed->data);
			for (size_t i = 0; i < packed->length; ++i)
			{
				vec[i] = std::string(reinterpret_cast<const char*>(buf[i]));
			}
		}
		else
		{
			throw std::runtime_error("Unsupported type for packed array deserialization");
		}

		current_index++;
		return *this;
	}

	// Regular CDTS array
	cdts& arr = static_cast<cdts&>(data[current_index]);
	vec.resize(arr.length);

	// Extract array elements
	cdts_cpp_serializer nested(arr);
	for (size_t i = 0; i < arr.length; ++i)
	{
		nested.set_index(i);
		nested >> vec[i];
	}

	current_index++;
	return *this;
}

template<typename T>
cdts_cpp_serializer& cdts_cpp_serializer::operator>>(T*& val)
{
	metaffi_handle handle{};
	*this >> handle;
	val = static_cast<T*>(handle);
	return *this;
}

} // namespace metaffi::utils
