#ifdef _MSC_VER
#include <corecrt.h> // https://www.reddit.com/r/cpp_questions/comments/qpo93t/error_c2039_invalid_parameter_is_not_a_member_of/
#endif
#include "metaffi_api_accessor.h"
#include <utils/foreign_function.h>
#include <utils/xllr_api_wrapper.h>
#include <runtime/metaffi_primitives.h>
#include <utils/scope_guard.hpp>
#include <runtime/xcall.h>
#include <runtime_manager/jvm/objects_table.h>
#include <runtime_manager/jvm/cdts_java_wrapper.h>
#include <runtime_manager/jvm/exception_macro.h>
#include <runtime_manager/jvm/jni_size_utils.h>
#include <runtime_manager/jvm/contexts.h>
#include <runtime_manager/jvm/runtime_id.h>
#include <mutex>

// JNI to call XLLR from java

using namespace metaffi::utils;

// Recursively walk a CDT tree and null out foreign-runtime handle release
// pointers.  This prevents cdt::free() from invoking foreign-runtime release
// callbacks.  JVM handles (runtime_id == JVM_RUNTIME_ID) are left intact
// because their jni_releaser is needed to clean up JNI global references.
static void null_foreign_handle_releasers(cdt* arr, metaffi_size length)
{
	if(!arr) return;
	for(metaffi_size i = 0; i < length; i++)
	{
		if(arr[i].type == metaffi_handle_type)
		{
			if(arr[i].cdt_val.handle_val &&
			   arr[i].cdt_val.handle_val->release &&
			   arr[i].cdt_val.handle_val->runtime_id != JVM_RUNTIME_ID)
			{
				arr[i].cdt_val.handle_val->release = nullptr;
			}
		}
		else if(arr[i].type & metaffi_array_type)
		{
			if(arr[i].cdt_val.array_val)
			{
				null_foreign_handle_releasers(arr[i].cdt_val.array_val->arr, arr[i].cdt_val.array_val->length);
			}
		}
	}
}

// Walk a cdts[N] buffer (params + returns) and null foreign handle releasers.
static void null_foreign_handle_releasers_buffer(cdts* buffer, int count)
{
	if(!buffer) return;
	for(int b = 0; b < count; b++)
	{
		if(buffer[b].arr && buffer[b].length > 0)
		{
			null_foreign_handle_releasers(buffer[b].arr, buffer[b].length);
		}
	}
}


//--------------------------------------------------------------------

std::unique_ptr<xllr_api_wrapper> xllr = std::make_unique<xllr_api_wrapper>();

