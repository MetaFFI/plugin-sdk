#include "module.h"
#include "runtime_manager.h"
#include "entity.h"
#include "jni_helpers.h"
#include <utils/entity_path_parser.h>
#include <utils/env_utils.h>

#include <filesystem>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <cstring>
#include <cstdlib>

namespace
{
	#ifdef _WIN32
	constexpr char classpath_separator = ';';
	std::string file_protocol = "file:///";
	#else
	constexpr char classpath_separator = ':';
	std::string file_protocol = "file://";
	#endif

	std::string to_slash_path(std::string value)
	{
		std::replace(value.begin(), value.end(), '\\', '/');
		return value;
	}

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

	std::string class_to_descriptor(JNIEnv* env, jclass cls)
	{
		std::string name = get_class_name(env, cls);
		if(name.empty())
		{
			return "Ljava/lang/Object;";
		}
		if(name[0] == '[')
		{
			std::replace(name.begin(), name.end(), '.', '/');
			return name;
		}
		if(name == "void")
		{
			return "V";
		}
		if(name == "boolean")
		{
			return "Z";
		}
		if(name == "byte")
		{
			return "B";
		}
		if(name == "short")
		{
			return "S";
		}
		if(name == "int")
		{
			return "I";
		}
		if(name == "long")
		{
			return "J";
		}
		if(name == "float")
		{
			return "F";
		}
		if(name == "double")
		{
			return "D";
		}
		if(name == "char")
		{
			return "C";
		}
		std::replace(name.begin(), name.end(), '.', '/');
		return "L" + name + ";";
	}

	std::string build_method_signature(JNIEnv* env, const std::vector<jclass>& params,
		const std::vector<jclass>& retvals,
		bool is_constructor)
	{
		if(retvals.size() > 1)
		{
			throw std::runtime_error("Java methods support a single return value");
		}

		std::string sig = "(";
		for(const auto& p : params)
		{
			sig += class_to_descriptor(env, p);
		}
		sig += ")";

		if(is_constructor || retvals.empty())
		{
			sig += "V";
		}
		else
		{
			sig += class_to_descriptor(env, retvals[0]);
		}

		return sig;
	}

