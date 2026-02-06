#include "cdts_jvm_serializer.h"
#include "runtime_id.h"
#include <runtime/xllr_capi_loader.h>
#include <algorithm>
#include <cstring>

namespace metaffi::utils
{
namespace
{
	struct metaffi_handle_cache
	{
		jclass cls = nullptr;
		jmethodID ctor = nullptr;
		jmethodID get_handle = nullptr;
		jmethodID get_runtime_id = nullptr;
		jmethodID get_releaser = nullptr;
	};

	struct caller_cache
	{
		jclass cls = nullptr;
		jmethodID create_caller = nullptr;
		jfieldID xcall_field = nullptr;
		jfieldID params_field = nullptr;
		jfieldID retvals_field = nullptr;
	};

	struct big_integer_cache
	{
		jclass cls = nullptr;
		jmethodID ctor = nullptr;
		jmethodID bit_length = nullptr;
		jmethodID long_value = nullptr;
		jfieldID zero_field = nullptr;
	};

	struct alias_array_info
	{
		std::string base;
		int dims = 0;
	};

	alias_array_info parse_alias_array(const char* alias)
	{
		alias_array_info info;
		if(!alias || !*alias)
		{
			return info;
		}
		info.base = alias;
		size_t pos = std::string::npos;
		while((pos = info.base.find("[]")) != std::string::npos)
		{
			info.base.erase(pos, 2);
			info.dims++;
		}
		return info;
	}

	std::string to_internal_name(std::string name)
	{
		std::replace(name.begin(), name.end(), '.', '/');
		return name;
	}

	metaffi_handle_cache& get_handle_cache(JNIEnv* env)
	{
		static metaffi_handle_cache cache;
		if(!cache.cls)
		{
			jclass tmp = env->FindClass("metaffi/api/accessor/MetaFFIHandle");
			if(!tmp)
			{
				throw std::runtime_error("Failed to find MetaFFIHandle class");
			}
			cache.cls = (jclass)env->NewGlobalRef(tmp);
			env->DeleteLocalRef(tmp);
		}
		if(!cache.get_handle)
		{
			cache.get_handle = env->GetMethodID(cache.cls, "Handle", "()J");
			if(!cache.get_handle)
			{
				throw std::runtime_error("Failed to get MetaFFIHandle.Handle()");
			}
		}
		if(!cache.get_runtime_id)
		{
			cache.get_runtime_id = env->GetMethodID(cache.cls, "RuntimeID", "()J");
			if(!cache.get_runtime_id)
			{
				throw std::runtime_error("Failed to get MetaFFIHandle.RuntimeID()");
			}
		}
		if(!cache.get_releaser)
		{
			cache.get_releaser = env->GetMethodID(cache.cls, "Releaser", "()J");
			if(!cache.get_releaser)
			{
				throw std::runtime_error("Failed to get MetaFFIHandle.Releaser()");
			}
		}
		if(!cache.ctor)
		{
			cache.ctor = env->GetMethodID(cache.cls, "<init>", "(JJJ)V");
			if(!cache.ctor)
			{
				throw std::runtime_error("Failed to get MetaFFIHandle constructor");
			}
		}
		return cache;
	}

	bool is_metaffi_handle(JNIEnv* env, jobject obj)
	{
		if(!obj)
		{
			return false;
		}
		auto& cache = get_handle_cache(env);
		return env->IsInstanceOf(obj, cache.cls) == JNI_TRUE;
	}

	cdt_metaffi_handle extract_metaffi_handle(JNIEnv* env, jobject obj)
	{
		auto& cache = get_handle_cache(env);
		cdt_metaffi_handle out{};
		out.handle = (void*)env->CallLongMethod(obj, cache.get_handle);
		out.runtime_id = (uint64_t)env->CallLongMethod(obj, cache.get_runtime_id);
		out.release = (releaser_fptr_t)env->CallLongMethod(obj, cache.get_releaser);
		return out;
	}

	jobject create_metaffi_handle_object(JNIEnv* env, const cdt_metaffi_handle& handle)
	{
		auto& cache = get_handle_cache(env);
		jobject obj = env->NewObject(cache.cls, cache.ctor,
		                             (jlong)handle.handle,
		                             (jlong)handle.runtime_id,
		                             (jlong)handle.release);
		if(!obj)
		{
			throw std::runtime_error("Failed to create MetaFFIHandle object");
		}
		return obj;
	}

	caller_cache& get_caller_cache(JNIEnv* env)
	{
		static caller_cache cache;
		if(!cache.cls)
		{
			jclass tmp = env->FindClass("metaffi/api/accessor/Caller");
			if(!tmp)
			{
				throw std::runtime_error("Failed to find Caller class");
			}
			cache.cls = (jclass)env->NewGlobalRef(tmp);
			env->DeleteLocalRef(tmp);
		}
		if(!cache.create_caller)
		{
			cache.create_caller = env->GetStaticMethodID(cache.cls, "createCaller", "(J[J[J)Lmetaffi/api/accessor/Caller;");
			if(!cache.create_caller)
			{
				throw std::runtime_error("Failed to get Caller.createCaller");
			}
		}
		if(!cache.xcall_field)
		{
			cache.xcall_field = env->GetFieldID(cache.cls, "xcallAndContext", "J");
			if(!cache.xcall_field)
			{
				throw std::runtime_error("Failed to get Caller.xcallAndContext field");
			}
		}
		if(!cache.params_field)
		{
			cache.params_field = env->GetFieldID(cache.cls, "parametersTypesArray", "[J");
			if(!cache.params_field)
			{
				throw std::runtime_error("Failed to get Caller.parametersTypesArray field");
			}
		}
		if(!cache.retvals_field)
		{
			cache.retvals_field = env->GetFieldID(cache.cls, "retvalsTypesArray", "[J");
			if(!cache.retvals_field)
			{
				throw std::runtime_error("Failed to get Caller.retvalsTypesArray field");
			}
		}
		return cache;
	}

