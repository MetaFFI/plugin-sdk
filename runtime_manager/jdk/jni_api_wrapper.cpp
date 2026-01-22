#include "jni_api_wrapper.h"

#include <stdexcept>
#include <iostream>

jni_api_wrapper::jni_api_wrapper(const std::string& libjvm_path)
{
	m_libjvm = std::make_shared<boost::dll::shared_library>();
	#ifdef _WIN32
	auto load_mode = boost::dll::load_mode::default_mode;
	#else
	auto load_mode = boost::dll::load_mode::rtld_now | boost::dll::load_mode::rtld_global;
	#endif
	std::cerr << "[jni_api_wrapper] loading libjvm: " << libjvm_path << std::endl;
	m_libjvm->load(libjvm_path, load_mode);

	if(!m_libjvm->has("JNI_CreateJavaVM"))
	{
		throw std::runtime_error("JNI_CreateJavaVM not found in libjvm");
	}
	if(!m_libjvm->has("JNI_GetCreatedJavaVMs"))
	{
		throw std::runtime_error("JNI_GetCreatedJavaVMs not found in libjvm");
	}

	create_java_vm = m_libjvm->get<JNI_CreateJavaVM_t>("JNI_CreateJavaVM");
	get_created_java_vms = m_libjvm->get<JNI_GetCreatedJavaVMs_t>("JNI_GetCreatedJavaVMs");
	std::cerr << "[jni_api_wrapper] JNI symbols loaded" << std::endl;
}
