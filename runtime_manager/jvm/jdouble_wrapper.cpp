#include "jdouble_wrapper.h"
#include <string>

jdouble_wrapper::jdouble_wrapper(jdouble value) : value(value)
{}

jdouble_wrapper::jdouble_wrapper(const jdouble_wrapper& other) : value(other.value)
{}

jdouble_wrapper::jdouble_wrapper(JNIEnv* env, jobject obj)
{
	jclass doubleClass = env->FindClass("java/lang/Double");
	jmethodID doubleValue = env->GetMethodID(doubleClass, "doubleValue", "()D");
	value = env->CallDoubleMethod(obj, doubleValue);
}

jdouble_wrapper::operator jdouble()
{
	return value;
}

jdouble_wrapper& jdouble_wrapper::operator=(jdouble val)
{
	value = val;
	return *this;
}

jdoubleArray jdouble_wrapper::new_1d_array(JNIEnv* env, jsize size, const jdouble* parr)
{
	if(size < 0)
	{
		throw std::runtime_error("Size cannot be negative");
	}
	
	jdoubleArray array = env->NewDoubleArray(size);
	if(!array)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	if(parr != nullptr)
	{
		env->SetDoubleArrayRegion(array, 0, size, parr);
	}
	
	return array;
}