//--------------------------------------------------------------------
// https://stackoverflow.com/questions/230689/best-way-to-throw-exceptions-in-jni-code
bool throwMetaFFIException( JNIEnv* env,  const char* message )
{
	jclass metaffiException = env->FindClass( "metaffi/api/accessor/MetaFFIException" );
	if(!metaffiException)
	{
		jclass noSuchClass = env->FindClass( "java/lang/NoClassDefFoundError" );
		return env->ThrowNew( noSuchClass, "Cannot find metaffi/api/accessor/MetaFFIException Class" ) == 0;
	}
	
	jint res = env->ThrowNew( metaffiException, message );
	
	
	return res == 0;
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_load_1runtime_1plugin(JNIEnv* env, jclass , jstring runtime_plugin)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);

		char* out_err_buf = nullptr;
		xllr->load_runtime_plugin(str_runtime_plugin, &out_err_buf);

		// release runtime_plugin
		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);

		// set to out_err
		if(out_err_buf)
		{
			throwMetaFFIException(env, out_err_buf);
			return;
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_free_1runtime_1plugin(JNIEnv* env, jclass , jstring runtime_plugin)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);

		char* out_err_buf = nullptr;
		xllr->free_runtime_plugin(str_runtime_plugin, &out_err_buf);

		// release runtime_plugin
		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);

		// set to out_err
		if(out_err_buf)
		{
			throwMetaFFIException(env, out_err_buf);
			free(out_err_buf);
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
jclass MetaFFITypeInfoClass = nullptr;
jfieldID typeFieldID = nullptr;
jfieldID aliasFieldID = nullptr;
jfieldID dimensionsFieldID = nullptr;
metaffi_type_info* convert_MetaFFITypeInfo_array_to_metaffi_type_with_info(JNIEnv* env, jobjectArray inputArray)
{
	// Get the length of the input array
	jsize length = env->GetArrayLength(inputArray);
	check_and_throw_jvm_exception(env, true);
	// Allocate the output array
	metaffi_type_info* outputArray = new metaffi_type_info[length];

	// Get the MetaFFITypeInfo class
	if(!MetaFFITypeInfoClass)
	{
		MetaFFITypeInfoClass = env->FindClass("metaffi/api/accessor/MetaFFITypeInfo");
		check_and_throw_jvm_exception(env, true);
	}

	// Get the field IDs
	if(!typeFieldID)
	{
		typeFieldID = env->GetFieldID(MetaFFITypeInfoClass, "value", "J"); // long
		check_and_throw_jvm_exception(env, true);
	}

	if(!aliasFieldID)
	{
		aliasFieldID = env->GetFieldID(MetaFFITypeInfoClass, "alias", "Ljava/lang/String;"); // String
		check_and_throw_jvm_exception(env, true);
	}

	if(!dimensionsFieldID)
	{
		dimensionsFieldID = env->GetFieldID(MetaFFITypeInfoClass, "dimensions", "I"); // int
		check_and_throw_jvm_exception(env, true);
	}

	// Iterate over the input array
	for (jsize i = 0; i < length; i++)
	{
		// Get the current MetaFFITypeInfo object
		jobject MetaFFITypeInfoObject = env->GetObjectArrayElement(inputArray, i);
		check_and_throw_jvm_exception(env, true);

		// Get the type and alias
		jlong type = env->GetLongField(MetaFFITypeInfoObject, typeFieldID);
		check_and_throw_jvm_exception(env, true);
		jstring aliasJString = (jstring)env->GetObjectField(MetaFFITypeInfoObject, aliasFieldID);
		check_and_throw_jvm_exception(env, true);
		jint dimensions = env->GetIntField(MetaFFITypeInfoObject, dimensionsFieldID);
		check_and_throw_jvm_exception(env, true);

		if(aliasJString)
		{
			const char* aliasCString = env->GetStringUTFChars(aliasJString, nullptr);
			check_and_throw_jvm_exception(env, true);
			jsize len = env->GetStringUTFLength(aliasJString);
			check_and_throw_jvm_exception(env, true);
			outputArray[i].alias = new char[len + 1];
			std::copy_n(aliasCString, len, outputArray[i].alias);
			outputArray[i].alias[len] = u8'\0';
			outputArray[i].is_free_alias = true;
			
			env->ReleaseStringUTFChars(aliasJString, aliasCString);
			check_and_throw_jvm_exception(env, true);
		}
		else
		{
			outputArray[i].alias = nullptr;
			outputArray[i].is_free_alias = false;
		}

		// Set the output array values
		outputArray[i].type = type;
		outputArray[i].fixed_dimensions = dimensions > 0 ? dimensions : MIXED_OR_UNKNOWN_DIMENSIONS;
	}

	// Return the output array as a jlong (this is actually a pointer)
	return reinterpret_cast<metaffi_type_info*>(outputArray);
}
//--------------------------------------------------------------------
jmethodID get_method_id_from_Method(JNIEnv* env, jobject methodObject, jstring jniSignature, jboolean& isStatic, jclass& outDeclaringClass)
{
	// Get Method and Modifier classes and method IDs we're going to need
	jclass classMethod = env->FindClass("java/lang/reflect/Method");
	jclass classModifier = env->FindClass("java/lang/reflect/Modifier");
	jmethodID midGetDeclaringClass = env->GetMethodID(classMethod, "getDeclaringClass", "()Ljava/lang/Class;");
	jmethodID midGetName = env->GetMethodID(classMethod, "getName", "()Ljava/lang/String;");
	jmethodID midGetModifiers = env->GetMethodID(classMethod, "getModifiers", "()I");
	jmethodID midIsStatic = env->GetStaticMethodID(classModifier, "isStatic", "(I)Z");

	// Call getDeclaringClass, getName, getModifiers methods
	jobject declaringClassObject = env->CallObjectMethod(methodObject, midGetDeclaringClass);
	jstring nameObject = (jstring)env->CallObjectMethod(methodObject, midGetName);
	jint modifiers = env->CallIntMethod(methodObject, midGetModifiers);

	// Call Modifier.isStatic method
	isStatic = env->CallStaticBooleanMethod(classModifier, midIsStatic, modifiers);

	// Get the declaring class
	outDeclaringClass = (jclass)env->NewLocalRef(declaringClassObject);

	// Get the method name
	const char* name = env->GetStringUTFChars(nameObject, 0);

	// Get the JNI signature
	const char* signature = env->GetStringUTFChars(jniSignature, 0);

	// Get the method ID
	jmethodID mid;
	if (isStatic == JNI_TRUE) {
		mid = env->GetStaticMethodID(outDeclaringClass, name, signature);
	} else {
		mid = env->GetMethodID(outDeclaringClass, name, signature);
	}

	// Clean up local references and release the strings
	env->ReleaseStringUTFChars(nameObject, name);
	env->ReleaseStringUTFChars(jniSignature, signature);

	return mid;
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_load_1callable(JNIEnv* env, jclass , jstring runtime_plugin, jobject method, jstring method_jni_signature, jobjectArray parameters_types, jobjectArray retval_types)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
		check_and_throw_jvm_exception(env, str_runtime_plugin);

		jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
		check_and_throw_jvm_exception(env, str_runtime_plugin_len);

		jboolean out_is_static = JNI_FALSE;
		jclass declaring_class = nullptr;
		jmethodID method_id = get_method_id_from_Method(env, method, method_jni_signature, out_is_static, declaring_class);


		jsize params_count = parameters_types == nullptr ? 0 : env->GetArrayLength(parameters_types);
		check_and_throw_jvm_exception(env, true);

		jsize retval_count = retval_types == nullptr ? 0 : env->GetArrayLength(retval_types);
		check_and_throw_jvm_exception(env, true);

		metaffi_type_info* pparams_types = parameters_types == nullptr ?
													nullptr :
													convert_MetaFFITypeInfo_array_to_metaffi_type_with_info(env, parameters_types);

		metaffi_type_info* pretval_types = retval_types == nullptr ?
		                                             nullptr :
		                                             convert_MetaFFITypeInfo_array_to_metaffi_type_with_info(env, retval_types);

		char* out_err_buf = nullptr;

		jvm_context* pctxt = new jvm_context();
		pctxt->cls = declaring_class;
		pctxt->method = method_id;
		pctxt->instance_required = out_is_static == JNI_FALSE;
		pctxt->constructor = false;

		void* xcall_and_context = xllr->make_callable(str_runtime_plugin, (void*)pctxt,
		                                              pparams_types, (int8_t)params_count,
		                                              pretval_types, (int8_t)retval_count,
		                                              &out_err_buf);

		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
		if(declaring_class)
		{
			env->DeleteLocalRef(declaring_class);
		}

		if(params_count > 0)
		{
			delete[] pparams_types;
		}

		if(retval_count > 0)
		{
			delete[] pretval_types;
		}

		// set to out_err
		if(!xcall_and_context)
		{
			throwMetaFFIException(env, out_err_buf);
			return -1;
		}

		return (jlong)xcall_and_context;
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_load_1function(JNIEnv* env, jclass , jstring runtime_plugin, jstring module_path, jstring entity_path, jobjectArray parameters_types, jobjectArray retval_types)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
		check_and_throw_jvm_exception(env, str_runtime_plugin);

		jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
		check_and_throw_jvm_exception(env, str_runtime_plugin_len);

		const char* str_module_path = env->GetStringUTFChars(module_path, nullptr);
		check_and_throw_jvm_exception(env, str_module_path);

		// Empty module path is valid for runtimes like xllr.test.
		env->GetStringLength(module_path);
		check_and_throw_jvm_exception(env, true);

		const char* str_entity_path = env->GetStringUTFChars(entity_path, nullptr);
		check_and_throw_jvm_exception(env, str_entity_path);

		jsize params_count = parameters_types == nullptr ? 0 : env->GetArrayLength(parameters_types);
		check_and_throw_jvm_exception(env, true);

		jsize retval_count = retval_types == nullptr ? 0 : env->GetArrayLength(retval_types);
		check_and_throw_jvm_exception(env, true);

		metaffi_type_info* pparams_types = parameters_types == nullptr ?
													nullptr :
													convert_MetaFFITypeInfo_array_to_metaffi_type_with_info(env, parameters_types);

		metaffi_type_info* pretval_types = retval_types == nullptr ?
		                                             nullptr :
		                                             convert_MetaFFITypeInfo_array_to_metaffi_type_with_info(env, retval_types);

		char* out_err_buf = nullptr;
		uint32_t out_err_len = 0;

		xcall* xcall_and_context = xllr->load_entity(str_runtime_plugin,
													str_module_path, str_entity_path,
			                                        pparams_types, (int8_t)params_count,
		                                            pretval_types, (int8_t)retval_count,
		                                            &out_err_buf);

		// release runtime_plugin
		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
		env->ReleaseStringUTFChars(module_path, str_module_path);
		env->ReleaseStringUTFChars(entity_path, str_entity_path);

		if(params_count > 0)
		{
			delete[] pparams_types;
		}

		if(retval_count > 0)
		{
			delete[] pretval_types;
		}

		// set to out_err
		if(!xcall_and_context)
		{
			throwMetaFFIException(env, out_err_buf);
			return -1;
		}

		return (jlong)xcall_and_context;
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_free_1function(JNIEnv* env, jclass , jstring runtime_plugin, jlong function_id)
{

}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_xcall_1params_1ret(JNIEnv* env, jclass , jlong vpxcall, jlong xcall_params)
{
	try
	{
		char* out_err_buf = nullptr;

		if(!vpxcall)
		{
			throwMetaFFIException(env, "internal error. xcall is null");
			return;
		}

		xcall* pxcall = (xcall*)vpxcall;

		(*pxcall)((cdts*) xcall_params, &out_err_buf);

		if (out_err_buf) // throw an exception in the JVM
		{
			throwMetaFFIException(env, out_err_buf);
			free(out_err_buf);
			return;
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}

}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_xcall_1no_1params_1ret(JNIEnv* env, jclass , jlong vpxcall, jlong xcall_params)
{
	char* out_err_buf = nullptr;

	if(!vpxcall)
	{
		throwMetaFFIException(env, "internal error. pointer to xcall is null");
		return;
	}

	xcall* pxcall = (xcall*)vpxcall;
	(*pxcall)((cdts*)xcall_params, &out_err_buf);

	if(out_err_buf) // throw an exception in the JVM
	{
		throwMetaFFIException(env, out_err_buf);
		free(out_err_buf);
		return;
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_xcall_1params_1no_1ret(JNIEnv* env, jclass , jlong vpxcall, jlong xcall_params)
{
	try
	{
		char* out_err_buf = nullptr;

		if(!vpxcall)
		{
			throwMetaFFIException(env, "internal error. pointer to xcall is null");
			return;
		}

		xcall* pxcall = (xcall*)vpxcall;
		(*pxcall)((cdts*) xcall_params, &out_err_buf);

		if (out_err_buf) // throw an exception in the JVM
		{
			throwMetaFFIException(env, out_err_buf);
			free(out_err_buf);
			return;
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_xcall_1no_1params_1no_1ret(JNIEnv* env, jclass , jlong vpxcall)
{
	try
	{
		char* out_err_buf = nullptr;

		if(!vpxcall)
		{
			throwMetaFFIException(env, "internal error. pointer to xcall is null");
			return;
		}

		xcall* pxcall = (xcall*)vpxcall;
		(*pxcall)(&out_err_buf);

		if (out_err_buf) // throw an exception in the JVM
		{
			throwMetaFFIException(env, out_err_buf);

			free(out_err_buf);

			return;
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_alloc_1cdts(JNIEnv* env, jclass, jbyte params_count, jbyte retval_count)
{
	try
	{
		return reinterpret_cast<jlong>(xllr_alloc_cdts_buffer(params_count, retval_count));
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_free_1cdts(JNIEnv* env, jclass, jlong pcdts)
{
	try
	{
		if(pcdts != 0)
		{
			cdts* buf = reinterpret_cast<cdts*>(pcdts);
			// Safety net: null foreign-runtime handle releasers before freeing
			// the buffer.  The per-element nulling in cdts_to_java should have
			// already done this, but any missed path (e.g. error returns,
			// any_echo edge-cases) is caught here.
			null_foreign_handle_releasers_buffer(buf, 2);
			xllr_free_cdts_buffer(buf);
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_get_1pcdt(JNIEnv* env, jclass, jlong pcdts, jbyte index)
{
	try
	{
		return reinterpret_cast<jlong>(&((cdts*)(pcdts))[index]);
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_set_1cdt_1int64(JNIEnv* env, jclass, jlong pcdt, jint index, jlong value)
{
	try
	{
		if(pcdt == 0)
		{
			throwMetaFFIException(env, "internal error. pointer to pcdt is null");
			return;
		}

		cdts* p = reinterpret_cast<cdts*>(pcdt);
		if(index < 0 || static_cast<metaffi_size>(index) >= p->length)
		{
			throwMetaFFIException(env, "internal error. cdt index out of bounds");
			return;
		}

		cdt& slot = p->arr[index];
		slot.free();
		slot.type = metaffi_int64_type;
		slot.cdt_val.int64_val = static_cast<metaffi_int64>(value);
		slot.free_required = false;
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT jobject JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_get_1object(JNIEnv* env, jclass, jlong phandle)
{
	if(!jvm_objects_table::instance().contains((jobject)phandle)){
		throwMetaFFIException(env, "Object is not found in Objects Table");
	}
	
	return (jobject)phandle;
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_remove_1object (JNIEnv* env, jclass, jlong phandle)
{
	jvm_objects_table::instance().remove(env, (jobject)phandle);
}
//--------------------------------------------------------------------
namespace
{
	struct array_dims_cache
	{
		JavaVM* vm = nullptr;
		jclass arr_bool = nullptr;
		jclass arr_byte = nullptr;
		jclass arr_short = nullptr;
		jclass arr_int = nullptr;
		jclass arr_long = nullptr;
		jclass arr_float = nullptr;
		jclass arr_double = nullptr;
		jclass cls_object = nullptr;
		jclass cls_class = nullptr;
		jmethodID mid_get_class = nullptr;
		jmethodID mid_get_name = nullptr;
	};

	array_dims_cache& get_array_dims_cache(JNIEnv* env)
	{
		static array_dims_cache cache;
		static std::mutex cache_mutex;

		std::lock_guard<std::mutex> lock(cache_mutex);
		JavaVM* current_vm = nullptr;
		env->GetJavaVM(&current_vm);
		check_and_throw_jvm_exception(env, current_vm);

		if(cache.vm == current_vm && cache.mid_get_class && cache.mid_get_name)
		{
			return cache;
		}

		cache = {};
		cache.vm = current_vm;

		auto make_global = [&](const char* class_name) -> jclass
		{
			jclass local = env->FindClass(class_name);
			check_and_throw_jvm_exception(env, local);
			jclass global = (jclass)env->NewGlobalRef(local);
			env->DeleteLocalRef(local);
			check_and_throw_jvm_exception(env, global);
			return global;
		};

		cache.arr_bool = make_global("[Z");
		cache.arr_byte = make_global("[B");
		cache.arr_short = make_global("[S");
		cache.arr_int = make_global("[I");
		cache.arr_long = make_global("[J");
		cache.arr_float = make_global("[F");
		cache.arr_double = make_global("[D");
		cache.cls_object = make_global("java/lang/Object");
		cache.cls_class = make_global("java/lang/Class");

		cache.mid_get_class = env->GetMethodID(cache.cls_object, "getClass", "()Ljava/lang/Class;");
		check_and_throw_jvm_exception(env, cache.mid_get_class);
		cache.mid_get_name = env->GetMethodID(cache.cls_class, "getName", "()Ljava/lang/String;");
		check_and_throw_jvm_exception(env, cache.mid_get_name);

		return cache;
	}
}

int get_array_dimensions(JNIEnv *env, jobjectArray arr)
{
	if(!arr)
	{
		return 0;
	}

	auto& cache = get_array_dims_cache(env);

	// Hot-path: all primitive JVM arrays are always 1D at the root.
	if(env->IsInstanceOf(arr, cache.arr_bool) ||
	   env->IsInstanceOf(arr, cache.arr_byte) ||
	   env->IsInstanceOf(arr, cache.arr_short) ||
	   env->IsInstanceOf(arr, cache.arr_int) ||
	   env->IsInstanceOf(arr, cache.arr_long) ||
	   env->IsInstanceOf(arr, cache.arr_float) ||
	   env->IsInstanceOf(arr, cache.arr_double))
	{
		return 1;
	}

	jobject clsObj = env->CallObjectMethod(arr, cache.mid_get_class);
	check_and_throw_jvm_exception(env, clsObj);
	jstring name = (jstring)env->CallObjectMethod(clsObj, cache.mid_get_name);
	check_and_throw_jvm_exception(env, name);
	const char* nameStr = env->GetStringUTFChars(name, nullptr);
	check_and_throw_jvm_exception(env, nameStr);
	
	int dimensions = 0;
	for (const char* c = nameStr; *c != '\0'; c++)
	{
		if (*c == '[') {
			dimensions++;
		}
	}
	
	env->ReleaseStringUTFChars(name, nameStr);
	env->DeleteLocalRef(name);
	env->DeleteLocalRef(clsObj);
	
	return dimensions;
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_java_1to_1cdts(JNIEnv* env, jclass, jlong pcdts, jobjectArray parameters, jlongArray types)
{
	try
	{
		jsize l = env->GetArrayLength(parameters);
		cdts_java_wrapper wrapper((cdts*)pcdts);

		jlong* types_elements = env->GetLongArrayElements(types, nullptr);
		check_and_throw_jvm_exception(env, types_elements);
		
		metaffi::utils::scope_guard sg([&](){ env->ReleaseLongArrayElements(types, types_elements, 0); });
		
		for(jsize i=0 ; i < l ; i++)
		{
			jvalue cur_object;
			cur_object.l = env->GetObjectArrayElement(parameters, i);
			check_and_throw_jvm_exception(env, true);
			metaffi_type_info type_to_expect = (types_elements[i] & metaffi_array_type) == 0 ?
												(types_elements[i] == metaffi_callable_type ? metaffi_type_info{metaffi_callable_type} : metaffi_type_info{(metaffi_type)types_elements[i]}) :
                                               metaffi_type_info{(uint64_t)types_elements[i], nullptr, false, get_array_dimensions(env, (jobjectArray)cur_object.l)};
			wrapper.from_jvalue(env, cur_object, 'L', type_to_expect, i);
			wrapper.switch_to_primitive(env, i, types_elements[i]);
		}
		
		// Null foreign-runtime handle releasers on input CDTs.
		// Java retains ownership of all handles it passes as arguments;
		// the CDT destructor must not release them.
		cdts* input_buf = (cdts*)wrapper;
		if(input_buf && input_buf->arr && input_buf->length > 0)
		{
			null_foreign_handle_releasers(input_buf->arr, input_buf->length);
		}
		
		return (jlong)input_buf;
	}
	catch(std::exception& exp)
	{
		throwMetaFFIException(env, exp.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT jobjectArray JNICALL Java_metaffi_api_accessor_MetaFFIAccessor_cdts_1to_1java(JNIEnv* env, jclass, jlong pcdts, jlong length)
{
	try
	{
		cdts_java_wrapper wrapper((cdts*)pcdts);
		jsize jlength = to_jsize(length);
		jobjectArray arr = env->NewObjectArray(jlength, env->FindClass("Ljava/lang/Object;"), nullptr);
		for (jsize i = 0; i < jlength; i++)
		{
			wrapper.switch_to_object(env, i);
			jvalue j = wrapper.to_jvalue(env, i);
			env->SetObjectArrayElement(arr, i, j.l);
			check_and_throw_jvm_exception(env, true);

			if(env->GetObjectRefType(j.l) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(j.l); // delete the global reference
			}

			// After extracting values into Java objects, null foreign handle
			// release pointers in this CDT element (including nested arrays)
			// so that freeing the CDT buffer will not release the underlying
			// objects from the foreign runtime's table.
			cdt& c = wrapper[i];
			null_foreign_handle_releasers(&c, 1);
		}

		return arr;
	}
	catch(std::exception& exp)
	{
		throwMetaFFIException(env, exp.what());
		return nullptr;
	}
}
//--------------------------------------------------------------------
