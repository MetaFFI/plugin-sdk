#include <runtime_manager/jvm/jvm.h>
#include <memory>

class jni_metaffi_handle
{
private:
	cdt_metaffi_handle value;
	static jclass metaffi_handle_class;
	static jmethodID get_handle_id;
	static jmethodID get_runtime_id_id;
	static jmethodID get_releaser_id;
	static jmethodID metaffi_handle_constructor;
	
	explicit jni_metaffi_handle(JNIEnv* env);
	
public:
	static bool is_metaffi_handle_wrapper_object(JNIEnv* env, jobject o);
	
	jni_metaffi_handle(JNIEnv* env, metaffi_handle v, uint64_t runtime_id, releaser_fptr_t releaser);
	jni_metaffi_handle(JNIEnv* env, jobject obj);
	
	[[nodiscard]] metaffi_handle get_handle() const;
	[[nodiscard]] uint64_t get_runtime_id() const;
	[[nodiscard]] void* get_releaser() const;
	
	explicit operator cdt_metaffi_handle*() const;
	
	jobject new_jvm_object(JNIEnv* env) const;
	static cdt_metaffi_handle* wrap_in_metaffi_handle(JNIEnv* p_env, jobject p_jobject, void* releaser);
};