	jobject get_reflect_method(JNIEnv* env, jclass cls, const std::string& name, bool instance_required)
	{
		jclass class_cls = env->FindClass("java/lang/Class");
		if(!class_cls)
		{
			env->ExceptionClear();
			throw std::runtime_error("Failed to find java/lang/Class");
		}
		jmethodID get_methods = env->GetMethodID(class_cls, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");
		if(!get_methods)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error("Failed to get Class.getDeclaredMethods");
		}

		jobjectArray methods = (jobjectArray)env->CallObjectMethod(cls, get_methods);
		if(env->ExceptionCheck() || !methods)
		{
			std::string error = get_exception_description(env);
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error(error.empty() ? "Failed to get declared methods" : error);
		}

		jclass method_cls = env->FindClass("java/lang/reflect/Method");
		if(!method_cls)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error("Failed to find java/lang/reflect/Method");
		}
		jmethodID get_name = env->GetMethodID(method_cls, "getName", "()Ljava/lang/String;");
		jmethodID get_modifiers = env->GetMethodID(method_cls, "getModifiers", "()I");
		jclass modifier_cls = env->FindClass("java/lang/reflect/Modifier");
		if(!modifier_cls)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(class_cls);
			env->DeleteLocalRef(method_cls);
			throw std::runtime_error("Failed to find java/lang/reflect/Modifier");
		}
		jmethodID is_static = env->GetStaticMethodID(modifier_cls, "isStatic", "(I)Z");

		if(!get_name || !get_modifiers || !is_static)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error("Failed to prepare reflection helpers");
		}

		jobject match = nullptr;
		jsize count = env->GetArrayLength(methods);
		for(jsize i = 0; i < count; i++)
		{
			jobject method = env->GetObjectArrayElement(methods, i);
			if(!method)
			{
				continue;
			}
			jstring name_obj = (jstring)env->CallObjectMethod(method, get_name);
			const char* name_chars = name_obj ? env->GetStringUTFChars(name_obj, nullptr) : nullptr;
			std::string cur_name = name_chars ? name_chars : "";
			if(name_chars)
			{
				env->ReleaseStringUTFChars(name_obj, name_chars);
			}
			if(name_obj)
			{
				env->DeleteLocalRef(name_obj);
			}

			if(cur_name == name)
			{
				jint mods = env->CallIntMethod(method, get_modifiers);
				jboolean is_static_flag = env->CallStaticBooleanMethod(modifier_cls, is_static, mods);
				bool is_static_method = is_static_flag == JNI_TRUE;
				if(is_static_method != (!instance_required))
				{
					env->DeleteLocalRef(method);
					continue;
				}

				if(match)
				{
					env->DeleteLocalRef(method);
					env->DeleteLocalRef(match);
					match = nullptr;
					break;
				}
				match = method;
				continue;
			}
			env->DeleteLocalRef(method);
		}

		env->DeleteLocalRef(methods);
		env->DeleteLocalRef(class_cls);
		env->DeleteLocalRef(method_cls);
		env->DeleteLocalRef(modifier_cls);

		if(!match)
		{
			throw std::runtime_error("Failed to resolve unique Java method for " + name);
		}

		return match;
	}

	jobject get_reflect_constructor(JNIEnv* env, jclass cls)
	{
		jclass class_cls = env->FindClass("java/lang/Class");
		jmethodID get_ctors = env->GetMethodID(class_cls, "getDeclaredConstructors", "()[Ljava/lang/reflect/Constructor;");
		if(!get_ctors)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error("Failed to get Class.getDeclaredConstructors");
		}

		jobjectArray ctors = (jobjectArray)env->CallObjectMethod(cls, get_ctors);
		if(env->ExceptionCheck() || !ctors)
		{
			std::string error = get_exception_description(env);
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error(error.empty() ? "Failed to get declared constructors" : error);
		}

		jsize count = env->GetArrayLength(ctors);
		if(count != 1)
		{
			env->DeleteLocalRef(ctors);
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error("Constructor resolution is ambiguous");
		}

		jobject ctor = env->GetObjectArrayElement(ctors, 0);
		env->DeleteLocalRef(ctors);
		env->DeleteLocalRef(class_cls);
		if(!ctor)
		{
			throw std::runtime_error("Failed to resolve constructor");
		}
		return ctor;
	}

	std::vector<jclass> get_reflect_params(JNIEnv* env, jobject method_or_ctor)
	{
		jclass exec_cls = env->FindClass("java/lang/reflect/Executable");
		if(!exec_cls)
		{
			env->ExceptionClear();
			throw std::runtime_error("Failed to find java/lang/reflect/Executable");
		}
		jmethodID get_params = env->GetMethodID(exec_cls, "getParameterTypes", "()[Ljava/lang/Class;");
		if(!get_params)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(exec_cls);
			throw std::runtime_error("Failed to get Executable.getParameterTypes");
		}

		jobjectArray params = (jobjectArray)env->CallObjectMethod(method_or_ctor, get_params);
		if(env->ExceptionCheck() || !params)
		{
			std::string error = get_exception_description(env);
			env->DeleteLocalRef(exec_cls);
			throw std::runtime_error(error.empty() ? "Failed to get parameter types" : error);
		}

		jsize count = env->GetArrayLength(params);
		std::vector<jclass> result;
		result.reserve(count);
		for(jsize i = 0; i < count; i++)
		{
			jclass type_cls = (jclass)env->GetObjectArrayElement(params, i);
			result.push_back(type_cls);
		}
		env->DeleteLocalRef(params);
		env->DeleteLocalRef(exec_cls);
		return result;
	}

	jclass get_reflect_return_type(JNIEnv* env, jobject method)
	{
		jclass method_cls = env->FindClass("java/lang/reflect/Method");
		if(!method_cls)
		{
			env->ExceptionClear();
			throw std::runtime_error("Failed to find java/lang/reflect/Method");
		}
		jmethodID get_ret = env->GetMethodID(method_cls, "getReturnType", "()Ljava/lang/Class;");
		if(!get_ret)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(method_cls);
			throw std::runtime_error("Failed to get Method.getReturnType");
		}
		jclass ret = (jclass)env->CallObjectMethod(method, get_ret);
		if(env->ExceptionCheck() || !ret)
		{
			std::string error = get_exception_description(env);
			env->DeleteLocalRef(method_cls);
			throw std::runtime_error(error.empty() ? "Failed to get method return type" : error);
		}
		env->DeleteLocalRef(method_cls);
		return ret;
	}

	jobject get_reflect_field(JNIEnv* env, jclass cls, const std::string& name, bool instance_required)
	{
		jclass class_cls = env->FindClass("java/lang/Class");
		if(!class_cls)
		{
			env->ExceptionClear();
			throw std::runtime_error("Failed to find java/lang/Class");
		}
		jmethodID get_field = env->GetMethodID(class_cls, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;");
		if(!get_field)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error("Failed to get Class.getDeclaredField");
		}

		jstring name_obj = env->NewStringUTF(name.c_str());
		jobject field = env->CallObjectMethod(cls, get_field, name_obj);
		env->DeleteLocalRef(name_obj);
		if(env->ExceptionCheck() || !field)
		{
			std::string error = get_exception_description(env);
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error(error.empty() ? "Failed to get field " + name : error);
		}

		jclass field_cls = env->FindClass("java/lang/reflect/Field");
		if(!field_cls)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(class_cls);
			env->DeleteLocalRef(field);
			throw std::runtime_error("Failed to find java/lang/reflect/Field");
		}
		jmethodID get_modifiers = env->GetMethodID(field_cls, "getModifiers", "()I");
		jclass modifier_cls = env->FindClass("java/lang/reflect/Modifier");
		if(!modifier_cls)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(class_cls);
			env->DeleteLocalRef(field_cls);
			env->DeleteLocalRef(field);
			throw std::runtime_error("Failed to find java/lang/reflect/Modifier");
		}
		jmethodID is_static = env->GetStaticMethodID(modifier_cls, "isStatic", "(I)Z");
		if(!get_modifiers || !is_static)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(class_cls);
			throw std::runtime_error("Failed to inspect field modifiers");
		}

		jint mods = env->CallIntMethod(field, get_modifiers);
		jboolean is_static_flag = env->CallStaticBooleanMethod(modifier_cls, is_static, mods);
		bool is_static_field = is_static_flag == JNI_TRUE;
		if(is_static_field != (!instance_required))
		{
			env->DeleteLocalRef(field);
			env->DeleteLocalRef(class_cls);
			env->DeleteLocalRef(field_cls);
			env->DeleteLocalRef(modifier_cls);
			throw std::runtime_error("Field static/instance mismatch for " + name);
		}

		env->DeleteLocalRef(class_cls);
		env->DeleteLocalRef(field_cls);
		env->DeleteLocalRef(modifier_cls);
		return field;
	}

	jclass get_reflect_field_type(JNIEnv* env, jobject field)
	{
		jclass field_cls = env->FindClass("java/lang/reflect/Field");
		if(!field_cls)
		{
			env->ExceptionClear();
			throw std::runtime_error("Failed to find java/lang/reflect/Field");
		}
		jmethodID get_type = env->GetMethodID(field_cls, "getType", "()Ljava/lang/Class;");
		if(!get_type)
		{
			env->ExceptionClear();
			env->DeleteLocalRef(field_cls);
			throw std::runtime_error("Failed to get Field.getType");
		}
		jclass ret = (jclass)env->CallObjectMethod(field, get_type);
		if(env->ExceptionCheck() || !ret)
		{
			std::string error = get_exception_description(env);
			env->DeleteLocalRef(field_cls);
			throw std::runtime_error(error.empty() ? "Failed to get field type" : error);
		}
		env->DeleteLocalRef(field_cls);
		return ret;
	}
}

