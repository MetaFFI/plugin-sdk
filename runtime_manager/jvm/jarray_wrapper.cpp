#include "jarray_wrapper.h"
#include "jni_class.h"
#include "jni_size_utils.h"
#include <mutex>
#include <string>

namespace
{
	struct jarray_class_cache
	{
		jclass object_array = nullptr; // [Ljava/lang/Object;
		jclass bool_array = nullptr; // [Z
		jclass byte_array = nullptr; // [B
		jclass char_array = nullptr; // [C
		jclass short_array = nullptr; // [S
		jclass int_array = nullptr; // [I
		jclass long_array = nullptr; // [J
		jclass float_array = nullptr; // [F
		jclass double_array = nullptr; // [D
		jclass object_class = nullptr; // java/lang/Object
	};

	jclass cache_global_class(JNIEnv* env, const char* class_name)
	{
		jclass local = env->FindClass(class_name);
		check_and_throw_jvm_exception(env, local);

		jclass global = (jclass)env->NewGlobalRef(local);
		env->DeleteLocalRef(local);
		check_and_throw_jvm_exception(env, global);
		return global;
	}

	const jarray_class_cache& get_jarray_class_cache(JNIEnv* env)
	{
		static jarray_class_cache cache;
		static JavaVM* cached_vm = nullptr;
		static std::mutex cache_mutex;

		std::lock_guard<std::mutex> lock(cache_mutex);
		JavaVM* current_vm = nullptr;
		env->GetJavaVM(&current_vm);
		check_and_throw_jvm_exception(env, current_vm);

		if(cache.object_array && current_vm == cached_vm)
		{
			return cache;
		}

		cache = {};
		cached_vm = current_vm;

		cache.object_array = cache_global_class(env, "[Ljava/lang/Object;");
		cache.bool_array = cache_global_class(env, "[Z");
		cache.byte_array = cache_global_class(env, "[B");
		cache.char_array = cache_global_class(env, "[C");
		cache.short_array = cache_global_class(env, "[S");
		cache.int_array = cache_global_class(env, "[I");
		cache.long_array = cache_global_class(env, "[J");
		cache.float_array = cache_global_class(env, "[F");
		cache.double_array = cache_global_class(env, "[D");
		cache.object_class = cache_global_class(env, "java/lang/Object");

		return cache;
	}
}

jarray_wrapper::jarray_wrapper(JNIEnv* env, jarray array, metaffi_type t) : env(env), array(array), type(t)
{}

int jarray_wrapper::size()
{
	jsize l = env->GetArrayLength(array);
	check_and_throw_jvm_exception(env, true);
	return l;
}