	bool is_caller(JNIEnv* env, jobject obj)
	{
		if(!obj)
		{
			return false;
		}
		auto& cache = get_caller_cache(env);
		return env->IsInstanceOf(obj, cache.cls) == JNI_TRUE;
	}

	cdt_metaffi_callable* extract_caller(JNIEnv* env, jobject caller_obj)
	{
		auto& cache = get_caller_cache(env);

		jlong xcall_and_context = env->GetLongField(caller_obj, cache.xcall_field);
		jlongArray params_array = (jlongArray)env->GetObjectField(caller_obj, cache.params_field);
		jlongArray retvals_array = (jlongArray)env->GetObjectField(caller_obj, cache.retvals_field);

		jsize params_len = params_array ? env->GetArrayLength(params_array) : 0;
		jsize retvals_len = retvals_array ? env->GetArrayLength(retvals_array) : 0;

		metaffi_type* params_types = nullptr;
		metaffi_type* retvals_types = nullptr;

		if(params_len > 0)
		{
			void* mem = xllr_alloc_memory(sizeof(metaffi_type) * params_len);
			if(!mem)
			{
				throw std::runtime_error("Failed to allocate parameters_types array");
			}
			params_types = static_cast<metaffi_type*>(mem);

			jlong* elements = env->GetLongArrayElements(params_array, nullptr);
			if(!elements)
			{
				throw std::runtime_error("Failed to get parametersTypesArray elements");
			}
			std::memcpy(params_types, elements, sizeof(metaffi_type) * params_len);
			env->ReleaseLongArrayElements(params_array, elements, 0);
		}

		if(retvals_len > 0)
		{
			void* mem = xllr_alloc_memory(sizeof(metaffi_type) * retvals_len);
			if(!mem)
			{
				throw std::runtime_error("Failed to allocate retvals_types array");
			}
			retvals_types = static_cast<metaffi_type*>(mem);

			jlong* elements = env->GetLongArrayElements(retvals_array, nullptr);
			if(!elements)
			{
				throw std::runtime_error("Failed to get retvalsTypesArray elements");
			}
			std::memcpy(retvals_types, elements, sizeof(metaffi_type) * retvals_len);
			env->ReleaseLongArrayElements(retvals_array, elements, 0);
		}

		void* callable_mem = xllr_alloc_memory(sizeof(cdt_metaffi_callable));
		if(!callable_mem)
		{
			throw std::runtime_error("Failed to allocate cdt_metaffi_callable");
		}

		auto* callable = new (callable_mem) cdt_metaffi_callable();
		callable->val = (metaffi_callable)xcall_and_context;
		callable->parameters_types = params_types;
		callable->params_types_length = static_cast<metaffi_int8>(params_len);
		callable->retval_types = retvals_types;
		callable->retval_types_length = static_cast<metaffi_int8>(retvals_len);

		return callable;
	}

	jobject create_caller_object(JNIEnv* env, const cdt_metaffi_callable& callable)
	{
		auto& cache = get_caller_cache(env);

		jsize params_len = callable.params_types_length > 0 ? callable.params_types_length : 0;
		jsize retvals_len = callable.retval_types_length > 0 ? callable.retval_types_length : 0;

		jlongArray params_array = env->NewLongArray(params_len);
		if(params_len > 0 && callable.parameters_types)
		{
			env->SetLongArrayRegion(params_array, 0, params_len, (const jlong*)callable.parameters_types);
		}

		jlongArray retvals_array = env->NewLongArray(retvals_len);
		if(retvals_len > 0 && callable.retval_types)
		{
			env->SetLongArrayRegion(retvals_array, 0, retvals_len, (const jlong*)callable.retval_types);
		}

		jobject obj = env->CallStaticObjectMethod(cache.cls, cache.create_caller,
		                                          (jlong)callable.val,
		                                          params_array,
		                                          retvals_array);
		if(params_array)
		{
			env->DeleteLocalRef(params_array);
		}
		if(retvals_array)
		{
			env->DeleteLocalRef(retvals_array);
		}
		if(!obj)
		{
			throw std::runtime_error("Failed to create Caller object");
		}
		return obj;
	}