Module::Module(jvm_runtime_manager* runtime_manager, const std::string& module_path)
	: m_runtimeManager(runtime_manager), m_modulePath(module_path)
{
	ensure_class_loader();
}

Module::Module(jvm_runtime_manager* runtime_manager, const std::string& module_path, const std::string& classpath)
	: m_runtimeManager(runtime_manager), m_modulePath(module_path), m_classpath(classpath)
{
	ensure_class_loader();
}

Module::~Module()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(m_classLoader && m_runtimeManager)
	{
		JNIEnv* env = nullptr;
		auto release_env = m_runtimeManager->get_env(&env);
		env->DeleteGlobalRef(m_classLoader);
		release_env();
		m_classLoader = nullptr;
	}
}

Module::Module(const Module& other)
	: m_runtimeManager(other.m_runtimeManager), m_modulePath(other.m_modulePath), m_classpath(other.m_classpath)
{
	std::lock_guard<std::mutex> lock(other.m_mutex);
	m_classLoader = other.m_classLoader;
	if(m_classLoader && m_runtimeManager)
	{
		JNIEnv* env = nullptr;
		auto release_env = m_runtimeManager->get_env(&env);
		m_classLoader = env->NewGlobalRef(m_classLoader);
		release_env();
	}
}

Module::Module(Module&& other) noexcept
	: m_runtimeManager(other.m_runtimeManager),
	  m_modulePath(std::move(other.m_modulePath)),
	  m_classpath(std::move(other.m_classpath)),
	  m_classLoader(other.m_classLoader)
{
	other.m_classLoader = nullptr;
}

