#include "jobject_wrapper.h"
#include "jstring_wrapper.h"
#include "jchar_wrapper.h"

jobject_wrapper::jobject_wrapper(JNIEnv* env, jobject obj) : env(env), obj(obj)
{}

void jobject_wrapper::add_global_ref()
{
	if(env->IsSameObject(obj, env->NewGlobalRef(obj)))
	{
		obj = env->NewGlobalRef(obj);
		if(obj == nullptr)
		{
			check_and_throw_jvm_exception(env, true);
			throw std::runtime_error("Failed to add global reference");
		}
	}
}

void jobject_wrapper::delete_global_ref()
{
	if(env->IsSameObject(obj, env->NewGlobalRef(obj)))
	{
		env->DeleteGlobalRef(obj);
		check_and_throw_jvm_exception(env, true);
	}
}

int32_t jobject_wrapper::get_as_int32()
{
	int32_t result = env->CallIntMethod(obj, env->GetMethodID(env->GetObjectClass(obj), "intValue", "()I"));
	if(env->ExceptionCheck())
	{
		check_and_throw_jvm_exception(env, true);
		throw std::runtime_error("Failed to convert to int32");
	}
	return result;
}

int64_t jobject_wrapper::get_as_int64()
{
	int64_t result = env->CallLongMethod(obj, env->GetMethodID(env->GetObjectClass(obj), "longValue", "()J"));
	if(env->ExceptionCheck())
	{
		check_and_throw_jvm_exception(env, true);
		throw std::runtime_error("Failed to convert to int64");
	}
	return result;
}

int8_t jobject_wrapper::get_as_int8()
{
	int8_t result = env->CallByteMethod(obj, env->GetMethodID(env->GetObjectClass(obj), "byteValue", "()B"));
	if(env->ExceptionCheck())
	{
		check_and_throw_jvm_exception(env, true);
		throw std::runtime_error("Failed to convert to int8");
	}
	return result;
}

int16_t jobject_wrapper::get_as_int16()
{
	int16_t result = env->CallShortMethod(obj, env->GetMethodID(env->GetObjectClass(obj), "shortValue", "()S"));
	if(env->ExceptionCheck())
	{
		check_and_throw_jvm_exception(env, true);
		throw std::runtime_error("Failed to convert to int16");
	}
	return result;
}

float jobject_wrapper::get_as_float32()
{
	float result = env->CallFloatMethod(obj, env->GetMethodID(env->GetObjectClass(obj), "floatValue", "()F"));
	if(env->ExceptionCheck())
	{
		check_and_throw_jvm_exception(env, true);
		throw std::runtime_error("Failed to convert to float32");
	}
	return result;
}

double jobject_wrapper::get_as_float64()
{
	double result = env->CallDoubleMethod(obj, env->GetMethodID(env->GetObjectClass(obj), "doubleValue", "()D"));
	if(env->ExceptionCheck())
	{
		check_and_throw_jvm_exception(env, true);
		throw std::runtime_error("Failed to convert to float64");
	}
	return result;
}

const char8_t* jobject_wrapper::get_as_string8()
{
	jstring_wrapper wrapper(env, static_cast<jstring>(obj));
	const char8_t* result = (const char8_t*) wrapper;
	if(result == nullptr)
	{
		check_and_throw_jvm_exception(env, true);
		throw std::runtime_error("Failed to convert to string8");
	}
	return result;
}

const char16_t* jobject_wrapper::get_as_string16()
{
	jstring_wrapper wrapper(env, static_cast<jstring>(obj));
	const char16_t* result = (const char16_t*) wrapper;
	if(result == nullptr)
	{
		check_and_throw_jvm_exception(env, true);
		throw std::runtime_error("Failed to convert to string16");
	}
	return result;
}

const char32_t* jobject_wrapper::get_as_string32()
{
	jstring_wrapper wrapper(env, static_cast<jstring>(obj));
	const char32_t* result = (const char32_t*) wrapper;
	if(result == nullptr)
	{
		check_and_throw_jvm_exception(env, true);
		throw std::runtime_error("Failed to convert to string32");
	}
	return result;
}

bool jobject_wrapper::get_as_bool()
{
	jboolean result = env->CallBooleanMethod(obj, env->GetMethodID(env->GetObjectClass(obj), "booleanValue", "()Z"));
	if(env->ExceptionCheck())
	{
		check_and_throw_jvm_exception(env, true);
		throw std::runtime_error("Failed to convert to bool");
	}
	return result != JNI_FALSE;
}