	big_integer_cache& get_big_integer_cache(JNIEnv* env)
	{
		static big_integer_cache cache;
		if(!cache.cls)
		{
			jclass tmp = env->FindClass("java/math/BigInteger");
			if(!tmp)
			{
				throw std::runtime_error("Failed to find BigInteger class");
			}
			cache.cls = (jclass)env->NewGlobalRef(tmp);
			env->DeleteLocalRef(tmp);
		}
		if(!cache.ctor)
		{
			cache.ctor = env->GetMethodID(cache.cls, "<init>", "(I[B)V");
			if(!cache.ctor)
			{
				throw std::runtime_error("Failed to get BigInteger constructor");
			}
		}
		if(!cache.bit_length)
		{
			cache.bit_length = env->GetMethodID(cache.cls, "bitLength", "()I");
			if(!cache.bit_length)
			{
				throw std::runtime_error("Failed to get BigInteger.bitLength");
			}
		}
		if(!cache.long_value)
		{
			cache.long_value = env->GetMethodID(cache.cls, "longValue", "()J");
			if(!cache.long_value)
			{
				throw std::runtime_error("Failed to get BigInteger.longValue");
			}
		}
		if(!cache.zero_field)
		{
			cache.zero_field = env->GetStaticFieldID(cache.cls, "ZERO", "Ljava/math/BigInteger;");
			if(!cache.zero_field)
			{
				throw std::runtime_error("Failed to get BigInteger.ZERO");
			}
		}
		return cache;
	}

	bool is_big_integer(JNIEnv* env, jobject obj)
	{
		if(!obj)
		{
			return false;
		}
		auto& cache = get_big_integer_cache(env);
		return env->IsInstanceOf(obj, cache.cls) == JNI_TRUE;
	}

	uint64_t big_integer_to_uint64(JNIEnv* env, jobject obj)
	{
		auto& cache = get_big_integer_cache(env);
		jint bit_len = env->CallIntMethod(obj, cache.bit_length);
		if(bit_len > 64)
		{
			throw std::runtime_error("BigInteger value exceeds 64 bits");
		}
		jlong val = env->CallLongMethod(obj, cache.long_value);
		return static_cast<uint64_t>(val);
	}