Module& Module::operator=(const Module& other)
{
	if(this != &other)
	{
		std::unique_lock<std::mutex> lock1(m_mutex, std::defer_lock);
		std::unique_lock<std::mutex> lock2(other.m_mutex, std::defer_lock);
		std::lock(lock1, lock2);

		m_runtimeManager = other.m_runtimeManager;
		m_modulePath = other.m_modulePath;
		m_classpath = other.m_classpath;
		m_classLoader = other.m_classLoader;
		if(m_classLoader && m_runtimeManager)
		{
			JNIEnv* env = nullptr;
			auto release_env = m_runtimeManager->get_env(&env);
			m_classLoader = env->NewGlobalRef(m_classLoader);
			release_env();
		}
	}
	return *this;
}

Module& Module::operator=(Module&& other) noexcept
{
	if(this != &other)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_runtimeManager = other.m_runtimeManager;
		m_modulePath = std::move(other.m_modulePath);
		m_classpath = std::move(other.m_classpath);
		m_classLoader = other.m_classLoader;
		other.m_classLoader = nullptr;
	}
	return *this;
}

const std::string& Module::get_module_path() const
{
	return m_modulePath;
}

jclass Module::load_class(const std::string& class_name)
{
	ensure_class_loader();

	if(!m_runtimeManager)
	{
		throw std::runtime_error("Runtime manager is null");
	}

	JNIEnv* env = nullptr;
	auto release_env = m_runtimeManager->get_env(&env);

	jclass local_class = nullptr;
	try
	{
		local_class = load_class(env, class_name);
	}
	catch(...)
	{
		release_env();
		throw;
	}

	if(!local_class)
	{
		release_env();
		throw std::runtime_error("Failed to load Java class");
	}

	jclass global_class = (jclass)env->NewGlobalRef(local_class);
	env->DeleteLocalRef(local_class);
	release_env();
	if(!global_class)
	{
		throw std::runtime_error("Failed to create global reference for Java class");
	}

	return global_class;
}

