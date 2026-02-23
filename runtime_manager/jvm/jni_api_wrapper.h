#pragma once

#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif

#include <memory>
#include <mutex>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#endif

class jni_api_wrapper
{
public:
	using JNI_CreateJavaVM_t = jint (JNICALL *)(JavaVM**, void**, void*);
	using JNI_GetDefaultJavaVMInitArgs_t = jint (JNICALL *)(void*);
	using JNI_GetCreatedJavaVMs_t = jint (JNICALL *)(JavaVM**, jsize, jsize*);

	explicit jni_api_wrapper(const std::string& libjvm_path);

	JNI_CreateJavaVM_t create_java_vm = nullptr;
	JNI_GetDefaultJavaVMInitArgs_t get_default_java_vm_init_args = nullptr;
	JNI_GetCreatedJavaVMs_t get_created_java_vms = nullptr;
	std::string get_loaded_path() const;

private:
#ifdef _WIN32
	HMODULE m_libjvm = nullptr;
	static HMODULE s_libjvm;
#else
	void* m_libjvm = nullptr;
	static void* s_libjvm;
	static std::string s_libjvm_path;
#endif
	static std::mutex s_libjvm_mutex;
};
