#pragma once
#include <runtime_manager/jdk/jvm.h>

#define check_and_throw_jvm_exception(env, var) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = jvm::get_exception_description(env, env->ExceptionOccurred());\
throw std::runtime_error(err_msg);\
}\
else if(!var)\
{\
throw std::runtime_error("Failed to get " #var);\
}

