#include "entity.h"
#include "runtime_manager.h"
#include "jni_helpers.h"

#include <stdexcept>

namespace
{
	enum class jni_value_type
	{
		void_type,
		boolean_type,
		byte_type,
		short_type,
		int_type,
		long_type,
		float_type,
		double_type,
		char_type,
		object_type
	};

	std::string get_class_name(JNIEnv* env, jclass cls)
	{
		if(!env || !cls)
		{
			return "";
		}

		jclass class_cls = env->FindClass("java/lang/Class");
		if(!class_cls)
		{
			env->ExceptionClear();
			throw std::runtime_error("Failed to find java/lang/Class");
		}

		jmethodID get_name = env->GetMethodID(class_cls, "getName", "()Ljava/lang/String;");
		if(!get_name)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error("Failed to get Class.getName");
		}

		jstring name_obj = (jstring)env->CallObjectMethod(cls, get_name);
		if(env->ExceptionCheck() || !name_obj)
		{
			std::string error = get_exception_description(env);
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error(error.empty() ? "Failed to call Class.getName" : error);
		}

		const char* name_chars = env->GetStringUTFChars(name_obj, nullptr);
		std::string name = name_chars ? name_chars : "";
		if(name_chars)
		{
			env->ReleaseStringUTFChars(name_obj, name_chars);
		}
		env->DeleteLocalRef(name_obj);
		env->DeleteLocalRef(class_cls);
		return name;
	}

	jni_value_type to_jni_value_type(JNIEnv* env, jclass cls)
	{
		std::string name = get_class_name(env, cls);
		if(name == "void")
		{
			return jni_value_type::void_type;
		}
		if(name == "boolean")
		{
			return jni_value_type::boolean_type;
		}
		if(name == "byte")
		{
			return jni_value_type::byte_type;
		}
		if(name == "short")
		{
			return jni_value_type::short_type;
		}
		if(name == "int")
		{
			return jni_value_type::int_type;
		}
		if(name == "long")
		{
			return jni_value_type::long_type;
		}
		if(name == "float")
		{
			return jni_value_type::float_type;
		}
		if(name == "double")
		{
			return jni_value_type::double_type;
		}
		if(name == "char")
		{
			return jni_value_type::char_type;
		}
		return jni_value_type::object_type;
	}

	jni_value_type get_return_type(JNIEnv* env, const std::vector<jclass>& retval_types)
	{
		if(retval_types.empty())
		{
			return jni_value_type::void_type;
		}
		return to_jni_value_type(env, retval_types[0]);
	}

	std::vector<jclass> make_global_types(JNIEnv* env, const std::vector<jclass>& types)
	{
		std::vector<jclass> result;
		result.reserve(types.size());
		for(jclass cls : types)
		{
			if(!cls)
			{
				result.push_back(nullptr);
				continue;
			}
			jclass global_ref = (jclass)env->NewGlobalRef(cls);
			if(!global_ref)
			{
				throw std::runtime_error("Failed to create global reference for Java type");
			}
			result.push_back(global_ref);
		}
		return result;
	}
}

CallableEntity::CallableEntity(jvm_runtime_manager* runtime_manager,
	jclass cls,
	jmethodID method,
	const std::vector<jclass>& params_types,
	const std::vector<jclass>& retval_types,
	bool instance_required)
	: m_runtimeManager(runtime_manager), m_methodId(method), m_instanceRequired(instance_required)
{
	if(!m_runtimeManager)
	{
		throw std::runtime_error("Runtime manager is null");
	}
	JNIEnv* env = nullptr;
	auto release_env = m_runtimeManager->get_env(&env);
	m_cls = (jclass)env->NewGlobalRef(cls);
	m_paramsTypes = make_global_types(env, params_types);
	m_retvalTypes = make_global_types(env, retval_types);
	release_env();
	if(!m_cls)
	{
		throw std::runtime_error("Failed to create global reference for Java class");
	}
}

CallableEntity::~CallableEntity()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(m_cls && m_runtimeManager)
	{
		JNIEnv* env = nullptr;
		auto release_env = m_runtimeManager->get_env(&env);
		for(auto* type_cls : m_paramsTypes)
		{
			if(type_cls)
			{
				env->DeleteGlobalRef(type_cls);
			}
		}
		for(auto* type_cls : m_retvalTypes)
		{
			if(type_cls)
			{
				env->DeleteGlobalRef(type_cls);
			}
		}
		env->DeleteGlobalRef(m_cls);
		release_env();
		m_cls = nullptr;
		m_paramsTypes.clear();
		m_retvalTypes.clear();
	}
}