std::shared_ptr<Entity> Module::load_entity(
	const std::string& entity_path,
	const std::vector<jclass>& params_types,
	const std::vector<jclass>& retval_types)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(!m_runtimeManager)
	{
		throw std::runtime_error("Runtime manager is null");
	}
	if(!m_classLoader)
	{
		throw std::runtime_error("Class loader is not initialized");
	}

	JNIEnv* env = nullptr;
	auto release_env = m_runtimeManager->get_env(&env);

	metaffi::utils::entity_path_parser fp(entity_path);
	if(fp.contains("callable"))
	{
		std::string class_name = fp["class"];
		std::string callable = fp["callable"];
		bool instance_required = fp.contains("instance_required");
		bool is_constructor = (callable == "<init>");

		jclass cls = load_class(env, class_name);
		std::vector<jclass> effective_params = params_types;
		std::vector<jclass> effective_retvals = retval_types;
		bool cleanup_params = false;
		bool cleanup_retvals = false;

		if(effective_params.empty() && effective_retvals.empty())
		{
			if(is_constructor)
			{
				jobject ctor = get_reflect_constructor(env, cls);
				effective_params = get_reflect_params(env, ctor);
				cleanup_params = true;
				env->DeleteLocalRef(ctor);
				effective_retvals = {cls};
			}
			else
			{
				jobject method_obj = get_reflect_method(env, cls, callable, instance_required);
				effective_params = get_reflect_params(env, method_obj);
				cleanup_params = true;
				jclass ret_type = get_reflect_return_type(env, method_obj);
				std::string ret_name = get_class_name(env, ret_type);
				if(ret_name != "void")
				{
					effective_retvals = {ret_type};
					cleanup_retvals = true;
				}
				else
				{
					env->DeleteLocalRef(ret_type);
				}
				env->DeleteLocalRef(method_obj);
			}
		}

		std::string signature = build_method_signature(env, effective_params, effective_retvals, is_constructor);

		jmethodID method = nullptr;
		if(is_constructor)
		{
			method = env->GetMethodID(cls, "<init>", signature.c_str());
		}
		else if(instance_required)
		{
			method = env->GetMethodID(cls, callable.c_str(), signature.c_str());
		}
		else
		{
			method = env->GetStaticMethodID(cls, callable.c_str(), signature.c_str());
		}

		if(env->ExceptionCheck())
		{
			std::string error = get_exception_description(env);
			env->DeleteLocalRef(cls);
			if(cleanup_params)
			{
				for(auto* type_cls : effective_params)
				{
					env->DeleteLocalRef(type_cls);
				}
			}
			if(cleanup_retvals)
			{
				for(auto* type_cls : effective_retvals)
				{
					env->DeleteLocalRef(type_cls);
				}
			}
			release_env();
			throw std::runtime_error(error.empty() ? "Failed to resolve Java method" : error);
		}
		if(!method)
		{
			env->DeleteLocalRef(cls);
			if(cleanup_params)
			{
				for(auto* type_cls : effective_params)
				{
					env->DeleteLocalRef(type_cls);
				}
			}
			if(cleanup_retvals)
			{
				for(auto* type_cls : effective_retvals)
				{
					env->DeleteLocalRef(type_cls);
				}
			}
			release_env();
			throw std::runtime_error("Failed to resolve Java method");
		}

		std::shared_ptr<Entity> entity;
		if(is_constructor)
		{
			entity = std::make_shared<JavaConstructor>(m_runtimeManager, cls, method, effective_params, effective_retvals, false);
		}
		else
		{
			entity = std::make_shared<JavaMethod>(m_runtimeManager, cls, method, effective_params, effective_retvals, instance_required);
		}
		env->DeleteLocalRef(cls);
		if(cleanup_params)
		{
			for(auto* type_cls : effective_params)
			{
				env->DeleteLocalRef(type_cls);
			}
		}
		if(cleanup_retvals)
		{
			for(auto* type_cls : effective_retvals)
			{
				env->DeleteLocalRef(type_cls);
			}
		}
		release_env();
		return entity;
	}
	else if(fp.contains("field"))
	{
		std::string class_name = fp["class"];
		std::string field_name = fp["field"];
		bool instance_required = fp.contains("instance_required");
		bool is_getter = fp.contains("getter");
		bool is_setter = fp.contains("setter");

		if(!is_getter && !is_setter)
		{
			release_env();
			throw std::runtime_error("Field entity path missing getter/setter flag");
		}

		jclass cls = load_class(env, class_name);
		std::vector<jclass> effective_params = params_types;
		std::vector<jclass> effective_retvals = retval_types;
		bool cleanup_params = false;
		bool cleanup_retvals = false;

		if(effective_params.empty() && effective_retvals.empty())
		{
			jobject field = get_reflect_field(env, cls, field_name, instance_required);
			jclass field_type = get_reflect_field_type(env, field);
			if(is_getter)
			{
				effective_retvals = {field_type};
				cleanup_retvals = true;
			}
			else
			{
				effective_params = {field_type};
				cleanup_params = true;
			}
			env->DeleteLocalRef(field);
		}
		else if(is_getter && effective_retvals.empty())
		{
			env->DeleteLocalRef(cls);
			release_env();
			throw std::runtime_error("Field getter requires a return type");
		}
		else if(is_setter && effective_params.empty())
		{
			env->DeleteLocalRef(cls);
			release_env();
			throw std::runtime_error("Field setter requires a parameter type");
		}

		std::string field_sig = is_getter ? class_to_descriptor(env, effective_retvals[0])
		                                  : class_to_descriptor(env, effective_params[0]);

		jfieldID field_id = nullptr;
		if(instance_required)
		{
			field_id = env->GetFieldID(cls, field_name.c_str(), field_sig.c_str());
		}
		else
		{
			field_id = env->GetStaticFieldID(cls, field_name.c_str(), field_sig.c_str());
		}

		if(env->ExceptionCheck())
		{
			std::string error = get_exception_description(env);
			env->DeleteLocalRef(cls);
			if(cleanup_params)
			{
				for(auto* type_cls : effective_params)
				{
					env->DeleteLocalRef(type_cls);
				}
			}
			if(cleanup_retvals)
			{
				for(auto* type_cls : effective_retvals)
				{
					env->DeleteLocalRef(type_cls);
				}
			}
			release_env();
			throw std::runtime_error(error.empty() ? "Failed to resolve Java field" : error);
		}
		if(!field_id)
		{
			env->DeleteLocalRef(cls);
			if(cleanup_params)
			{
				for(auto* type_cls : effective_params)
				{
					env->DeleteLocalRef(type_cls);
				}
			}
			if(cleanup_retvals)
			{
				for(auto* type_cls : effective_retvals)
				{
					env->DeleteLocalRef(type_cls);
				}
			}
			release_env();
			throw std::runtime_error("Failed to resolve Java field");
		}

		std::shared_ptr<Entity> entity;
		if(is_getter)
		{
			entity = std::make_shared<JavaFieldGetter>(m_runtimeManager, cls, field_id, effective_params, effective_retvals, instance_required);
		}
		else
		{
			entity = std::make_shared<JavaFieldSetter>(m_runtimeManager, cls, field_id, effective_params, effective_retvals, instance_required);
		}
		env->DeleteLocalRef(cls);
		if(cleanup_params)
		{
			for(auto* type_cls : effective_params)
			{
				env->DeleteLocalRef(type_cls);
			}
		}
		if(cleanup_retvals)
		{
			for(auto* type_cls : effective_retvals)
			{
				env->DeleteLocalRef(type_cls);
			}
		}
		release_env();
		return entity;
	}

	release_env();
	throw std::runtime_error("Entity path must contain callable or field");
}

