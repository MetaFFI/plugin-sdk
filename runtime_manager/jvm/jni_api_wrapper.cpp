#include "jni_api_wrapper.h"

#include <filesystem>
#include <iostream>
#include <utils/env_utils.h>
#include <utils/logger.hpp>
#include <stdexcept>
#include <vector>

static auto LOG = metaffi::get_logger("jvm_runtime_manager");

namespace
{
	bool diag_enabled()
	{
		const std::string raw = get_env_var("METAFFI_JVM_DIAG");
		return !raw.empty() && raw != "0";
	}

	void diag(const std::string& msg)
	{
		if(diag_enabled())
		{
			std::cerr << "+++ " << msg << std::endl;
		}
	}
}

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

		diag(std::string("jni_api_wrapper requested_libjvm=") + libjvm_path.string());
		diag(std::string("jni_api_wrapper add_dll_dir bin=") + bin_dir.string());
		diag(std::string("jni_api_wrapper add_dll_dir server=") + server_dir.string());

		if(!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS))
		{
			const auto last_error = static_cast<unsigned long>(GetLastError());
			diag(std::string("jni_api_wrapper SetDefaultDllDirectories failed error=") + std::to_string(last_error));
		}

		auto bin_cookie = AddDllDirectory(bin_dir.c_str());
		if(!bin_cookie)
		{
			const auto last_error = static_cast<unsigned long>(GetLastError());
			diag(std::string("jni_api_wrapper AddDllDirectory(bin) failed error=") + std::to_string(last_error));
			throw std::runtime_error("Failed to add JVM bin directory to DLL search path");
		}
		diag("jni_api_wrapper AddDllDirectory(bin) succeeded");

		auto server_cookie = AddDllDirectory(server_dir.c_str());
		if(!server_cookie)
		{
			const auto last_error = static_cast<unsigned long>(GetLastError());
			diag(std::string("jni_api_wrapper AddDllDirectory(server) failed error=") + std::to_string(last_error));
			throw std::runtime_error("Failed to add JVM server directory to DLL search path");
		}
		diag("jni_api_wrapper AddDllDirectory(server) succeeded");

		HMODULE module = LoadLibraryExW(L"jvm.dll", nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
		if(!module)
		{
			const auto last_error = static_cast<unsigned long>(GetLastError());
			diag(std::string("jni_api_wrapper LoadLibraryExW(jvm.dll) failed error=") + std::to_string(last_error));
			throw std::runtime_error("Failed to load jvm.dll with configured search paths");
		}

		const auto loaded_path = get_module_path(module);
		diag(std::string("jni_api_wrapper loaded_jvm_path=") + std::filesystem::path(loaded_path).string());

		return module;
	}
}
#endif

jni_api_wrapper::jni_api_wrapper(const std::string& libjvm_path)
{
	std::lock_guard<std::mutex> lock(s_libjvm_mutex);
	diag(std::string("jni_api_wrapper ctor libjvm_path=") + libjvm_path);
#ifdef _WIN32
	if(!s_libjvm)
	{
		METAFFI_DEBUG(LOG, "jni_api_wrapper: loading libjvm: {}", libjvm_path);
		s_libjvm = load_jvm_with_search_paths(std::filesystem::path(libjvm_path));
	}
	else
	{
		diag(std::string("jni_api_wrapper reusing existing_jvm_path=") +
			std::filesystem::path(get_module_path(s_libjvm)).string());
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
	diag(std::string("jni_api_wrapper jni_symbols_loaded loaded_path=") + get_loaded_path());
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
