#pragma once

#include <string>
#include <vector>
#include <mutex>

#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif

class jdk_runtime_manager;

class Entity
{
public:
	virtual ~Entity() = default;
	virtual const std::vector<jclass>& get_params_types() const = 0;
	virtual const std::vector<jclass>& get_retval_types() const = 0;
};

class CallableEntity : public Entity
{
public:
	jvalue call(const std::vector<jvalue>& args);
	jvalue call(jobject instance, const std::vector<jvalue>& args);
	const std::vector<jclass>& get_params_types() const override;
	const std::vector<jclass>& get_retval_types() const override;

protected:
	CallableEntity(jdk_runtime_manager* runtime_manager,
		jclass cls,
		jmethodID method,
		const std::vector<jclass>& params_types,
		const std::vector<jclass>& retval_types,
		bool instance_required);
	virtual ~CallableEntity();
	CallableEntity(const CallableEntity&) = delete;
	CallableEntity& operator=(const CallableEntity&) = delete;
	CallableEntity(CallableEntity&&) = delete;
	CallableEntity& operator=(CallableEntity&&) = delete;

	jdk_runtime_manager* m_runtimeManager = nullptr;
	jclass m_cls = nullptr;
	jmethodID m_methodId = nullptr;
	std::vector<jclass> m_paramsTypes;
	std::vector<jclass> m_retvalTypes;
	bool m_instanceRequired = false;
	bool m_isConstructor = false;
	mutable std::mutex m_mutex;

	void ensure_ready() const;
};

class VariableEntity : public Entity
{
public:
	jvalue get(jobject instance);
	void set(jobject instance, jvalue value);
	const std::vector<jclass>& get_params_types() const override;
	const std::vector<jclass>& get_retval_types() const override;

protected:
	VariableEntity(jdk_runtime_manager* runtime_manager,
		jclass cls,
		jfieldID field,
		const std::vector<jclass>& params_types,
		const std::vector<jclass>& retval_types,
		bool instance_required);
	virtual ~VariableEntity();
	VariableEntity(const VariableEntity&) = delete;
	VariableEntity& operator=(const VariableEntity&) = delete;
	VariableEntity(VariableEntity&&) = delete;
	VariableEntity& operator=(VariableEntity&&) = delete;

	jdk_runtime_manager* m_runtimeManager = nullptr;
	jclass m_cls = nullptr;
	jfieldID m_fieldId = nullptr;
	std::vector<jclass> m_paramsTypes;
	std::vector<jclass> m_retvalTypes;
	bool m_instanceRequired = false;
	mutable std::mutex m_mutex;

	void ensure_ready() const;
};

class JavaMethod : public CallableEntity
{
public:
	JavaMethod(jdk_runtime_manager* runtime_manager,
		jclass cls,
		jmethodID method,
		const std::vector<jclass>& params_types,
		const std::vector<jclass>& retval_types,
		bool instance_required);
	~JavaMethod() override = default;
};

class JavaConstructor : public CallableEntity
{
public:
	JavaConstructor(jdk_runtime_manager* runtime_manager,
		jclass cls,
		jmethodID method,
		const std::vector<jclass>& params_types,
		const std::vector<jclass>& retval_types,
		bool instance_required);
	~JavaConstructor() override = default;
};

class JavaFieldGetter : public VariableEntity
{
public:
	JavaFieldGetter(jdk_runtime_manager* runtime_manager,
		jclass cls,
		jfieldID field,
		const std::vector<jclass>& params_types,
		const std::vector<jclass>& retval_types,
		bool instance_required);
	~JavaFieldGetter() override = default;
};

class JavaFieldSetter : public VariableEntity
{
public:
	JavaFieldSetter(jdk_runtime_manager* runtime_manager,
		jclass cls,
		jfieldID field,
		const std::vector<jclass>& params_types,
		const std::vector<jclass>& retval_types,
		bool instance_required);
	~JavaFieldSetter() override = default;
};
