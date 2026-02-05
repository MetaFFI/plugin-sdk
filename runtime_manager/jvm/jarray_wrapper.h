#pragma once

#include <jni.h>
#include "exception_macro.h"

class jarray_wrapper
{
private:
	JNIEnv* env;
	jarray array;
	metaffi_type type;
	
public:
	static jobjectArray create_object_array(JNIEnv* env, const char* class_name, int size, int dimensions);
	static std::pair<metaffi_type_info, jint> get_array_info(JNIEnv* env, jarray array, const metaffi_type_info& root_info);
	static bool is_array(JNIEnv* env, jobject obj);
	static std::pair<jvalue, char> get_element(JNIEnv* env, jarray array, const metaffi_size* index, metaffi_size index_length);
	static jarray create_jni_array(JNIEnv* env, metaffi_type t, metaffi_int64 fixed_dimensions, metaffi_size length);
public:
	explicit jarray_wrapper(JNIEnv* env, jarray array, metaffi_type t);
	
	
	int size();
	
	std::pair<jvalue, char> get(int index);
	void set(int index, jvalue obj);
	
	explicit operator jobject();
	explicit operator jarray();
};
