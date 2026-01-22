#include "cdts_jvm_serializer.h"
#include <runtime/xllr_capi_loader.h>
#include <cstring>

namespace metaffi::utils
{

// ===== Phase 1: Core Infrastructure =====

//--------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------

cdts_jvm_serializer::cdts_jvm_serializer(JNIEnv* env, cdts& pcdts)
	: env(env), data(pcdts), current_index(0)
{
	if (!env) {
		throw std::invalid_argument("JNIEnv pointer cannot be null");
	}
}

//--------------------------------------------------------------------
// RAII Helpers
//--------------------------------------------------------------------

cdts_jvm_serializer::utf_string_guard::utf_string_guard(JNIEnv* env, jstring jstr)
	: env(env), jstr(jstr), chars(nullptr)
{
	if (jstr) {
		chars = env->GetStringUTFChars(jstr, nullptr);
		if (!chars) {
			throw std::runtime_error("Failed to get UTF-8 characters from jstring");
		}
	}
}

cdts_jvm_serializer::utf_string_guard::~utf_string_guard()
{
	if (chars) {
		env->ReleaseStringUTFChars(jstr, chars);
	}
}

//--------------------------------------------------------------------
// Validation
//--------------------------------------------------------------------

void cdts_jvm_serializer::check_bounds(metaffi_size index) const
{
	if (index >= data.length) {
		std::stringstream ss;
		ss << "Index " << index << " out of bounds (size: " << data.length << ")";
		throw std::out_of_range(ss.str());
	}
}

void cdts_jvm_serializer::check_jni_exception(const char* operation)
{
	if (env->ExceptionCheck()) {
		jthrowable ex = env->ExceptionOccurred();
		env->ExceptionClear();

		// Extract exception message
		std::string error_msg = operation;
		error_msg += " failed";

		if (ex) {
			jclass throwableClass = env->FindClass("java/lang/Throwable");
			if (throwableClass) {
				jmethodID getMessage = env->GetMethodID(throwableClass, "getMessage", "()Ljava/lang/String;");
				if (getMessage) {
					jstring msg = (jstring)env->CallObjectMethod(ex, getMessage);
					if (msg) {
						const char* msgChars = env->GetStringUTFChars(msg, nullptr);
						if (msgChars) {
							error_msg += ": ";
							error_msg += msgChars;
							env->ReleaseStringUTFChars(msg, msgChars);
						}
						env->DeleteLocalRef(msg);
					}
				}
				env->DeleteLocalRef(throwableClass);
			}
			env->DeleteLocalRef(ex);
		}

		throw std::runtime_error(error_msg);
	}
}

//--------------------------------------------------------------------
// Range Validation (Like Python3 Serializer)
//--------------------------------------------------------------------

void cdts_jvm_serializer::validate_byte_range(jlong value, metaffi_type target_type)
{
	switch(target_type) {
		case metaffi_int8_type:
			if (value < -128 || value > 127) {
				throw std::runtime_error("Value out of range for int8 [-128, 127]");
			}
			break;
		case metaffi_uint8_type:
			if (value < 0 || value > 255) {
				throw std::runtime_error("Value out of range for uint8 [0, 255]");
			}
			break;
		case metaffi_char8_type:
			if (value < 0 || value > 255) {
				throw std::runtime_error("Value out of range for char8 [0, 255]");
			}
			break;
		default:
			throw std::runtime_error("Invalid byte target type");
	}
}

void cdts_jvm_serializer::validate_short_range(jlong value, metaffi_type target_type)
{
	switch(target_type) {
		case metaffi_int16_type:
			if (value < -32768 || value > 32767) {
				throw std::runtime_error("Value out of range for int16 [-32768, 32767]");
			}
			break;
		case metaffi_uint16_type:
			if (value < 0 || value > 65535) {
				throw std::runtime_error("Value out of range for uint16 [0, 65535]");
			}
			break;
		default:
			throw std::runtime_error("Invalid short target type");
	}
}

void cdts_jvm_serializer::validate_int_range(jlong value, metaffi_type target_type)
{
	switch(target_type) {
		case metaffi_int32_type:
			if (value < -2147483648LL || value > 2147483647LL) {
				throw std::runtime_error("Value out of range for int32 [-2147483648, 2147483647]");
			}
			break;
		case metaffi_uint32_type:
			if (value < 0 || value > 4294967295ULL) {
				throw std::runtime_error("Value out of range for uint32 [0, 4294967295]");
			}
			break;
		case metaffi_char32_type:
			// All int values fit in char32
			break;
		default:
			throw std::runtime_error("Invalid int target type");
	}
}

void cdts_jvm_serializer::validate_char_range(jlong value, metaffi_type target_type)
{
	switch(target_type) {
		case metaffi_char8_type:
			if (value < 0 || value > 255) {
				throw std::runtime_error("Value out of range for char8 [0, 255]");
			}
			break;
		case metaffi_char16_type:
			if (value < 0 || value > 65535) {
				throw std::runtime_error("Value out of range for char16 [0, 65535]");
			}
			break;
		case metaffi_char32_type:
			// All values fit in char32
			break;
		default:
			throw std::runtime_error("Invalid char target type");
	}
}

//--------------------------------------------------------------------
// Utility Methods
//--------------------------------------------------------------------

void cdts_jvm_serializer::reset()
{
	current_index = 0;
}

metaffi_size cdts_jvm_serializer::get_index() const
{
	return current_index;
}

void cdts_jvm_serializer::set_index(metaffi_size index)
{
	check_bounds(index);
	current_index = index;
}

metaffi_size cdts_jvm_serializer::size() const
{
	return data.length;
}

bool cdts_jvm_serializer::has_more() const
{
	return current_index < data.length;
}

//--------------------------------------------------------------------
// Type Introspection
//--------------------------------------------------------------------

metaffi_type cdts_jvm_serializer::peek_type() const
{
	check_bounds(current_index);
	return data[current_index].type;
}

bool cdts_jvm_serializer::is_null() const
{
	check_bounds(current_index);
	return data[current_index].type == metaffi_null_type;
}

cdts_jvm_serializer& cdts_jvm_serializer::null()
{
	check_bounds(current_index);
	data[current_index].type = metaffi_null_type;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

// ===== Placeholder implementations for remaining methods =====
// These will be implemented in subsequent phases

//--------------------------------------------------------------------
// Phase 2: Type Detection
//--------------------------------------------------------------------

bool cdts_jvm_serializer::is_instance_of(jobject obj, const char* class_name)
{
	if (!obj) return false;

	jclass cls = env->FindClass(class_name);
	if (!cls) {
		check_jni_exception("FindClass");
		return false;
	}

	jboolean result = env->IsInstanceOf(obj, cls);
	env->DeleteLocalRef(cls);
	check_jni_exception("IsInstanceOf");

	return result == JNI_TRUE;
}

std::pair<int, metaffi_type> cdts_jvm_serializer::detect_array_info(jarray obj)
{
	if (!obj) {
		throw std::runtime_error("Cannot detect array info for null object");
	}

	// Get class object
	jclass objClass = env->GetObjectClass(obj);
	if (!objClass) {
		check_jni_exception("GetObjectClass");
		throw std::runtime_error("Failed to get object class");
	}
	local_ref_guard guard1(env, objClass);

	// Get Class.getName() method
	jclass classClass = env->FindClass("java/lang/Class");
	if (!classClass) {
		check_jni_exception("FindClass(java/lang/Class)");
		throw std::runtime_error("Failed to find Class class");
	}
	local_ref_guard guard2(env, classClass);

	jmethodID getName = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
	if (!getName) {
		check_jni_exception("GetMethodID(getName)");
		throw std::runtime_error("Failed to get getName method");
	}

	// Call getName()
	jstring className = (jstring)env->CallObjectMethod(objClass, getName);
	if (!className) {
		check_jni_exception("CallObjectMethod(getName)");
		throw std::runtime_error("Failed to get class name");
	}
	local_ref_guard guard3(env, className);

	// Get class name as C string
	const char* classNameChars = env->GetStringUTFChars(className, nullptr);
	if (!classNameChars) {
		check_jni_exception("GetStringUTFChars");
		throw std::runtime_error("Failed to get class name string");
	}

	std::string classNameStr(classNameChars);
	env->ReleaseStringUTFChars(className, classNameChars);

	// Count dimensions (number of '[' characters)
	int dimensions = 0;
	for (char c : classNameStr) {
		if (c == '[') dimensions++;
		else break;
	}

	if (dimensions == 0) {
		throw std::runtime_error("Object is not an array");
	}

	// Parse element type from class name
	// Format: "[I" (int[]), "[[I" (int[][]), "[Ljava/lang/Integer;" (Integer[])
	const char* elementSignature = classNameStr.c_str() + dimensions;
	metaffi_type element_type;

	switch (elementSignature[0]) {
		case 'Z': element_type = metaffi_bool_type; break;       // boolean
		case 'B': element_type = metaffi_int8_type; break;       // byte
		case 'S': element_type = metaffi_int16_type; break;      // short
		case 'I': element_type = metaffi_int32_type; break;      // int
		case 'J': element_type = metaffi_int64_type; break;      // long
		case 'F': element_type = metaffi_float32_type; break;    // float
		case 'D': element_type = metaffi_float64_type; break;    // double
		case 'C': element_type = metaffi_char16_type; break;     // char
		case 'L':
			// Object type
			if (std::strstr(elementSignature, "Ljava/lang/Integer;")) {
				element_type = metaffi_int32_type;
			} else if (std::strstr(elementSignature, "Ljava/lang/Long;")) {
				element_type = metaffi_int64_type;
			} else if (std::strstr(elementSignature, "Ljava/lang/Short;")) {
				element_type = metaffi_int16_type;
			} else if (std::strstr(elementSignature, "Ljava/lang/Byte;")) {
				element_type = metaffi_int8_type;
			} else if (std::strstr(elementSignature, "Ljava/lang/Float;")) {
				element_type = metaffi_float32_type;
			} else if (std::strstr(elementSignature, "Ljava/lang/Double;")) {
				element_type = metaffi_float64_type;
			} else if (std::strstr(elementSignature, "Ljava/lang/Boolean;")) {
				element_type = metaffi_bool_type;
			} else if (std::strstr(elementSignature, "Ljava/lang/Character;")) {
				element_type = metaffi_char16_type;
			} else if (std::strstr(elementSignature, "Ljava/lang/String;")) {
				element_type = metaffi_string8_type;
			} else {
				// Generic object - treat as handle
				element_type = metaffi_handle_type;
			}
			break;
		default:
			throw std::runtime_error(std::string("Unknown array element type: ") + elementSignature);
	}

	return {dimensions, element_type};
}

metaffi_type cdts_jvm_serializer::detect_type(jobject obj)
{
	if (!obj) {
		return metaffi_null_type;
	}

	// Check for wrapper types (in order of likelihood)
	if (is_instance_of(obj, "java/lang/Integer")) {
		return metaffi_int32_type;
	} else if (is_instance_of(obj, "java/lang/Long")) {
		return metaffi_int64_type;
	} else if (is_instance_of(obj, "java/lang/String")) {
		return metaffi_string8_type;
	} else if (is_instance_of(obj, "java/lang/Double")) {
		return metaffi_float64_type;
	} else if (is_instance_of(obj, "java/lang/Float")) {
		return metaffi_float32_type;
	} else if (is_instance_of(obj, "java/lang/Boolean")) {
		return metaffi_bool_type;
	} else if (is_instance_of(obj, "java/lang/Short")) {
		return metaffi_int16_type;
	} else if (is_instance_of(obj, "java/lang/Byte")) {
		return metaffi_int8_type;
	} else if (is_instance_of(obj, "java/lang/Character")) {
		return metaffi_char16_type;
	}

	// Check if it's an array
	jclass objClass = env->GetObjectClass(obj);
	if (objClass) {
		jmethodID isArray = env->GetMethodID(env->FindClass("java/lang/Class"), "isArray", "()Z");
		if (isArray) {
			jboolean isArr = env->CallBooleanMethod(objClass, isArray);
			env->DeleteLocalRef(objClass);
			if (isArr == JNI_TRUE) {
				// It's an array, but we return a generic marker
				// Actual array type determined by detect_array_info
				return metaffi_array_type;
			}
		}
		env->DeleteLocalRef(objClass);
	}

	// Fallback: treat as handle
	return metaffi_handle_type;
}

//--------------------------------------------------------------------
// Phase 3: Wrapper Handling
//--------------------------------------------------------------------

jint cdts_jvm_serializer::extract_int_from_wrapper(jobject obj)
{
	jclass integerClass = env->FindClass("java/lang/Integer");
	if (!integerClass) {
		check_jni_exception("FindClass(java/lang/Integer)");
		throw std::runtime_error("Failed to find Integer class");
	}
	local_ref_guard guard(env, integerClass);

	jmethodID intValue = env->GetMethodID(integerClass, "intValue", "()I");
	if (!intValue) {
		check_jni_exception("GetMethodID(intValue)");
		throw std::runtime_error("Failed to get intValue method");
	}

	jint result = env->CallIntMethod(obj, intValue);
	check_jni_exception("CallIntMethod(intValue)");
	return result;
}

jlong cdts_jvm_serializer::extract_long_from_wrapper(jobject obj)
{
	jclass longClass = env->FindClass("java/lang/Long");
	if (!longClass) {
		check_jni_exception("FindClass(java/lang/Long)");
		throw std::runtime_error("Failed to find Long class");
	}
	local_ref_guard guard(env, longClass);

	jmethodID longValue = env->GetMethodID(longClass, "longValue", "()J");
	if (!longValue) {
		check_jni_exception("GetMethodID(longValue)");
		throw std::runtime_error("Failed to get longValue method");
	}

	jlong result = env->CallLongMethod(obj, longValue);
	check_jni_exception("CallLongMethod(longValue)");
	return result;
}

jshort cdts_jvm_serializer::extract_short_from_wrapper(jobject obj)
{
	jclass shortClass = env->FindClass("java/lang/Short");
	if (!shortClass) {
		check_jni_exception("FindClass(java/lang/Short)");
		throw std::runtime_error("Failed to find Short class");
	}
	local_ref_guard guard(env, shortClass);

	jmethodID shortValue = env->GetMethodID(shortClass, "shortValue", "()S");
	if (!shortValue) {
		check_jni_exception("GetMethodID(shortValue)");
		throw std::runtime_error("Failed to get shortValue method");
	}

	jshort result = env->CallShortMethod(obj, shortValue);
	check_jni_exception("CallShortMethod(shortValue)");
	return result;
}

jbyte cdts_jvm_serializer::extract_byte_from_wrapper(jobject obj)
{
	jclass byteClass = env->FindClass("java/lang/Byte");
	if (!byteClass) {
		check_jni_exception("FindClass(java/lang/Byte)");
		throw std::runtime_error("Failed to find Byte class");
	}
	local_ref_guard guard(env, byteClass);

	jmethodID byteValue = env->GetMethodID(byteClass, "byteValue", "()B");
	if (!byteValue) {
		check_jni_exception("GetMethodID(byteValue)");
		throw std::runtime_error("Failed to get byteValue method");
	}

	jbyte result = env->CallByteMethod(obj, byteValue);
	check_jni_exception("CallByteMethod(byteValue)");
	return result;
}

jfloat cdts_jvm_serializer::extract_float_from_wrapper(jobject obj)
{
	jclass floatClass = env->FindClass("java/lang/Float");
	if (!floatClass) {
		check_jni_exception("FindClass(java/lang/Float)");
		throw std::runtime_error("Failed to find Float class");
	}
	local_ref_guard guard(env, floatClass);

	jmethodID floatValue = env->GetMethodID(floatClass, "floatValue", "()F");
	if (!floatValue) {
		check_jni_exception("GetMethodID(floatValue)");
		throw std::runtime_error("Failed to get floatValue method");
	}

	jfloat result = env->CallFloatMethod(obj, floatValue);
	check_jni_exception("CallFloatMethod(floatValue)");
	return result;
}

jdouble cdts_jvm_serializer::extract_double_from_wrapper(jobject obj)
{
	jclass doubleClass = env->FindClass("java/lang/Double");
	if (!doubleClass) {
		check_jni_exception("FindClass(java/lang/Double)");
		throw std::runtime_error("Failed to find Double class");
	}
	local_ref_guard guard(env, doubleClass);

	jmethodID doubleValue = env->GetMethodID(doubleClass, "doubleValue", "()D");
	if (!doubleValue) {
		check_jni_exception("GetMethodID(doubleValue)");
		throw std::runtime_error("Failed to get doubleValue method");
	}

	jdouble result = env->CallDoubleMethod(obj, doubleValue);
	check_jni_exception("CallDoubleMethod(doubleValue)");
	return result;
}

jboolean cdts_jvm_serializer::extract_boolean_from_wrapper(jobject obj)
{
	jclass booleanClass = env->FindClass("java/lang/Boolean");
	if (!booleanClass) {
		check_jni_exception("FindClass(java/lang/Boolean)");
		throw std::runtime_error("Failed to find Boolean class");
	}
	local_ref_guard guard(env, booleanClass);

	jmethodID booleanValue = env->GetMethodID(booleanClass, "booleanValue", "()Z");
	if (!booleanValue) {
		check_jni_exception("GetMethodID(booleanValue)");
		throw std::runtime_error("Failed to get booleanValue method");
	}

	jboolean result = env->CallBooleanMethod(obj, booleanValue);
	check_jni_exception("CallBooleanMethod(booleanValue)");
	return result;
}

jchar cdts_jvm_serializer::extract_char_from_wrapper(jobject obj)
{
	jclass characterClass = env->FindClass("java/lang/Character");
	if (!characterClass) {
		check_jni_exception("FindClass(java/lang/Character)");
		throw std::runtime_error("Failed to find Character class");
	}
	local_ref_guard guard(env, characterClass);

	jmethodID charValue = env->GetMethodID(characterClass, "charValue", "()C");
	if (!charValue) {
		check_jni_exception("GetMethodID(charValue)");
		throw std::runtime_error("Failed to get charValue method");
	}

	jchar result = env->CallCharMethod(obj, charValue);
	check_jni_exception("CallCharMethod(charValue)");
	return result;
}

//--------------------------------------------------------------------
// Phase 4: String Conversion
//--------------------------------------------------------------------

metaffi_string8 cdts_jvm_serializer::jstring_to_string8(jstring str)
{
	if (!str) {
		return nullptr;
	}

	const char* utf8 = env->GetStringUTFChars(str, nullptr);
	if (!utf8) {
		check_jni_exception("GetStringUTFChars");
		throw std::runtime_error("Failed to get UTF-8 characters from jstring");
	}

	// Allocate with XLLR
	metaffi_size len = std::strlen(utf8);
	metaffi_string8 result = xllr_alloc_string8((const char8_t*)utf8, len);

	env->ReleaseStringUTFChars(str, utf8);

	return result;
}

metaffi_string16 cdts_jvm_serializer::jstring_to_string16(jstring str)
{
	if (!str) {
		return nullptr;
	}

	const jchar* utf16 = env->GetStringChars(str, nullptr);
	if (!utf16) {
		check_jni_exception("GetStringChars");
		throw std::runtime_error("Failed to get UTF-16 characters from jstring");
	}

	jsize len = env->GetStringLength(str);

	// Allocate with XLLR
	metaffi_string16 result = xllr_alloc_string16((const char16_t*)utf16, len);

	env->ReleaseStringChars(str, utf16);

	return result;
}

jstring cdts_jvm_serializer::string8_to_jstring(metaffi_string8 str)
{
	if (!str) {
		return nullptr;
	}

	jstring result = env->NewStringUTF((const char*)str);
	if (!result) {
		check_jni_exception("NewStringUTF");
		throw std::runtime_error("Failed to create jstring from UTF-8");
	}

	return result;
}

jstring cdts_jvm_serializer::string16_to_jstring(metaffi_string16 str)
{
	if (!str) {
		return nullptr;
	}

	// Get length
	metaffi_size len = 0;
	while (str[len] != 0) len++;

	jstring result = env->NewString((const jchar*)str, len);
	if (!result) {
		check_jni_exception("NewString");
		throw std::runtime_error("Failed to create jstring from UTF-16");
	}

	return result;
}

jstring cdts_jvm_serializer::string32_to_jstring(metaffi_string32 str)
{
	if (!str) {
		return nullptr;
	}

	// Convert UTF-32 to UTF-16 (simplified - doesn't handle surrogates properly)
	// For now, truncate to 16-bit (this is a limitation)
	metaffi_size len = 0;
	while (str[len] != 0) len++;

	std::vector<jchar> utf16;
	utf16.reserve(len);

	for (metaffi_size i = 0; i < len; i++) {
		if (str[i] <= 0xFFFF) {
			utf16.push_back((jchar)str[i]);
		} else {
			// Surrogate pair encoding for characters > 0xFFFF
			char32_t codepoint = str[i];
			codepoint -= 0x10000;
			utf16.push_back((jchar)(0xD800 + (codepoint >> 10)));
			utf16.push_back((jchar)(0xDC00 + (codepoint & 0x3FF)));
		}
	}

	jstring result = env->NewString(utf16.data(), utf16.size());
	if (!result) {
		check_jni_exception("NewString");
		throw std::runtime_error("Failed to create jstring from UTF-32");
	}

	return result;
}

//--------------------------------------------------------------------
// Phase 5: Primitive Serialization/Deserialization
//--------------------------------------------------------------------

cdts_jvm_serializer& cdts_jvm_serializer::add(jbyte val, metaffi_type target_type)
{
	check_bounds(current_index);
	// For uint8/char8, interpret jbyte as unsigned for validation
	if (target_type == metaffi_uint8_type || target_type == metaffi_char8_type) {
		validate_byte_range(static_cast<jlong>(static_cast<uint8_t>(val)), target_type);
	} else {
		validate_byte_range(static_cast<jlong>(val), target_type);
	}

	data[current_index].type = target_type;
	data[current_index].free_required = false;

	switch(target_type) {
		case metaffi_int8_type:
			data[current_index].cdt_val.int8_val = static_cast<metaffi_int8>(val);
			break;
		case metaffi_uint8_type:
			data[current_index].cdt_val.uint8_val = static_cast<metaffi_uint8>(static_cast<uint8_t>(val));
			break;
		case metaffi_char8_type:
		{
			// jbyte is signed (-128 to 127), convert to unsigned char8_t
			char8_t utf8_char[4] = {static_cast<char8_t>(static_cast<uint8_t>(val)), u8'\0', u8'\0', u8'\0'};
			data[current_index].cdt_val.char8_val = metaffi_char8(utf8_char);
			break;
		}
		default:
			throw std::runtime_error("Invalid target type for jbyte");
	}

	current_index++;
	return *this;
}

cdts_jvm_serializer& cdts_jvm_serializer::add(jshort val, metaffi_type target_type)
{
	check_bounds(current_index);
	// For uint16, interpret jshort as unsigned for validation
	if (target_type == metaffi_uint16_type) {
		validate_short_range(static_cast<jlong>(static_cast<uint16_t>(val)), target_type);
	} else {
		validate_short_range(static_cast<jlong>(val), target_type);
	}

	data[current_index].type = target_type;
	data[current_index].free_required = false;

	switch(target_type) {
		case metaffi_int16_type:
			data[current_index].cdt_val.int16_val = static_cast<metaffi_int16>(val);
			break;
		case metaffi_uint16_type:
			data[current_index].cdt_val.uint16_val = static_cast<metaffi_uint16>(static_cast<uint16_t>(val));
			break;
		default:
			throw std::runtime_error("Invalid target type for jshort");
	}

	current_index++;
	return *this;
}

cdts_jvm_serializer& cdts_jvm_serializer::add(jint val, metaffi_type target_type)
{
	check_bounds(current_index);
	// For uint32, interpret jint as unsigned for validation
	if (target_type == metaffi_uint32_type) {
		validate_int_range(static_cast<jlong>(static_cast<uint32_t>(val)), target_type);
	} else {
		validate_int_range(static_cast<jlong>(val), target_type);
	}

	data[current_index].type = target_type;
	data[current_index].free_required = false;

	switch(target_type) {
		case metaffi_int32_type:
			data[current_index].cdt_val.int32_val = static_cast<metaffi_int32>(val);
			break;
		case metaffi_uint32_type:
			data[current_index].cdt_val.uint32_val = static_cast<metaffi_uint32>(static_cast<uint32_t>(val));
			break;
		case metaffi_char32_type:
			data[current_index].cdt_val.char32_val = static_cast<metaffi_char32>(val);
			break;
		default:
			throw std::runtime_error("Invalid target type for jint");
	}

	current_index++;
	return *this;
}

cdts_jvm_serializer& cdts_jvm_serializer::add(jlong val, metaffi_type target_type)
{
	check_bounds(current_index);
	// jlong can represent any uint64 value when reinterpreted as unsigned
	// No validation needed - any jlong value is valid for both int64 and uint64
	// (negative jlong values represent high uint64 values when cast to unsigned)

	data[current_index].type = target_type;
	data[current_index].free_required = false;

	switch(target_type) {
		case metaffi_int64_type:
			data[current_index].cdt_val.int64_val = static_cast<metaffi_int64>(val);
			break;
		case metaffi_uint64_type:
			data[current_index].cdt_val.uint64_val = static_cast<metaffi_uint64>(static_cast<uint64_t>(val));
			break;
		default:
			throw std::runtime_error("Invalid target type for jlong");
	}

	current_index++;
	return *this;
}

cdts_jvm_serializer& cdts_jvm_serializer::add(jfloat val, metaffi_type target_type)
{
	check_bounds(current_index);

	if (target_type != metaffi_float32_type) {
		throw std::runtime_error("Invalid target type for jfloat (must be metaffi_float32_type)");
	}

	data[current_index].type = metaffi_float32_type;
	data[current_index].cdt_val.float32_val = static_cast<metaffi_float32>(val);
	data[current_index].free_required = false;

	current_index++;
	return *this;
}

cdts_jvm_serializer& cdts_jvm_serializer::add(jdouble val, metaffi_type target_type)
{
	check_bounds(current_index);

	if (target_type != metaffi_float64_type) {
		throw std::runtime_error("Invalid target type for jdouble (must be metaffi_float64_type)");
	}

	data[current_index].type = metaffi_float64_type;
	data[current_index].cdt_val.float64_val = static_cast<metaffi_float64>(val);
	data[current_index].free_required = false;

	current_index++;
	return *this;
}

cdts_jvm_serializer& cdts_jvm_serializer::add(jboolean val, metaffi_type target_type)
{
	check_bounds(current_index);

	data[current_index].type = metaffi_bool_type;
	data[current_index].cdt_val.bool_val = (val == JNI_TRUE);
	data[current_index].free_required = false;

	current_index++;
	return *this;
}

cdts_jvm_serializer& cdts_jvm_serializer::add(jchar val, metaffi_type target_type)
{
	check_bounds(current_index);
	validate_char_range(val, target_type);

	data[current_index].type = target_type;
	data[current_index].free_required = false;

	switch(target_type) {
		case metaffi_char8_type:
		{
			// jchar is 16-bit UTF-16, convert to UTF-8
			// For values < 256, use single-byte UTF-8 encoding
			if (val > 255) {
				throw std::runtime_error("jchar value > 255 cannot be converted to char8");
			}
			char8_t utf8_char[4] = {static_cast<char8_t>(val), u8'\0', u8'\0', u8'\0'};
			data[current_index].cdt_val.char8_val = metaffi_char8(utf8_char);
			break;
		}
		case metaffi_char16_type:
		{
			// jchar is already 16-bit, create char16_t array
			char16_t utf16_char[2] = {static_cast<char16_t>(val), u'\0'};
			data[current_index].cdt_val.char16_val = metaffi_char16(utf16_char);
			break;
		}
		case metaffi_char32_type:
		{
			// Use char32_t constructor directly
			data[current_index].cdt_val.char32_val = metaffi_char32(static_cast<char32_t>(val));
			break;
		}
		default:
			throw std::runtime_error("Invalid target type for jchar");
	}

	current_index++;
	return *this;
}

// Extraction methods

jbyte cdts_jvm_serializer::extract_byte()
{
	check_bounds(current_index);

	cdt& current = data[current_index];
	jbyte result;

	switch(current.type) {
		case metaffi_int8_type:
			result = static_cast<jbyte>(current.cdt_val.int8_val);
			break;
		case metaffi_uint8_type:
			result = static_cast<jbyte>(current.cdt_val.uint8_val);
			break;
		case metaffi_char8_type:
			// Extract first byte from char8 array
			result = static_cast<jbyte>(current.cdt_val.char8_val.c[0]);
			break;
		default:
			throw std::runtime_error("Type mismatch: expected int8/uint8/char8");
	}

	current_index++;
	return result;
}

jshort cdts_jvm_serializer::extract_short()
{
	check_bounds(current_index);

	cdt& current = data[current_index];
	jshort result;

	switch(current.type) {
		case metaffi_int16_type:
			result = static_cast<jshort>(current.cdt_val.int16_val);
			break;
		case metaffi_uint16_type:
			result = static_cast<jshort>(current.cdt_val.uint16_val);
			break;
		default:
			throw std::runtime_error("Type mismatch: expected int16/uint16");
	}

	current_index++;
	return result;
}

jint cdts_jvm_serializer::extract_int()
{
	check_bounds(current_index);

	cdt& current = data[current_index];
	jint result;

	switch(current.type) {
		case metaffi_int32_type:
			result = static_cast<jint>(current.cdt_val.int32_val);
			break;
		case metaffi_uint32_type:
			result = static_cast<jint>(current.cdt_val.uint32_val);
			break;
		case metaffi_char32_type:
			// Extract char32_t value from struct
			result = static_cast<jint>(current.cdt_val.char32_val.c);
			break;
		default:
			throw std::runtime_error("Type mismatch: expected int32/uint32/char32");
	}

	current_index++;
	return result;
}

jlong cdts_jvm_serializer::extract_long()
{
	check_bounds(current_index);

	cdt& current = data[current_index];
	jlong result;

	switch(current.type) {
		case metaffi_int64_type:
			result = static_cast<jlong>(current.cdt_val.int64_val);
			break;
		case metaffi_uint64_type:
			result = static_cast<jlong>(current.cdt_val.uint64_val);
			break;
		default:
			throw std::runtime_error("Type mismatch: expected int64/uint64");
	}

	current_index++;
	return result;
}

jfloat cdts_jvm_serializer::extract_float()
{
	check_bounds(current_index);

	cdt& current = data[current_index];

	if (current.type != metaffi_float32_type) {
		throw std::runtime_error("Type mismatch: expected float32");
	}

	jfloat result = static_cast<jfloat>(current.cdt_val.float32_val);
	current_index++;
	return result;
}

jdouble cdts_jvm_serializer::extract_double()
{
	check_bounds(current_index);

	cdt& current = data[current_index];

	if (current.type != metaffi_float64_type) {
		throw std::runtime_error("Type mismatch: expected float64");
	}

	jdouble result = static_cast<jdouble>(current.cdt_val.float64_val);
	current_index++;
	return result;
}

jboolean cdts_jvm_serializer::extract_boolean()
{
	check_bounds(current_index);

	cdt& current = data[current_index];

	if (current.type != metaffi_bool_type) {
		throw std::runtime_error("Type mismatch: expected bool");
	}

	jboolean result = current.cdt_val.bool_val ? JNI_TRUE : JNI_FALSE;
	current_index++;
	return result;
}

jchar cdts_jvm_serializer::extract_char()
{
	check_bounds(current_index);

	cdt& current = data[current_index];
	jchar result;

	switch(current.type) {
		case metaffi_char8_type:
			// Extract first byte and zero-extend to jchar
			result = static_cast<jchar>(static_cast<uint8_t>(current.cdt_val.char8_val.c[0]));
			break;
		case metaffi_char16_type:
			// Extract first char16_t from array
			result = static_cast<jchar>(current.cdt_val.char16_val.c[0]);
			break;
		case metaffi_char32_type:
			// Truncate to 16-bit (may lose data)
			result = static_cast<jchar>(current.cdt_val.char32_val.c & 0xFFFF);
			break;
		default:
			throw std::runtime_error("Type mismatch: expected char8/char16/char32");
	}

	current_index++;
	return result;
}

jstring cdts_jvm_serializer::extract_string()
{
	check_bounds(current_index);

	cdt& current = data[current_index];
	jstring result;

	switch(current.type) {
		case metaffi_string8_type:
			result = string8_to_jstring(current.cdt_val.string8_val);
			break;
		case metaffi_string16_type:
			result = string16_to_jstring(current.cdt_val.string16_val);
			break;
		case metaffi_string32_type:
			result = string32_to_jstring(current.cdt_val.string32_val);
			break;
		default:
			throw std::runtime_error("Type mismatch: expected string8/string16/string32");
	}

	current_index++;
	return result;
}

//--------------------------------------------------------------------
// Phase 6: Array Handling
//--------------------------------------------------------------------

void cdts_jvm_serializer::serialize_array(jarray arr, int dimensions, metaffi_type element_type)
{
	check_bounds(current_index);

	if (!arr) {
		data[current_index].type = metaffi_null_type;
		data[current_index].free_required = false;
		current_index++;
		return;
	}

	// Get array length
	jsize length = env->GetArrayLength(arr);
	check_jni_exception("GetArrayLength");

	// For 1D arrays, convert directly
	if (dimensions == 1) {
		// Allocate CDTS array using set_new_array
		data[current_index].set_new_array(length, dimensions, static_cast<metaffi_types>(element_type));
		cdts& arr_cdts = *data[current_index].cdt_val.array_val;

		// Copy elements based on type
		switch(element_type) {
			case metaffi_int8_type: {
				jbyteArray byteArr = (jbyteArray)arr;
				jbyte* elements = env->GetByteArrayElements(byteArr, nullptr);
				for (jsize i = 0; i < length; i++) {
					arr_cdts[i].cdt_val.int8_val = elements[i];
					arr_cdts[i].type = metaffi_int8_type;
				}
				env->ReleaseByteArrayElements(byteArr, elements, JNI_ABORT);
				break;
			}
			case metaffi_int16_type: {
				jshortArray shortArr = (jshortArray)arr;
				jshort* elements = env->GetShortArrayElements(shortArr, nullptr);
				for (jsize i = 0; i < length; i++) {
					arr_cdts[i].cdt_val.int16_val = elements[i];
					arr_cdts[i].type = metaffi_int16_type;
				}
				env->ReleaseShortArrayElements(shortArr, elements, JNI_ABORT);
				break;
			}
			case metaffi_int32_type: {
				jintArray intArr = (jintArray)arr;
				jint* elements = env->GetIntArrayElements(intArr, nullptr);
				for (jsize i = 0; i < length; i++) {
					arr_cdts[i].cdt_val.int32_val = elements[i];
					arr_cdts[i].type = metaffi_int32_type;
				}
				env->ReleaseIntArrayElements(intArr, elements, JNI_ABORT);
				break;
			}
			case metaffi_int64_type: {
				jlongArray longArr = (jlongArray)arr;
				jlong* elements = env->GetLongArrayElements(longArr, nullptr);
				for (jsize i = 0; i < length; i++) {
					arr_cdts[i].cdt_val.int64_val = elements[i];
					arr_cdts[i].type = metaffi_int64_type;
				}
				env->ReleaseLongArrayElements(longArr, elements, JNI_ABORT);
				break;
			}
			case metaffi_float32_type: {
				jfloatArray floatArr = (jfloatArray)arr;
				jfloat* elements = env->GetFloatArrayElements(floatArr, nullptr);
				for (jsize i = 0; i < length; i++) {
					arr_cdts[i].cdt_val.float32_val = elements[i];
					arr_cdts[i].type = metaffi_float32_type;
				}
				env->ReleaseFloatArrayElements(floatArr, elements, JNI_ABORT);
				break;
			}
			case metaffi_float64_type: {
				jdoubleArray doubleArr = (jdoubleArray)arr;
				jdouble* elements = env->GetDoubleArrayElements(doubleArr, nullptr);
				for (jsize i = 0; i < length; i++) {
					arr_cdts[i].cdt_val.float64_val = elements[i];
					arr_cdts[i].type = metaffi_float64_type;
				}
				env->ReleaseDoubleArrayElements(doubleArr, elements, JNI_ABORT);
				break;
			}
			case metaffi_bool_type: {
				jbooleanArray boolArr = (jbooleanArray)arr;
				jboolean* elements = env->GetBooleanArrayElements(boolArr, nullptr);
				for (jsize i = 0; i < length; i++) {
					arr_cdts[i].cdt_val.bool_val = (elements[i] == JNI_TRUE);
					arr_cdts[i].type = metaffi_bool_type;
				}
				env->ReleaseBooleanArrayElements(boolArr, elements, JNI_ABORT);
				break;
			}
			case metaffi_char16_type: {
				jcharArray charArr = (jcharArray)arr;
				jchar* elements = env->GetCharArrayElements(charArr, nullptr);
				for (jsize i = 0; i < length; i++) {
					char16_t utf16_char[2] = {static_cast<char16_t>(elements[i]), u'\0'};
					arr_cdts[i].cdt_val.char16_val = metaffi_char16(utf16_char);
					arr_cdts[i].type = metaffi_char16_type;
				}
				env->ReleaseCharArrayElements(charArr, elements, JNI_ABORT);
				break;
			}
			case metaffi_string8_type: {
				// Object array of Strings
				jobjectArray objArr = (jobjectArray)arr;
				for (jsize i = 0; i < length; i++) {
					jstring jstr = (jstring)env->GetObjectArrayElement(objArr, i);
					if (jstr) {
						arr_cdts[i].cdt_val.string8_val = jstring_to_string8(jstr);
						arr_cdts[i].type = metaffi_string8_type;
						arr_cdts[i].free_required = true;
						env->DeleteLocalRef(jstr);
					} else {
						arr_cdts[i].type = metaffi_null_type;
					}
				}
				break;
			}
			default:
				// For wrapper types or other objects - extract from object array
				jobjectArray objArr = (jobjectArray)arr;
				for (jsize i = 0; i < length; i++) {
					jobject obj = env->GetObjectArrayElement(objArr, i);
					if (obj) {
						// Extract value based on element type
						switch(element_type) {
							case metaffi_int32_type:
								arr_cdts[i].cdt_val.int32_val = extract_int_from_wrapper(obj);
								break;
							case metaffi_int64_type:
								arr_cdts[i].cdt_val.int64_val = extract_long_from_wrapper(obj);
								break;
							case metaffi_int16_type:
								arr_cdts[i].cdt_val.int16_val = extract_short_from_wrapper(obj);
								break;
							case metaffi_int8_type:
								arr_cdts[i].cdt_val.int8_val = extract_byte_from_wrapper(obj);
								break;
							case metaffi_float32_type:
								arr_cdts[i].cdt_val.float32_val = extract_float_from_wrapper(obj);
								break;
							case metaffi_float64_type:
								arr_cdts[i].cdt_val.float64_val = extract_double_from_wrapper(obj);
								break;
							case metaffi_bool_type:
								arr_cdts[i].cdt_val.bool_val = (extract_boolean_from_wrapper(obj) == JNI_TRUE);
								break;
							case metaffi_char16_type: {
								jchar ch = extract_char_from_wrapper(obj);
								char16_t utf16_char[2] = {static_cast<char16_t>(ch), u'\0'};
								arr_cdts[i].cdt_val.char16_val = metaffi_char16(utf16_char);
								break;
							}
							default:
								// Store as handle
								arr_cdts[i].cdt_val.handle_val = new cdt_metaffi_handle();
								arr_cdts[i].cdt_val.handle_val->handle = env->NewGlobalRef(obj);
								arr_cdts[i].cdt_val.handle_val->runtime_id = 3; // JVM runtime ID
								arr_cdts[i].free_required = true;
								break;
						}
						arr_cdts[i].type = element_type;
						env->DeleteLocalRef(obj);
					} else {
						arr_cdts[i].type = metaffi_null_type;
					}
				}
				break;
		}

		// Array is already set up by set_new_array() above
		current_index++;
	} else {
		// Multi-dimensional array - recursive
		throw std::runtime_error("Multi-dimensional arrays not yet implemented");
	}
}

jarray cdts_jvm_serializer::create_primitive_array(cdts& arr_cdts, metaffi_type element_type)
{
	metaffi_size length = arr_cdts.length;

	switch(element_type) {
		case metaffi_int8_type: {
			jbyteArray result = env->NewByteArray(length);
			std::vector<jbyte> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.int8_val;
			}
			env->SetByteArrayRegion(result, 0, length, elements.data());
			return result;
		}
		case metaffi_int16_type: {
			jshortArray result = env->NewShortArray(length);
			std::vector<jshort> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.int16_val;
			}
			env->SetShortArrayRegion(result, 0, length, elements.data());
			return result;
		}
		case metaffi_int32_type: {
			jintArray result = env->NewIntArray(length);
			std::vector<jint> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.int32_val;
			}
			env->SetIntArrayRegion(result, 0, length, elements.data());
			return result;
		}
		case metaffi_int64_type: {
			jlongArray result = env->NewLongArray(length);
			std::vector<jlong> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.int64_val;
			}
			env->SetLongArrayRegion(result, 0, length, elements.data());
			return result;
		}
		case metaffi_float32_type: {
			jfloatArray result = env->NewFloatArray(length);
			std::vector<jfloat> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.float32_val;
			}
			env->SetFloatArrayRegion(result, 0, length, elements.data());
			return result;
		}
		case metaffi_float64_type: {
			jdoubleArray result = env->NewDoubleArray(length);
			std::vector<jdouble> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.float64_val;
			}
			env->SetDoubleArrayRegion(result, 0, length, elements.data());
			return result;
		}
		case metaffi_bool_type: {
			jbooleanArray result = env->NewBooleanArray(length);
			std::vector<jboolean> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.bool_val ? JNI_TRUE : JNI_FALSE;
			}
			env->SetBooleanArrayRegion(result, 0, length, elements.data());
			return result;
		}
		case metaffi_char16_type: {
			jcharArray result = env->NewCharArray(length);
			std::vector<jchar> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = static_cast<jchar>(arr_cdts[i].cdt_val.char16_val.c[0]);
			}
			env->SetCharArrayRegion(result, 0, length, elements.data());
			return result;
		}
		default:
			throw std::runtime_error("Unsupported primitive array element type");
	}
}

