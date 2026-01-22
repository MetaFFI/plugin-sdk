#pragma once

#include <jni.h>
#include <runtime/cdt.h>
#include <runtime/metaffi_primitives.h>
#include <stdexcept>
#include <sstream>
#include <string>
#include <utility>

namespace metaffi::utils
{

/**
 * @brief CDTS JVM Serializer/Deserializer
 *
 * Provides bidirectional conversion between JVM types (via JNI) and MetaFFI CDTs.
 * Uses explicit type specification for primitives (like Python3 serializer) to prevent
 * information loss, and auto-detection for wrapper objects.
 *
 * Key Features:
 * - Explicit type for primitives: add(jbyte, metaffi_uint8_type) with range validation
 * - Auto-detection for wrappers: Integer → int32, Long → int64
 * - Store primitive values directly in CDTS (not wrapper objects or handles)
 * - Multiple typed extraction methods: extract_int(), extract_long(), extract_char()
 * - RAII: Automatic JNI local reference cleanup
 * - Global references: Only for handles (metaffi_handle_type)
 *
 * Usage Examples:
 *
 * // Serialization (JVM → CDTS) - primitives with explicit type
 * cdts params(3);
 * cdts_jvm_serializer ser(env, params);
 * ser.add((jint)42, metaffi_int32_type);          // Explicit type required
 * ser.add((jbyte)255, metaffi_uint8_type);        // Validates range, casts to signed byte
 * ser.add((jchar)'A', metaffi_char8_type);        // Validates < 256
 *
 * // Serialization (JVM → CDTS) - wrapper objects with auto-detection
 * jobject integerObj = ...; // Integer object
 * ser << integerObj;  // Auto-detects as int32, extracts value, copies to CDTS
 *
 * // Deserialization (CDTS → JVM) - typed extraction methods
 * cdts_jvm_serializer deser(env, params);
 * jint val = deser.extract_int();      // From int32/uint32
 * jchar ch = deser.extract_char();     // From char8/char16/char32
 * jstring str = deser.extract_string(); // From string8/16/32
 */
class cdts_jvm_serializer
{
private:
	JNIEnv* env;                   // JNI environment pointer (must remain valid)
	cdts& data;                    // Reference to CDTS
	metaffi_size current_index;    // Current position in CDTS

	// ===== RAII Helper Classes =====

	/**
	 * @brief RAII wrapper for JNI local references
	 * Automatically deletes local reference on destruction
	 */
	class local_ref_guard
	{
		JNIEnv* env;
		jobject obj;
	public:
		local_ref_guard(JNIEnv* env, jobject obj) : env(env), obj(obj) {}
		~local_ref_guard() { if(obj) env->DeleteLocalRef(obj); }
		local_ref_guard(const local_ref_guard&) = delete;
		local_ref_guard& operator=(const local_ref_guard&) = delete;
	};

	/**
	 * @brief RAII wrapper for GetStringUTFChars
	 * Automatically releases string on destruction
	 */
	class utf_string_guard
	{
		JNIEnv* env;
		jstring jstr;
		const char* chars;
	public:
		utf_string_guard(JNIEnv* env, jstring jstr);
		~utf_string_guard();
		const char* get() const { return chars; }
		utf_string_guard(const utf_string_guard&) = delete;
		utf_string_guard& operator=(const utf_string_guard&) = delete;
	};

	// ===== Range Validation (Like Python3 Serializer) =====

	/**
	 * @brief Validate byte value fits in target type
	 * @throws std::runtime_error if value out of range
	 */
	void validate_byte_range(jlong value, metaffi_type target_type);
	void validate_short_range(jlong value, metaffi_type target_type);
	void validate_int_range(jlong value, metaffi_type target_type);
	void validate_char_range(jlong value, metaffi_type target_type);

	// ===== Type Detection =====

	/**
	 * @brief Detect metaffi_type from jobject using JNI introspection
	 * @param obj Java object (wrapper, array, or string)
	 * @return Corresponding metaffi_type
	 * @throws std::runtime_error if type cannot be determined
	 */
	metaffi_type detect_type(jobject obj);

	/**
	 * @brief Check if jobject is a specific wrapper class
	 * @param obj Object to check
	 * @param class_name Fully qualified class name (e.g., "java/lang/Integer")
	 * @return true if obj is instance of class_name
	 */
	bool is_instance_of(jobject obj, const char* class_name);

	/**
	 * @brief Detect array dimensions and element type from class name
	 * @param obj Java array object
	 * @return Pair of (dimensions, metaffi_type of elements)
	 *
	 * Examples:
	 *   "[I" → (1, metaffi_int32_type)
	 *   "[[I" → (2, metaffi_int32_type)
	 *   "[Ljava/lang/Integer;" → (1, metaffi_int32_type)
	 */
	std::pair<int, metaffi_type> detect_array_info(jarray obj);

