#include "cdts_java_wrapper.h"
#include "exception_macro.h"
#include "jarray_wrapper.h"
#include "jboolean_wrapper.h"
#include "jbyte_wrapper.h"
#include "jchar_wrapper.h"
#include "jdouble_wrapper.h"
#include "jfloat_wrapper.h"
#include "jint_wrapper.h"
#include "jlong_wrapper.h"
#include "jni_caller.h"
#include "jni_class.h"
#include "jni_metaffi_handle.h"
#include "jni_size_utils.h"
#include "jobject_wrapper.h"
#include "jshort_wrapper.h"
#include "jstring_wrapper.h"
#include "runtime_id.h"
#include "utils/defines.h"
#include <algorithm>
#include <runtime/cdts_traverse_construct.h>
#include <utility>
#include <utils/defines.h>
#include <utils/logger.hpp>


extern std::shared_ptr<jvm> pjvm;
static auto LOG = metaffi::get_logger("jvm.api");

//--------------------------------------------------------------------

DLL_PRIVATE void on_traverse_float64(const metaffi_size* index, metaffi_size index_size, metaffi_float64 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.d = val;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray)pair->second.l;
		
		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[D")))// if jobject is jdoubleArray
		{
			jdoubleArray array = static_cast<jdoubleArray>(obj);
			env->SetDoubleArrayRegion(array, to_jsize(index[index_size - 1]), 1, &val);
		}

		// if context is a pair of JNIEnv* and jobject, where jobject is jdoubleArray
		// set the float in the array by the given indices
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Double;")) ||
		        env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Object;")))
		{
			jobjectArray array = static_cast<jobjectArray>(obj);
			jclass doubleClass = env->FindClass("java/lang/Double");
			jmethodID constructor = env->GetMethodID(doubleClass, "<init>", "(D)V");
			jobject doubleObject = env->NewObject(doubleClass, constructor, val);
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), doubleObject);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be either jdoubleArray or jobjectArray of Double");
		}
	}
}