	jobject uint64_to_big_integer(JNIEnv* env, uint64_t value)
	{
		auto& cache = get_big_integer_cache(env);
		if(value == 0)
		{
			jobject zero = env->GetStaticObjectField(cache.cls, cache.zero_field);
			if(!zero)
			{
				throw std::runtime_error("Failed to get BigInteger.ZERO");
			}
			return zero;
		}

		jbyteArray bytes = env->NewByteArray(8);
		if(!bytes)
		{
			throw std::runtime_error("Failed to allocate BigInteger byte array");
		}

		jbyte buf[8];
		for(int i = 0; i < 8; i++)
		{
			buf[7 - i] = static_cast<jbyte>((value >> (i * 8)) & 0xFF);
		}

		env->SetByteArrayRegion(bytes, 0, 8, buf);
		jobject big = env->NewObject(cache.cls, cache.ctor, 1, bytes);
		env->DeleteLocalRef(bytes);
		if(!big)
		{
			throw std::runtime_error("Failed to create BigInteger object");
		}
		return big;
	}
}

// ===== Phase 1: Core Infrastructure =====

//--------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------

cdts_jvm_serializer::cdts_jvm_serializer(JNIEnv* env, cdts& pcdts, jobject class_loader)
	: env(env), class_loader(class_loader), data(pcdts), current_index(0)
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
			} else if (std::strstr(elementSignature, "Ljava/math/BigInteger;")) {
				element_type = metaffi_uint64_type;
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
			} else if (std::strstr(elementSignature, "Lmetaffi/api/accessor/MetaFFIHandle;")) {
				element_type = metaffi_handle_type;
			} else if (std::strstr(elementSignature, "Lmetaffi/api/accessor/Caller;")) {
				element_type = metaffi_callable_type;
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

	if (is_metaffi_handle(env, obj)) {
		return metaffi_handle_type;
	}
	if (is_caller(env, obj)) {
		return metaffi_callable_type;
	}
	if (is_big_integer(env, obj)) {
		return metaffi_uint64_type;
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

metaffi_string32 cdts_jvm_serializer::jstring_to_string32(jstring str)
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

	std::vector<char32_t> utf32;
	utf32.reserve(len);

	for (jsize i = 0; i < len; i++) {
		char16_t ch = static_cast<char16_t>(utf16[i]);
		if (ch >= 0xD800 && ch <= 0xDBFF) {
			// High surrogate - expect low surrogate next
			if (i + 1 >= len) {
				env->ReleaseStringChars(str, utf16);
				throw std::runtime_error("Invalid UTF-16 surrogate pair (missing low surrogate)");
			}

			char16_t low = static_cast<char16_t>(utf16[i + 1]);
			if (low < 0xDC00 || low > 0xDFFF) {
				env->ReleaseStringChars(str, utf16);
				throw std::runtime_error("Invalid UTF-16 surrogate pair (invalid low surrogate)");
			}

			char32_t codepoint = 0x10000;
			codepoint += (static_cast<char32_t>(ch) - 0xD800) << 10;
			codepoint += static_cast<char32_t>(low) - 0xDC00;
			utf32.push_back(codepoint);
			i++; // consumed low surrogate
			continue;
		}

		if (ch >= 0xDC00 && ch <= 0xDFFF) {
			env->ReleaseStringChars(str, utf16);
			throw std::runtime_error("Invalid UTF-16 surrogate pair (unexpected low surrogate)");
		}

		utf32.push_back(static_cast<char32_t>(ch));
	}

	env->ReleaseStringChars(str, utf16);

	metaffi_string32 result = xllr_alloc_string32(utf32.data(), static_cast<metaffi_size>(utf32.size()));
	if (!result) {
		throw std::runtime_error("Failed to allocate UTF-32 string");
	}

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

	jstring result = env->NewString((const jchar*)str, static_cast<jsize>(len));
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

	jstring result = env->NewString(utf16.data(), static_cast<jsize>(utf16.size()));
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
		case metaffi_size_type:
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

cdts_jvm_serializer& cdts_jvm_serializer::add(jstring val, metaffi_type target_type)
{
	check_bounds(current_index);

	if (!val) {
		return null();
	}

	switch(target_type) {
		case metaffi_string8_type:
			data[current_index].cdt_val.string8_val = jstring_to_string8(val);
			data[current_index].type = metaffi_string8_type;
			data[current_index].free_required = true;
			break;
		case metaffi_string16_type:
			data[current_index].cdt_val.string16_val = jstring_to_string16(val);
			data[current_index].type = metaffi_string16_type;
			data[current_index].free_required = true;
			break;
		case metaffi_string32_type:
			data[current_index].cdt_val.string32_val = jstring_to_string32(val);
			data[current_index].type = metaffi_string32_type;
			data[current_index].free_required = true;
			break;
		default:
			throw std::runtime_error("Invalid target type for jstring");
	}

	current_index++;
	return *this;
}

cdts_jvm_serializer& cdts_jvm_serializer::add_handle(jobject val)
{
	check_bounds(current_index);

	if (!val) {
		return null();
	}

	void* handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
	if (!handle_mem) {
		throw std::runtime_error("Failed to allocate cdt_metaffi_handle");
	}

	data[current_index].type = metaffi_handle_type;
	data[current_index].cdt_val.handle_val = new (handle_mem) cdt_metaffi_handle();
	if (is_metaffi_handle(env, val)) {
		cdt_metaffi_handle h = extract_metaffi_handle(env, val);
		data[current_index].cdt_val.handle_val->handle = h.handle;
		data[current_index].cdt_val.handle_val->runtime_id = h.runtime_id;
		data[current_index].cdt_val.handle_val->release = h.release;
	} else {
		data[current_index].cdt_val.handle_val->handle = env->NewGlobalRef(val);
		data[current_index].cdt_val.handle_val->runtime_id = JVM_RUNTIME_ID;
		data[current_index].cdt_val.handle_val->release = nullptr;
	}
	data[current_index].free_required = true;

	current_index++;
	return *this;
}

cdts_jvm_serializer& cdts_jvm_serializer::add_callable(jobject val)
{
	check_bounds(current_index);

	if (!val) {
		return null();
	}
	if (!is_caller(env, val)) {
		throw std::runtime_error("Expected Caller object for callable");
	}

	data[current_index].type = metaffi_callable_type;
	data[current_index].cdt_val.callable_val = extract_caller(env, val);
	data[current_index].free_required = true;
	current_index++;
	return *this;
}

cdts_jvm_serializer& cdts_jvm_serializer::add_array(jarray arr, int dimensions, metaffi_type element_type)
{
	check_bounds(current_index);

	if (!arr) {
		return null();
	}

	if (dimensions <= 0) {
		throw std::runtime_error("Array dimensions must be positive");
	}

	metaffi_type base_type = (element_type & metaffi_array_type) ? (element_type & ~metaffi_array_type) : element_type;
	serialize_array(arr, dimensions, base_type);
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
		case metaffi_size_type:
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

void cdts_jvm_serializer::serialize_array_into(jarray arr, cdt& target, int dimensions, metaffi_type element_type)
{
	// Helper function to serialize an array directly into a cdt element
	// Used for recursive multi-dimensional array serialization

	if (!arr) {
		target.type = metaffi_null_type;
		target.free_required = false;
		return;
	}

	jsize length = env->GetArrayLength(arr);
	check_jni_exception("GetArrayLength");

	if (dimensions == 1) {
		// Base case: 1D array
		target.set_new_array(length, 1, static_cast<metaffi_types>(element_type));
		cdts& arr_cdts = *target.cdt_val.array_val;

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
			case metaffi_uint64_type:
			case metaffi_size_type: {
				if(env->IsInstanceOf(arr, env->FindClass("[J"))) {
					jlongArray longArr = (jlongArray)arr;
					jlong* elements = env->GetLongArrayElements(longArr, nullptr);
					for (jsize i = 0; i < length; i++) {
						arr_cdts[i].cdt_val.uint64_val = static_cast<metaffi_uint64>(elements[i]);
						arr_cdts[i].type = element_type;
					}
					env->ReleaseLongArrayElements(longArr, elements, JNI_ABORT);
				} else {
					jobjectArray objArr = (jobjectArray)arr;
					for (jsize i = 0; i < length; i++) {
						jobject obj = env->GetObjectArrayElement(objArr, i);
						if (obj) {
							if(is_big_integer(env, obj)) {
								arr_cdts[i].cdt_val.uint64_val = big_integer_to_uint64(env, obj);
								arr_cdts[i].type = element_type;
							} else if (is_instance_of(obj, "java/lang/Long")) {
								arr_cdts[i].cdt_val.uint64_val = static_cast<metaffi_uint64>(extract_long_from_wrapper(obj));
								arr_cdts[i].type = element_type;
							} else {
								env->DeleteLocalRef(obj);
								throw std::runtime_error("Expected BigInteger or Long for uint64 array");
							}
							env->DeleteLocalRef(obj);
						} else {
							arr_cdts[i].type = metaffi_null_type;
						}
					}
				}
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
			case metaffi_handle_type:
			default: {
				jobjectArray objArr = (jobjectArray)arr;
				for (jsize i = 0; i < length; i++) {
					jobject obj = env->GetObjectArrayElement(objArr, i);
					if (obj) {
						void* handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
						if (!handle_mem) {
							env->DeleteLocalRef(obj);
							throw std::runtime_error("Failed to allocate cdt_metaffi_handle");
						}
						arr_cdts[i].cdt_val.handle_val = new (handle_mem) cdt_metaffi_handle();
						arr_cdts[i].cdt_val.handle_val->handle = env->NewGlobalRef(obj);
						arr_cdts[i].cdt_val.handle_val->runtime_id = JVM_RUNTIME_ID;
						arr_cdts[i].cdt_val.handle_val->release = nullptr;
						arr_cdts[i].type = metaffi_handle_type;
						arr_cdts[i].free_required = true;
						env->DeleteLocalRef(obj);
					} else {
						arr_cdts[i].type = metaffi_null_type;
					}
				}
				break;
			}
		}
	} else {
		// Recursive case: multi-dimensional array
		target.set_new_array(length, dimensions, static_cast<metaffi_types>(element_type));
		cdts& arr_cdts = *target.cdt_val.array_val;

		jobjectArray objArr = (jobjectArray)arr;
		for (jsize i = 0; i < length; i++) {
			jarray subArr = (jarray)env->GetObjectArrayElement(objArr, i);
			serialize_array_into(subArr, arr_cdts[i], dimensions - 1, element_type);
			if (subArr) {
				env->DeleteLocalRef(subArr);
			}
		}
	}
}

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
			case metaffi_handle_type: {
				jobjectArray objArr = (jobjectArray)arr;
				for (jsize i = 0; i < length; i++) {
					jobject obj = env->GetObjectArrayElement(objArr, i);
					if (obj) {
						void* handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
						if (!handle_mem) {
							env->DeleteLocalRef(obj);
							throw std::runtime_error("Failed to allocate cdt_metaffi_handle");
						}
						arr_cdts[i].cdt_val.handle_val = new (handle_mem) cdt_metaffi_handle();
						if (is_metaffi_handle(env, obj)) {
							cdt_metaffi_handle h = extract_metaffi_handle(env, obj);
							arr_cdts[i].cdt_val.handle_val->handle = h.handle;
							arr_cdts[i].cdt_val.handle_val->runtime_id = h.runtime_id;
							arr_cdts[i].cdt_val.handle_val->release = h.release;
						} else {
							arr_cdts[i].cdt_val.handle_val->handle = env->NewGlobalRef(obj);
							arr_cdts[i].cdt_val.handle_val->runtime_id = JVM_RUNTIME_ID;
							arr_cdts[i].cdt_val.handle_val->release = nullptr;
						}
						arr_cdts[i].free_required = true;
						arr_cdts[i].type = metaffi_handle_type;
						env->DeleteLocalRef(obj);
					} else {
						arr_cdts[i].type = metaffi_null_type;
					}
				}
				break;
			}
			case metaffi_callable_type: {
				jobjectArray objArr = (jobjectArray)arr;
				for (jsize i = 0; i < length; i++) {
					jobject obj = env->GetObjectArrayElement(objArr, i);
					if (obj) {
						arr_cdts[i].cdt_val.callable_val = extract_caller(env, obj);
						arr_cdts[i].type = metaffi_callable_type;
						arr_cdts[i].free_required = true;
						env->DeleteLocalRef(obj);
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
								{
									void* handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
									if (!handle_mem) {
										env->DeleteLocalRef(obj);
										throw std::runtime_error("Failed to allocate cdt_metaffi_handle");
									}
									arr_cdts[i].cdt_val.handle_val = new (handle_mem) cdt_metaffi_handle();
									arr_cdts[i].cdt_val.handle_val->handle = env->NewGlobalRef(obj);
									arr_cdts[i].cdt_val.handle_val->runtime_id = JVM_RUNTIME_ID;
									arr_cdts[i].cdt_val.handle_val->release = nullptr;
								}
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
		// Treat as object array where each element is a sub-array
		jobjectArray objArr = (jobjectArray)arr;

		// Allocate CDTS array - each element will be an array type
		metaffi_type array_element_type = static_cast<metaffi_type>(metaffi_array_type | element_type);
		data[current_index].set_new_array(length, dimensions, static_cast<metaffi_types>(element_type));
		cdts& arr_cdts = *data[current_index].cdt_val.array_val;

		// Recursively serialize each sub-array
		for (jsize i = 0; i < length; i++) {
			jarray subArr = (jarray)env->GetObjectArrayElement(objArr, i);
			if (subArr) {
				// Recursively serialize this sub-array into arr_cdts[i]
				serialize_array_into(subArr, arr_cdts[i], dimensions - 1, element_type);
				env->DeleteLocalRef(subArr);
			} else {
				arr_cdts[i].type = metaffi_null_type;
				arr_cdts[i].free_required = false;
			}
		}

		current_index++;
	}
}

jarray cdts_jvm_serializer::create_primitive_array(cdts& arr_cdts, metaffi_type element_type)
{
	metaffi_size length = arr_cdts.length;

	switch(element_type) {
		case metaffi_int8_type: {
			jsize jni_length = static_cast<jsize>(length);
			jbyteArray result = env->NewByteArray(jni_length);
			std::vector<jbyte> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.int8_val;
			}
			env->SetByteArrayRegion(result, 0, jni_length, elements.data());
			return result;
		}
		case metaffi_int16_type: {
			jsize jni_length = static_cast<jsize>(length);
			jshortArray result = env->NewShortArray(jni_length);
			std::vector<jshort> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.int16_val;
			}
			env->SetShortArrayRegion(result, 0, jni_length, elements.data());
			return result;
		}
		case metaffi_int32_type: {
			jsize jni_length = static_cast<jsize>(length);
			jintArray result = env->NewIntArray(jni_length);
			std::vector<jint> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.int32_val;
			}
			env->SetIntArrayRegion(result, 0, jni_length, elements.data());
			return result;
		}
		case metaffi_int64_type: {
			jsize jni_length = static_cast<jsize>(length);
			jlongArray result = env->NewLongArray(jni_length);
			std::vector<jlong> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.int64_val;
			}
			env->SetLongArrayRegion(result, 0, jni_length, elements.data());
			return result;
		}
		case metaffi_float32_type: {
			jsize jni_length = static_cast<jsize>(length);
			jfloatArray result = env->NewFloatArray(jni_length);
			std::vector<jfloat> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.float32_val;
			}
			env->SetFloatArrayRegion(result, 0, jni_length, elements.data());
			return result;
		}
		case metaffi_float64_type: {
			jsize jni_length = static_cast<jsize>(length);
			jdoubleArray result = env->NewDoubleArray(jni_length);
			std::vector<jdouble> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.float64_val;
			}
			env->SetDoubleArrayRegion(result, 0, jni_length, elements.data());
			return result;
		}
		case metaffi_bool_type: {
			jsize jni_length = static_cast<jsize>(length);
			jbooleanArray result = env->NewBooleanArray(jni_length);
			std::vector<jboolean> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = arr_cdts[i].cdt_val.bool_val ? JNI_TRUE : JNI_FALSE;
			}
			env->SetBooleanArrayRegion(result, 0, jni_length, elements.data());
			return result;
		}
		case metaffi_char16_type: {
			jsize jni_length = static_cast<jsize>(length);
			jcharArray result = env->NewCharArray(jni_length);
			std::vector<jchar> elements(length);
			for (metaffi_size i = 0; i < length; i++) {
				elements[i] = static_cast<jchar>(arr_cdts[i].cdt_val.char16_val.c[0]);
			}
			env->SetCharArrayRegion(result, 0, jni_length, elements.data());
			return result;
		}
		default:
			throw std::runtime_error("Unsupported primitive array element type");
	}
}

