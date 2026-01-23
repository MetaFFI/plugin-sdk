#include "jchar_wrapper.h"
#include "runtime/metaffi_primitives.h"
#include "exception_macro.h"


jchar_wrapper::jchar_wrapper(JNIEnv* env, metaffi_char8 c8) : env(env)
{
	metaffi_char16 c16 = (metaffi_char16)c8;
	if(c16.is_surrogate())
	{
		throw std::invalid_argument("Java char does not support UTF-16 surrogate pair of characters");
	}
	
	value = (jchar)c16.c[0];
}

jchar_wrapper::jchar_wrapper(JNIEnv* env, metaffi_char16 c) : env(env)
{
	if(c.is_surrogate())
	{
		throw std::invalid_argument("Java char does not support UTF-16 surrogate pair of characters");
	}
	
	value = static_cast<jchar>(c.c[0]);
}

jchar_wrapper::jchar_wrapper(JNIEnv* env, metaffi_char32 c) : env(env)
{
	metaffi_char16 c16 = (metaffi_char16)c;
	if(c16.is_surrogate())
	{
		throw std::invalid_argument("Java char does not support UTF-16 surrogate pair of characters");
	}
	
	value = static_cast<jchar>(c16.c[0]);
}

jchar_wrapper::operator jchar() const
{
	return value;
}

jchar_wrapper::operator metaffi_char8() const
{
	metaffi_char8 utf8 = {};
	std::mbstate_t state = std::mbstate_t();
	c16rtomb((char*)utf8.c, value, &state);
	return utf8;
}

jchar_wrapper::operator metaffi_char16() const
{
	metaffi_char16 utf16 = {};
	utf16.c[0] = value; // Java doesn't support surrogate pairs
	return utf16;
}

jchar_wrapper::operator metaffi_char32() const
{
	return static_cast<metaffi_char32>(value);
}

jcharArray jchar_wrapper::new_1d_array(JNIEnv* env, const char8_t* s, metaffi_size length)
{
	jcharArray array = env->NewCharArray(static_cast<jsize>(length));
	jchar* elements = env->GetCharArrayElements(array, nullptr);
	for(size_t i = 0; i < std::char_traits<char8_t>::length(s); ++i)
	{
		elements[i] = static_cast<jchar>(s[i]);
	}
	env->ReleaseCharArrayElements(array, elements, 0);
	return array;
}

jcharArray jchar_wrapper::new_1d_array(JNIEnv* env, const char16_t* s, metaffi_size length)
{
	jcharArray array = env->NewCharArray(static_cast<jsize>(length));
	jchar* elements = env->GetCharArrayElements(array, nullptr);
	for(size_t i = 0; i < std::char_traits<char16_t>::length(s); ++i)
	{
		elements[i] = static_cast<jchar>(s[i]);
	}
	env->ReleaseCharArrayElements(array, elements, 0);
	return array;
}

jcharArray jchar_wrapper::new_1d_array(JNIEnv* env, const char32_t* s, metaffi_size length)
{
	std::u16string u16(reinterpret_cast<const char16_t*>(s), length);
	jcharArray array = env->NewCharArray(static_cast<jsize>(u16.length()));
	jchar* elements = env->GetCharArrayElements(array, nullptr);
	for(size_t i = 0; i < u16.length(); ++i)
	{
		elements[i] = static_cast<jchar>(u16[i]);
	}
	env->ReleaseCharArrayElements(array, elements, 0);
	return array;
}

jchar_wrapper::jchar_wrapper(JNIEnv* env, jobject obj)
{
	jclass charClass = env->FindClass("java/lang/Character");
	jmethodID charValue = env->GetMethodID(charClass, "charValue", "()C");
	value = env->CallCharMethod(obj, charValue);
	check_and_throw_jvm_exception(env, true);
}


