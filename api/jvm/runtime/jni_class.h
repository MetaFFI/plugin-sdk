#pragma once
#include <runtime_manager/jdk/jvm.h>

#include "argument_definition.h"
#include <vector>
#include <runtime/cdts_wrapper.h>
#include "cdts_java_wrapper.h"


class jni_class
{
private:
	JNIEnv* env;
	jclass cls;

public:
	static std::string get_object_class_name(JNIEnv* env, jobject obj);
	
public:
	jni_class(JNIEnv* env, jclass cls);
	~jni_class() = default;
	
	jmethodID load_method(const std::string& method_name, const argument_definition& return_type, const std::vector<argument_definition>& parameters_types, bool is_static);
	jfieldID load_field(const std::string& field_name, const argument_definition& field_type, bool is_static);
	
	void write_field_to_cdts(int index, cdts_java_wrapper& wrapper, jobject obj, jfieldID field_id, const metaffi_type_info& t);
	void write_cdts_to_field(int index, cdts_java_wrapper& wrapper, jobject obj, jfieldID field_id);
	
	void call(const cdts_java_wrapper& params_wrapper, const cdts_java_wrapper& retval_wrapper, const metaffi_type_info& retval_type, bool instance_required, bool is_constructor, const std::set<uint8_t>& any_type_indices, jmethodID method);
	
	operator jclass(){ return cls; }
	
};


