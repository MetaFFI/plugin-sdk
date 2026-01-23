#pragma once

#include <jni.h>
#include "exception_macro.h"

class jlong_wrapper
{
private:
	jlong value;

public:
	jlong_wrapper(jlong value);
	jlong_wrapper(const jlong_wrapper& other);
	
	operator jlong();
	
	jlong_wrapper& operator=(jlong val);
	
	static jlongArray new_1d_array(JNIEnv* env, jsize size, const jlong* parr = nullptr);
	
	template <typename metaffi_type_t>
	static metaffi_type_t* to_c_array(JNIEnv* env, jarray jarr)
	{
		jsize length = env->GetArrayLength(jarr);
		jlong* elements = env->GetLongArrayElements((jlongArray)jarr, nullptr);
		metaffi_type_t* c_array = new metaffi_type_t[length];
		for (jsize i = 0; i < length; ++i) {
			c_array[i] = static_cast<metaffi_type_t>(elements[i]);
		}
		env->ReleaseLongArrayElements((jlongArray)jarr, elements, JNI_ABORT);
		return c_array;
	}
	
	jlong_wrapper(JNIEnv* p_env, jobject p_jobject);
};