jobjectArray cdts_jvm_serializer::create_object_array(cdts& arr_cdts, metaffi_type element_type)
{
	return create_object_array(arr_cdts, element_type, "");
}

jobjectArray cdts_jvm_serializer::create_object_array(cdts& arr_cdts, metaffi_type element_type, const std::string& element_class_override)
{
	metaffi_size length = arr_cdts.length;

	// Determine element class
	const char* className = nullptr;
	if(!element_class_override.empty())
	{
		className = element_class_override.c_str();
	}
	else
	{
		switch(element_type) {
			case metaffi_int8_type: className = "java/lang/Byte"; break;
			case metaffi_int16_type: className = "java/lang/Short"; break;
			case metaffi_int32_type: className = "java/lang/Integer"; break;
			case metaffi_int64_type: className = "java/lang/Long"; break;
			case metaffi_size_type: className = "java/lang/Long"; break;
			case metaffi_uint64_type: className = "java/math/BigInteger"; break;
			case metaffi_float32_type: className = "java/lang/Float"; break;
			case metaffi_float64_type: className = "java/lang/Double"; break;
			case metaffi_bool_type: className = "java/lang/Boolean"; break;
			case metaffi_char16_type: className = "java/lang/Character"; break;
			case metaffi_string8_type: className = "java/lang/String"; break;
			case metaffi_handle_type:
			case metaffi_callable_type:
			default: className = "java/lang/Object"; break;
		}
	}

	jclass elementClass = nullptr;
	if(!element_class_override.empty() && class_loader)
	{
		jclass class_cls = env->FindClass("java/lang/Class");
		if(!class_cls)
		{
			check_jni_exception("FindClass java/lang/Class");
			throw std::runtime_error("Failed to find java/lang/Class");
		}
		local_ref_guard class_guard(env, class_cls);

		jmethodID for_name = env->GetStaticMethodID(class_cls, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
		if(!for_name)
		{
			check_jni_exception("GetStaticMethodID Class.forName");
			throw std::runtime_error("Failed to get Class.forName");
		}

		std::string binary_name = element_class_override;
		std::replace(binary_name.begin(), binary_name.end(), '/', '.');
		jstring name_obj = env->NewStringUTF(binary_name.c_str());
		if(!name_obj)
		{
			check_jni_exception("NewStringUTF");
			throw std::runtime_error("Failed to allocate class name string");
		}
		local_ref_guard name_guard(env, name_obj);

		jobject cls_obj = env->CallStaticObjectMethod(class_cls, for_name, name_obj, JNI_FALSE, class_loader);
		if(env->ExceptionCheck() || !cls_obj)
		{
			check_jni_exception("Class.forName");
			throw std::runtime_error("Failed to resolve element class: " + binary_name);
		}
		elementClass = (jclass)cls_obj;
	}
	else
	{
		elementClass = env->FindClass(className);
		if (!elementClass) {
			check_jni_exception("FindClass");
			throw std::runtime_error("Failed to find element class");
		}
	}
	local_ref_guard guard(env, elementClass);

	jobjectArray result = env->NewObjectArray(static_cast<jsize>(length), elementClass, nullptr);
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
				case metaffi_int64_type:
				case metaffi_size_type: {
					jclass cls = env->FindClass("java/lang/Long");
					jmethodID constructor = env->GetMethodID(cls, "<init>", "(J)V");
					jlong val = element_type == metaffi_size_type
						? static_cast<jlong>(arr_cdts[i].cdt_val.uint64_val)
						: static_cast<jlong>(arr_cdts[i].cdt_val.int64_val);
					element = env->NewObject(cls, constructor, val);
					env->DeleteLocalRef(cls);
					break;
				}
				case metaffi_uint64_type: {
					element = uint64_to_big_integer(env, arr_cdts[i].cdt_val.uint64_val);
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
				case metaffi_handle_type: {
					if(arr_cdts[i].cdt_val.handle_val) {
						const cdt_metaffi_handle& h = *arr_cdts[i].cdt_val.handle_val;
						if(h.runtime_id == JVM_RUNTIME_ID) {
							element = (jobject)h.handle;
						} else {
							element = create_metaffi_handle_object(env, h);
						}
					}
					break;
				}
				case metaffi_callable_type: {
					if(arr_cdts[i].cdt_val.callable_val) {
						element = create_caller_object(env, *arr_cdts[i].cdt_val.callable_val);
					}
					break;
				}
				// Add more types as needed
				default:
					break;
			}
		}

		env->SetObjectArrayElement(result, static_cast<jsize>(i), element);
		if (element && env->GetObjectRefType(element) == JNILocalRefType) {
			env->DeleteLocalRef(element);
		}
	}

	return result;
}

