#pragma once

#include <jni.h>
#include "exception_macro.h"

class jfloat_wrapper
{
private:
	jfloat value;

public:
	jfloat_wrapper(jfloat value);
	jfloat_wrapper(const jfloat_wrapper& other);
	jfloat_wrapper(JNIEnv* env, jobject obj);
	
	operator jfloat();
	
	jfloat_wrapper& operator=(jfloat val);
	
	static jfloatArray new_1d_array(JNIEnv* env, jsize size, const jfloat* parr = nullptr);
	
	template <typename metaffi_type_t>
	static metaffi_type_t* to_c_array(JNIEnv* env, jarray jarr)
	{
		jsize length = env->GetArrayLength((jfloatArray)jarr);
		jfloat* elements = env->GetFloatArrayElements((jfloatArray)jarr, nullptr);
		metaffi_type_t* c_array = new metaffi_type_t[length];
		for (jsize i = 0; i < length; ++i) {
			c_array[i] = static_cast<metaffi_type_t>(elements[i]);
		}
		env->ReleaseFloatArrayElements((jfloatArray)jarr, elements, JNI_ABORT);
		return c_array;
	}
};