jobjectArray cdts_jvm_serializer::create_object_array(cdts& arr_cdts, metaffi_type element_type)
{
	metaffi_size length = arr_cdts.length;

	// Determine element class
	const char* className = nullptr;
	switch(element_type) {
		case metaffi_int8_type: className = "java/lang/Byte"; break;
		case metaffi_int16_type: className = "java/lang/Short"; break;
		case metaffi_int32_type: className = "java/lang/Integer"; break;
		case metaffi_int64_type: className = "java/lang/Long"; break;
		case metaffi_float32_type: className = "java/lang/Float"; break;
		case metaffi_float64_type: className = "java/lang/Double"; break;
		case metaffi_bool_type: className = "java/lang/Boolean"; break;
		case metaffi_char16_type: className = "java/lang/Character"; break;
		case metaffi_string8_type: className = "java/lang/String"; break;
		default: className = "java/lang/Object"; break;
	}

	jclass elementClass = env->FindClass(className);
	if (!elementClass) {
		check_jni_exception("FindClass");
		throw std::runtime_error("Failed to find element class");
	}
	local_ref_guard guard(env, elementClass);

	jobjectArray result = env->NewObjectArray(length, elementClass, nullptr);
	if (!result) {
		check_jni_exception("NewObjectArray");
		throw std::runtime_error("Failed to create object array");
	}

	// Fill array
	for (metaffi_size i = 0; i < length; i++) {
		jobject element = nullptr;

		if (arr_cdts[i].type != metaffi_null_type) {
			switch(element_type) {
				case metaffi_int32_type: {
					jclass cls = env->FindClass("java/lang/Integer");
					jmethodID constructor = env->GetMethodID(cls, "<init>", "(I)V");
					element = env->NewObject(cls, constructor, arr_cdts[i].cdt_val.int32_val);
					env->DeleteLocalRef(cls);
					break;
				}
				case metaffi_int64_type: {
					jclass cls = env->FindClass("java/lang/Long");
					jmethodID constructor = env->GetMethodID(cls, "<init>", "(J)V");
					element = env->NewObject(cls, constructor, arr_cdts[i].cdt_val.int64_val);
					env->DeleteLocalRef(cls);
					break;
				}
				case metaffi_float64_type: {
					jclass cls = env->FindClass("java/lang/Double");
					jmethodID constructor = env->GetMethodID(cls, "<init>", "(D)V");
					element = env->NewObject(cls, constructor, arr_cdts[i].cdt_val.float64_val);
					env->DeleteLocalRef(cls);
					break;
				}
				case metaffi_string8_type: {
					element = string8_to_jstring(arr_cdts[i].cdt_val.string8_val);
					break;
				}
				// Add more types as needed
				default:
					break;
			}
		}

		env->SetObjectArrayElement(result, i, element);
		if (element) env->DeleteLocalRef(element);
	}

	return result;
}

