#include <jni.h>
#include <string>
#include "runtime/metaffi_primitives.h"

class jstring_wrapper
{
private:
	JNIEnv* env;
	jstring value;

public:
	jstring_wrapper(JNIEnv* env, const char8_t* s);
	jstring_wrapper(JNIEnv* env, const char16_t* s);
	jstring_wrapper(JNIEnv* env, const char32_t* s);
	jstring_wrapper(JNIEnv* env, jstring s);
	
	explicit operator jstring();
	explicit operator metaffi_string8();
	explicit operator metaffi_string16();
	explicit operator metaffi_string32();
	
	static jobjectArray new_1d_array(JNIEnv* env, const metaffi_string8* s, jsize length);
	static jobjectArray new_1d_array(JNIEnv* env, const metaffi_string16* s, jsize length);
	static jobjectArray new_1d_array(JNIEnv* env, const metaffi_string32* s, jsize length);
	
	template<typename metaffi_type_t>
	static metaffi_type_t* to_c_array(JNIEnv* env, jarray arr);
};

template<>
inline metaffi_string8* jstring_wrapper::to_c_array<metaffi_string8>(JNIEnv* env, jarray arr)
{
	jsize length = env->GetArrayLength(arr);
	metaffi_string8* c_array = new metaffi_string8[length];
	for(jsize i = 0; i < length; ++i)
	{
		jstring jstr = static_cast<jstring>(env->GetObjectArrayElement((jobjectArray) arr, i));
		const char* original = env->GetStringUTFChars(jstr, nullptr);
		char8_t* copy = new char8_t[std::strlen(original) + 1];
		std::copy(original, original + std::strlen(original), copy);
		copy[std::strlen(original)] = u8'\0'; // null terminate the string
		env->ReleaseStringUTFChars(jstr, original);
		c_array[i] = copy;
	}
	return c_array;
}

template<>
inline metaffi_string16* jstring_wrapper::to_c_array<metaffi_string16>(JNIEnv* env, jarray arr)
{
	jsize length = env->GetArrayLength(arr);
	metaffi_string16* c_array = new metaffi_string16[length];
	for(jsize i = 0; i < length; ++i)
	{
		jstring jstr = static_cast<jstring>(env->GetObjectArrayElement((jobjectArray) arr, i));
		const char* original = env->GetStringUTFChars(jstr, nullptr);
		std::u16string temp(original, original + std::strlen(original));
		env->ReleaseStringUTFChars(jstr, original);
		char16_t* copy = new char16_t[temp.length() + 1];
		std::copy(temp.begin(), temp.end(), copy);
		copy[temp.length()] = u'\0'; // null terminate the string
		c_array[i] = copy;
	}
	return c_array;
}

template<>
inline metaffi_string32* jstring_wrapper::to_c_array<metaffi_string32>(JNIEnv* env, jarray arr)
{
	jsize length = env->GetArrayLength(arr);
	metaffi_string32* c_array = new metaffi_string32[length];
	for(jsize i = 0; i < length; ++i)
	{
		jstring jstr = static_cast<jstring>(env->GetObjectArrayElement((jobjectArray) arr, i));
		const char* original = env->GetStringUTFChars(jstr, nullptr);
		std::u32string temp(original, original + std::strlen(original));
		env->ReleaseStringUTFChars(jstr, original);
		char32_t* copy = new char32_t[temp.length() + 1];
		std::copy(temp.begin(), temp.end(), copy);
		copy[temp.length()] = U'\0'; // null terminate the string
		c_array[i] = copy;
	}
	return c_array;
}
