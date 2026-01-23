#pragma once

#include <jni.h>
#include "exception_macro.h"

class jshort_wrapper
{
private:
	jshort value;

public:
	jshort_wrapper(jshort value);
	jshort_wrapper(const jshort_wrapper& other);
	
	operator jshort();
	
	jshort_wrapper& operator=(jshort val);
	
	static jshortArray new_1d_array(JNIEnv* env, jsize size, const jshort* parr = nullptr);
	
	template <typename metaffi_type_t>
	static metaffi_type_t* to_c_array(JNIEnv* env, jarray jarr)
	{
		jsize length = env->GetArrayLength(jarr);
		jshort* elements = env->GetShortArrayElements((jshortArray)jarr, nullptr);
		metaffi_type_t* c_array = new metaffi_type_t[length];
		for (jsize i = 0; i < length; ++i) {
			c_array[i] = static_cast<metaffi_type_t>(elements[i]);
		}
		env->ReleaseShortArrayElements((jshortArray)jarr, elements, JNI_ABORT);
		return c_array;
	}
	
	jshort_wrapper(JNIEnv* p_env, jobject p_jobject);
};