jarray cdts_jvm_serializer::extract_array()
{
	check_bounds(current_index);

	cdt& current = data[current_index];

	if (!(current.type & metaffi_array_type)) {
		throw std::runtime_error("Type mismatch: expected array");
	}

	cdts* arr_cdts = current.cdt_val.array_val;
	if (!arr_cdts || arr_cdts->length == 0) {
		current_index++;
		return nullptr;
	}

	// Determine element type from first element
	metaffi_type element_type = (*arr_cdts)[0].type;

	// Check if primitive array
	bool is_primitive = false;
	switch(element_type) {
		case metaffi_int8_type:
		case metaffi_int16_type:
		case metaffi_int32_type:
		case metaffi_int64_type:
		case metaffi_float32_type:
		case metaffi_float64_type:
		case metaffi_bool_type:
		case metaffi_char16_type:
			is_primitive = true;
			break;
	}

	jarray result;
	if (is_primitive) {
		result = create_primitive_array(*arr_cdts, element_type);
	} else {
		result = (jarray)create_object_array(*arr_cdts, element_type);
	}

	current_index++;
	return result;
}

//--------------------------------------------------------------------
// Phase 7: Handle and Callable
//--------------------------------------------------------------------

jobject cdts_jvm_serializer::extract_handle()
{
	check_bounds(current_index);

	cdt& current = data[current_index];

	if (current.type != metaffi_handle_type && current.type != metaffi_callable_type) {
		throw std::runtime_error("Type mismatch: expected handle or callable");
	}

	jobject result = nullptr;

	if (current.type == metaffi_handle_type) {
		if (current.cdt_val.handle_val && current.cdt_val.handle_val->handle) {
			result = (jobject)current.cdt_val.handle_val->handle;
		}
	} else if (current.type == metaffi_callable_type) {
		if (current.cdt_val.callable_val && current.cdt_val.callable_val->val) {
			result = (jobject)current.cdt_val.callable_val->val;
		}
	}

	current_index++;
	return result;
}

