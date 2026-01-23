#include "jlong_wrapper.h"
#include <string>

jlong_wrapper::jlong_wrapper(jlong value) : value(value)
{}

jlong_wrapper::jlong_wrapper(const jlong_wrapper& other) : value(other.value)
{}

jlong_wrapper::operator jlong()
{
	return value;
}

jlong_wrapper& jlong_wrapper::operator=(jlong val)
{
	value = val;
	return *this;
}

jlongArray jlong_wrapper::new_1d_array(JNIEnv* env, jsize size, const jlong* parr)
{
	if(size < 0)
	{
		throw std::runtime_error("Size cannot be negative");
	}
	
	jlongArray array = env->NewLongArray(size);
	if(!array)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	if(parr != nullptr)
	{
		env->SetLongArrayRegion(array, 0, size, parr);
	}
	
	return array;
}

jlong_wrapper::jlong_wrapper(JNIEnv* p_env, jobject p_jobject)
{
	// obj holds a java.lang.Long object, convert to jlong
	// and set to value
	jclass longClass = p_env->FindClass("java/lang/Long");
	jmethodID longValue = p_env->GetMethodID(longClass, "longValue", "()J");
	value = p_env->CallLongMethod(p_jobject, longValue);
}


