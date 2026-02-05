#pragma once

#include <jni.h>
#include "exception_macro.h"

class jboolean_wrapper
{
private:
	jboolean value;

public:
	jboolean_wrapper(jboolean value);
	jboolean_wrapper(const jboolean_wrapper& other);
	jboolean_wrapper(JNIEnv* env, jobject obj);
	
	operator jboolean();
	
	jboolean_wrapper& operator=(jboolean val);
	
	static jbooleanArray new_1d_array(JNIEnv* env, jsize size, const jboolean* parr = nullptr);
	
	template <typename metaffi_type_t>
	static metaffi_type_t* to_c_array(JNIEnv* env, jarray jarr)
	{
		jsize length = env->GetArrayLength((jbooleanArray)jarr);
		jboolean* elements = env->GetBooleanArrayElements((jbooleanArray)jarr, nullptr);
		metaffi_type_t* c_array = new metaffi_type_t[length];
		for (jsize i = 0; i < length; ++i) {
			c_array[i] = static_cast<metaffi_type_t>(elements[i]);
		}
		env->ReleaseBooleanArrayElements((jbooleanArray)jarr, elements, JNI_ABORT);
		return c_array;
	}
};
