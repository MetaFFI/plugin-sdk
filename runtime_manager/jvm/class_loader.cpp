#include "class_loader.h"

#include <filesystem>
#include <set>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <utility>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "utils/env_utils.h"

jclass jni_class_loader::class_loader_class = nullptr;
jmethodID jni_class_loader::get_system_class_loader_method = nullptr;
jclass jni_class_loader::url_class_loader = nullptr;
jmethodID jni_class_loader::url_class_loader_constructor = nullptr;
jobject jni_class_loader::classLoaderInstance = nullptr;
jclass jni_class_loader::url_class = nullptr;
jmethodID jni_class_loader::url_class_constructor = nullptr;
jclass jni_class_loader::class_class = nullptr;
jmethodID jni_class_loader::for_name_method = nullptr;
jmethodID jni_class_loader::add_url = nullptr;
jobject jni_class_loader::childURLClassLoader = nullptr;
bool jni_class_loader::is_bridge_added = false;
bool jni_class_loader::is_metaffi_handle_loaded = false;
std::set<std::string> jni_class_loader::loaded_paths;
std::unordered_map<std::string,jclass> jni_class_loader::loaded_classes;

#ifdef _WIN32
std::string file_protocol("file:///");
constexpr char classpath_separator = ';';
#else
std::string file_protocol("file://");
constexpr char classpath_separator = ':';
#endif


#define check_and_throw_jvm_exception(env, var, before_throw_code) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = get_exception_description(env, env->ExceptionOccurred());\
env->ExceptionClear();\
before_throw_code \
throw std::runtime_error(err_msg);\
}\
else if(!var)\
{\
before_throw_code; \
throw std::runtime_error("Failed to get " #var);\
}

#define if_exception_throw_jvm_exception(env, before_throw_code) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = get_exception_description(env, env->ExceptionOccurred());\
env->ExceptionClear();\
before_throw_code; \
throw std::runtime_error(err_msg);\
}


