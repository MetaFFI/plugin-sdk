#pragma once
#ifdef _MSC_VER
#include <corecrt.h> // https://www.reddit.com/r/cpp_questions/comments/qpo93t/error_c2039_invalid_parameter_is_not_a_member_of/
#endif
#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif

#include <functional>
#include <memory>
#include <string>
#include <stdexcept>
#include <runtime/cdt.h>

#include "runtime_manager.h"

class jni_api_wrapper;

class jvm
{
public:
	jvm();
	jvm(const jvm_installed_info& info, const std::string& classpath_option);
	~jvm() = default;

	void fini();

	// returns release environment function
	// TODO: add scoped wrapper
	std::function<void()> get_environment(JNIEnv** env) const;

	static std::string get_exception_description(JNIEnv* env, jthrowable throwable);
	std::string get_exception_description(jthrowable throwable) const;

	explicit operator JavaVM*() const;

private:
	void load_or_create_with_info(const std::string& classpath_option);
	static void check_throw_error(jint err);

	jvm_installed_info m_info{};
	std::shared_ptr<jni_api_wrapper> m_jni_api;
	JavaVM* m_jvm = nullptr;
	bool m_is_destroy = false;
};
