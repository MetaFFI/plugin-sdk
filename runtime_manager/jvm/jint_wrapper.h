#pragma once

#include <jni.h>
#include "exception_macro.h"

class jint_wrapper
{
private:
	jint value;

public:
	jint_wrapper(jint value);
	jint_wrapper(const jint_wrapper& other);
	
	operator jint();
	
	jint_wrapper& operator=(jint val);
	
	static jintArray new_1d_array(JNIEnv* env, jsize size, const jint* parr = nullptr);
	
	template <typename metaffi_type_t>
	static metaffi_type_t* to_c_array(JNIEnv* env, jarray jarr)
	{
		jsize length = env->GetArrayLength(jarr);
		jint* elements = env->GetIntArrayElements((jintArray)jarr, nullptr);
		metaffi_type_t* c_array = new metaffi_type_t[length];
		for (jsize i = 0; i < length; ++i) {
			c_array[i] = static_cast<metaffi_type_t>(elements[i]);
		}
		env->ReleaseIntArrayElements((jintArray)jarr, elements, JNI_ABORT);
		return c_array;
	}
	
	jint_wrapper(JNIEnv* p_env, jobject p_jobject);
};