std::string get_exception_description(JNIEnv* penv, jthrowable throwable)
{
	jclass throwable_class = penv->FindClass("java/lang/Throwable");
	if(!throwable_class)
	{
		penv->ExceptionDescribe();
		throw std::runtime_error("failed to FindClass java/lang/Throwable");
	}
	
	jclass StringWriter_class = penv->FindClass("java/io/StringWriter");
	if(!StringWriter_class)
	{
		penv->ExceptionDescribe();
		throw std::runtime_error("failed to FindClass java/io/StringWriter");
	}
	
	jclass PrintWriter_class = penv->FindClass("java/io/PrintWriter");
	if(!PrintWriter_class)
	{
		penv->ExceptionDescribe();
		throw std::runtime_error("failed to FindClass java/io/PrintWriter");
	}
	
	jmethodID throwable_printStackTrace = penv->GetMethodID(throwable_class,"printStackTrace","(Ljava/io/PrintWriter;)V");
	if(!throwable_printStackTrace)
	{
		penv->ExceptionDescribe();
		throw std::runtime_error("failed to GetMethodID throwable_printStackTrace");
	}
	
	jmethodID StringWriter_Constructor = penv->GetMethodID(StringWriter_class,"<init>","()V");
	if(!StringWriter_Constructor)
	{
		penv->ExceptionDescribe();
		throw std::runtime_error("failed to GetMethodID StringWriter_Constructor");
	}
	
	jmethodID PrintWriter_Constructor = penv->GetMethodID(PrintWriter_class,"<init>","(Ljava/io/Writer;)V");
	if(!PrintWriter_Constructor)
	{
		penv->ExceptionDescribe();
		throw std::runtime_error("failed to GetMethodID PrintWriter_Constructor");
	}
	
	jmethodID StringWriter_toString = penv->GetMethodID(StringWriter_class,"toString","()Ljava/lang/String;");
	if(!StringWriter_toString)
	{
		penv->ExceptionDescribe();
		throw std::runtime_error("failed to GetMethodID StringWriter_toString");
	}
	
	// StringWriter sw = new StringWriter();
	jobject sw = penv->NewObject(StringWriter_class, StringWriter_Constructor);
	if(!sw)
	{
		penv->ExceptionDescribe();
		throw std::runtime_error("Failed to create StringWriter object");
	}
	
	// PrintWriter pw = new PrintWriter(sw)
	jobject pw = penv->NewObject(PrintWriter_class, PrintWriter_Constructor, sw);
	if(!pw)
	{
		penv->ExceptionDescribe();
		throw std::runtime_error("Failed to create PrintWriter object");
	}
	
	// throwable.printStackTrace(pw);
	jobject st = penv->CallObjectMethod(throwable, throwable_printStackTrace, pw);
	if(!st)
	{
		penv->ExceptionDescribe();
		penv->DeleteLocalRef(pw);
		throw std::runtime_error("Failed to call printStackTrace");
	}
	
	// sw.toString()
	jobject str = penv->CallObjectMethod(sw, StringWriter_toString);
	if(!str)
	{
		penv->ExceptionDescribe();
		penv->DeleteLocalRef(pw);
		penv->DeleteLocalRef(sw);
		throw std::runtime_error("Failed to call printStackTrace");
	}
	
	std::string res(penv->GetStringUTFChars((jstring)str, nullptr));
	
	penv->DeleteLocalRef(sw);
	penv->DeleteLocalRef(pw);
	penv->DeleteLocalRef(str);
	
	return res;
}
//--------------------------------------------------------------------
jni_class_loader::jni_class_loader(JNIEnv* env, std::string class_path):env(env),class_path(std::move(class_path))
{}
//--------------------------------------------------------------------
jni_class jni_class_loader::load_class(const std::string& class_name)
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4297)
#endif
	// if class already loaded - return jclass
	if(auto it = loaded_classes.find(class_name); it != loaded_classes.end())
	{
		return jni_class(env, it->second);
	}

	auto resolve_metaffi_api_jar = [&]() -> std::string
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
	};
	
	// get class loader
	if(!class_loader_class)
	{
		class_loader_class = env->FindClass("java/lang/ClassLoader");
		check_and_throw_jvm_exception(env, class_loader_class,);
		class_loader_class = (jclass)env->NewGlobalRef(class_loader_class);
	}
	
	if(!get_system_class_loader_method)
	{
		get_system_class_loader_method = env->GetStaticMethodID(class_loader_class, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
		check_and_throw_jvm_exception(env, get_system_class_loader_method,);
	}
	
	if(!url_class_loader)
	{
		url_class_loader = env->FindClass("java/net/URLClassLoader");
		check_and_throw_jvm_exception(env, url_class_loader,);
		url_class_loader = (jclass)env->NewGlobalRef(url_class_loader);
	}
	
	if(!url_class_loader_constructor)
	{
		url_class_loader_constructor = env->GetMethodID(url_class_loader, "<init>", "([Ljava/net/URL;Ljava/lang/ClassLoader;)V");
		check_and_throw_jvm_exception(env, url_class_loader_constructor,);
	}
	
	if(!add_url)
	{
		add_url = env->GetMethodID(url_class_loader, "addURL", "(Ljava/net/URL;)V");
		check_and_throw_jvm_exception(env, add_url,);
	}
	
	if(!classLoaderInstance)
	{
		// classLoaderInstance = ClassLoader.getSystemClassLoader()
		jobject local_loader = env->CallStaticObjectMethod(class_loader_class, get_system_class_loader_method);
		check_and_throw_jvm_exception(env, local_loader,);
		classLoaderInstance = env->NewGlobalRef(local_loader);
		env->DeleteLocalRef(local_loader);
	}
	
	if(!url_class)
	{
		// new URL[]{ urlInstance }
		url_class = env->FindClass("java/net/URL");
		check_and_throw_jvm_exception(env, url_class,);

		url_class = (jclass)env->NewGlobalRef(url_class);
	}
	
	if(!url_class_constructor)
	{
		url_class_constructor = env->GetMethodID(url_class, "<init>", "(Ljava/lang/String;)V");
		check_and_throw_jvm_exception(env, url_class_constructor,);
	}
	
	if(!class_class)
	{
		// Class targetClass = Class.forName(class_name, true, child);
		class_class = env->FindClass("java/lang/Class");
		check_and_throw_jvm_exception(env, class_class,);

		class_class = (jclass)env->NewGlobalRef(class_class);
	}
	
	if(!for_name_method)
	{
		for_name_method = env->GetStaticMethodID(class_class, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
		check_and_throw_jvm_exception(env, for_name_method,);
	}
	
	if(!childURLClassLoader)
	{
		// URLClassLoader childURLClassLoader = new URLClassLoader( jarURLArray, classLoaderInstance ) ;
		
		// initialize with "$METAFFI_HOME/jvm/metaffi.api.jar" (fallback to sdk/api/jvm)
		std::string api_jar_path = resolve_metaffi_api_jar();
		if(api_jar_path.empty())
		{
			throw std::runtime_error("Failed to locate metaffi.api.jar");
		}
		std::string jvm_bridge_url = file_protocol + api_jar_path;
		
		jobjectArray jarURLArray = env->NewObjectArray(1, url_class, nullptr); // URL[]{}
		check_and_throw_jvm_exception(env, jarURLArray,);
		jstring jvm_bridge_url_str = env->NewStringUTF(jvm_bridge_url.c_str());
		check_and_throw_jvm_exception(env, jvm_bridge_url_str, env->DeleteLocalRef(jarURLArray););
		jobject jvm_bridge_url_obj = env->NewObject(url_class, url_class_constructor, jvm_bridge_url_str);
		check_and_throw_jvm_exception(env, jvm_bridge_url_obj, env->DeleteLocalRef(jvm_bridge_url_str); env->DeleteLocalRef(jarURLArray););
		env->SetObjectArrayElement(jarURLArray, 0, jvm_bridge_url_obj);
		check_and_throw_jvm_exception(env, true, env->DeleteLocalRef(jvm_bridge_url_obj); env->DeleteLocalRef(jvm_bridge_url_str); env->DeleteLocalRef(jarURLArray););
		env->DeleteLocalRef(jvm_bridge_url_obj);
		env->DeleteLocalRef(jvm_bridge_url_str);
		
		jobject local_child = env->NewObject(url_class_loader, url_class_loader_constructor, jarURLArray, classLoaderInstance);
		check_and_throw_jvm_exception(env, local_child, env->DeleteLocalRef(jarURLArray););
		childURLClassLoader = env->NewGlobalRef(local_child);
		env->DeleteLocalRef(local_child);
		env->DeleteLocalRef(jarURLArray);
	}
	
	if(!is_bridge_added)
	{
		std::string api_jar_path = resolve_metaffi_api_jar();
		if(api_jar_path.empty())
		{
			throw std::runtime_error("Failed to locate metaffi.api.jar");
		}
		std::string jvm_bridge = file_protocol + api_jar_path;

#ifdef _WIN32
		boost::replace_all(jvm_bridge, "\\", "/");
#endif
		
		jstring jvm_bridge_str = env->NewStringUTF(jvm_bridge.c_str());
		check_and_throw_jvm_exception(env, jvm_bridge_str,);
		jobject urlInstance = env->NewObject(url_class, url_class_constructor, jvm_bridge_str);
		check_and_throw_jvm_exception(env, urlInstance, env->DeleteLocalRef(jvm_bridge_str););
		env->CallVoidMethod(childURLClassLoader, add_url, urlInstance);
		check_and_throw_jvm_exception(env, true, env->DeleteLocalRef(urlInstance); env->DeleteLocalRef(jvm_bridge_str););
		env->DeleteLocalRef(urlInstance);
		env->DeleteLocalRef(jvm_bridge_str);
		
		is_bridge_added = true;
	}
	
	if(!is_metaffi_handle_loaded)
	{
		jobject targetClass = env->FindClass("metaffi/api/accessor/MetaFFIHandle");
		if(env->ExceptionCheck() == JNI_TRUE || !targetClass)
		{
			env->ExceptionClear();
			jstring metaffi_handle_class_name = env->NewStringUTF("metaffi.api.accessor.MetaFFIHandle");
			check_and_throw_jvm_exception(env, metaffi_handle_class_name,);
			targetClass = env->CallStaticObjectMethod(class_class, for_name_method, metaffi_handle_class_name, JNI_TRUE, childURLClassLoader);
			check_and_throw_jvm_exception(env, targetClass, env->DeleteLocalRef(metaffi_handle_class_name););
			env->DeleteLocalRef(metaffi_handle_class_name);
		}
		jclass global_metaffi_handle = (jclass)env->NewGlobalRef(targetClass);
		loaded_classes["metaffi/api/accessor/MetaFFIHandle"] = global_metaffi_handle;
		loaded_classes["metaffi.api.accessor.MetaFFIHandle"] = global_metaffi_handle;
		is_metaffi_handle_loaded = true;

		if(class_name == "metaffi/api/accessor/MetaFFIHandle" || class_name == "metaffi.api.accessor.MetaFFIHandle")
		{
			return {env, (jclass)targetClass};
		}
		env->DeleteLocalRef(targetClass);
	}
	
	std::string tmp;
	std::stringstream ss(class_path);
	std::vector<std::string> classpath_vec;
	while(std::getline(ss, tmp, classpath_separator))
	{
		classpath_vec.push_back(std::filesystem::absolute(tmp).generic_string());
	}
	
	// every URL that is NOT loaded - add URL
	for(const auto & i : classpath_vec)
	{
		std::string url_path = file_protocol+i;
		if(loaded_paths.find(url_path) != loaded_paths.end())
		{
			continue;
		}

#ifdef _WIN32
		boost::replace_all(url_path, "\\", "/");
#endif
		
		if(url_path.find(".class") != std::string::npos)
		{
			url_path = url_path.substr(0, url_path.rfind('/'));
			url_path += '/';
		}
		
		jstring url_path_str = env->NewStringUTF(url_path.c_str());
		check_and_throw_jvm_exception(env, url_path_str,);
		jobject urlInstance = env->NewObject(url_class, url_class_constructor, url_path_str);
		check_and_throw_jvm_exception(env, urlInstance, env->DeleteLocalRef(url_path_str););
		env->CallVoidMethod(childURLClassLoader, add_url, urlInstance);
		check_and_throw_jvm_exception(env, true, env->DeleteLocalRef(urlInstance); env->DeleteLocalRef(url_path_str););
		env->DeleteLocalRef(urlInstance);
		env->DeleteLocalRef(url_path_str);
		
		
		loaded_paths.insert(url_path);
	}
	
	std::string class_name_for_lookup = class_name;
	boost::replace_all(class_name_for_lookup, "/", ".");
	jstring class_name_str = env->NewStringUTF(class_name_for_lookup.c_str());
	check_and_throw_jvm_exception(env, class_name_str,);
	jobject targetClass = env->CallStaticObjectMethod(class_class, for_name_method, class_name_str, JNI_TRUE, childURLClassLoader);
	
	check_and_throw_jvm_exception(env, targetClass, env->DeleteLocalRef(class_name_str););
	env->DeleteLocalRef(class_name_str);
	jclass global_class = (jclass)env->NewGlobalRef(targetClass);
	env->DeleteLocalRef(targetClass);
	loaded_classes[class_name] = global_class;
	loaded_classes[class_name_for_lookup] = global_class;
	
	return {env, global_class};
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

jobject jni_class_loader::get_child_class_loader()
{
	return childURLClassLoader;
}




