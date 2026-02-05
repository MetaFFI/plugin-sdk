#pragma once

#include <jni.h>
#include "runtime/metaffi_primitives.h"

class jbyte_wrapper
{
private:
	jbyte value;

public:
	explicit jbyte_wrapper(jbyte value);
	jbyte_wrapper(const jbyte_wrapper& other);
	jbyte_wrapper(JNIEnv* env, jobject obj);
	
	operator jbyte() const;
	explicit operator metaffi_uint8() const;
	
	jbyte_wrapper& operator=(jbyte val);
	
	static jbyteArray new_1d_array(JNIEnv* env, jsize length, const jbyte* parr = nullptr);
	
	
	template <typename metaffi_type_t>
	static metaffi_type_t* to_c_array(JNIEnv* env, jarray jarr)
	{
		jsize length = env->GetArrayLength(jarr);
		jbyte* elements = env->GetByteArrayElements((jbyteArray)jarr, nullptr);
		metaffi_type_t* c_array = new metaffi_type_t[length];
		
		for (jsize i = 0; i < length; ++i) {
			c_array[i] = static_cast<metaffi_type_t>(elements[i]);
		}
		
		env->ReleaseByteArrayElements((jbyteArray)jarr, elements, JNI_ABORT);
		return c_array;
	}
};