jobjectArray cdts_jvm_serializer::extract_multidim_array(cdts& arr_cdts)
{
	// Helper function to extract multi-dimensional arrays recursively
	metaffi_size length = arr_cdts.length;
	if (length == 0) {
		return nullptr;
	}

	// Determine the element class based on the first element's sub-array type
	// For int[][], we need [I (int[]) as element class, not [Ljava/lang/Object;
	jclass elementClass = nullptr;

	// Look at first non-null element to determine type
	for (metaffi_size i = 0; i < length && !elementClass; i++) {
		cdt& first_elem = arr_cdts[i];
		if (first_elem.type == metaffi_null_type) {
			continue;
		}

		if (first_elem.type & metaffi_array_type) {
			cdts* sub_arr = first_elem.cdt_val.array_val;
			if (sub_arr && sub_arr->length > 0) {
				metaffi_type sub_elem_type = (*sub_arr)[0].type;

				if (sub_elem_type & metaffi_array_type) {
					// Deeper nesting - use Object[] as element class
					elementClass = env->FindClass("[Ljava/lang/Object;");
				} else {
					// Base level - use appropriate primitive/object array class
					switch(sub_elem_type) {
						case metaffi_int8_type: elementClass = env->FindClass("[B"); break;
						case metaffi_int16_type: elementClass = env->FindClass("[S"); break;
						case metaffi_int32_type: elementClass = env->FindClass("[I"); break;
						case metaffi_int64_type: elementClass = env->FindClass("[J"); break;
						case metaffi_float32_type: elementClass = env->FindClass("[F"); break;
						case metaffi_float64_type: elementClass = env->FindClass("[D"); break;
						case metaffi_bool_type: elementClass = env->FindClass("[Z"); break;
						case metaffi_char16_type: elementClass = env->FindClass("[C"); break;
						case metaffi_string8_type: elementClass = env->FindClass("[Ljava/lang/String;"); break;
						default: elementClass = env->FindClass("[Ljava/lang/Object;"); break;
					}
				}
			}
		}
	}

	// Fallback to Object[] if we couldn't determine type
	if (!elementClass) {
		elementClass = env->FindClass("[Ljava/lang/Object;");
	}
	check_jni_exception("FindClass for array element");

	jobjectArray result = env->NewObjectArray(static_cast<jsize>(length), elementClass, nullptr);
	if (!result) {
		check_jni_exception("NewObjectArray");
		throw std::runtime_error("Failed to create multi-dimensional array");
	}

	for (metaffi_size i = 0; i < length; i++) {
		cdt& elem = arr_cdts[i];

		if (elem.type == metaffi_null_type) {
			env->SetObjectArrayElement(result, static_cast<jsize>(i), nullptr);
			continue;
		}

		if (elem.type & metaffi_array_type) {
			// Sub-array - recursively extract
			cdts* sub_arr_cdts = elem.cdt_val.array_val;
			if (sub_arr_cdts && sub_arr_cdts->length > 0) {
				metaffi_type sub_element_type = (*sub_arr_cdts)[0].type;

				jarray sub_array;
				if (sub_element_type & metaffi_array_type) {
					// Even deeper nesting
					sub_array = extract_multidim_array(*sub_arr_cdts);
				} else {
					// Base level - extract as 1D array
					bool is_primitive = false;
					switch(sub_element_type) {
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

					if (is_primitive) {
						sub_array = create_primitive_array(*sub_arr_cdts, sub_element_type);
					} else {
						sub_array = (jarray)create_object_array(*sub_arr_cdts, sub_element_type);
					}
				}

				env->SetObjectArrayElement(result, static_cast<jsize>(i), sub_array);
				if (sub_array) {
					env->DeleteLocalRef(sub_array);
				}
			} else {
				env->SetObjectArrayElement(result, static_cast<jsize>(i), nullptr);
			}
		} else {
			// Unexpected: element is not an array in multi-dim context
			// This shouldn't happen in well-formed data, fail fast
			throw std::runtime_error("Expected array element in multi-dimensional array, got non-array type");
		}
	}

	env->DeleteLocalRef(elementClass);
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

	// Check if elements are themselves arrays (multi-dimensional)
	bool is_multi_dimensional = (element_type & metaffi_array_type) != 0;

	jarray result;
	if (is_multi_dimensional) {
		// Multi-dimensional array - recursively extract sub-arrays
		result = extract_multidim_array(*arr_cdts);
	} else {
		// 1D array
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

		if (is_primitive) {
			result = create_primitive_array(*arr_cdts, element_type);
		} else {
			result = (jarray)create_object_array(*arr_cdts, element_type);
		}
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
			const cdt_metaffi_handle& h = *current.cdt_val.handle_val;
			if (h.runtime_id == JVM_RUNTIME_ID) {
				result = (jobject)h.handle;
			} else {
				result = create_metaffi_handle_object(env, h);
			}
		}
	} else if (current.type == metaffi_callable_type) {
		if (current.cdt_val.callable_val && current.cdt_val.callable_val->val) {
			result = create_caller_object(env, *current.cdt_val.callable_val);
		}
	}

	current_index++;
	return result;
}