void Module::unload()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	// Module cleanup handled by destructor (RAII)
}

void Module::ensure_class_loader()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(m_classLoader)
	{
		return;
	}

	if(!m_runtimeManager)
	{
		throw std::runtime_error("Runtime manager is null");
	}
	if(m_modulePath.empty())
	{
		throw std::runtime_error("Module path is empty");
	}

	std::vector<std::string> entries = split_classpath(m_modulePath);
	if(!m_classpath.empty())
	{
		auto extra_entries = split_classpath(m_classpath);
		entries.insert(entries.end(), extra_entries.begin(), extra_entries.end());
	}
	std::string metaffi_home = get_env_var("METAFFI_HOME");
	if(!metaffi_home.empty())
	{
		std::filesystem::path api_jar = std::filesystem::path(metaffi_home) / "jvm" / "metaffi.api.jar";
		if(!std::filesystem::exists(api_jar))
		{
			api_jar = std::filesystem::path(metaffi_home) / "sdk" / "api" / "jvm" / "metaffi.api.jar";
		}

		if(std::filesystem::exists(api_jar))
		{
			entries.push_back(api_jar.string());
		}
	}

	for(const auto& entry : entries)
	{
		std::filesystem::path entry_path(entry);
		if(!std::filesystem::exists(entry_path))
		{
			throw std::runtime_error("Module path entry does not exist: " + entry);
		}
	}

	JNIEnv* env = nullptr;
	auto release_env = m_runtimeManager->get_env(&env);

	jclass m_classLoaderclass = env->FindClass("java/lang/ClassLoader");
	if(!m_classLoaderclass)
	{
		std::string error = get_exception_description(env);
		release_env();
		throw std::runtime_error(error.empty() ? "Failed to find ClassLoader" : error);
	}

	jmethodID get_system_class_loader = env->GetStaticMethodID(m_classLoaderclass, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
	if(!get_system_class_loader)
	{
		std::string error = get_exception_description(env);
		release_env();
		throw std::runtime_error(error.empty() ? "Failed to get ClassLoader.getSystemClassLoader" : error);
	}

	jobject parent_loader = env->CallStaticObjectMethod(m_classLoaderclass, get_system_class_loader);
	if(env->ExceptionCheck() || !parent_loader)
	{
		std::string error = get_exception_description(env);
		release_env();
		throw std::runtime_error(error.empty() ? "Failed to get system class loader" : error);
	}

	jclass url_class = env->FindClass("java/net/URL");
	jclass url_class_loader = env->FindClass("java/net/URLClassLoader");
	if(!url_class || !url_class_loader)
	{
		std::string error = get_exception_description(env);
		release_env();
		throw std::runtime_error(error.empty() ? "Failed to find URL classes" : error);
	}

	jmethodID url_ctor = env->GetMethodID(url_class, "<init>", "(Ljava/lang/String;)V");
	jmethodID url_m_classLoaderctor = env->GetMethodID(url_class_loader, "<init>", "([Ljava/net/URL;Ljava/lang/ClassLoader;)V");
	if(!url_ctor || !url_m_classLoaderctor)
	{
		std::string error = get_exception_description(env);
		release_env();
		throw std::runtime_error(error.empty() ? "Failed to get URLClassLoader constructors" : error);
	}

	jobjectArray url_array = env->NewObjectArray((jsize)entries.size(), url_class, nullptr);
	for(jsize i = 0; i < (jsize)entries.size(); i++)
	{
		std::string url_path = to_url_path(entries[i]);
		jstring url_str = env->NewStringUTF(url_path.c_str());
		jobject url_obj = env->NewObject(url_class, url_ctor, url_str);
		env->SetObjectArrayElement(url_array, i, url_obj);
		env->DeleteLocalRef(url_str);
		env->DeleteLocalRef(url_obj);
		if(env->ExceptionCheck())
		{
			std::string error = get_exception_description(env);
			release_env();
			throw std::runtime_error(error.empty() ? "Failed to create URL array" : error);
		}
	}

	jobject loader = env->NewObject(url_class_loader, url_m_classLoaderctor, url_array, parent_loader);
	if(env->ExceptionCheck() || !loader)
	{
		std::string error = get_exception_description(env);
		release_env();
		throw std::runtime_error(error.empty() ? "Failed to create URLClassLoader" : error);
	}

	m_classLoader = env->NewGlobalRef(loader);
	env->DeleteLocalRef(loader);
	env->DeleteLocalRef(url_array);
	env->DeleteLocalRef(parent_loader);
	env->DeleteLocalRef(m_classLoaderclass);
	env->DeleteLocalRef(url_class);
	env->DeleteLocalRef(url_class_loader);
	release_env();
}