std::pair<jvalue, char> jarray_wrapper::get(int index)
{
    jvalue value;
    char type_char;

    switch(type)
    {
        case metaffi_float64_type:
        {
            jdoubleArray arrayElement = reinterpret_cast<jdoubleArray>(array);
            jdouble element;
            env->GetDoubleArrayRegion(arrayElement, index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.d = element;
            type_char = 'D';
            break;
        }
        case metaffi_float32_type:
        {
            jfloatArray arrayElement = reinterpret_cast<jfloatArray>(array);
            jfloat element;
            env->GetFloatArrayRegion(arrayElement, index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.f = element;
            type_char = 'F';
            break;
        }
        case metaffi_int8_type:
        case metaffi_uint8_type:
        {
            jbyteArray arrayElement = reinterpret_cast<jbyteArray>(array);
            jbyte element;
            env->GetByteArrayRegion(arrayElement, index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.b = element;
            type_char = 'B';
            break;
        }
        case metaffi_int16_type:
        case metaffi_uint16_type:
        {
            jshortArray arrayElement = reinterpret_cast<jshortArray>(array);
            jshort element;
            env->GetShortArrayRegion(arrayElement, index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.s = element;
            type_char = 'S';
            break;
        }
        case metaffi_int32_type:
        case metaffi_uint32_type:
        {
            jintArray arrayElement = reinterpret_cast<jintArray>(array);
            jint element;
            env->GetIntArrayRegion(arrayElement, index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.i = element;
            type_char = 'I';
            break;
        }
        case metaffi_int64_type:
        case metaffi_uint64_type:
        {
            jlongArray arrayElement = reinterpret_cast<jlongArray>(array);
            jlong element;
            env->GetLongArrayRegion(arrayElement, index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.j = element;
            type_char = 'J';
            break;
        }
        case metaffi_bool_type:
        {
            jbooleanArray arrayElement = reinterpret_cast<jbooleanArray>(array);
            jboolean element;
            env->GetBooleanArrayRegion(arrayElement, index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.z = element;
            type_char = 'Z';
            break;
        }
        case metaffi_char8_type:
        case metaffi_char16_type:
        case metaffi_char32_type:
        {
            jcharArray arrayElement = reinterpret_cast<jcharArray>(array);
            jchar element;
            env->GetCharArrayRegion(arrayElement, index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.c = element;
            type_char = 'C';
            break;
        }
        case metaffi_string8_type:
        {
            jobjectArray arrayElement = reinterpret_cast<jobjectArray>(array);
            value.l = env->GetObjectArrayElement(arrayElement, index);
            type_char = 'L';
            break;
        }
		
		case metaffi_array_type:
        case metaffi_handle_type:
        case metaffi_any_type:
        {
            jobjectArray arrayElement = reinterpret_cast<jobjectArray>(array);
            value.l = env->GetObjectArrayElement(arrayElement, index);
            type_char = 'L';
            break;
        }
        default:
        {
            throw std::invalid_argument("Unsupported metaffi type");
        }
    }

    return std::pair(value, type_char);
}

void jarray_wrapper::set(int index, jvalue value)
{
    switch(type)
    {
        case metaffi_float64_type:
        {
            jdoubleArray arrayElement = reinterpret_cast<jdoubleArray>(array);
            env->SetDoubleArrayRegion(arrayElement, index, 1, &value.d);
            break;
        }
        case metaffi_float32_type:
        {
            jfloatArray arrayElement = reinterpret_cast<jfloatArray>(array);
            env->SetFloatArrayRegion(arrayElement, index, 1, &value.f);
            break;
        }
        case metaffi_int8_type:
        case metaffi_uint8_type:
        {
            jbyteArray arrayElement = reinterpret_cast<jbyteArray>(array);
            env->SetByteArrayRegion(arrayElement, index, 1, &value.b);
            break;
        }
        case metaffi_int16_type:
        case metaffi_uint16_type:
        {
            jshortArray arrayElement = reinterpret_cast<jshortArray>(array);
            env->SetShortArrayRegion(arrayElement, index, 1, &value.s);
            break;
        }
        case metaffi_int32_type:
        case metaffi_uint32_type:
        {
            jintArray arrayElement = reinterpret_cast<jintArray>(array);
            env->SetIntArrayRegion(arrayElement, index, 1, &value.i);
            break;
        }
        case metaffi_int64_type:
        case metaffi_uint64_type:
        {
            jlongArray arrayElement = reinterpret_cast<jlongArray>(array);
            env->SetLongArrayRegion(arrayElement, index, 1, &value.j);
            break;
        }
        case metaffi_bool_type:
        {
            jbooleanArray arrayElement = reinterpret_cast<jbooleanArray>(array);
            env->SetBooleanArrayRegion(arrayElement, index, 1, &value.z);
            break;
        }
        case metaffi_char8_type:
        case metaffi_char16_type:
        case metaffi_char32_type:
        {
            jcharArray arrayElement = reinterpret_cast<jcharArray>(array);
            env->SetCharArrayRegion(arrayElement, index, 1, &value.c);
            break;
        }
        case metaffi_string8_type:
        case metaffi_handle_type:
        case metaffi_any_type:
		case metaffi_array_type:
        {
            jobjectArray arrayElement = reinterpret_cast<jobjectArray>(array);
            env->SetObjectArrayElement(arrayElement, index, value.l);
            break;
        }
        default:
        {
            throw std::invalid_argument("Unsupported metaffi_type");
        }
    }

    check_and_throw_jvm_exception(env, true);
}

jobjectArray jarray_wrapper::create_object_array(JNIEnv* env, const char* class_name, int size, int dimensions)
{
	std::string class_str(dimensions - 1, '[');
	class_str += class_name;
	
	jclass cls = env->FindClass(class_str.c_str());
	if(!cls)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	jobjectArray arr = env->NewObjectArray(size, cls, nullptr);
	if(!arr)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	return arr;
}

jarray_wrapper::operator jobject()
{
	return static_cast<jobject>(array);
}

jarray_wrapper::operator jarray()
{
	return array;
}

std::pair<metaffi_type_info, jint> jarray_wrapper::get_array_info(JNIEnv* env, jarray array, const metaffi_type_info& root_info)
{
	metaffi_type_info tinfo;
	
	std::string clsname = jni_class::get_object_class_name(env, array);
	tinfo.fixed_dimensions = std::count(clsname.begin(), clsname.end(), '[');
	
	jint length = env->GetArrayLength(array);
	
	if(clsname.ends_with("[Z") || clsname.ends_with("[Ljava/lang/Boolean;"))
	{
		tinfo.type = metaffi_bool_array_type | metaffi_array_type;
	}
	else if(clsname.ends_with("[B") || clsname.ends_with("[Ljava/lang/Byte;"))
	{
		tinfo.type = root_info.type & metaffi_uint8_type ? metaffi_uint8_array_type | metaffi_array_type : metaffi_int8_array_type | metaffi_array_type;
	}
	else if(clsname.ends_with("[C") || clsname.ends_with("[Ljava/lang/Character;"))
	{
		tinfo.type = metaffi_char8_type | metaffi_array_type;
	}
	else if(clsname.ends_with("[S") || clsname.ends_with("[Ljava/lang/Short;"))
	{
		tinfo.type = root_info.type & metaffi_uint16_type ? metaffi_uint16_array_type | metaffi_array_type : metaffi_int16_array_type | metaffi_array_type;
	}
	else if(clsname.ends_with("[I") || clsname.ends_with("[Ljava/lang/Integer;"))
	{
		tinfo.type = root_info.type & metaffi_uint32_type ? metaffi_uint32_array_type | metaffi_array_type : metaffi_int32_array_type | metaffi_array_type;
	}
	else if(clsname.ends_with("[J") || clsname.ends_with("[Ljava/lang/Long;"))
	{
		tinfo.type = root_info.type & metaffi_uint64_type ? metaffi_uint64_array_type | metaffi_array_type : metaffi_int64_array_type | metaffi_array_type;
	}
	else if(clsname.ends_with("[F") || clsname.ends_with("[Ljava/lang/Float;"))
	{
		tinfo.type = metaffi_float32_type | metaffi_array_type;
	}
	else if(clsname.ends_with("[D") || clsname.ends_with("[Ljava/lang/Double;"))
	{
		tinfo.type = metaffi_float64_type | metaffi_array_type;
	}
	else if(clsname.ends_with("[Ljava.lang.String;"))
	{
		tinfo.type = metaffi_string8_type | metaffi_array_type;
	}
	else if(clsname.ends_with("[metaffi/api/accessor/Caller"))
	{
		tinfo.type = metaffi_callable_type | metaffi_array_type;
		tinfo.set_copy_alias(clsname.c_str(), (int)clsname.size());
	}
	else
	{
		tinfo.type = metaffi_handle_type | metaffi_array_type;
		tinfo.set_copy_alias(clsname.c_str(), (int)clsname.size());
	}
	
	return std::make_pair(tinfo, length);
}

std::pair<jvalue, char> jarray_wrapper::get_element(JNIEnv* env, jarray obj, const metaffi_size* index, metaffi_size index_length)
{
	if(index_length == 0){
		return {jvalue{.l = obj}, 'L'};
	}

	const auto& cls_cache = get_jarray_class_cache(env);
	
    jarray current_array = (jarray)obj;
    jvalue value;
    char type_char;

    for(metaffi_size i = 0; i < index_length; ++i)
    {
		jsize element_index = to_jsize(index[i]);
        if(env->IsInstanceOf(current_array, cls_cache.object_array))
        {
            current_array = (jarray)env->GetObjectArrayElement((jobjectArray) current_array, element_index);
			value.l = current_array;
			type_char = 'L';
        }
        else if(env->IsInstanceOf(current_array, cls_cache.bool_array))
        {
            jboolean element;
            env->GetBooleanArrayRegion((jbooleanArray) current_array, element_index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.z = element;
            type_char = 'Z';
        }
        else if(env->IsInstanceOf(current_array, cls_cache.byte_array))
        {
            jbyte element;
            env->GetByteArrayRegion((jbyteArray) current_array, element_index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.b = element;
            type_char = 'B';
        }
        else if(env->IsInstanceOf(current_array, cls_cache.char_array))
        {
            jchar element;
            env->GetCharArrayRegion((jcharArray) current_array, element_index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.c = element;
            type_char = 'C';
        }
        else if(env->IsInstanceOf(current_array, cls_cache.short_array))
        {
            jshort element;
            env->GetShortArrayRegion((jshortArray) current_array, element_index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.s = element;
            type_char = 'S';
        }
        else if(env->IsInstanceOf(current_array, cls_cache.int_array))
        {
            jint element;
            env->GetIntArrayRegion((jintArray) current_array, element_index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.i = element;
			type_char = 'I';
        }
        else if(env->IsInstanceOf(current_array, cls_cache.long_array))
        {
            jlong element;
            env->GetLongArrayRegion((jlongArray) current_array, element_index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.j = element;
            type_char = 'J';
        }
        else if(env->IsInstanceOf(current_array, cls_cache.float_array))
        {
            jfloat element;
            env->GetFloatArrayRegion((jfloatArray) current_array, element_index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.f = element;
            type_char = 'F';
        }
        else if(env->IsInstanceOf(current_array, cls_cache.double_array))
        {
            jdouble element;
            env->GetDoubleArrayRegion((jdoubleArray) current_array, element_index, 1, &element);
            check_and_throw_jvm_exception(env, true);
            value.d = element;
            type_char = 'D';
        }
        else
        {
            throw std::invalid_argument("Unsupported array type");
        }
    }
    return std::make_pair(value, type_char);
}

jarray jarray_wrapper::create_jni_array(JNIEnv* env, metaffi_type t, metaffi_int64 fixed_dimensions, metaffi_size length)
{
	jsize jlength = to_jsize(length);
	if(fixed_dimensions == -1)
	{
		const auto& cls_cache = get_jarray_class_cache(env);
		return env->NewObjectArray(jlength, cls_cache.object_class, nullptr);
	}
	
	std::string arraySignature(fixed_dimensions-1, '[');
	
	switch(t)
	{
		case metaffi_float64_type:
			if(fixed_dimensions == 1)
			{
				return env->NewDoubleArray(jlength);
			}
			arraySignature += "D";
			break;
		case metaffi_float32_type:
			if(fixed_dimensions == 1)
			{
				return env->NewFloatArray(jlength);
			}
			arraySignature += "F";
			break;
		case metaffi_int8_type:
		case metaffi_uint8_type:
			if(fixed_dimensions == 1)
			{
				return env->NewByteArray(jlength);
			}
			arraySignature += "B";
			break;
		case metaffi_int16_type:
		case metaffi_uint16_type:
			if(fixed_dimensions == 1)
			{
				return env->NewShortArray(jlength);
			}
			arraySignature += "S";
			break;
		case metaffi_int32_type:
		case metaffi_uint32_type:
			if(fixed_dimensions == 1)
			{
				return env->NewIntArray(jlength);
			}
			arraySignature += "I";
			break;
		case metaffi_int64_type:
		case metaffi_uint64_type:
			if(fixed_dimensions == 1)
			{
				return env->NewLongArray(jlength);
			}
			arraySignature += "J";
			break;
		case metaffi_bool_type:
			if(fixed_dimensions == 1)
			{
				return env->NewBooleanArray(jlength);
			}
			arraySignature += "Z";
			break;
		case metaffi_char8_type:
		case metaffi_char16_type:
		case metaffi_char32_type:
			if(fixed_dimensions == 1)
			{
				return env->NewCharArray(jlength);
			}
			arraySignature += "C";
			break;
		case metaffi_string8_type:
			arraySignature += "Ljava/lang/String;";
			break;
		case metaffi_handle_type:
		case metaffi_any_type:
			arraySignature += "Ljava/lang/Object;";
			break;
		case metaffi_callable_type:
			arraySignature += "Lmetaffi/api/accessor/Caller;";
			break;
		default:
			throw std::invalid_argument("Unsupported metaffi_type");
	}
	
	jclass arrayClass = env->FindClass(arraySignature.c_str());
	return env->NewObjectArray(jlength, arrayClass, nullptr);
}

bool jarray_wrapper::is_array(JNIEnv* env, jobject obj)
{
	std::string cls = jni_class::get_object_class_name(env, obj);
	return cls[0] == '[';
}