void CallableEntity::ensure_ready() const
{
	if(!m_runtimeManager)
	{
		throw std::runtime_error("Runtime manager is null");
	}
	if(!m_cls || !m_methodId)
	{
		throw std::runtime_error("Callable entity is not initialized");
	}
}

	jvalue CallableEntity::call(const std::vector<jvalue>& args)
{
	return call(nullptr, args);
}

jvalue CallableEntity::call(jobject instance, const std::vector<jvalue>& args)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	ensure_ready();

	if(m_instanceRequired && !instance)
	{
		throw std::runtime_error("Instance is required for Java method call");
	}

	JNIEnv* env = nullptr;
	auto release_env = m_runtimeManager->get_env(&env);

	const jvalue* argv = args.empty() ? nullptr : args.data();
	jvalue result{};

	if(m_isConstructor)
	{
		jobject obj = env->NewObjectA(m_cls, m_methodId, argv);
		if(env->ExceptionCheck() || !obj)
		{
			std::string error = get_exception_description(env);
			release_env();
			throw std::runtime_error(error.empty() ? "Failed to create Java object" : error);
		}
		result.l = obj;
		release_env();
		return result;
	}

	jni_value_type ret_type = get_return_type(env, m_retvalTypes);
	if(m_instanceRequired)
	{
		switch(ret_type)
		{
			case jni_value_type::void_type:
				env->CallVoidMethodA(instance, m_methodId, argv);
				break;
			case jni_value_type::boolean_type:
				result.z = env->CallBooleanMethodA(instance, m_methodId, argv);
				break;
			case jni_value_type::byte_type:
				result.b = env->CallByteMethodA(instance, m_methodId, argv);
				break;
			case jni_value_type::short_type:
				result.s = env->CallShortMethodA(instance, m_methodId, argv);
				break;
			case jni_value_type::int_type:
				result.i = env->CallIntMethodA(instance, m_methodId, argv);
				break;
			case jni_value_type::long_type:
				result.j = env->CallLongMethodA(instance, m_methodId, argv);
				break;
			case jni_value_type::float_type:
				result.f = env->CallFloatMethodA(instance, m_methodId, argv);
				break;
			case jni_value_type::double_type:
				result.d = env->CallDoubleMethodA(instance, m_methodId, argv);
				break;
			case jni_value_type::char_type:
				result.c = env->CallCharMethodA(instance, m_methodId, argv);
				break;
			case jni_value_type::object_type:
				result.l = env->CallObjectMethodA(instance, m_methodId, argv);
				break;
		}
	}
	else
	{
		switch(ret_type)
		{
			case jni_value_type::void_type:
				env->CallStaticVoidMethodA(m_cls, m_methodId, argv);
				break;
			case jni_value_type::boolean_type:
				result.z = env->CallStaticBooleanMethodA(m_cls, m_methodId, argv);
				break;
			case jni_value_type::byte_type:
				result.b = env->CallStaticByteMethodA(m_cls, m_methodId, argv);
				break;
			case jni_value_type::short_type:
				result.s = env->CallStaticShortMethodA(m_cls, m_methodId, argv);
				break;
			case jni_value_type::int_type:
				result.i = env->CallStaticIntMethodA(m_cls, m_methodId, argv);
				break;
			case jni_value_type::long_type:
				result.j = env->CallStaticLongMethodA(m_cls, m_methodId, argv);
				break;
			case jni_value_type::float_type:
				result.f = env->CallStaticFloatMethodA(m_cls, m_methodId, argv);
				break;
			case jni_value_type::double_type:
				result.d = env->CallStaticDoubleMethodA(m_cls, m_methodId, argv);
				break;
			case jni_value_type::char_type:
				result.c = env->CallStaticCharMethodA(m_cls, m_methodId, argv);
				break;
			case jni_value_type::object_type:
				result.l = env->CallStaticObjectMethodA(m_cls, m_methodId, argv);
				break;
		}
	}

	if(env->ExceptionCheck())
	{
		std::string error = get_exception_description(env);
		release_env();
		throw std::runtime_error(error.empty() ? "Failed to call Java method" : error);
	}

	release_env();
	return result;
}

const std::vector<jclass>& CallableEntity::get_params_types() const
{
	return m_paramsTypes;
}

const std::vector<jclass>& CallableEntity::get_retval_types() const
{
	return m_retvalTypes;
}

VariableEntity::VariableEntity(jvm_runtime_manager* runtime_manager,
	jclass cls,
	jfieldID field,
	const std::vector<jclass>& params_types,
	const std::vector<jclass>& retval_types,
	bool instance_required)
	: m_runtimeManager(runtime_manager), m_fieldId(field), m_instanceRequired(instance_required)
{
	if(!m_runtimeManager)
	{
		throw std::runtime_error("Runtime manager is null");
	}
	JNIEnv* env = nullptr;
	auto release_env = m_runtimeManager->get_env(&env);
	m_cls = (jclass)env->NewGlobalRef(cls);
	m_paramsTypes = make_global_types(env, params_types);
	m_retvalTypes = make_global_types(env, retval_types);
	release_env();
	if(!m_cls)
	{
		throw std::runtime_error("Failed to create global reference for Java class");
	}
}