jclass Module::load_class(JNIEnv* env, const std::string& class_name)
{
	jclass class_class = env->FindClass("java/lang/Class");
	if(!class_class)
	{
		std::string error = get_exception_description(env);
		throw std::runtime_error(error.empty() ? "Failed to find java/lang/Class" : error);
	}

	jmethodID for_name = env->GetStaticMethodID(class_class, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
	if(!for_name)
	{
		std::string error = get_exception_description(env);
		env->DeleteLocalRef(class_class);
		throw std::runtime_error(error.empty() ? "Failed to get Class.forName" : error);
	}

	std::string normalized = normalize_class_name(class_name);
	std::vector<std::string> candidates;
	candidates.push_back(normalized);

	std::string nested_candidate = normalized;
	for(size_t i = nested_candidate.find_last_of('.'); i != std::string::npos; i = nested_candidate.find_last_of('.', i - 1))
	{
		std::string temp = nested_candidate;
		temp[i] = '$';
		candidates.push_back(temp);
		if(i == 0)
		{
			break;
		}
	}

	std::string last_error;
	for(const auto& candidate : candidates)
	{
		jstring class_name_str = env->NewStringUTF(candidate.c_str());
		jobject cls_obj = env->CallStaticObjectMethod(class_class, for_name, class_name_str, JNI_TRUE, m_classLoader);
		env->DeleteLocalRef(class_name_str);
		if(!env->ExceptionCheck() && cls_obj)
		{
			env->DeleteLocalRef(class_class);
			return (jclass)cls_obj;
		}
		if(env->ExceptionCheck())
		{
			last_error = get_exception_description(env);
		}
		else
		{
			last_error = "Class not found: " + candidate;
		}
	}

	env->DeleteLocalRef(class_class);
	throw std::runtime_error(last_error.empty() ? "Failed to load Java class" : last_error);
}

std::vector<std::string> Module::split_classpath(const std::string& classpath)
{
	std::vector<std::string> result;
	std::stringstream ss(classpath);
	std::string item;
	while(std::getline(ss, item, classpath_separator))
	{
		if(!item.empty())
		{
			result.push_back(item);
		}
	}
	if(result.empty())
	{
		result.push_back(classpath);
	}
	return result;
}

std::string Module::to_url_path(const std::string& path)
{
	std::filesystem::path p(path);
	std::string url_path = std::filesystem::absolute(p).generic_string();
	if(url_path.find(".class") != std::string::npos)
	{
		url_path = std::filesystem::path(url_path).parent_path().generic_string();
	}
	return file_protocol + url_path;
}

std::string Module::normalize_class_name(const std::string& class_name)
{
	std::string normalized = class_name;
	std::replace(normalized.begin(), normalized.end(), '/', '.');
	return normalized;
}
