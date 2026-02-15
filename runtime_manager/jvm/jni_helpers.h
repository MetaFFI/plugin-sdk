#pragma once

#include <string>
#include <stdexcept>

#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif

class jni_env_guard
{
public:
	explicit jni_env_guard(JavaVM* jvm): m_jvm(jvm)
	{
		if(!m_jvm)
		{
			throw std::runtime_error("JVM is null");
		}

		auto res = m_jvm->GetEnv(reinterpret_cast<void**>(&m_env), JNI_VERSION_1_8);
		if(res == JNI_EDETACHED)
		{
			if(m_jvm->AttachCurrentThread(reinterpret_cast<void**>(&m_env), nullptr) != JNI_OK)
			{
				throw std::runtime_error("Failed to attach current thread to JVM");
			}
			m_attached = true;
		}
		else if(res == JNI_EVERSION)
		{
			throw std::runtime_error("Unsupported JNI version");
		}
	}

	~jni_env_guard()
	{
		if(m_attached && m_jvm)
		{
			m_jvm->DetachCurrentThread();
		}
	}

	JNIEnv* get() const { return m_env; }

private:
	JavaVM* m_jvm = nullptr;
	JNIEnv* m_env = nullptr;
	bool m_attached = false;
};

inline std::string get_exception_description(JNIEnv* env)
{
	if(!env)
	{
		return "JNIEnv is null";
	}

	jthrowable throwable = env->ExceptionOccurred();
	if(!throwable)
	{
		return "";
	}

	env->ExceptionClear();

	// Unwrap InvocationTargetException / UndeclaredThrowableException to get the root cause.
	// Java reflection wraps checked and unchecked exceptions in these wrappers.
	jclass ite_class = env->FindClass("java/lang/reflect/InvocationTargetException");
	jclass ute_class = env->FindClass("java/lang/reflect/UndeclaredThrowableException");
	if(ite_class || ute_class)
	{
		jclass throwable_class_tmp = env->FindClass("java/lang/Throwable");
		jmethodID get_cause = throwable_class_tmp ? env->GetMethodID(throwable_class_tmp, "getCause", "()Ljava/lang/Throwable;") : nullptr;
		if(get_cause)
		{
			// Walk the cause chain to unwrap reflection wrappers
			while((ite_class && env->IsInstanceOf(throwable, ite_class)) ||
			      (ute_class && env->IsInstanceOf(throwable, ute_class)))
			{
				jthrowable cause = (jthrowable)env->CallObjectMethod(throwable, get_cause);
				if(!cause) break;
				env->DeleteLocalRef(throwable);
				throwable = cause;
			}
		}
		if(throwable_class_tmp) env->DeleteLocalRef(throwable_class_tmp);
	}
	if(ite_class) env->DeleteLocalRef(ite_class);
	if(ute_class) env->DeleteLocalRef(ute_class);
	if(env->ExceptionCheck()) env->ExceptionClear();

	jclass throwable_class = env->FindClass("java/lang/Throwable");
	if(!throwable_class)
	{
		env->ExceptionClear();
		return "Failed to find java/lang/Throwable";
	}

	jmethodID to_string = env->GetMethodID(throwable_class, "toString", "()Ljava/lang/String;");
	if(!to_string)
	{
		env->ExceptionClear();
		return "Failed to get Throwable.toString";
	}

	jobject str_obj = env->CallObjectMethod(throwable, to_string);
	if(!str_obj)
	{
		if(env->ExceptionCheck())
		{
			env->ExceptionClear();
		}
		return "Failed to call Throwable.toString";
	}

	const char* cstr = env->GetStringUTFChars((jstring)str_obj, nullptr);
	std::string result = cstr ? cstr : "";
	if(cstr)
	{
		env->ReleaseStringUTFChars((jstring)str_obj, cstr);
	}
	env->DeleteLocalRef(throwable);
	env->DeleteLocalRef(throwable_class);
	env->DeleteLocalRef(str_obj);
	return result;
}
