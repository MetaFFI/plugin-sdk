#include "jni_metaffi_handle.h"

#include <utility>
#include "jni_class.h"
#include "class_loader.h"
#include "exception_macro.h"
#include "runtime_id.h"
#include "utils/env_utils.h"
#include <filesystem>

namespace
{
	std::string resolve_metaffi_api_jar_path()
	{
		std::string metaffi_home = get_env_var("METAFFI_HOME");
		if(metaffi_home.empty())
		{
			return "";
		}

		std::filesystem::path api_jar = std::filesystem::path(metaffi_home) / "jvm" / "metaffi.api.jar";
		if(!std::filesystem::exists(api_jar))
		{
			api_jar = std::filesystem::path(metaffi_home) / "sdk" / "api" / "jvm" / "metaffi.api.jar";
		}

		if(!std::filesystem::exists(api_jar))
		{
			return "";
		}

		return api_jar.generic_string();
	}
}


jclass jni_metaffi_handle::metaffi_handle_class = nullptr;
jmethodID jni_metaffi_handle::get_handle_id = nullptr;
jmethodID jni_metaffi_handle::get_runtime_id_id = nullptr;
jmethodID jni_metaffi_handle::get_releaser_id = nullptr;
jmethodID jni_metaffi_handle::metaffi_handle_constructor = nullptr;



//--------------------------------------------------------------------
jni_metaffi_handle::jni_metaffi_handle(JNIEnv* env)
{
	if(!metaffi_handle_class)
	{
		std::string api_jar = resolve_metaffi_api_jar_path();
		if(api_jar.empty())
		{
			throw std::runtime_error("Failed to locate metaffi.api.jar");
		}
		std::string jvm_bridge_url = std::string("file://") + api_jar;
		jni_class_loader clsloader(env, jvm_bridge_url);
		auto tmp = (jclass)clsloader.load_class("metaffi/api/accessor/MetaFFIHandle");
		metaffi_handle_class = (jclass)env->NewGlobalRef(tmp); // make global so GC doesn't delete
		env->DeleteLocalRef(tmp);
	}
	
	if(!get_handle_id)
	{
		get_handle_id = env->GetMethodID(metaffi_handle_class, "Handle", "()J");
		check_and_throw_jvm_exception(env, true);
	}
	
	if(!get_runtime_id_id)
	{
		get_runtime_id_id = env->GetMethodID(metaffi_handle_class, "RuntimeID", "()J");
		check_and_throw_jvm_exception(env, true);
	}
	
	if(!get_releaser_id)
	{
		get_releaser_id = env->GetMethodID(metaffi_handle_class, "Releaser", "()J");
		check_and_throw_jvm_exception(env, true);
	}
	
	if(!metaffi_handle_constructor)
	{
		metaffi_handle_constructor = env->GetMethodID(metaffi_handle_class, "<init>", "(JJJ)V");
		check_and_throw_jvm_exception(env, true);
	}
}
//--------------------------------------------------------------------
jni_metaffi_handle::jni_metaffi_handle(JNIEnv* env, metaffi_handle v, uint64_t runtime_id, releaser_fptr_t releaser):jni_metaffi_handle(env)
{
	this->value.handle = v;
	this->value.runtime_id = runtime_id;
	this->value.release = releaser;
}
//--------------------------------------------------------------------
jni_metaffi_handle::jni_metaffi_handle(JNIEnv* env, jobject obj):jni_metaffi_handle(env)
{
	this->value.handle = (void*)env->CallLongMethod(obj, get_handle_id);
	check_and_throw_jvm_exception(env, true);
	
	this->value.runtime_id = (uint64_t)env->CallLongMethod(obj, get_runtime_id_id);
	check_and_throw_jvm_exception(env, true);
	
	this->value.release = (releaser_fptr_t)env->CallLongMethod(obj, get_releaser_id);
	check_and_throw_jvm_exception(env, true);
}
//--------------------------------------------------------------------
jobject jni_metaffi_handle::new_jvm_object(JNIEnv* env) const
{
	jobject res = env->NewObject(metaffi_handle_class, metaffi_handle_constructor, (jlong)this->value.handle, (jlong)this->value.runtime_id, (jlong)this->value.release);
	check_and_throw_jvm_exception(env, true);
	
	return res;
}
//--------------------------------------------------------------------
bool jni_metaffi_handle::is_metaffi_handle_wrapper_object(JNIEnv* env, jobject o)
{
	if(!metaffi_handle_class)
	{
		std::string api_jar = resolve_metaffi_api_jar_path();
		std::string jvm_bridge_url = api_jar.empty() ? "" : (std::string("file://") + api_jar);
		jni_class_loader clsloader(env, jvm_bridge_url);
		metaffi_handle_class = (jclass)clsloader.load_class("metaffi/api/accessor/MetaFFIHandle");
		metaffi_handle_class = (jclass)env->NewGlobalRef(metaffi_handle_class);
	}

	if(!get_handle_id)
	{
		get_handle_id = env->GetMethodID(metaffi_handle_class, "Handle", "()J");
		check_and_throw_jvm_exception(env, true);
	}

	if(!get_runtime_id_id)
	{
		get_runtime_id_id = env->GetMethodID(metaffi_handle_class, "RuntimeID", "()J");
		check_and_throw_jvm_exception(env, true);
	}

	if(!metaffi_handle_constructor)
	{
		metaffi_handle_constructor = env->GetMethodID(metaffi_handle_class, "<init>", "(JJJ)V");
		check_and_throw_jvm_exception(env, true);
	};
	
	return env->IsInstanceOf(o, metaffi_handle_class) != JNI_FALSE;
}
//--------------------------------------------------------------------
metaffi_handle jni_metaffi_handle::get_handle() const
{
	return this->value.handle;
}
//--------------------------------------------------------------------
uint64_t jni_metaffi_handle::get_runtime_id() const
{
	return this->value.runtime_id;
}
//--------------------------------------------------------------------
void* jni_metaffi_handle::get_releaser() const
{
	return (void*)this->value.release;
}
//--------------------------------------------------------------------
jni_metaffi_handle::operator cdt_metaffi_handle*() const
{
	cdt_metaffi_handle* res = new cdt_metaffi_handle();
	res->handle = this->value.handle;
	res->runtime_id = this->value.runtime_id;
	res->release = this->value.release;
	
	return res;
}
//--------------------------------------------------------------------
cdt_metaffi_handle* jni_metaffi_handle::wrap_in_metaffi_handle(JNIEnv* env, jobject jobj, void* releaser)
{
	jobj = env->NewGlobalRef(jobj);
	return new cdt_metaffi_handle{(void*)jobj, JVM_RUNTIME_ID, (releaser_fptr_t)releaser};
}
//--------------------------------------------------------------------