jarray cdts_jvm_serializer::extract_array(const metaffi_type_info& type_info)
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

	// Check if elements are themselves arrays (multi-dimensional)
	bool is_multi_dimensional = (element_type & metaffi_array_type) != 0;

	std::string override_class;
	if(!is_multi_dimensional)
	{
		metaffi_type expected_base = (type_info.type & metaffi_array_type) ? (type_info.type & ~metaffi_array_type) : type_info.type;
		if(expected_base == metaffi_handle_type && type_info.alias && *type_info.alias)
		{
			auto parsed = parse_alias_array(type_info.alias);
			if(!parsed.base.empty())
			{
				int dims = type_info.fixed_dimensions > 0 ? static_cast<int>(type_info.fixed_dimensions) : parsed.dims;
				if(dims > 1)
				{
					override_class.assign(static_cast<size_t>(dims - 1), '[');
					override_class += "L" + to_internal_name(parsed.base) + ";";
				}
				else
				{
					override_class = to_internal_name(parsed.base);
				}
			}
		}
	}

	jarray result;
	if (is_multi_dimensional) {
		// Multi-dimensional array - recursively extract sub-arrays
		result = extract_multidim_array(*arr_cdts);
	} else {
		// 1D array
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

		if (is_primitive) {
			result = create_primitive_array(*arr_cdts, element_type);
		} else {
			if(!override_class.empty())
			{
				result = (jarray)create_object_array(*arr_cdts, element_type, override_class);
			}
			else
			{
				result = (jarray)create_object_array(*arr_cdts, element_type);
			}
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
		case metaffi_uint64_type: {
			uint64_t value = 0;
			if (is_big_integer(env, val)) {
				value = big_integer_to_uint64(env, val);
			} else if (is_instance_of(val, "java/lang/Long")) {
				value = static_cast<uint64_t>(extract_long_from_wrapper(val));
			} else {
				throw std::runtime_error("Type mismatch: expected BigInteger for uint64");
			}
			data[current_index].type = metaffi_uint64_type;
			data[current_index].cdt_val.uint64_val = value;
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
		case metaffi_callable_type: {
			data[current_index].type = metaffi_callable_type;
			data[current_index].cdt_val.callable_val = extract_caller(env, val);
			data[current_index].free_required = true;
			break;
		}
		case metaffi_handle_type:
		default: {
			// Store as handle with global reference or extract MetaFFIHandle
			data[current_index].type = metaffi_handle_type;
			void* handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
			if (!handle_mem) {
				throw std::runtime_error("Failed to allocate cdt_metaffi_handle");
			}
			data[current_index].cdt_val.handle_val = new (handle_mem) cdt_metaffi_handle();
			if (is_metaffi_handle(env, val)) {
				cdt_metaffi_handle h = extract_metaffi_handle(env, val);
				data[current_index].cdt_val.handle_val->handle = h.handle;
				data[current_index].cdt_val.handle_val->runtime_id = h.runtime_id;
				data[current_index].cdt_val.handle_val->release = h.release;
			} else {
				data[current_index].cdt_val.handle_val->handle = env->NewGlobalRef(val);
				data[current_index].cdt_val.handle_val->runtime_id = JVM_RUNTIME_ID;
				data[current_index].cdt_val.handle_val->release = nullptr;
			}
			data[current_index].free_required = true;
			break;
		}
	}

	current_index++;
	return *this;
}

} // namespace metaffi::utils