DLL_PRIVATE void on_traverse_float32(const metaffi_size* index, metaffi_size index_size, metaffi_float32 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.f = val;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[F")))// if jobject is jfloatArray
		{
			jfloatArray array = static_cast<jfloatArray>(obj);
			env->SetFloatArrayRegion(array, to_jsize(index[index_size - 1]), 1, &val);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Float;")) ||// if jobject is jobjectArray of Float
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jobjectArray array = static_cast<jobjectArray>(obj);
			jclass floatClass = env->FindClass("java/lang/Float");
			jmethodID constructor = env->GetMethodID(floatClass, "<init>", "(F)V");
			jobject floatObject = env->NewObject(floatClass, constructor, val);
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), floatObject);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be either jfloatArray or jobjectArray of Float or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_int8(const metaffi_size* index, metaffi_size index_size, metaffi_int8 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.b = val;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[B")))// if jobject is jbyteArray
		{
			jbyteArray array = static_cast<jbyteArray>(obj);
			env->SetByteArrayRegion(array, to_jsize(index[index_size - 1]), 1, &val);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Byte;")) ||// if jobject is jobjectArray of Byte
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jobjectArray array = static_cast<jobjectArray>(obj);
			jclass byteClass = env->FindClass("java/lang/Byte");
			jmethodID constructor = env->GetMethodID(byteClass, "<init>", "(B)V");
			jobject byteObject = env->NewObject(byteClass, constructor, val);
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), byteObject);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be either jbyteArray or jobjectArray of Byte or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_uint8(const metaffi_size* index, metaffi_size index_size, metaffi_uint8 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.b = (jbyte) val;// Cast to signed byte
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[B")))// if jobject is jbyteArray
		{
			jbyteArray array = static_cast<jbyteArray>(obj);
			jbyte jval = (jbyte) val;// Cast to signed byte
			env->SetByteArrayRegion(array, to_jsize(index[index_size - 1]), 1, &jval);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Byte;")) ||// if jobject is jobjectArray of Byte
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jobjectArray array = static_cast<jobjectArray>(obj);
			jclass byteClass = env->FindClass("java/lang/Byte");
			jmethodID constructor = env->GetMethodID(byteClass, "<init>", "(B)V");
			jobject byteObject = env->NewObject(byteClass, constructor, (jbyte) val);// Cast to signed byte
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), byteObject);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be either jbyteArray or jobjectArray of Byte or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_int16(const metaffi_size* index, metaffi_size index_size, metaffi_int16 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.s = val;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[S")))// if jobject is jshortArray
		{
			jshortArray array = static_cast<jshortArray>(obj);
			env->SetShortArrayRegion(array, to_jsize(index[index_size - 1]), 1, &val);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Short;")) ||// if jobject is jobjectArray of Short
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jobjectArray array = static_cast<jobjectArray>(obj);
			jclass shortClass = env->FindClass("java/lang/Short");
			jmethodID constructor = env->GetMethodID(shortClass, "<init>", "(S)V");
			jobject shortObject = env->NewObject(shortClass, constructor, val);
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), shortObject);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be either jshortArray or jobjectArray of Short or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_uint16(const metaffi_size* index, metaffi_size index_size, metaffi_uint16 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.s = (jshort) val;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[S")))// if jobject is jshortArray
		{
			jshortArray array = static_cast<jshortArray>(obj);
			jshort jval = (jshort) val;// Cast to signed short
			env->SetShortArrayRegion(array, to_jsize(index[index_size - 1]), 1, &jval);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Short;")) ||// if jobject is jobjectArray of Short
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jobjectArray array = static_cast<jobjectArray>(obj);
			jclass shortClass = env->FindClass("java/lang/Short");
			jmethodID constructor = env->GetMethodID(shortClass, "<init>", "(S)V");
			jobject shortObject = env->NewObject(shortClass, constructor, (jshort) val);// Cast to signed short
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), shortObject);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be either jshortArray or jobjectArray of Short or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_int32(const metaffi_size* index, metaffi_size index_size, metaffi_int32 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.i = val;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[I")))// if jobject is jintArray
		{
			jintArray array = static_cast<jintArray>(obj);
			env->SetIntArrayRegion(array, to_jsize(index[index_size - 1]), 1, (jint*) &val);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Integer;")) ||
		        // if jobject is jobjectArray of Integer
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jobjectArray array = static_cast<jobjectArray>(obj);
			jclass integerClass = env->FindClass("java/lang/Integer");
			jmethodID constructor = env->GetMethodID(integerClass, "<init>", "(I)V");
			jobject integerObject = env->NewObject(integerClass, constructor, val);
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), integerObject);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be either jintArray or jobjectArray of Integer or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_uint32(const metaffi_size* index, metaffi_size index_size, metaffi_uint32 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.i = (jint) val;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[I")))// if jobject is jintArray
		{
			jintArray array = (jintArray) obj;
			jint jval = (jint) val;// Cast to signed int
			env->SetIntArrayRegion(array, to_jsize(index[index_size - 1]), 1, &jval);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Integer;")) ||
		        // if jobject is jobjectArray of Integer
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jobjectArray array = (jobjectArray) obj;
			jclass integerClass = env->FindClass("java/lang/Integer");
			jmethodID constructor = env->GetMethodID(integerClass, "<init>", "(I)V");
			jobject integerObject = env->NewObject(integerClass, constructor, (jint) val);// Cast to signed int
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), integerObject);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be either jintArray or jobjectArray of Integer or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_int64(const metaffi_size* index, metaffi_size index_size, metaffi_int64 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.j = val;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[J")))// if jobject is jlongArray
		{
			jlongArray array = static_cast<jlongArray>(obj);
			env->SetLongArrayRegion(array, to_jsize(index[index_size - 1]), 1, &val);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Long;")) ||// if jobject is jobjectArray of Long
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jobjectArray array = static_cast<jobjectArray>(obj);
			jclass longClass = env->FindClass("java/lang/Long");
			jmethodID constructor = env->GetMethodID(longClass, "<init>", "(J)V");
			jobject longObject = env->NewObject(longClass, constructor, val);
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), longObject);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be either jlongArray or jobjectArray of Long or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_uint64(const metaffi_size* index, metaffi_size index_size, metaffi_uint64 val, void* context)
{
	if(index_size == 0)
	{
		auto* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.j = (jlong) val;// Cast to signed long
	}
	else// if is part of an array
	{
		auto* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[J")))// if jobject is jlongArray
		{
			auto array = (jlongArray) obj;
			auto jval = (jlong) val;// Cast to signed long
			env->SetLongArrayRegion(array, to_jsize(index[index_size - 1]), 1, &jval);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Long;")) ||// if jobject is jobjectArray of Long
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			auto array = (jobjectArray) obj;
			jclass longClass = env->FindClass("java/lang/Long");
			jmethodID constructor = env->GetMethodID(longClass, "<init>", "(J)V");
			jobject longObject = env->NewObject(longClass, constructor, (jlong) val);// Cast to signed long
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), longObject);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be either jlongArray or jobjectArray of Long or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_bool(const metaffi_size* index, metaffi_size index_size, metaffi_bool val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.z = val;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[Z")))// if jobject is jbooleanArray
		{
			jbooleanArray array = static_cast<jbooleanArray>(obj);
			jboolean jval = val ? JNI_TRUE : JNI_FALSE;
			env->SetBooleanArrayRegion(array, to_jsize(index[index_size - 1]), 1, &jval);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Boolean;")) ||
		        // if jobject is jobjectArray of Boolean
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jobjectArray array = static_cast<jobjectArray>(obj);
			jclass booleanClass = env->FindClass("java/lang/Boolean");
			jmethodID constructor = env->GetMethodID(booleanClass, "<init>", "(Z)V");
			jobject booleanObject = env->NewObject(booleanClass, constructor, val);
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), booleanObject);
		}
		else
		{
			throw std::invalid_argument(
			        "Expected jobject to be either jbooleanArray or jobjectArray of Boolean or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_char8(const metaffi_size* index, metaffi_size index_size, metaffi_char8 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		jvalue jval = (jvalue) pair->second;
		jchar_wrapper wrapper(pair->first, val);
		jval.c = (jchar) wrapper;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[C")))// if jobject is jcharArray
		{
			jcharArray array = static_cast<jcharArray>(obj);
			jchar_wrapper wrapper(env, val);
			jchar jval = static_cast<jchar>(wrapper);
			env->SetCharArrayRegion(array, to_jsize(index[index_size - 1]), 1, &jval);
		}
		else if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Character;")) ||
		        // if jobject is jobjectArray of Character
		        env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jobjectArray array = static_cast<jobjectArray>(obj);
			jchar_wrapper wrapper(env, val);
			jchar jval = static_cast<jchar>(wrapper);
			jclass characterClass = env->FindClass("java/lang/Character");
			jmethodID constructor = env->GetMethodID(characterClass, "<init>", "(C)V");
			jobject characterObject = env->NewObject(characterClass, constructor, jval);
			env->SetObjectArrayElement(array, to_jsize(index[index_size - 1]), characterObject);
		}
		else
		{
			throw std::invalid_argument(
			        "Expected jobject to be either jcharArray or jobjectArray of Character or Object");
		}
	}
}

DLL_PRIVATE void on_traverse_string8(const metaffi_size* index, metaffi_size index_size, metaffi_string8 val, void* context)
{
	
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		jstring_wrapper wrapper(pair->first, val);
		pair->second.l = (jstring)wrapper;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/String;")))// if jobject is jobjectArray of String
		{
			jobjectArray jarr = (jobjectArray) obj;
			jstring_wrapper wrapper(env, val);
			jstring jstr = static_cast<jstring>(wrapper);
			env->SetObjectArrayElement(jarr, to_jsize(index[0]), jstr);
		}
	}
}

DLL_PRIVATE void on_traverse_char16(const metaffi_size* index, metaffi_size index_size, metaffi_char16 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.c = (jchar) val.c[0];
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[C")))// if jobject is jcharArray
		{
			jcharArray jarr = (jcharArray) obj;
			jchar jch = static_cast<jchar>(val.c[0]);
			env->SetCharArrayRegion(jarr, to_jsize(index[0]), 1, &jch);
		}
		else if(env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Character;")))// if jobject is jobjectArray of Character
		{
			jobjectArray jarr = (jobjectArray) obj;
			jclass charClass = env->FindClass("java/lang/Character");
			jmethodID charConstructor = env->GetMethodID(charClass, "<init>", "(C)V");
			jobject jcharObj = env->NewObject(charClass, charConstructor, static_cast<jchar>(val.c[0]));
			env->SetObjectArrayElement(jarr, to_jsize(index[0]), jcharObj);
		}
	}
}

DLL_PRIVATE void on_traverse_string16(const metaffi_size* index, metaffi_size index_size, metaffi_string16 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		jstring_wrapper wrapper(pair->first, val);
		pair->second.l = static_cast<jstring>(wrapper);
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/String;")))// if jobject is jobjectArray of String
		{
			jobjectArray jarr = (jobjectArray) obj;
			jstring_wrapper wrapper(env, val);
			jstring jstr = static_cast<jstring>(wrapper);
			env->SetObjectArrayElement(jarr, to_jsize(index[0]), jstr);
		}
	}
}

DLL_PRIVATE void on_traverse_char32(const metaffi_size* index, metaffi_size index_size, metaffi_char32 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.c = (jchar) val.c;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[C")))// if jobject is jcharArray
		{
			jcharArray jarr = (jcharArray) obj;
			jchar jch = static_cast<jchar>(val.c);
			env->SetCharArrayRegion(jarr, to_jsize(index[0]), 1, &jch);
		}
		else if(env->IsInstanceOf(obj,
		                          env->FindClass("[Ljava/lang/Character;")))// if jobject is jobjectArray of Character
		{
			jobjectArray jarr = (jobjectArray) obj;
			jclass charClass = env->FindClass("java/lang/Character");
			jmethodID charConstructor = env->GetMethodID(charClass, "<init>", "(C)V");
			jobject jcharObj = env->NewObject(charClass, charConstructor, static_cast<jchar>(val.c));
			env->SetObjectArrayElement(jarr, to_jsize(index[0]), jcharObj);
		}
	}
}

DLL_PRIVATE void on_traverse_string32(const metaffi_size* index, metaffi_size index_size, metaffi_string32 val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		jstring_wrapper wrapper(pair->first, val);
		pair->second.l = static_cast<jstring>(wrapper);
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/String;")))// if jobject is jobjectArray of String
		{
			jobjectArray jarr = (jobjectArray) obj;
			jstring_wrapper wrapper(env, val);
			jstring jstr = static_cast<jstring>(wrapper);
			env->SetObjectArrayElement(jarr, to_jsize(index[0]), jstr);
		}
	}
}

DLL_PRIVATE void on_traverse_handle(const metaffi_size* index, metaffi_size index_size, const cdt_metaffi_handle& val, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;

		if(val.runtime_id != JVM_RUNTIME_ID)
		{
			if(val.handle == nullptr)
			{
				// returned null
				pair->second.l = nullptr;
			}
			else
			{
				jni_metaffi_handle h(env, val.handle, val.runtime_id, val.release);
				pair->second.l = h.new_jvm_object(env);
			}
		}
		else
		{
			pair->second.l = (jobject) val.handle;
		}
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Object;")))// if jobject is jobjectArray of Object
		{
			jobjectArray jarr = (jobjectArray) obj;
			jobject jobj = nullptr;
			if(val.runtime_id != JVM_RUNTIME_ID)
			{
				if(val.handle == nullptr)
				{
					// returned null
					pair->second.l = nullptr;
				}
				else
				{
					jni_metaffi_handle h(env, val.handle, val.runtime_id, val.release);
					jobj = h.new_jvm_object(env);
				}				
			}
			else
			{
				jobj = (jobject) val.handle;
			}

			env->SetObjectArrayElement(jarr, to_jsize(index[0]), jobj);
		}
		else
		{
			throw std::invalid_argument("Expected jobject to be jobjectArray of Object");
		}
	}
}

DLL_PRIVATE void on_traverse_callable(const metaffi_size* index, metaffi_size index_size, const cdt_metaffi_callable& val, void* context)
{
	auto pcreate_caller = [](JNIEnv* env, const cdt_metaffi_callable& val) -> jobject {
		jclass load_callable_cls = env->FindClass("metaffi/api/accessor/Caller");
		check_and_throw_jvm_exception(env, true);

		jmethodID create_caller = env->GetStaticMethodID(load_callable_cls, "createCaller",
		                                                 "(J[J[J)Lmetaffi/api/accessor/Caller;");
		check_and_throw_jvm_exception(env, true);

		// Convert parameters_types to Java long[]
		jlongArray jParametersTypesArray = env->NewLongArray(val.params_types_length);
		env->SetLongArrayRegion(jParametersTypesArray, 0, val.params_types_length,
		                        (const jlong*) val.parameters_types);

		// Convert retval_types to Java long[]
		jlongArray jRetvalsTypesArray = env->NewLongArray(val.retval_types_length);
		env->SetLongArrayRegion(jRetvalsTypesArray, 0, val.retval_types_length,
		                        (const jlong*) val.retval_types);


		jobject o = env->CallStaticObjectMethod(load_callable_cls, create_caller,
		                                        val.val,
		                                        jParametersTypesArray,
		                                        jRetvalsTypesArray);
		check_and_throw_jvm_exception(env, true);

		if(env->GetObjectRefType(jParametersTypesArray) == JNILocalRefType)
		{
			env->DeleteLocalRef(jParametersTypesArray);
		}

		if(env->GetObjectRefType(jRetvalsTypesArray) == JNILocalRefType)
		{
			env->DeleteLocalRef(jRetvalsTypesArray);
		}

		return o;
	};

	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.l = pcreate_caller(pair->first, val);
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		if(env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Object;")))// if jobject is jobjectArray of String
		{
			jobjectArray jarr = (jobjectArray) obj;
			jobject jobj = pcreate_caller(env, val);
			env->SetObjectArrayElement(jarr, to_jsize(index[0]), jobj);
		}
	}
}

DLL_PRIVATE void on_traverse_null(const metaffi_size* index, metaffi_size index_size, void* context)
{
	if(index_size == 0)
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		pair->second.l = nullptr;
	}
	else// if is part of an array
	{
		std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
		JNIEnv* env = pair->first;
		jarray root = (jarray) pair->second.l;

		// traverse to the array holding this value

		// for 1D arrays, there's no "previous" index to hold the result, but the previous is "root"
		auto element = jarray_wrapper::get_element(env, root, index, index_size == 1 ? index_size : index_size - 1);
		check_and_throw_jvm_exception(env, true);
		jarray obj = index_size == 1 ? root : (jarray) element.first.l;

		pair->first->SetObjectArrayElement((jobjectArray) obj, to_jsize(index[0]), nullptr);
	}
}

DLL_PRIVATE metaffi_bool on_traverse_array(const metaffi_size* index, metaffi_size index_size, const cdts& val, metaffi_int64 fixed_dimensions, metaffi_type common_type, void* context)
{
	// traverse to the last dimension based on the indices array in "index" and "index_size"
	// and create another array of "common_type" and set it to the jobject "context"
	std::pair<JNIEnv*, jvalue&>* pair = static_cast<std::pair<JNIEnv*, jvalue&>*>(context);
	JNIEnv* env = pair->first;
	jobject obj = pair->second.l;

	if(obj && env->IsInstanceOf(obj, env->FindClass("[Ljava/lang/Object;")))// if jobject is jobjectArray of Object
	{
		jvalue new_arr;
		new_arr.l = jarray_wrapper::create_jni_array(env, common_type, fixed_dimensions, val.length);

		jarray hosting_array = (jarray) obj;
		if(index_size > 1)
		{
			auto elem = jarray_wrapper::get_element(env, (jarray) obj, index, index_size - 1);
			hosting_array = (jarray) elem.first.l;
		}

		jarray_wrapper jarr(env, (jarray) hosting_array, metaffi_array_type);
		jarr.set(to_jsize(index[index_size - 1]), new_arr);
	}
	else if(!obj)
	{
		pair->second.l = jarray_wrapper::create_jni_array(env, common_type, fixed_dimensions, val.length);
	}
	else
	{
		throw std::invalid_argument("Expected jobject to be jarray of Object");
	}

	return 1;
}

DLL_PRIVATE metaffi::runtime::traverse_cdts_callbacks jni_get_traverse_cdts_callback(void* context)
{
	metaffi::runtime::traverse_cdts_callbacks tcc = {
	        context,
	        &on_traverse_float64,
	        &on_traverse_float32,
	        &on_traverse_int8,
	        &on_traverse_uint8,
	        &on_traverse_int16,
	        &on_traverse_uint16,
	        &on_traverse_int32,
	        &on_traverse_uint32,
	        &on_traverse_int64,
	        &on_traverse_uint64,
	        &on_traverse_bool,
	        &on_traverse_char8,
	        &on_traverse_string8,
	        &on_traverse_char16,
	        &on_traverse_string16,
	        &on_traverse_char32,
	        &on_traverse_string32,
	        &on_traverse_handle,
	        &on_traverse_callable,
	        &on_traverse_null,
	        &on_traverse_array};

	return tcc;
}

#define JNIEnv_context(ctxt) std::get<0>(*(static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(ctxt)))
#define jvalue_context(ctxt) std::get<1>(*(static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(ctxt)))
#define jvalue_type_context(ctxt) std::get<2>(*(static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(ctxt)))
#define metaffi_type_info_context(ctxt) std::get<3>(*(static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(ctxt)))

DLL_PRIVATE metaffi_size on_construct_array_metadata(const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_fixed_dimension, metaffi_bool* is_1d_array, metaffi_type* common_type, metaffi_bool* is_manually_construct_array, void* context)
{
	auto elem = jarray_wrapper::get_element(JNIEnv_context(context), (jarray) jvalue_context(context).l, index, index_length);
	if(elem.second != 'L') { throw std::invalid_argument("Expected jobject to be an array"); }

	auto res = jarray_wrapper::get_array_info(JNIEnv_context(context), (jarray) elem.first.l, metaffi_type_info_context(context));

	*common_type = res.first.type;
	*is_fixed_dimension = true; // JVM only supports fixed dimensions
	*is_1d_array = res.first.fixed_dimensions == 1;
	*is_manually_construct_array = 0;

	return res.second;
}

DLL_PRIVATE metaffi_size on_get_root_elements_count(void* context)
{

	return JNIEnv_context(context)->GetArrayLength((jarray) jvalue_context(context).l);
}

DLL_PRIVATE metaffi_type_info on_get_type_info(const metaffi_size* index, metaffi_size index_size, void* context)
{
	auto type_info_from_jvalue = [](JNIEnv* env, jvalue& jval, char jval_type, metaffi_type_info root_type_info) -> metaffi_type_info {
		if(jval_type == 'L')
		{
			if(jarray_wrapper::is_array(env, (jarray) jval.l))
			{
				auto mtype = jarray_wrapper::get_array_info(env, (jarray) jval.l, root_type_info);
				return mtype.first;
			}
			else
			{
				std::string clsname = jni_class::get_object_class_name(env, jval.l);
				if(clsname == "java.lang.Byte")
				{
					return {metaffi_int8_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				}
				else if(clsname == "java.lang.Short")
				{
					return {metaffi_int16_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				}
				else if(clsname == "java.lang.Integer")
				{
					return {metaffi_int32_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				}
				else if(clsname == "java.lang.Long")
				{
					return {metaffi_int64_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				}
				else if(clsname == "java.lang.Float")
				{
					return {metaffi_float32_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				}
				else if(clsname == "java.lang.Double")
				{
					return {metaffi_float64_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				}
				else if(clsname == "java.lang.Boolean")
				{
					return {metaffi_bool_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				}
				else if(clsname == "java.lang.Character")
				{
					return {metaffi_char8_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				}
				else if(clsname == "java.lang.String")
				{
					return {metaffi_string8_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				}
				else
				{
					return {metaffi_handle_type, clsname.c_str(), true, MIXED_OR_UNKNOWN_DIMENSIONS};
				}
			}
		}
		else
		{
			switch(jval_type)
			{
				case 'D':
					return {metaffi_float64_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				case 'F':
					return {metaffi_float32_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				case 'B':
					if(root_type_info.type & metaffi_uint8_type)
						return {metaffi_uint8_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
					else if(root_type_info.type & metaffi_int8_type)
						return {metaffi_int8_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
					else
					{
						std::stringstream ss;
						ss << "Expected metaffi_uint8_type or metaffi_int8_type. Got: " << root_type_info.type;
						throw std::invalid_argument(ss.str());
					}
				case 'S':
					if(root_type_info.type & metaffi_uint16_type)
						return {metaffi_uint16_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
					else if(root_type_info.type & metaffi_int16_type)
						return {metaffi_int16_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
					else
					{
						std::stringstream ss;
						ss << "Expected metaffi_uint16_type or metaffi_int16_type. Got: " << root_type_info.type;
						throw std::invalid_argument(ss.str());
					}
				case 'I':
					if(root_type_info.type & metaffi_uint32_type)
						return {metaffi_uint32_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
					else if(root_type_info.type & metaffi_int32_type)
						return {metaffi_int32_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
					else
					{
						std::stringstream ss;
						ss << "Expected metaffi_uint32_type or metaffi_int32_type. Got: " << root_type_info.type;
						throw std::invalid_argument(ss.str());
					}
				case 'J':
					if(root_type_info.type & metaffi_uint64_type)
						return {metaffi_uint64_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
					else if(root_type_info.type & metaffi_int64_type)
						return {metaffi_int64_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
					else
					{
						std::stringstream ss;
						ss << "Expected metaffi_uint64_type or metaffi_int64_type. Got: " << root_type_info.type;
						throw std::invalid_argument(ss.str());
					}
				case 'Z':
					return {metaffi_bool_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				case 'C':
					return {metaffi_char8_type, nullptr, false, MIXED_OR_UNKNOWN_DIMENSIONS};
				default: {
					std::stringstream ss;
					ss << "Unexpected JNI type: " << jval_type;
					throw std::invalid_argument(ss.str());
				}
			}
		}
	};

	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);
	const metaffi_type_info& root_type_info = metaffi_type_info_context(context);

	if(index_size == 0)// asking for "root" type info
	{
		if(root_type_info.type != metaffi_any_type)
		{
			return root_type_info;
		}

		// metaffi_any_type - we need to check what is the type of the jobject
		char jval_type = jvalue_type_context(context);

		return type_info_from_jvalue(env, jval, jval_type, root_type_info);
	}

	auto res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
	return type_info_from_jvalue(env, res.first, res.second, root_type_info);
}

DLL_PRIVATE metaffi_float64 on_construct_float64(const metaffi_size* index, metaffi_size index_size, void* context)
{

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.

		jvalue& jval = jvalue_context(context);
		if(JNIEnv_context(context)->IsInstanceOf(jval.l, JNIEnv_context(context)->FindClass("java/lang/Double")))
		{
			return (metaffi_float64) ((jdouble) jdouble_wrapper(JNIEnv_context(context), jval.l));
		}
		else
		{
			return jval.d;
		}
	}
	else
	{
		JNIEnv* env = JNIEnv_context(context);
		jobject obj = jvalue_context(context).l;

		auto res = jarray_wrapper::get_element(env, (jarray) obj, index, index_size);

		jvalue jval = res.first;

		if(res.second == 'D')
		{
			return (metaffi_float64) std::get<0>(res).d;
		}
		else if(env->IsInstanceOf(jval.l, env->FindClass("Ljava/lang/Double;")) ||
		        // if jobject is jobjectArray of Double
		        env->IsInstanceOf(jval.l,
		                          env->FindClass("Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			return (metaffi_float64) jdouble_wrapper(env, jval.l);
		}
		else
		{
			throw std::invalid_argument(
			        "Expected jobject to be either jdoubleArray or jobjectArray of Double or Object");
		}
	}
}

DLL_PRIVATE metaffi_float32 on_construct_float32(const metaffi_size* index, metaffi_size index_size, void* context)
{

	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L' && env->IsInstanceOf(jval.l, env->FindClass("java/lang/Float")))
		{
			return (metaffi_float32) ((jfloat) jfloat_wrapper(env, jval.l));
		}
		else if(jvalue_type == 'F')
		{
			return jval.f;
		}
		else
		{
			throw std::runtime_error("Expected a float or Float instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue val = res.first;

		if(res.second == 'F')
		{
			return (metaffi_float32) val.f;
		}
		else if(env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Float;")) ||// if jobject is jobjectArray of Float
		        env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Object;"))) // or if jobject is jobjectArray of Object
		{
			return (metaffi_float32) jfloat_wrapper(env, val.l);
		}
		else
		{
			throw std::runtime_error("Expected a Float or Object instance");
		}
	}
}

DLL_PRIVATE metaffi_int8 on_construct_int8(const metaffi_size* index, metaffi_size index_size, void* context)
{

	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_int8) ((jbyte) jbyte_wrapper(env, jval.l));
			}
			else
			{
				throw std::runtime_error("Expected a byte/Byte instance");
			}
		}
		else if(jvalue_type == 'B')
		{
			return jval.b;
		}
		else
		{
			throw std::runtime_error("Expected a byte/Byte instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue elem = res.first;
		char jvalue_type = res.second;
		
		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_int8) ((jbyte) jbyte_wrapper(env, elem.l));
			}
			else
			{
				throw std::runtime_error("Expected a byte/Byte instance");
			}
		}
		else if(jvalue_type == 'B')
		{
			return elem.b;
		}
		else
		{
			throw std::runtime_error("Expected a byte/Byte instance");
		}
	}
}

DLL_PRIVATE metaffi_uint8 on_construct_uint8(const metaffi_size* index, metaffi_size index_size, void* context)
{
	std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>* context_data = static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(context);
	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_uint8) ((jbyte) jbyte_wrapper(env, jval.l));
			}
			else
			{
				throw std::runtime_error("Expected a byte/Byte instance");
			}
		}
		else if(jvalue_type == 'B')
		{
			return jval.b;
		}
		else
		{
			throw std::runtime_error("Expected a byte/Byte instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue elem = res.first;
		char jvalue_type = res.second;

		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_uint8) ((jbyte) jbyte_wrapper(env, elem.l));
			}
			else
			{
				throw std::runtime_error("Expected a byte/Byte instance");
			}
		}
		else if(jvalue_type == 'B')
		{
			return elem.b;
		}
		else
		{
			throw std::runtime_error("Expected a byte/Byte instance");
		}
	}
}

DLL_PRIVATE metaffi_int16 on_construct_int16(const metaffi_size* index, metaffi_size index_size, void* context)
{
	std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>* context_data = static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(context);
	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);


	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_int16) ((jshort) jshort_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_int16) ((jbyte) jbyte_wrapper(env, jval.l));
			}
			else
			{
				throw std::runtime_error("Expected a short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'S')
		{
			return jval.s;
		}
		else if(jvalue_type == 'B')
		{
			return jval.b;
		}
		else
		{
			throw std::runtime_error("Expected a short/Short/byte/Byte instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue elem = res.first;
		char jvalue_type = res.second;

		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_int16) ((jshort) jshort_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_int16) ((jbyte) jbyte_wrapper(env, elem.l));
			}
			else
			{
				throw std::runtime_error("Expected a short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'S')
		{
			return elem.s;
		}
		else if(jvalue_type == 'B')
		{
			return elem.b;
		}
		else
		{
			throw std::runtime_error("Expected a short/Short/byte/Byte instance");
		}
	}
}

DLL_PRIVATE metaffi_uint16 on_construct_uint16(const metaffi_size* index, metaffi_size index_size, void* context)
{
	std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>* context_data = static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(context);
	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_uint16) ((jshort) jshort_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_uint16) ((jbyte) jbyte_wrapper(env, jval.l));
			}
			else
			{
				throw std::runtime_error("Expected a int/Integer/short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'S')
		{
			return jval.s;
		}
		else if(jvalue_type == 'B')
		{
			return jval.b;
		}
		else
		{
			throw std::runtime_error("Expected a int/Integer/short/Short/byte/Byte instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue elem = res.first;
		char jvalue_type = res.second;

		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_uint16) ((jshort) jshort_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_uint16) ((jbyte) jbyte_wrapper(env, elem.l));
			}
			else
			{
				throw std::runtime_error("Expected a short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'S')
		{
			return elem.s;
		}
		else if(jvalue_type == 'B')
		{
			return elem.b;
		}
		else
		{
			throw std::runtime_error("Expected a short/Short/byte/Byte instance");
		}
	}
}

DLL_PRIVATE metaffi_int32 on_construct_int32(const metaffi_size* index, metaffi_size index_size, void* context)
{

	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);


	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Integer")))
			{
				return (metaffi_int32) ((jint) jint_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_int32) ((jshort) jshort_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_int32) ((jbyte) jbyte_wrapper(env, jval.l));
			}
			else
			{
				throw std::runtime_error("Expected a int/Integer/short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'I')
		{
			return jval.i;
		}
		else if(jvalue_type == 'S')
		{
			return jval.s;
		}
		else if(jvalue_type == 'B')
		{
			return jval.b;
		}
		else
		{
			throw std::runtime_error("Expected a int/Integer/short/Short/byte/Byte instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue elem = res.first;
		char jvalue_type = res.second;

		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Integer")))
			{
				return (metaffi_int32) ((jint) jint_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_int32) ((jshort) jshort_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_int32) ((jbyte) jbyte_wrapper(env, elem.l));
			}
			else
			{
				throw std::runtime_error("Expected a int/Integer/short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'I')
		{
			return elem.i;
		}
		else if(jvalue_type == 'S')
		{
			return elem.s;
		}
		else if(jvalue_type == 'B')
		{
			return elem.b;
		}
		else
		{
			throw std::runtime_error("Expected a int/Integer/short/Short/byte/Byte instance");
		}
	}
}

DLL_PRIVATE metaffi_uint32 on_construct_uint32(const metaffi_size* index, metaffi_size index_size, void* context)
{

	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Integer")))
			{
				return (metaffi_uint32) ((jint) jint_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_uint32) ((jshort) jshort_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_uint32) ((jbyte) jbyte_wrapper(env, jval.l));
			}
			else
			{
				throw std::runtime_error("Expected a int/Integer/short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'I')
		{
			return jval.i;
		}
		else if(jvalue_type == 'S')
		{
			return jval.s;
		}
		else if(jvalue_type == 'B')
		{
			return jval.b;
		}
		else
		{
			throw std::runtime_error("Expected a int/Integer/short/Short/byte/Byte instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue elem = res.first;
		char jvalue_type = res.second;

		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Integer")))
			{
				return (metaffi_uint32) ((jint) jint_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_uint32) ((jshort) jshort_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_uint32) ((jbyte) jbyte_wrapper(env, elem.l));
			}
			else
			{
				throw std::runtime_error("Expected a int/Integer/short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'I')
		{
			return elem.i;
		}
		else if(jvalue_type == 'S')
		{
			return elem.s;
		}
		else if(jvalue_type == 'B')
		{
			return elem.b;
		}
		else
		{
			throw std::runtime_error("Expected a int/Integer/short/Short/byte/Byte instance");
		}
	}
}

DLL_PRIVATE metaffi_int64 on_construct_int64(const metaffi_size* index, metaffi_size index_size, void* context)
{

	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	if(index_size == 0)
	{
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Long")))
			{
				return (metaffi_int64) ((jlong) jlong_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Integer")))
			{
				return (metaffi_int64) ((jint) jint_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_int64) ((jshort) jshort_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_int64) ((jbyte) jbyte_wrapper(env, jval.l));
			}
			else
			{
				throw std::runtime_error("Expected a long/Long/int/Integer/short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'J')
		{
			return jval.j;
		}
		else if(jvalue_type == 'I')
		{
			return jval.i;
		}
		else if(jvalue_type == 'S')
		{
			return jval.s;
		}
		else if(jvalue_type == 'B')
		{
			return jval.b;
		}
		else
		{
			throw std::runtime_error("Expected a long/Long/int/Integer/short/Short/byte/Byte instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue elem = res.first;
		char jvalue_type = res.second;

		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Long")))
			{
				return (metaffi_int64) ((jlong) jlong_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Integer")))
			{
				return (metaffi_int64) ((jint) jint_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_int64) ((jshort) jshort_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_int64) ((jbyte) jbyte_wrapper(env, elem.l));
			}
			else
			{
				std::stringstream ss;
				ss << "Expected a long/Long/int/Integer/short/Short/byte/Byte instance. Got: " << jni_class::get_object_class_name(env, jval.l);
				throw std::runtime_error(ss.str());
			}
		}
		else if(jvalue_type == 'J')
		{
			return elem.j;
		}
		else if(jvalue_type == 'I')
		{
			return elem.i;
		}
		else if(jvalue_type == 'S')
		{
			return elem.s;
		}
		else if(jvalue_type == 'B')
		{
			return elem.b;
		}
		else
		{
			throw std::runtime_error("Expected a long/Long/int/Integer/short/Short/byte/Byte instance");
		}
	}
}

DLL_PRIVATE metaffi_uint64 on_construct_uint64(const metaffi_size* index, metaffi_size index_size, void* context)
{

	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Long")))
			{
				return (metaffi_uint64) ((jlong) jlong_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Integer")))
			{
				return (metaffi_uint64) ((jint) jint_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_uint64) ((jshort) jshort_wrapper(env, jval.l));
			}
			else if(env->IsInstanceOf(jval.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_uint64) ((jbyte) jbyte_wrapper(env, jval.l));
			}
			else
			{
				throw std::runtime_error("Expected a long/Long/int/Integer/short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'J')
		{
			return jval.j;
		}
		else if(jvalue_type == 'I')
		{
			return jval.i;
		}
		else if(jvalue_type == 'S')
		{
			return jval.s;
		}
		else if(jvalue_type == 'B')
		{
			return jval.b;
		}
		else
		{
			throw std::runtime_error("Expected a long/Long/int/Integer/short/Short/byte/Byte instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue elem = res.first;
		char jvalue_type = res.second;

		if(jvalue_type == 'L')
		{
			if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Long")))
			{
				return (metaffi_uint64) ((jlong) jlong_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Integer")))
			{
				return (metaffi_uint64) ((jint) jint_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Short")))
			{
				return (metaffi_uint64) ((jshort) jshort_wrapper(env, elem.l));
			}
			else if(env->IsInstanceOf(elem.l, env->FindClass("java/lang/Byte")))
			{
				return (metaffi_uint64) ((jbyte) jbyte_wrapper(env, elem.l));
			}
			else
			{
				throw std::runtime_error("Expected a long/Long/int/Integer/short/Short/byte/Byte instance");
			}
		}
		else if(jvalue_type == 'J')
		{
			return elem.j;
		}
		else if(jvalue_type == 'I')
		{
			return elem.i;
		}
		else if(jvalue_type == 'S')
		{
			return elem.s;
		}
		else if(jvalue_type == 'B')
		{
			return elem.b;
		}
		else
		{
			throw std::runtime_error("Expected a long/Long/int/Integer/short/Short/byte/Byte instance");
		}
	}
}

DLL_PRIVATE metaffi_bool on_construct_bool(const metaffi_size* index, metaffi_size index_size, void* context)
{

	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L' && env->IsInstanceOf(jval.l, env->FindClass("java/lang/Boolean")))
		{
			return (metaffi_bool) ((jboolean) jboolean_wrapper(env, jval.l));
		}
		else if(jvalue_type == 'Z')
		{
			return jval.z;
		}
		else
		{
			throw std::runtime_error("Expected a boolean or Boolean instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue val = res.first;

		if(res.second == 'Z')
		{
			return (metaffi_bool) val.z;
		}
		else if(env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Boolean;")) ||
		        // if jobject is jobjectArray of Boolean
		        env->IsInstanceOf(val.l,
		                          env->FindClass("Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			jclass booleanClass = env->FindClass("java/lang/Boolean");
			jmethodID booleanValue = env->GetMethodID(booleanClass, "booleanValue", "()Z");
			jboolean jbool = env->CallBooleanMethod(val.l, booleanValue);
			return (metaffi_bool) jbool;
		}
		else
		{
			throw std::runtime_error("Expected a Boolean or Object instance");
		}
	}
}

DLL_PRIVATE metaffi_char8 on_construct_char8(const metaffi_size* index, metaffi_size index_size, void* context)
{

	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L' && env->IsInstanceOf(jval.l, env->FindClass("java/lang/Character")))
		{
			return (metaffi_char8) jchar_wrapper(env, jval.l);
		}
		else if(jvalue_type == 'C')
		{
			return (metaffi_char8) jchar_wrapper(env, jval.c);
		}
		else
		{
			throw std::runtime_error("Expected a char instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue val = res.first;

		if(res.second == 'C')
		{
			return (metaffi_char8) jchar_wrapper(env, val.c);
		}
		else if(env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Character;")) ||
		        // if jobject is jobjectArray of Character
		        env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			return (metaffi_char8) jchar_wrapper(env, val.l);
		}
		else
		{
			throw std::runtime_error("Expected a Character or Object instance");
		}
	}
}

DLL_PRIVATE metaffi_string8 on_construct_string8(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
{
	
	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);
	*is_free_required = true;
	
	if(index_size == 0)
	{
		
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L' && env->IsInstanceOf(jval.l, env->FindClass("java/lang/String")))
		{
			return (metaffi_string8) jstring_wrapper(env, (jstring) jval.l);
		}
		else
		{
			throw std::runtime_error("Expected a char instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue val = res.first;

		if(env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/String;")) ||// if jobject is jobjectArray of String
		   env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Object;")))  // or if jobject is jobjectArray of Object
		{
			jstring_wrapper wrapper(env, (jstring) val.l);
			return (metaffi_string8) wrapper;
		}
		else
		{
			throw std::runtime_error("Expected a String or Object instance");
		}
	}
}

DLL_PRIVATE metaffi_char16 on_construct_char16(const metaffi_size* index, metaffi_size index_size, void* context)
{

	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L' && env->IsInstanceOf(jval.l, env->FindClass("java/lang/Character")))
		{
			return (metaffi_char16) jchar_wrapper(env, jval.l);
		}
		else if(jvalue_type == 'C')
		{
			return (metaffi_char16) jchar_wrapper(env, jval.c);
		}
		else
		{
			throw std::runtime_error("Expected a char instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue val = res.first;

		if(res.second == 'C')
		{
			return (metaffi_char16) jchar_wrapper(env, val.c);
		}
		else if(env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Character;")) ||
		        // if jobject is jobjectArray of Character
		        env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			return (metaffi_char16) jchar_wrapper(env, val.l);
		}
		else
		{
			throw std::runtime_error("Expected a Character or Object instance");
		}
	}
}

DLL_PRIVATE metaffi_string16 on_construct_string16(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
{
	std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>* context_data = static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(context);
	JNIEnv* env = std::get<0>(*context_data);
	jvalue& jval = std::get<1>(*context_data);
	*is_free_required = true;
	
	std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
	jvalue val = res.first;

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L' && env->IsInstanceOf(jval.l, env->FindClass("java/lang/String")))
		{
			return (metaffi_string16) jstring_wrapper(env, (jstring) jval.l);
		}
		else
		{
			throw std::runtime_error("Expected a char instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue val = res.first;

		if(env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/String;")) ||// if jobject is jobjectArray of String
		   env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Object;")))  // or if jobject is jobjectArray of Object
		{
			jstring_wrapper wrapper(env, (jstring) val.l);
			return (metaffi_string16) wrapper;
		}
		else
		{
			throw std::runtime_error("Expected a String or Object instance");
		}
	}
}

DLL_PRIVATE metaffi_char32 on_construct_char32(const metaffi_size* index, metaffi_size index_size, void* context)
{
	std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>* context_data = static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(context);
	JNIEnv* env = JNIEnv_context(context);
	jvalue& jval = jvalue_context(context);

	std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
	jvalue val = res.first;

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L' && env->IsInstanceOf(jval.l, env->FindClass("java/lang/Character")))
		{
			return (metaffi_char32) jchar_wrapper(env, jval.l);
		}
		else if(jvalue_type == 'C')
		{
			return (metaffi_char32) jchar_wrapper(env, jval.c);
		}
		else
		{
			throw std::runtime_error("Expected a char instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue val = res.first;

		if(res.second == 'C')
		{
			return (metaffi_char32) jchar_wrapper(env, val.c);
		}
		else if(env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Character;")) ||
		        // if jobject is jobjectArray of Character
		        env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Object;")))// or if jobject is jobjectArray of Object
		{
			return (metaffi_char32) jchar_wrapper(env, val.l);
		}
		else
		{
			throw std::runtime_error("Expected a Character or Object instance");
		}
	}
}

DLL_PRIVATE metaffi_string32 on_construct_string32(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
{
	std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>* context_data = static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(context);
	JNIEnv* env = std::get<0>(*context_data);
	jvalue& jval = std::get<1>(*context_data);
	*is_free_required = true;

	std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
	jvalue val = res.first;

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L' && env->IsInstanceOf(jval.l, env->FindClass("java/lang/String")))
		{
			return (metaffi_string32) jstring_wrapper(env, (jstring) jval.l);
		}
		else
		{
			throw std::runtime_error("Expected a char instance");
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue val = res.first;

		if(env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/String;")) ||// if jobject is jobjectArray of String
		   env->IsInstanceOf(val.l, env->FindClass("Ljava/lang/Object;")))  // or if jobject is jobjectArray of Object
		{
			jstring_wrapper wrapper(env, (jstring) val.l);
			return (metaffi_string32) wrapper;
		}
		else
		{
			throw std::runtime_error("Expected a String or Object instance");
		}
	}
}

bool is_valid_jobject(JNIEnv* env, jobject obj)
{
	if (obj == nullptr) {
		return false; // Null pointer, object is not valid
	}
	
	jclass objectClass = env->FindClass("java/lang/Object");
	if (objectClass == nullptr) {
		return false; // Couldn't find java/lang/Object class, something is very wrong
	}

	jboolean isInstance = env->IsInstanceOf(obj, objectClass);
	env->DeleteLocalRef(objectClass); // Clean up local reference

	return isInstance == JNI_TRUE;
}

void jni_releaser(cdt_metaffi_handle* ptr)
{
	if(!ptr){
		return;
	}
	
    JNIEnv* env = nullptr;
	auto release_env = pjvm->get_environment(&env);
	
	if(!env)
	{
		METAFFI_ERROR(LOG, "Failed to get JNIEnv in jni_releaser");
		return;
	}
	
	if(!is_valid_jobject(env, (jobject)ptr->handle))
	{
		METAFFI_ERROR(LOG, "jni_releaser received an invalid jobject");
		return;
	}
	
	auto refType = env->GetObjectRefType((jobject)ptr->handle);
	
	if(refType == JNILocalRefType)
	{
		env->DeleteLocalRef((jobject)ptr->handle); // remove local ref from the object
	}
	else if(refType == JNIGlobalRefType)
	{
		env->DeleteGlobalRef((jobject)ptr->handle); // remove global ref from the object
	}
	else if(refType == JNIWeakGlobalRefType)
	{
		env->DeleteWeakGlobalRef((jweak)ptr->handle); // remove weak global ref from the object
	}
	
	ptr->handle = nullptr;
	ptr->runtime_id = 0;
	ptr->release = nullptr;
	
	release_env();
}

DLL_PRIVATE cdt_metaffi_handle* on_construct_handle(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
{
	std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>* context_data = static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(context);
	JNIEnv* env = std::get<0>(*context_data);
	jvalue& jval = std::get<1>(*context_data);
	*is_free_required = true;

	if(index_size == 0)
	{
		// jvalue is expected to be either double or Double.
		// extract the value and return it.
		char jvalue_type = jvalue_type_context(context);
		if(jvalue_type == 'L' && jni_metaffi_handle::is_metaffi_handle_wrapper_object(env, jval.l))
		{
			return (cdt_metaffi_handle*)jni_metaffi_handle(env, jval.l);
		}
		else
		{
			return jni_metaffi_handle::wrap_in_metaffi_handle(env, jval.l, (void*)jni_releaser);
		}
	}
	else
	{
		std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
		jvalue val = res.first;

		if(jni_metaffi_handle::is_metaffi_handle_wrapper_object(env, val.l))
		{
			jni_metaffi_handle wrapper(env, val.l);
			return (cdt_metaffi_handle*)wrapper;
		}
		else// if jobject is jobjectArray of Object
		{
			return (cdt_metaffi_handle*)jni_metaffi_handle::wrap_in_metaffi_handle(env, val.l, (void*)jni_releaser);
		}
	}
}

DLL_PRIVATE cdt_metaffi_callable* on_construct_callable(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
{
	std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>* context_data = static_cast<std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>*>(context);
	JNIEnv* env = std::get<0>(*context_data);
	jvalue& jval = std::get<1>(*context_data);
	*is_free_required = true;

	std::pair<jvalue, char> res = jarray_wrapper::get_element(env, (jarray) jval.l, index, index_size);
	jvalue val = res.first;

	cdt_metaffi_callable* callable = new cdt_metaffi_callable{};
	fill_callable_cdt(env, val.l, callable->val, callable->parameters_types, callable->params_types_length,
	                  callable->retval_types, callable->retval_types_length);

	return callable;
}

DLL_PRIVATE metaffi::runtime::construct_cdts_callbacks jni_get_construct_cdts_callbacks(void* context)
{
	metaffi::runtime::construct_cdts_callbacks callbacks{};
	callbacks.context = context;
	callbacks.get_array_metadata = &on_construct_array_metadata;
	callbacks.construct_cdt_array = nullptr;
	callbacks.get_root_elements_count = &on_get_root_elements_count;
	callbacks.get_type_info = &on_get_type_info;
	callbacks.get_float64 = &on_construct_float64;
	callbacks.get_float32 = &on_construct_float32;
	callbacks.get_int8 = &on_construct_int8;
	callbacks.get_uint8 = &on_construct_uint8;
	callbacks.get_int16 = &on_construct_int16;
	callbacks.get_uint16 = &on_construct_uint16;
	callbacks.get_int32 = &on_construct_int32;
	callbacks.get_uint32 = &on_construct_uint32;
	callbacks.get_int64 = &on_construct_int64;
	callbacks.get_uint64 = &on_construct_uint64;
	callbacks.get_bool = &on_construct_bool;
	callbacks.get_char8 = &on_construct_char8;
	callbacks.get_string8 = &on_construct_string8;
	callbacks.get_char16 = &on_construct_char16;
	callbacks.get_string16 = &on_construct_string16;
	callbacks.get_char32 = &on_construct_char32;
	callbacks.get_string32 = &on_construct_string32;
	callbacks.get_handle = &on_construct_handle;
	callbacks.get_callable = &on_construct_callable;

	return callbacks;
}

//--------------------------------------------------------------------
cdts_java_wrapper::cdts_java_wrapper(cdts* pcdts)
{
	this->pcdts = pcdts;
}
//--------------------------------------------------------------------

jvalue cdts_java_wrapper::to_jvalue(JNIEnv* env, int index) const
{
	jvalue jval{};
	cdt& c = this->pcdts->at(index);

	auto context = std::pair<JNIEnv*, jvalue&>(env, jval);
	metaffi::runtime::traverse_cdt(c, jni_get_traverse_cdts_callback(&context));
	return jval;
}

//--------------------------------------------------------------------
void cdts_java_wrapper::from_jvalue(JNIEnv* env, jvalue jval, char jval_type, const metaffi_type_info& type, int index) const
{
	cdt& c = this->pcdts->at(index);

	auto context = std::tuple<JNIEnv*, jvalue&, char, const metaffi_type_info&>(env, jval, jval_type, type);
	metaffi::runtime::construct_cdt(c, jni_get_construct_cdts_callbacks(&context));
}

//--------------------------------------------------------------------
void cdts_java_wrapper::switch_to_object(JNIEnv* env, int i) const
{
	cdt& c = this->pcdts->at(i);

	switch(c.type)
	{
		case metaffi_int32_type: {
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_int32) v.i);
		}
		break;
		case metaffi_int64_type: {
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_int64) v.j);
		}
		break;
		case metaffi_int16_type: {
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_int16) v.s);
		}
		break;
		case metaffi_int8_type: {
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_int8) v.b);
		}
		break;
		case metaffi_float32_type: {
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_float32) v.f);
		}
		break;
		case metaffi_float64_type: {
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_float64) v.d);
		}
		break;
		case metaffi_bool_type: {
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, v.z != JNI_FALSE);
		}
		break;
	}
}

//--------------------------------------------------------------------
void cdts_java_wrapper::switch_to_primitive(JNIEnv* env, int i, metaffi_type t /*= metaffi_any_type*/) const
{
	char jni_sig = get_jni_primitive_signature_from_object_form_of_primitive(env, i);
	if(jni_sig == 0)
	{
		return;
	}

	if(jni_sig == 'L')
	{
		if(t & metaffi_array_type)
		{
			return;
		}
		else if(is_jstring(env, i))
		{
			if(t != metaffi_any_type && t != metaffi_string8_type && t != metaffi_string16_type &&
			   t != metaffi_string32_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received String";
				throw std::runtime_error(ss.str());
			}

			if(t == metaffi_any_type || t == metaffi_string8_type)
			{
				jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
				this->pcdts->set(i, std::move(cdt(jobject_wrapper(env, obj).get_as_string8())));
				env->DeleteGlobalRef(obj);
			}
			else if(t == metaffi_string16_type)
			{
				jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
				this->pcdts->set(i, std::move(cdt(jobject_wrapper(env, obj).get_as_string16())));
				env->DeleteGlobalRef(obj);
			}
			else if(t == metaffi_string32_type)
			{
				jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
				this->pcdts->set(i, std::move(cdt(jobject_wrapper(env, obj).get_as_string32())));
				env->DeleteGlobalRef(obj);
			}
			else
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received String";
				throw std::runtime_error(ss.str());
			}
		}
		else
		{
			(*this->pcdts)[i].type = metaffi_handle_type;
			return;
		}
	}

	switch(jni_sig)
	{
		case 'I': {
			if(t != metaffi_any_type && t != metaffi_int32_type && t != metaffi_uint32_type &&
			   t != metaffi_int64_type && t != metaffi_uint64_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Integer";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
			this->pcdts->set(i, std::move(cdt(jobject_wrapper(env, obj).get_as_int32())));

			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;

		case 'J': {
			if(t != metaffi_any_type && t != metaffi_int64_type && t != metaffi_uint64_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Long";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
			this->pcdts->set(i, std::move(cdt(jobject_wrapper(env, obj).get_as_int64())));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;

		case 'S': {
			if(t != metaffi_any_type && t != metaffi_int16_type && t != metaffi_uint16_type &&
			   t != metaffi_int32_type && t != metaffi_uint32_type &&
			   t != metaffi_int64_type && t != metaffi_uint64_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Short";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
			this->pcdts->set(i, std::move(cdt(jobject_wrapper(env, obj).get_as_int16())));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;

		case 'B': {
			if(t != metaffi_any_type && t != metaffi_int8_type && t != metaffi_uint8_type && t != metaffi_int16_type &&
			   t != metaffi_uint16_type && t != metaffi_int32_type && t != metaffi_uint32_type &&
			   t != metaffi_int64_type && t != metaffi_uint64_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Byte";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
			this->pcdts->set(i, std::move(cdt(jobject_wrapper(env, obj).get_as_int8())));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;

		case 'C': {
			if(t != metaffi_any_type && t != metaffi_char8_type && t != metaffi_char16_type && t != metaffi_char32_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Char";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
			if(t == metaffi_any_type || t == metaffi_char8_type)
			{
				this->pcdts->set(i, std::move(cdt((metaffi_char8) jchar_wrapper(env, obj))));
			}
			else if(t == metaffi_char16_type)
			{
				this->pcdts->set(i, std::move(cdt((metaffi_char16) jchar_wrapper(env, obj))));
			}
			else if(t == metaffi_char32_type)
			{
				this->pcdts->set(i, std::move(cdt((metaffi_char32) jchar_wrapper(env, obj))));
			}
			else
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Char";
				throw std::runtime_error(ss.str());
			}

			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;

		case 'F': {
			if(t != metaffi_any_type && t != metaffi_float32_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Float";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
			this->pcdts->set(i, std::move(cdt(jobject_wrapper(env, obj).get_as_float32())));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;

		case 'D': {
			if(t != metaffi_any_type && t != metaffi_float64_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Double";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
			this->pcdts->set(i, std::move(cdt(jobject_wrapper(env, obj).get_as_float64())));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;

		case 'Z': {
			if(t != metaffi_any_type && t != metaffi_bool_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Boolean";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject) this->pcdts->at(i).cdt_val.handle_val->handle;
			this->pcdts->set(i, std::move(cdt(jobject_wrapper(env, obj).get_as_bool())));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;
	}
}

//--------------------------------------------------------------------
bool cdts_java_wrapper::is_jstring(JNIEnv* env, int index) const
{
	if(this->pcdts->at(index).type != metaffi_handle_type)
	{
		return false;
	}

	jobject obj = (jobject) (this->pcdts->at(index).cdt_val.handle_val->handle);
	return env->IsInstanceOf(obj, env->FindClass("java/lang/String")) != JNI_FALSE;
}

//--------------------------------------------------------------------
char cdts_java_wrapper::get_jni_primitive_signature_from_object_form_of_primitive(JNIEnv* env, int index) const
{
	if(this->pcdts->at(index).type & metaffi_array_type)
	{
		return 'L';
	}

	if(this->pcdts->at(index).type != metaffi_handle_type && this->pcdts->at(index).type != metaffi_any_type)
	{
		return 0;
	}

	if(this->pcdts->at(index).type == metaffi_handle_type &&
	   this->pcdts->at(index).cdt_val.handle_val->runtime_id != JVM_RUNTIME_ID)
	{
		return 0;
	}

	jobject obj = (jobject) (this->pcdts->at(index).cdt_val.handle_val->handle);

	// Check if the object is an instance of a primitive type
	if(env->IsInstanceOf(obj, env->FindClass("java/lang/Integer")))
	{
		return 'I';
	}
	else if(env->IsInstanceOf(obj, env->FindClass("java/lang/Long")))
	{
		return 'J';
	}
	else if(env->IsInstanceOf(obj, env->FindClass("java/lang/Short")))
	{
		return 'S';
	}
	else if(env->IsInstanceOf(obj, env->FindClass("java/lang/Byte")))
	{
		return 'B';
	}
	else if(env->IsInstanceOf(obj, env->FindClass("java/lang/Character")))
	{
		return 'C';
	}
	else if(env->IsInstanceOf(obj, env->FindClass("java/lang/Float")))
	{
		return 'F';
	}
	else if(env->IsInstanceOf(obj, env->FindClass("java/lang/Double")))
	{
		return 'D';
	}
	else if(env->IsInstanceOf(obj, env->FindClass("java/lang/Boolean")))
	{
		return 'Z';
	}
	else
	{
		// If it's not a primitive type, it's an object
		return 'L';
	}
}

//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_int32 val) const
{
	jclass cls = env->FindClass("java/lang/Integer");
	check_and_throw_jvm_exception(env, true);

	jmethodID constructor = env->GetMethodID(cls, "<init>", "(I)V");
	check_and_throw_jvm_exception(env, true);

	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);

	cdt& c = this->pcdts->at(index);
	c.set_handle(new cdt_metaffi_handle{obj, JVM_RUNTIME_ID, jni_releaser});
}

//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, bool val) const
{
	jclass cls = env->FindClass("java/lang/Boolean");
	check_and_throw_jvm_exception(env, true);

	jmethodID constructor = env->GetMethodID(cls, "<init>", "(Z)V");
	check_and_throw_jvm_exception(env, true);

	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);

	//	obj = env->NewGlobalRef(obj);

	cdt& c = this->pcdts->at(index);
	c.cdt_val.handle_val = new cdt_metaffi_handle{obj, JVM_RUNTIME_ID, reinterpret_cast<void (*)(cdt_metaffi_handle*)>(jni_releaser)};
	c.type = metaffi_handle_type;
}

//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_int8 val) const
{
	jclass cls = env->FindClass("java/lang/Byte");
	check_and_throw_jvm_exception(env, true);

	jmethodID constructor = env->GetMethodID(cls, "<init>", "(B)V");
	check_and_throw_jvm_exception(env, true);

	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);

	cdt& c = this->pcdts->at(index);
	c.cdt_val.handle_val = new cdt_metaffi_handle{obj, JVM_RUNTIME_ID, reinterpret_cast<void (*)(cdt_metaffi_handle*)>(jni_releaser)};
	c.type = metaffi_handle_type;
}

//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_uint16 val) const
{
	jclass cls = env->FindClass("java/lang/Character");
	check_and_throw_jvm_exception(env, true);

	jmethodID constructor = env->GetMethodID(cls, "<init>", "(C)V");
	check_and_throw_jvm_exception(env, true);

	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);

	cdt& c = this->pcdts->at(index);
	c.cdt_val.handle_val = new cdt_metaffi_handle{obj, JVM_RUNTIME_ID, reinterpret_cast<void (*)(cdt_metaffi_handle*)>(jni_releaser)};
	c.type = metaffi_handle_type;
}

//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_int16 val) const
{
	jclass cls = env->FindClass("java/lang/Short");
	check_and_throw_jvm_exception(env, true);

	jmethodID constructor = env->GetMethodID(cls, "<init>", "(S)V");
	check_and_throw_jvm_exception(env, true);

	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);

	cdt& c = this->pcdts->at(index);
	c.cdt_val.handle_val = new cdt_metaffi_handle{obj, JVM_RUNTIME_ID, reinterpret_cast<void (*)(cdt_metaffi_handle*)>(jni_releaser)};
	c.type = metaffi_handle_type;
}

//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_int64 val) const
{
	jclass cls = env->FindClass("java/lang/Long");
	check_and_throw_jvm_exception(env, true);

	jmethodID constructor = env->GetMethodID(cls, "<init>", "(J)V");
	check_and_throw_jvm_exception(env, true);

	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);

	cdt& c = this->pcdts->at(index);
	c.cdt_val.handle_val = new cdt_metaffi_handle{obj, JVM_RUNTIME_ID, reinterpret_cast<void (*)(cdt_metaffi_handle*)>(jni_releaser)};
	c.type = metaffi_handle_type;
}

//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_float32 val) const
{
	jclass cls = env->FindClass("java/lang/Float");
	check_and_throw_jvm_exception(env, true);

	jmethodID constructor = env->GetMethodID(cls, "<init>", "(F)V");
	check_and_throw_jvm_exception(env, true);

	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);

	cdt& c = this->pcdts->at(index);
	c.cdt_val.handle_val = new cdt_metaffi_handle{obj, JVM_RUNTIME_ID, reinterpret_cast<void (*)(cdt_metaffi_handle*)>(jni_releaser)};
	c.type = metaffi_handle_type;
}

//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_float64 val) const
{
	jclass cls = env->FindClass("java/lang/Double");
	check_and_throw_jvm_exception(env, true);

	jmethodID constructor = env->GetMethodID(cls, "<init>", "(D)V");
	check_and_throw_jvm_exception(env, true);

	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);

	cdt& c = this->pcdts->at(index);
	c.cdt_val.handle_val = new cdt_metaffi_handle{obj, JVM_RUNTIME_ID, reinterpret_cast<void (*)(cdt_metaffi_handle*)>(jni_releaser)};
	c.type = metaffi_handle_type;
}

//--------------------------------------------------------------------
bool cdts_java_wrapper::is_jobject(int i) const
{
	cdt& c = this->pcdts->at(i);
	return c.type == metaffi_handle_type || c.type == metaffi_string8_type ||
	       c.type == metaffi_string16_type || (c.type & metaffi_array_type) != 0;
}

//--------------------------------------------------------------------
cdt& cdts_java_wrapper::operator[](int index) const
{
	return this->pcdts->at(index);
}

//--------------------------------------------------------------------
metaffi_size cdts_java_wrapper::length() const
{

	return this->pcdts == nullptr ? 0 : this->pcdts->length;
}

//--------------------------------------------------------------------
cdts_java_wrapper::operator cdts*() const
{
	return this->pcdts;
}
//--------------------------------------------------------------------
