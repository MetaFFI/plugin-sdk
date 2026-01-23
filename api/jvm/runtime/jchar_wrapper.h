#include <jni.h>
#include <string>
#include "runtime/metaffi_primitives.h"
#include <vector>

class jchar_wrapper
{
private:
	JNIEnv* env;
	jchar value;

public:
	jchar_wrapper(JNIEnv* env, metaffi_char8 c);
	jchar_wrapper(JNIEnv* env, metaffi_char16 c);
	jchar_wrapper(JNIEnv* env, metaffi_char32 c);
	
	jchar_wrapper(JNIEnv* env, jchar c) : env(env), value(c){}
	jchar_wrapper(JNIEnv* env, jobject obj);
	
	explicit operator jchar() const;
	explicit operator metaffi_char8() const;
	explicit operator metaffi_char16() const;
	explicit operator metaffi_char32() const;
	
	static jcharArray new_1d_array(JNIEnv* env, const char8_t* s, metaffi_size length);
	static jcharArray new_1d_array(JNIEnv* env, const char16_t* s, metaffi_size length);
	static jcharArray new_1d_array(JNIEnv* env, const char32_t* s, metaffi_size length);
	
	template<typename T>
	static T* to_c_array(JNIEnv* env, jarray jarr);
};


template<>
inline char16_t* jchar_wrapper::to_c_array<char16_t>(JNIEnv* env, jarray jarr)
{
	jsize length = env->GetArrayLength(jarr);
	char16_t* c_array = new char16_t[length];
	jchar* elements = env->GetCharArrayElements((jcharArray)jarr, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		c_array[i] = elements[i];
	}
	env->ReleaseCharArrayElements((jcharArray)jarr, elements, 0);
	return c_array;
}

template<>
inline char32_t* jchar_wrapper::to_c_array<char32_t>(JNIEnv* env, jarray jarr)
{
	jsize length = env->GetArrayLength(jarr);
	char32_t* c_array = new char32_t[length];
	jchar* elements = env->GetCharArrayElements((jcharArray)jarr, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		c_array[i] = (char32_t)elements[i];
	}
	env->ReleaseCharArrayElements((jcharArray)jarr, elements, 0);
	return c_array;
}
