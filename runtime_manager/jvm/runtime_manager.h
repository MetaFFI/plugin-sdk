#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <functional>

#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif

class Module;
class jvm;

enum class jvm_vendor
{
	openjdk,
	oracle,
	microsoft,
	adoptium,
	amazon,
	azul,
	graalvm,
	ibm,
	sap,
	bellsoft,
	redhat,
	alibaba,
	tencent,
	huawei,
	jetbrains,
	apple,
	unknown
};

struct jvm_installed_info
{
	jvm_vendor vendor = jvm_vendor::unknown;
	std::string version;
	std::string home;
	std::string libjvm_path;
};

class jvm_runtime_manager
{
public:
	explicit jvm_runtime_manager(const jvm_installed_info& info);
	explicit jvm_runtime_manager(const jvm_installed_info& info, const std::string& classpath_option);
	~jvm_runtime_manager();

	static std::vector<jvm_installed_info> detect_installed_jvms();
	static jvm_installed_info select_highest_installed_jvm();
	static std::shared_ptr<jvm_runtime_manager> create(const jvm_installed_info& info, const std::string& classpath_option = "");

	void load_runtime();
	void release_runtime();

	std::shared_ptr<Module> load_module(const std::string& module_path);
	std::shared_ptr<Module> load_module(const std::string& module_path, const std::string& classpath);

	bool is_runtime_loaded() const;
	const jvm_installed_info& get_jvm_info() const;

	// Returns true if the environment needs to be released via release_env().
	bool get_env(JNIEnv** env) const;

	// Detaches the current thread from the JVM. Only call when get_env() returned true.
	void release_env() const;

private:
	jvm_installed_info m_info;
	std::shared_ptr<jvm> m_jvm;
	bool m_isRuntimeLoaded = false;
	mutable std::mutex m_mutex;
	std::string m_classpath_option;

	void ensure_jvm_loaded();
	static std::string normalize_vendor(const std::string& vendor);
public:
	static jvm_vendor map_vendor(const std::string& vendor);
private:
};