//--------------------------------------------------------------------
// Phase 8: Generic jobject Operator
//--------------------------------------------------------------------

cdts_jvm_serializer& cdts_jvm_serializer::operator<<(jobject val)
{
	check_bounds(current_index);

	if (!val) {
		return null();
	}

	// Detect type
	metaffi_type detected_type = detect_type(val);

	// Handle based on detected type
	switch(detected_type) {
		case metaffi_int8_type: {
			jbyte value = extract_byte_from_wrapper(val);
			data[current_index].type = metaffi_int8_type;
			data[current_index].cdt_val.int8_val = value;
			data[current_index].free_required = false;
			break;
		}
		case metaffi_int16_type: {
			jshort value = extract_short_from_wrapper(val);
			data[current_index].type = metaffi_int16_type;
			data[current_index].cdt_val.int16_val = value;
			data[current_index].free_required = false;
			break;
		}
		case metaffi_int32_type: {
			jint value = extract_int_from_wrapper(val);
			data[current_index].type = metaffi_int32_type;
			data[current_index].cdt_val.int32_val = value;
			data[current_index].free_required = false;
			break;
		}
		case metaffi_int64_type: {
			jlong value = extract_long_from_wrapper(val);
			data[current_index].type = metaffi_int64_type;
			data[current_index].cdt_val.int64_val = value;
			data[current_index].free_required = false;
			break;
		}
		case metaffi_float32_type: {
			jfloat value = extract_float_from_wrapper(val);
			data[current_index].type = metaffi_float32_type;
			data[current_index].cdt_val.float32_val = value;
			data[current_index].free_required = false;
			break;
		}
		case metaffi_float64_type: {
			jdouble value = extract_double_from_wrapper(val);
			data[current_index].type = metaffi_float64_type;
			data[current_index].cdt_val.float64_val = value;
			data[current_index].free_required = false;
			break;
		}
		case metaffi_bool_type: {
			jboolean value = extract_boolean_from_wrapper(val);
			data[current_index].type = metaffi_bool_type;
			data[current_index].cdt_val.bool_val = (value == JNI_TRUE);
			data[current_index].free_required = false;
			break;
		}
		case metaffi_char16_type: {
			jchar value = extract_char_from_wrapper(val);
			data[current_index].type = metaffi_char16_type;
			data[current_index].cdt_val.char16_val.c[0] = static_cast<char16_t>(value);
			data[current_index].cdt_val.char16_val.c[1] = 0; // Null terminate
			data[current_index].free_required = false;
			break;
		}
		case metaffi_string8_type: {
			jstring str = (jstring)val;
			data[current_index].cdt_val.string8_val = jstring_to_string8(str);
			data[current_index].type = metaffi_string8_type;
			data[current_index].free_required = true;
			break;
		}
		case metaffi_array_type: {
			// Detect array info and serialize
			auto [dimensions, element_type] = detect_array_info((jarray)val);
			serialize_array((jarray)val, dimensions, element_type);
			return *this; // serialize_array already increments index
		}
		case metaffi_handle_type:
		default: {
			// Store as handle with global reference
			data[current_index].type = metaffi_handle_type;
			data[current_index].cdt_val.handle_val = new cdt_metaffi_handle();
			data[current_index].cdt_val.handle_val->handle = env->NewGlobalRef(val);
			data[current_index].cdt_val.handle_val->runtime_id = 3; // JVM runtime ID
			data[current_index].cdt_val.handle_val->release = nullptr;
			data[current_index].free_required = true;
			break;
		}
	}

	current_index++;
	return *this;
}

} // namespace metaffi::utils
