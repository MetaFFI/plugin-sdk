#pragma once

#include <jni.h>
#include "exception_macro.h"

class jdouble_wrapper
{
private:
	jdouble value;

public:
	explicit jdouble_wrapper(jdouble value);
	jdouble_wrapper(const jdouble_wrapper& other);
	jdouble_wrapper(JNIEnv* env, jobject obj);
	
	explicit operator jdouble();
	
	jdouble_wrapper& operator=(jdouble val);
	
	static jdoubleArray new_1d_array(JNIEnv* env, jsize size, const jdouble* parr = nullptr);
	
	template <typename metaffi_type_t>
	static metaffi_type_t* to_c_array(JNIEnv* env, jarray jarr)
	{
		jsize length = env->GetArrayLength((jdoubleArray)jarr);
		jdouble* elements = env->GetDoubleArrayElements((jdoubleArray)jarr, nullptr);
		metaffi_type_t* c_array = new metaffi_type_t[length];
		for (jsize i = 0; i < length; ++i) {
			c_array[i] = static_cast<metaffi_type_t>(elements[i]);
		}
		env->ReleaseDoubleArrayElements((jdoubleArray)jarr, elements, JNI_ABORT);
		return c_array;
	}
};
