#ifdef _MSC_VER
#include <corecrt.h>
#endif
#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include "argument_definition.h"
#include "jni_class.h"

// NOTICE: although it is "extern C" the function does throw an exception!
typedef jclass (*load_class_t)(JNIEnv* env, const char* path, const char* class_name);
extern "C" jclass load_class(JNIEnv* env, const char* class_path, const char* class_name);


class jni_class_loader
{
private:
	JNIEnv* env;
	std::string class_path;
	std::shared_ptr<jvm> pjvm;
	
	static jclass class_loader_class;
	static jmethodID get_system_class_loader_method;
	static jclass url_class_loader;
	static jmethodID url_class_loader_constructor;
	static jobject classLoaderInstance;
	static jclass url_class;
	static jmethodID url_class_constructor;
	static jclass class_class;
	static jmethodID for_name_method;
	static jmethodID add_url;
	static jobject childURLClassLoader;
	static bool is_bridge_added;
	static std::set<std::string> loaded_paths;
	static std::unordered_map<std::string,jclass> loaded_classes;
	static bool is_metaffi_handle_loaded;


public:
	jni_class_loader(JNIEnv* env, std::string class_path);
	~jni_class_loader() = default;
	
	jni_class load_class(const std::string& class_name);
};
