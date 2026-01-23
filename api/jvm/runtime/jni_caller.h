#pragma once

#include <runtime_manager/jdk/jvm.h>


bool is_caller_class(JNIEnv* env, jobject obj);
void fill_callable_cdt(JNIEnv* env, jobject caller_obj, metaffi_callable& out_xcall_and_context, metaffi_type*& out_parameters_types_array, int8_t& out_parameters_types_array_length, metaffi_type*& out_retval_types_array, int8_t& out_retval_types_array_length);
jobject new_caller(JNIEnv* env, metaffi_callable xcall_and_context, const metaffi_type* parameters_types_array, int8_t parameters_types_array_length, const metaffi_type* retvals_types_array, int8_t retvals_types_array_length);
