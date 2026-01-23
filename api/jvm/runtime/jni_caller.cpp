#include "jni_caller.h"
#include <utils/scope_guard.hpp>
#include "exception_macro.h"
#include <cstring>

jclass callercls = nullptr;

//--------------------------------------------------------------------
bool is_caller_class(JNIEnv* env, jobject obj)
{
    if(!callercls)
    {
        callercls = env->FindClass("metaffi/api/accessor/Caller");
        check_and_throw_jvm_exception(env, true);
        callercls = (jclass)env->NewGlobalRef(callercls);
    }

    return env->IsInstanceOf(obj, callercls) == JNI_TRUE;
}
//--------------------------------------------------------------------
void fill_callable_cdt(JNIEnv* env, jobject caller_obj, metaffi_callable& out_xcall_and_context, metaffi_type*& out_parameters_types_array, int8_t& out_parameters_types_array_length, metaffi_type*& out_retval_types_array, int8_t& out_retval_types_array_length)
{
    // Find the Caller class
    jclass callerClass = env->GetObjectClass(caller_obj);
    check_and_throw_jvm_exception(env, true);

    // Get field IDs
    jfieldID xcallAndContextField = env->GetFieldID(callerClass, "xcallAndContext", "J");
    check_and_throw_jvm_exception(env, true);
    jfieldID parametersTypesArrayField = env->GetFieldID(callerClass, "parametersTypesArray", "[J");
    check_and_throw_jvm_exception(env, true);
    jfieldID retvalsTypesArrayField = env->GetFieldID(callerClass, "retvalsTypesArray", "[J");
    check_and_throw_jvm_exception(env, true);

    // Get field values
    out_xcall_and_context = (void*)env->GetLongField(caller_obj, xcallAndContextField);
    check_and_throw_jvm_exception(env, true);

    jlongArray parametersTypesArray = (jlongArray)env->GetObjectField(caller_obj, parametersTypesArrayField);
    check_and_throw_jvm_exception(env, true);
    out_parameters_types_array_length = env->GetArrayLength(parametersTypesArray);
    check_and_throw_jvm_exception(env, true);
    jlong* parametersArrayElements = env->GetLongArrayElements(parametersTypesArray, nullptr);
    check_and_throw_jvm_exception(env, true);
    metaffi::utils::scope_guard sg1([&]()
    {
        env->ReleaseLongArrayElements(parametersTypesArray, parametersArrayElements, 0);
    });

    out_parameters_types_array = (metaffi_type*)new jlong[out_parameters_types_array_length];
    std::memcpy(out_parameters_types_array, parametersArrayElements, out_parameters_types_array_length * sizeof(jlong));


    jlongArray retvalsTypesArray = (jlongArray)env->GetObjectField(caller_obj, retvalsTypesArrayField);
    check_and_throw_jvm_exception(env, true);
    out_retval_types_array_length = env->GetArrayLength(retvalsTypesArray);
    check_and_throw_jvm_exception(env, true);
    jlong* retvalsArrayElements = env->GetLongArrayElements(retvalsTypesArray, nullptr);
    check_and_throw_jvm_exception(env, true);
    metaffi::utils::scope_guard sg2([&]()
    {
        env->ReleaseLongArrayElements(retvalsTypesArray, retvalsArrayElements, 0);
    });

    out_retval_types_array = (metaffi_type*)new jlong[out_retval_types_array_length];
    std::memcpy(out_retval_types_array, retvalsArrayElements, out_retval_types_array_length * sizeof(jlong));
}
//--------------------------------------------------------------------
jobject new_caller(JNIEnv* env, metaffi_callable xcall_and_context, const metaffi_type* parameters_types_array, int8_t parameters_types_array_length, const metaffi_type* retvals_types_array, int8_t retvals_types_array_length)
{
    // Find the Caller class
    jclass callerClass = env->FindClass("metaffi/api/accessor/Caller");
    check_and_throw_jvm_exception(env, true);

    // Find the createCaller method in Caller class
    jmethodID createCallerMethod = env->GetStaticMethodID(callerClass, "createCaller", "(J[J[J)Lmetaffi/api/accessor/Caller;");
    check_and_throw_jvm_exception(env, true);

    // Convert jlong* to jlongArray
    jlongArray parametersArray = env->NewLongArray(parameters_types_array_length);
    env->SetLongArrayRegion(parametersArray, 0, parameters_types_array_length, (jlong*)parameters_types_array);
    check_and_throw_jvm_exception(env, true);
    metaffi::utils::scope_guard sg1([&]()
    {
        env->DeleteLocalRef(parametersArray);
    });

    jlongArray retvalsArray = env->NewLongArray(retvals_types_array_length);
    env->SetLongArrayRegion(retvalsArray, 0, retvals_types_array_length, (jlong*)retvals_types_array);
    check_and_throw_jvm_exception(env, true);
    metaffi::utils::scope_guard sg2([&]()
    {
        env->DeleteLocalRef(retvalsArray);
    });

    // Call the createCaller method
    jobject callerObject = env->CallStaticObjectMethod(callerClass, createCallerMethod, xcall_and_context, parametersArray, retvalsArray);
    check_and_throw_jvm_exception(env, true);

    return callerObject;
}
//--------------------------------------------------------------------