VariableEntity::~VariableEntity()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(m_cls && m_runtimeManager)
	{
		JNIEnv* env = nullptr;
		auto release_env = m_runtimeManager->get_env(&env);
		for(auto* type_cls : m_paramsTypes)
		{
			if(type_cls)
			{
				env->DeleteGlobalRef(type_cls);
			}
		}
		for(auto* type_cls : m_retvalTypes)
		{
			if(type_cls)
			{
				env->DeleteGlobalRef(type_cls);
			}
		}
		env->DeleteGlobalRef(m_cls);
		release_env();
		m_cls = nullptr;
		m_paramsTypes.clear();
		m_retvalTypes.clear();
	}
}

void VariableEntity::ensure_ready() const
{
	if(!m_runtimeManager)
	{
		throw std::runtime_error("Runtime manager is null");
	}
	if(!m_cls || !m_fieldId)
	{
		throw std::runtime_error("Variable entity is not initialized");
	}
}

jvalue VariableEntity::get(jobject instance)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	ensure_ready();

	if(m_instanceRequired && !instance)
	{
		throw std::runtime_error("Instance is required for Java field access");
	}

	JNIEnv* env = nullptr;
	auto release_env = m_runtimeManager->get_env(&env);

	jvalue result{};
	jni_value_type field_type;
	if(!m_retvalTypes.empty())
	{
		field_type = to_jni_value_type(env, m_retvalTypes[0]);
	}
	else if(!m_paramsTypes.empty())
	{
		field_type = to_jni_value_type(env, m_paramsTypes[0]);
	}
	else
	{
		release_env();
		throw std::runtime_error("Field type information is missing");
	}

	if(m_instanceRequired)
	{
		switch(field_type)
		{
			case jni_value_type::boolean_type:
				result.z = env->GetBooleanField(instance, m_fieldId);
				break;
			case jni_value_type::byte_type:
				result.b = env->GetByteField(instance, m_fieldId);
				break;
			case jni_value_type::short_type:
				result.s = env->GetShortField(instance, m_fieldId);
				break;
			case jni_value_type::int_type:
				result.i = env->GetIntField(instance, m_fieldId);
				break;
			case jni_value_type::long_type:
				result.j = env->GetLongField(instance, m_fieldId);
				break;
			case jni_value_type::float_type:
				result.f = env->GetFloatField(instance, m_fieldId);
				break;
			case jni_value_type::double_type:
				result.d = env->GetDoubleField(instance, m_fieldId);
				break;
			case jni_value_type::char_type:
				result.c = env->GetCharField(instance, m_fieldId);
				break;
			case jni_value_type::object_type:
				result.l = env->GetObjectField(instance, m_fieldId);
				break;
			case jni_value_type::void_type:
				release_env();
				throw std::runtime_error("Void is not valid for field getter");
		}
	}
	else
	{
		switch(field_type)
		{
			case jni_value_type::boolean_type:
				result.z = env->GetStaticBooleanField(m_cls, m_fieldId);
				break;
			case jni_value_type::byte_type:
				result.b = env->GetStaticByteField(m_cls, m_fieldId);
				break;
			case jni_value_type::short_type:
				result.s = env->GetStaticShortField(m_cls, m_fieldId);
				break;
			case jni_value_type::int_type:
				result.i = env->GetStaticIntField(m_cls, m_fieldId);
				break;
			case jni_value_type::long_type:
				result.j = env->GetStaticLongField(m_cls, m_fieldId);
				break;
			case jni_value_type::float_type:
				result.f = env->GetStaticFloatField(m_cls, m_fieldId);
				break;
			case jni_value_type::double_type:
				result.d = env->GetStaticDoubleField(m_cls, m_fieldId);
				break;
			case jni_value_type::char_type:
				result.c = env->GetStaticCharField(m_cls, m_fieldId);
				break;
			case jni_value_type::object_type:
				result.l = env->GetStaticObjectField(m_cls, m_fieldId);
				break;
			case jni_value_type::void_type:
				release_env();
				throw std::runtime_error("Void is not valid for field getter");
		}
	}

	if(env->ExceptionCheck())
	{
		std::string error = get_exception_description(env);
		release_env();
		throw std::runtime_error(error.empty() ? "Failed to get Java field" : error);
	}

	release_env();
	return result;
}

