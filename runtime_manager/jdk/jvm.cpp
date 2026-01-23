#include "jvm.h"
#include "jni_api_wrapper.h"
#include "runtime_manager.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <utils/scope_guard.hpp>
#include <utils/env_utils.h>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace metaffi::utils;

namespace
{
	void set_env_var(const char* key, const std::string& value)
	{
#ifdef _WIN32
		_putenv_s(key, value.c_str());
#else
		setenv(key, value.c_str(), 1);
#endif
	}

#ifdef _WIN32
	void prepare_windows_jvm(const jvm_installed_info& info)
	{
		if(info.home.empty())
		{
			return;
		}

		std::filesystem::path bin_path = std::filesystem::path(info.home) / "bin";
		std::filesystem::path server_path = bin_path / "server";

		SetDllDirectoryW(bin_path.wstring().c_str());
		SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
		AddDllDirectory(bin_path.wstring().c_str());
		AddDllDirectory(server_path.wstring().c_str());

		std::string existing_path = get_env_var("PATH");
		std::string prefix = bin_path.string() + ";" + server_path.string();
		std::string new_path = prefix + (existing_path.empty() ? "" : ";" + existing_path);
		set_env_var("PATH", new_path);
		set_env_var("JAVA_HOME", info.home);
		set_env_var("JDK_HOME", info.home);

		if(!info.libjvm_path.empty())
		{
			std::wstring wjvm = std::filesystem::path(info.libjvm_path).wstring();
			LoadLibraryExW(wjvm.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
		}
	}
#endif
}

//--------------------------------------------------------------------
jvm::jvm()
{
	jsize n_vms = 0;
	check_throw_error(JNI_GetCreatedJavaVMs(nullptr, 0, &n_vms));

	if(n_vms > 0)
	{
		check_throw_error(JNI_GetCreatedJavaVMs(&m_jvm, 1, &n_vms));
		return;
	}

#ifdef _WIN32
#define SEPARATOR ';'
#else
#define SEPARATOR ':'
#endif

	std::string metaffi_home = get_env_var("METAFFI_HOME");
	if(metaffi_home.empty())
	{
		throw std::runtime_error("METAFFI_HOME environment variable is not set");
	}

	std::stringstream ss;
	ss << "-Djava.class.path=." << SEPARATOR << ".." << SEPARATOR << metaffi_home << "/sdk/api/jvm/metaffi.api.jar";
	std::string classpath = get_env_var("CLASSPATH");
	if(!classpath.empty())
	{
		ss << SEPARATOR << classpath;
	}

	std::string cp_option = ss.str();

	JavaVMInitArgs vm_args = {};
	vm_args.version = JNI_VERSION_1_8;
	vm_args.nOptions = 1;
	vm_args.options = new JavaVMOption[1];
	vm_args.options[0].optionString = const_cast<char*>(cp_option.c_str());
	vm_args.ignoreUnrecognized = JNI_FALSE;

	JNIEnv* penv = nullptr;
	jint res = JNI_CreateJavaVM(&m_jvm, reinterpret_cast<void**>(&penv), &vm_args);
	delete[] vm_args.options;
	vm_args.options = nullptr;
	check_throw_error(res);

	m_is_destroy = true;
}
//--------------------------------------------------------------------
jvm::jvm(const jvm_installed_info& info, const std::string& classpath_option)
	: m_info(info)
{
	load_or_create_with_info(classpath_option);
}
//--------------------------------------------------------------------
void jvm::load_or_create_with_info(const std::string& classpath_option)
{
	if(m_info.libjvm_path.empty())
	{
		throw std::runtime_error("libjvm_path is empty in jvm_installed_info");
	}

#ifdef _WIN32
	prepare_windows_jvm(m_info);
#else
	if(!m_info.home.empty())
	{
		set_env_var("JAVA_HOME", m_info.home);
		set_env_var("JDK_HOME", m_info.home);
	}
#endif

	m_jni_api = std::make_shared<jni_api_wrapper>(m_info.libjvm_path);

	jsize n_vms = 0;
	check_throw_error(m_jni_api->get_created_java_vms(nullptr, 0, &n_vms));
	if(n_vms > 0)
	{
		check_throw_error(m_jni_api->get_created_java_vms(&m_jvm, 1, &n_vms));
		return;
	}

	JavaVMOption options[1];
	options[0].optionString = const_cast<char*>(classpath_option.c_str());
	options[0].extraInfo = nullptr;

	JavaVMInitArgs vm_args{};
	vm_args.version = JNI_VERSION_21;
	vm_args.nOptions = 1;
	vm_args.options = options;
	vm_args.ignoreUnrecognized = JNI_FALSE;

	JNIEnv* env = nullptr;
	check_throw_error(m_jni_api->create_java_vm(&m_jvm, reinterpret_cast<void**>(&env), &vm_args));
}
//--------------------------------------------------------------------
void jvm::fini()
{
	if(m_jvm && m_is_destroy)
	{
		jint res = m_jvm->DestroyJavaVM();
		if(res != JNI_OK)
		{
			printf("Failed to destroy JVM: %ld\n", res);
		}
		m_jvm = nullptr;
	}
}
//--------------------------------------------------------------------
std::function<void()> jvm::get_environment(JNIEnv** env) const
{
	if(!m_jvm)
	{
		throw std::runtime_error("JVM is not initialized");
	}

	bool did_attach_thread = false;
	auto get_env_result = m_jvm->GetEnv(reinterpret_cast<void**>(env), JNI_VERSION_1_8);
	if(get_env_result == JNI_EDETACHED)
	{
		if(m_jvm->AttachCurrentThread(reinterpret_cast<void**>(env), nullptr) == JNI_OK)
		{
			did_attach_thread = true;
		}
		else
		{
			throw std::runtime_error("Failed to attach environment to current thread");
		}
	}
	else if(get_env_result == JNI_EVERSION)
	{
		throw std::runtime_error("Failed to get JVM environment - unsupported JNI version");
	}

	return did_attach_thread ? std::function<void()>([this](){ m_jvm->DetachCurrentThread(); }) : []() {};
}
//--------------------------------------------------------------------
void jvm::check_throw_error(jint err)
{
	if(err == JNI_OK)
	{
		return;
	}

	switch(err)
	{
		case JNI_ERR:
			throw std::runtime_error("JNI error");
		case JNI_EDETACHED:
			throw std::runtime_error("Thread detached from the JVM");
		case JNI_EVERSION:
			throw std::runtime_error("JNI version error");
		case JNI_ENOMEM:
			throw std::runtime_error("Not enough memory");
		case JNI_EEXIST:
			throw std::runtime_error("JVM already created");
		case JNI_EINVAL:
			throw std::runtime_error("Invalid JVM argument");
		default:
			throw std::runtime_error("Unknown JNI error");
	}
}
//--------------------------------------------------------------------
std::string jvm::get_exception_description(jthrowable throwable) const
{
	JNIEnv* penv = nullptr;
	auto release_env = get_environment(&penv);
	scope_guard sg_env([&](){ release_env(); });

	return get_exception_description(penv, throwable);
}
//--------------------------------------------------------------------
std::string jvm::get_exception_description(JNIEnv* penv, jthrowable throwable)
{
	if(!penv || !throwable)
	{
		return "JNI exception";
	}

	penv->ExceptionClear();

	jclass throwable_class = penv->FindClass("java/lang/Throwable");
	if(!throwable_class)
	{
		penv->ExceptionClear();
		return "Failed to find java/lang/Throwable";
	}

	jmethodID throwable_toString = penv->GetMethodID(throwable_class, "toString", "()Ljava/lang/String;");
	if(!throwable_toString)
	{
		penv->ExceptionClear();
		return "Failed to get Throwable.toString";
	}

	jobject str = penv->CallObjectMethod(throwable, throwable_toString);
	if(!str)
	{
		penv->ExceptionClear();
		return "Failed to call Throwable.toString";
	}

	scope_guard sg([&](){ penv->DeleteLocalRef(str); });

	const char* cstr = penv->GetStringUTFChars((jstring)str, nullptr);
	std::string res = cstr ? cstr : "";
	if(cstr)
	{
		penv->ReleaseStringUTFChars((jstring)str, cstr);
	}

	return res;
}
//--------------------------------------------------------------------
jvm::operator JavaVM*() const
{
	return m_jvm;
}
