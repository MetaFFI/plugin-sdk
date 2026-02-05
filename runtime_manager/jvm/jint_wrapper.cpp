#include "jint_wrapper.h"
#include <string>

jint_wrapper::jint_wrapper(jint value) : value(value)
{}

jint_wrapper::jint_wrapper(const jint_wrapper& other) : value(other.value)
{}

jint_wrapper::operator jint()
{
	return value;
}

jint_wrapper& jint_wrapper::operator=(jint val)
{
	value = val;
	return *this;
}

jintArray jint_wrapper::new_1d_array(JNIEnv* env, jsize size, const jint* parr)
{
	if(size < 0)
	{
		throw std::runtime_error("Size cannot be negative");
	}
	
	jintArray array = env->NewIntArray(size);
	if(!array)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	if(parr != nullptr)
	{
		env->SetIntArrayRegion(array, 0, size, parr);
	}
	
	return array;
}

jint_wrapper::jint_wrapper(JNIEnv* p_env, jobject p_jobject)
{
	// obj holds a java.lang.Integer object, convert to jint
	// and set to value
	jclass intClass = p_env->FindClass("java/lang/Integer");
	jmethodID intValue = p_env->GetMethodID(intClass, "intValue", "()I");
	value = p_env->CallIntMethod(p_jobject, intValue);
	check_and_throw_jvm_exception(p_env, true);
}