void VariableEntity::set(jobject instance, jvalue value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	ensure_ready();

	if(m_instanceRequired && !instance)
	{
		throw std::runtime_error("Instance is required for Java field access");
	}

	JNIEnv* env = nullptr;
	auto release_env = m_runtimeManager->get_env(&env);

	jni_value_type field_type;
	if(!m_paramsTypes.empty())
	{
		field_type = to_jni_value_type(env, m_paramsTypes[0]);
	}
	else if(!m_retvalTypes.empty())
	{
		field_type = to_jni_value_type(env, m_retvalTypes[0]);
	}
	else
	{
		release_env();
		throw std::runtime_error("Field type information is missing");
	}

	if(m_instanceRequired)
	{
		switch(field_type)
		{
			case jni_value_type::boolean_type:
				env->SetBooleanField(instance, m_fieldId, value.z);
				break;
			case jni_value_type::byte_type:
				env->SetByteField(instance, m_fieldId, value.b);
				break;
			case jni_value_type::short_type:
				env->SetShortField(instance, m_fieldId, value.s);
				break;
			case jni_value_type::int_type:
				env->SetIntField(instance, m_fieldId, value.i);
				break;
			case jni_value_type::long_type:
				env->SetLongField(instance, m_fieldId, value.j);
				break;
			case jni_value_type::float_type:
				env->SetFloatField(instance, m_fieldId, value.f);
				break;
			case jni_value_type::double_type:
				env->SetDoubleField(instance, m_fieldId, value.d);
				break;
			case jni_value_type::char_type:
				env->SetCharField(instance, m_fieldId, value.c);
				break;
			case jni_value_type::object_type:
				env->SetObjectField(instance, m_fieldId, value.l);
				break;
			case jni_value_type::void_type:
				release_env();
				throw std::runtime_error("Void is not valid for field setter");
		}
	}
	else
	{
		switch(field_type)
		{
			case jni_value_type::boolean_type:
				env->SetStaticBooleanField(m_cls, m_fieldId, value.z);
				break;
			case jni_value_type::byte_type:
				env->SetStaticByteField(m_cls, m_fieldId, value.b);
				break;
			case jni_value_type::short_type:
				env->SetStaticShortField(m_cls, m_fieldId, value.s);
				break;
			case jni_value_type::int_type:
				env->SetStaticIntField(m_cls, m_fieldId, value.i);
				break;
			case jni_value_type::long_type:
				env->SetStaticLongField(m_cls, m_fieldId, value.j);
				break;
			case jni_value_type::float_type:
				env->SetStaticFloatField(m_cls, m_fieldId, value.f);
				break;
			case jni_value_type::double_type:
				env->SetStaticDoubleField(m_cls, m_fieldId, value.d);
				break;
			case jni_value_type::char_type:
				env->SetStaticCharField(m_cls, m_fieldId, value.c);
				break;
			case jni_value_type::object_type:
				env->SetStaticObjectField(m_cls, m_fieldId, value.l);
				break;
			case jni_value_type::void_type:
				release_env();
				throw std::runtime_error("Void is not valid for field setter");
		}
	}

	if(env->ExceptionCheck())
	{
		std::string error = get_exception_description(env);
		release_env();
		throw std::runtime_error(error.empty() ? "Failed to set Java field" : error);
	}

	release_env();
}

const std::vector<jclass>& VariableEntity::get_params_types() const
{
	return m_paramsTypes;
}

const std::vector<jclass>& VariableEntity::get_retval_types() const
{
	return m_retvalTypes;
}

JavaMethod::JavaMethod(jvm_runtime_manager* runtime_manager,
	jclass cls,
	jmethodID method,
	const std::vector<jclass>& params_types,
	const std::vector<jclass>& retval_types,
	bool instance_required)
	: CallableEntity(runtime_manager, cls, method, params_types, retval_types, instance_required)
{
}

JavaConstructor::JavaConstructor(jvm_runtime_manager* runtime_manager,
	jclass cls,
	jmethodID method,
	const std::vector<jclass>& params_types,
	const std::vector<jclass>& retval_types,
	bool instance_required)
	: CallableEntity(runtime_manager, cls, method, params_types, retval_types, instance_required)
{
	m_isConstructor = true;
}

JavaFieldGetter::JavaFieldGetter(jvm_runtime_manager* runtime_manager,
	jclass cls,
	jfieldID field,
	const std::vector<jclass>& params_types,
	const std::vector<jclass>& retval_types,
	bool instance_required)
	: VariableEntity(runtime_manager, cls, field, params_types, retval_types, instance_required)
{
}

JavaFieldSetter::JavaFieldSetter(jvm_runtime_manager* runtime_manager,
	jclass cls,
	jfieldID field,
	const std::vector<jclass>& params_types,
	const std::vector<jclass>& retval_types,
	bool instance_required)
	: VariableEntity(runtime_manager, cls, field, params_types, retval_types, instance_required)
{
}
