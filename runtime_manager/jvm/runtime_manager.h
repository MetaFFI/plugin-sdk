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
	~jvm_runtime_manager();

	static std::vector<jvm_installed_info> detect_installed_jvms();

	void load_runtime();
	void release_runtime();

	std::shared_ptr<Module> load_module(const std::string& module_path);

	bool is_runtime_loaded() const;
	const jvm_installed_info& get_jvm_info() const;

	std::function<void()> get_env(JNIEnv** env) const;

private:
	jvm_installed_info m_info;
	std::shared_ptr<jvm> m_jvm;
	bool m_isRuntimeLoaded = false;
	mutable std::mutex m_mutex;

	void ensure_jvm_loaded();
	static std::string normalize_vendor(const std::string& vendor);
public:
	static jvm_vendor map_vendor(const std::string& vendor);
private:
};