	// ===== Wrapper Value Extraction =====

	/**
	 * @brief Extract primitive value from wrapper object
	 * Uses reflection to call intValue(), longValue(), etc.
	 */
	jint extract_int_from_wrapper(jobject obj);
	jlong extract_long_from_wrapper(jobject obj);
	jshort extract_short_from_wrapper(jobject obj);
	jbyte extract_byte_from_wrapper(jobject obj);
	jfloat extract_float_from_wrapper(jobject obj);
	jdouble extract_double_from_wrapper(jobject obj);
	jboolean extract_boolean_from_wrapper(jobject obj);
	jchar extract_char_from_wrapper(jobject obj);

	// ===== String Conversion =====

	/**
	 * @brief Convert jstring to metaffi_string8 (UTF-8)
	 * Allocates memory with xllr_alloc_string8
	 */
	metaffi_string8 jstring_to_string8(jstring str);

	/**
	 * @brief Convert jstring to metaffi_string16 (UTF-16)
	 * Allocates memory with xllr_alloc_string16
	 */
	metaffi_string16 jstring_to_string16(jstring str);

	/**
	 * @brief Convert metaffi_string8 to jstring
	 */
	jstring string8_to_jstring(metaffi_string8 str);

	/**
	 * @brief Convert metaffi_string16 to jstring
	 */
	jstring string16_to_jstring(metaffi_string16 str);

	/**
	 * @brief Convert metaffi_string32 to jstring
	 */
	jstring string32_to_jstring(metaffi_string32 str);

	// ===== Array Handling =====

	/**
	 * @brief Serialize Java array to CDTS array
	 * @param arr Java array (primitive or object)
	 * @param dimensions Number of array dimensions
	 * @param element_type MetaFFI type of array elements
	 */
	void serialize_array(jarray arr, int dimensions, metaffi_type element_type);

	/**
	 * @brief Deserialize CDTS array to Java array
	 * @param cdt_index Index in CDTS containing array
	 * @return Java array object (primitive array or object array)
	 */
	jarray deserialize_array(metaffi_size cdt_index);

	/**
	 * @brief Create Java primitive array from CDTS
	 * @param cdt CDTS reference containing array data
	 * @param element_type Type of array elements
	 * @return jarray (specific type like jintArray, jdoubleArray, etc.)
	 */
	jarray create_primitive_array(cdts& cdt, metaffi_type element_type);

	/**
	 * @brief Create Java object array from CDTS
	 * @param cdt CDTS reference containing array data
	 * @param element_type Type of array elements
	 * @return jobjectArray
	 */
	jobjectArray create_object_array(cdts& cdt, metaffi_type element_type);

	// ===== Validation =====

	/**
	 * @brief Check if index is within bounds
	 * @throws std::out_of_range if index >= data.length
	 */
	void check_bounds(metaffi_size index) const;

	/**
	 * @brief Check for JNI exceptions and throw C++ exception
	 * @throws std::runtime_error if JNI exception occurred
	 */
	void check_jni_exception(const char* operation);

public:
	/**
	 * @brief Construct serializer with JNI environment and CDTS
	 * @param env JNI environment pointer (must remain valid)
	 * @param pcdts Reference to CDTS
	 */
	explicit cdts_jvm_serializer(JNIEnv* env, cdts& pcdts);

	// ===== SERIALIZATION (Java → CDTS) =====

	// Primitives - explicit type required (like Python3 serializer)

	/**
	 * @brief Serialize jbyte with explicit type
	 * @param val Java byte value (-128 to 127)
	 * @param target_type metaffi_int8_type, metaffi_uint8_type, or metaffi_char8_type
	 * @return Reference to this for chaining
	 * @throws std::runtime_error if value out of range for target type
	 *
	 * Examples:
	 *   ser.add((jbyte)-128, metaffi_int8_type);   // OK
	 *   ser.add((jbyte)255, metaffi_uint8_type);   // OK (cast to signed)
	 *   ser.add((jbyte)-1, metaffi_uint8_type);    // Error: negative for unsigned
	 */
	cdts_jvm_serializer& add(jbyte val, metaffi_type target_type);

	/**
	 * @brief Serialize jshort with explicit type
	 * @param val Java short value
	 * @param target_type metaffi_int16_type or metaffi_uint16_type
	 */
	cdts_jvm_serializer& add(jshort val, metaffi_type target_type);

	/**
	 * @brief Serialize jint with explicit type
	 * @param val Java int value
	 * @param target_type metaffi_int32_type, metaffi_uint32_type, or metaffi_char32_type
	 */
	cdts_jvm_serializer& add(jint val, metaffi_type target_type);

