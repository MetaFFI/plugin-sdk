#pragma once

#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif

#include <boost/dll.hpp>
#include <memory>
#include <string>

class jni_api_wrapper
{
public:
	using JNI_CreateJavaVM_t = jint (JNICALL *)(JavaVM**, void**, void*);
	using JNI_GetCreatedJavaVMs_t = jint (JNICALL *)(JavaVM**, jsize, jsize*);

	explicit jni_api_wrapper(const std::string& libjvm_path);

	JNI_CreateJavaVM_t create_java_vm = nullptr;
	JNI_GetCreatedJavaVMs_t get_created_java_vms = nullptr;

private:
	std::shared_ptr<boost::dll::shared_library> m_libjvm;
};
