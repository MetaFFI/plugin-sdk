#include "jstring_wrapper.h"
#include <iostream>

jstring_wrapper::jstring_wrapper(JNIEnv* env, const char8_t* s) : env(env)
{
	
	value = env->NewStringUTF((const char*)s);
}

jstring_wrapper::jstring_wrapper(JNIEnv* env, const char16_t* s) : env(env)
{
	value = env->NewString(reinterpret_cast<const jchar*>(s), (jsize)std::char_traits<char16_t>::length(s));
}

jstring_wrapper::jstring_wrapper(JNIEnv* env, const char32_t* s) : env(env)
{
	std::u16string tmp;
	while (*s != U'\0') {
		char32_t codepoint = *s++;
		if (codepoint <= 0xFFFF) {
			// Single code unit (BMP character)
			tmp.push_back(static_cast<char16_t>(codepoint));
		} else {
			// Surrogate pair (non-BMP character)
			codepoint -= 0x10000;
			char16_t high_surrogate = static_cast<char16_t>((codepoint >> 10) + 0xD800);
			char16_t low_surrogate = static_cast<char16_t>((codepoint & 0x3FF) + 0xDC00);
			tmp.push_back(high_surrogate);
			tmp.push_back(low_surrogate);
		}
	}
	
	value = env->NewString(reinterpret_cast<const jchar*>(tmp.c_str()), (jsize)tmp.length());
}

jstring_wrapper::jstring_wrapper(JNIEnv* env, jstring s) : env(env), value(s) {}

jobjectArray jstring_wrapper::new_1d_array(JNIEnv* env, const metaffi_string8* s, jsize length)
{
	jclass stringClass = env->FindClass("java/lang/String");
	jobjectArray array = env->NewObjectArray(length, stringClass, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		jstring_wrapper wrapper(env, s[i]);
		env->SetObjectArrayElement(array, i, (jstring)wrapper);
	}
	return array;
}

jobjectArray jstring_wrapper::new_1d_array(JNIEnv* env, const metaffi_string16* s, jsize length)
{
	jclass stringClass = env->FindClass("java/lang/String");
	jobjectArray array = env->NewObjectArray(length, stringClass, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		jstring_wrapper wrapper(env, s[i]);
		env->SetObjectArrayElement(array, i, (jstring)wrapper);
	}
	return array;
}

jobjectArray jstring_wrapper::new_1d_array(JNIEnv* env, const metaffi_string32* s, jsize length)
{
	jclass stringClass = env->FindClass("java/lang/String");
	jobjectArray array = env->NewObjectArray(length, stringClass, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		jstring_wrapper wrapper(env, s[i]);
		env->SetObjectArrayElement(array, i, (jstring)wrapper);
	}
	return array;
}

jstring_wrapper::operator jstring()
{
	return value;
}

jstring_wrapper::operator metaffi_string8()
{
	const char* original = env->GetStringUTFChars(value, nullptr);
	jsize len = env->GetStringUTFLength(value);
	char8_t* copy = new char8_t[len + 1];
	std::copy(original, original + len, copy);
	copy[len] = '\0'; // null terminate the string
	env->ReleaseStringUTFChars(value, original);
	return copy;
}

jstring_wrapper::operator metaffi_string16()
{
	const jchar* temp = env->GetStringChars(value, nullptr);
	char16_t* utf16String = (char16_t*)malloc((std::char_traits<char16_t>::length(reinterpret_cast<const char16_t*>(temp)) + 1) * sizeof(char16_t));
	std::copy(temp, temp + std::char_traits<char16_t>::length(reinterpret_cast<const char16_t*>(temp)), utf16String);
	env->ReleaseStringChars(value, temp);
	return utf16String;
}

jstring_wrapper::operator metaffi_string32()
{
	const jchar* temp = env->GetStringChars(value, nullptr);
	std::u16string u16(reinterpret_cast<const char16_t*>(temp), env->GetStringLength(value));
	char32_t* utf32String = (char32_t*)malloc((u16.length() + 1) * sizeof(char32_t));
	std::copy(u16.begin(), u16.end(), utf32String);
	utf32String[u16.length()] = '\0';
	env->ReleaseStringChars(value, temp);
	return utf32String;
}

