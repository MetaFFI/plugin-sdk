#include <jni.h>
#include <stdexcept>
#include "exception_macro.h"

class jobject_wrapper
{
	JNIEnv* env;
	jobject obj;

public:
	jobject_wrapper(JNIEnv* env, jobject obj);
	
	void add_global_ref();
	void delete_global_ref();
	
	int32_t get_as_int32();
	int64_t get_as_int64();
	int8_t get_as_int8();
	int16_t get_as_int16();
	float get_as_float32();
	double get_as_float64();
	bool get_as_bool();
	
	const char8_t* get_as_string8();
	const char16_t* get_as_string16();
	const char32_t* get_as_string32();
	
	
};