	/**
	 * @brief Serialize jlong with explicit type
	 * @param val Java long value
	 * @param target_type metaffi_int64_type or metaffi_uint64_type
	 */
	cdts_jvm_serializer& add(jlong val, metaffi_type target_type);

	/**
	 * @brief Serialize jfloat
	 * @param val Java float value
	 * @param target_type metaffi_float32_type (type parameter for API consistency)
	 */
	cdts_jvm_serializer& add(jfloat val, metaffi_type target_type);

	/**
	 * @brief Serialize jdouble
	 * @param val Java double value
	 * @param target_type metaffi_float64_type (type parameter for API consistency)
	 */
	cdts_jvm_serializer& add(jdouble val, metaffi_type target_type);

	/**
	 * @brief Serialize jboolean
	 * @param val Java boolean value
	 * @param target_type metaffi_bool_type (type parameter ignored)
	 */
	cdts_jvm_serializer& add(jboolean val, metaffi_type target_type);

	/**
	 * @brief Serialize jchar with explicit type
	 * @param val Java char value (16-bit UTF-16 code unit)
	 * @param target_type metaffi_char8_type, metaffi_char16_type, or metaffi_char32_type
	 * @throws std::runtime_error if val > 255 for char8
	 *
	 * Examples:
	 *   ser.add((jchar)'A', metaffi_char8_type);    // OK (65 < 256)
	 *   ser.add((jchar)'ã', metaffi_char8_type);    // OK (227 < 256)
	 *   ser.add((jchar)'中', metaffi_char8_type);   // Error: 20013 > 255
	 *   ser.add((jchar)'中', metaffi_char16_type);  // OK
	 */
	cdts_jvm_serializer& add(jchar val, metaffi_type target_type);

	// Wrapper objects and other objects - auto-detect type

	/**
	 * @brief Serialize jobject with automatic type detection
	 * @param val Java object (Integer, Long, String, array, or arbitrary object)
	 * @return Reference to this for chaining
	 *
	 * Type detection:
	 * - Integer → int32 (auto-detects, extracts intValue())
	 * - Long → int64
	 * - Byte → int8
	 * - Short → int16
	 * - Float → float32
	 * - Double → float64
	 * - Boolean → bool
	 * - Character → char16
	 * - String → string8 (UTF-8 conversion)
	 * - Arrays → auto-detects dimensions and element type
	 * - Other objects → handle (NewGlobalRef)
	 */
	cdts_jvm_serializer& operator<<(jobject val);

	/**
	 * @brief Insert null value
	 */
	cdts_jvm_serializer& null();

	// ===== DESERIALIZATION (CDTS → JVM) =====

	/**
	 * @brief Extract value as jbyte (from int8 or uint8)
	 * For uint8, cast from unsigned to signed (bitwise)
	 * @return Java byte value
	 * @throws std::runtime_error if type mismatch
	 */
	jbyte extract_byte();

	/**
	 * @brief Extract value as jshort (from int16 or uint16)
	 */
	jshort extract_short();

	/**
	 * @brief Extract value as jint (from int32, uint32, or char32)
	 */
	jint extract_int();

	/**
	 * @brief Extract value as jlong (from int64 or uint64)
	 */
	jlong extract_long();

	/**
	 * @brief Extract value as jfloat (from float32)
	 */
	jfloat extract_float();

	/**
	 * @brief Extract value as jdouble (from float64)
	 */
	jdouble extract_double();

	/**
	 * @brief Extract value as jboolean (from bool)
	 */
	jboolean extract_boolean();

	/**
	 * @brief Extract value as jchar (from char8, char16, or char32)
	 * char8 (0-255) zero-extended to jchar (0-65535)
	 * char16 direct copy
	 * char32 truncated to 16-bit (may lose data if > 65535)
	 */
	jchar extract_char();

	/**
	 * @brief Extract value as jstring (from string8, string16, or string32)
	 * Performs UTF-8/UTF-16/UTF-32 → UTF-16 conversion as needed
	 * @return jstring (local reference, caller should not delete)
	 */
	jstring extract_string();

	/**
	 * @brief Extract value as jarray (from array types)
	 * Returns primitive array (int[], double[]) or object array (Integer[], String[])
	 * @return jarray (local reference)
	 */
	jarray extract_array();

	/**
	 * @brief Extract value as jobject (from handle type)
	 * Returns the global reference stored in handle
	 * @return jobject (global reference, DO NOT delete)
	 */
	jobject extract_handle();

	// ===== TYPE INTROSPECTION =====

	/**
	 * @brief Query type at current index without extracting
	 * @return MetaFFI type of current element
	 * @throws std::out_of_range if current_index >= size
	 */
	metaffi_type peek_type() const;

	/**
	 * @brief Check if current element is null
	 * @throws std::out_of_range if current_index >= size
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
	 * @throws std::out_of_range if index >= size
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

} // namespace metaffi::utils
