#pragma once

#include <string>
#include <memory>
#include <vector>
#include <mutex>

#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif

class jvm_runtime_manager;
class Entity;

class Module
{
public:
	Module(jvm_runtime_manager* runtime_manager, const std::string& module_path);
	~Module();

	Module(const Module& other);
	Module(Module&& other) noexcept;
	Module& operator=(const Module& other);
	Module& operator=(Module&& other) noexcept;

	const std::string& get_module_path() const;

	std::shared_ptr<Entity> load_entity(
		const std::string& entity_path,
		const std::vector<jclass>& params_types,
		const std::vector<jclass>& retval_types
	);

	void unload();

private:
	jvm_runtime_manager* m_runtimeManager = nullptr;
	std::string m_modulePath;
	jobject m_classLoader = nullptr;
	mutable std::mutex m_mutex;

	void ensure_class_loader();
	jclass load_class(JNIEnv* env, const std::string& class_name);
	static std::vector<std::string> split_classpath(const std::string& classpath);
	static std::string to_url_path(const std::string& path);
	static std::string normalize_class_name(const std::string& class_name);
};
