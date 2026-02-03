#include "jni_api_wrapper.h"

#include <filesystem>
#include <utils/logger.hpp>
#include <stdexcept>
#include <vector>

static auto LOG = metaffi::get_logger("jdk.runtime_manager");

#ifdef _WIN32
HMODULE jni_api_wrapper::s_libjvm = nullptr;
#else
std::shared_ptr<boost::dll::shared_library> jni_api_wrapper::s_libjvm = nullptr;
#endif
std::mutex jni_api_wrapper::s_libjvm_mutex;

#ifdef _WIN32
namespace
{
	std::wstring get_module_path(HMODULE module)
	{
		std::vector<wchar_t> buffer(MAX_PATH);
		while(true)
		{
			DWORD len = GetModuleFileNameW(module, buffer.data(), static_cast<DWORD>(buffer.size()));
			if(len == 0)
			{
				return L"";
			}

			if(len < buffer.size() - 1)
			{
				return std::wstring(buffer.data(), len);
			}

			buffer.resize(buffer.size() * 2);
		}
	}

	HMODULE load_jvm_with_search_paths(const std::filesystem::path& libjvm_path)
	{
		auto server_dir = libjvm_path.parent_path();
		auto bin_dir = server_dir.parent_path();

		SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

		auto bin_cookie = AddDllDirectory(bin_dir.c_str());
		if(!bin_cookie)
		{
			throw std::runtime_error("Failed to add JVM bin directory to DLL search path");
		}

		auto server_cookie = AddDllDirectory(server_dir.c_str());
		if(!server_cookie)
		{
			throw std::runtime_error("Failed to add JVM server directory to DLL search path");
		}

		HMODULE module = LoadLibraryExW(L"jvm.dll", nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
		if(!module)
		{
			throw std::runtime_error("Failed to load jvm.dll with configured search paths");
		}

		return module;
	}
}
#endif

jni_api_wrapper::jni_api_wrapper(const std::string& libjvm_path)
{
	std::lock_guard<std::mutex> lock(s_libjvm_mutex);
#ifdef _WIN32
	if(!s_libjvm)
	{
		METAFFI_DEBUG(LOG, "jni_api_wrapper: loading libjvm: {}", libjvm_path);
		s_libjvm = load_jvm_with_search_paths(std::filesystem::path(libjvm_path));
	}

	m_libjvm = s_libjvm;
#else
	if(!s_libjvm || !s_libjvm->is_loaded())
	{
		s_libjvm = std::make_shared<boost::dll::shared_library>();
#ifdef _WIN32
		auto load_mode = boost::dll::load_mode::load_with_altered_search_path;
#else
		auto load_mode = boost::dll::load_mode::rtld_now | boost::dll::load_mode::rtld_global;
#endif
		METAFFI_DEBUG(LOG, "jni_api_wrapper: loading libjvm: {}", libjvm_path);
		s_libjvm->load(libjvm_path, load_mode);
	}

	// Keep libjvm loaded for the process lifetime (JVM doesn't support unload).
	m_libjvm = s_libjvm;
#endif

#ifdef _WIN32
	create_java_vm = reinterpret_cast<JNI_CreateJavaVM_t>(GetProcAddress(m_libjvm, "JNI_CreateJavaVM"));
	if(!create_java_vm)
	{
		throw std::runtime_error("JNI_CreateJavaVM not found in libjvm");
	}

	get_default_java_vm_init_args = reinterpret_cast<JNI_GetDefaultJavaVMInitArgs_t>(
		GetProcAddress(m_libjvm, "JNI_GetDefaultJavaVMInitArgs"));
	if(!get_default_java_vm_init_args)
	{
		throw std::runtime_error("JNI_GetDefaultJavaVMInitArgs not found in libjvm");
	}

	get_created_java_vms = reinterpret_cast<JNI_GetCreatedJavaVMs_t>(
		GetProcAddress(m_libjvm, "JNI_GetCreatedJavaVMs"));
	if(!get_created_java_vms)
	{
		throw std::runtime_error("JNI_GetCreatedJavaVMs not found in libjvm");
	}
#else
	if(!m_libjvm->has("JNI_CreateJavaVM"))
	{
		throw std::runtime_error("JNI_CreateJavaVM not found in libjvm");
	}
	if(!m_libjvm->has("JNI_GetDefaultJavaVMInitArgs"))
	{
		throw std::runtime_error("JNI_GetDefaultJavaVMInitArgs not found in libjvm");
	}
	if(!m_libjvm->has("JNI_GetCreatedJavaVMs"))
	{
		throw std::runtime_error("JNI_GetCreatedJavaVMs not found in libjvm");
	}

	create_java_vm = m_libjvm->get<JNI_CreateJavaVM_t>("JNI_CreateJavaVM");
	get_default_java_vm_init_args = m_libjvm->get<JNI_GetDefaultJavaVMInitArgs_t>("JNI_GetDefaultJavaVMInitArgs");
	get_created_java_vms = m_libjvm->get<JNI_GetCreatedJavaVMs_t>("JNI_GetCreatedJavaVMs");
#endif
	METAFFI_DEBUG(LOG, "jni_api_wrapper: JNI symbols loaded");
}

std::string jni_api_wrapper::get_loaded_path() const
{
#ifdef _WIN32
	if(!m_libjvm)
	{
		return {};
	}

	auto path = get_module_path(m_libjvm);
	if(path.empty())
	{
		return {};
	}

	return std::filesystem::path(path).string();
#else
	if(!m_libjvm)
	{
		return {};
	}

	return m_libjvm->location().string();
#endif
}
