#include "jvm.h"
#include "jni_api_wrapper.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <mutex>
#ifdef _WIN32
#include <windows.h>
#include <winver.h>
#pragma comment(lib, "Version.lib")
#endif
#include <utils/scope_guard.hpp>
#include <utils/env_utils.h>


using namespace metaffi::utils;

namespace
{
	std::mutex s_jvm_mutex;
	JavaVM* s_shared_jvm = nullptr;

	void set_env_var(const char* key, const std::string& value)
	{
#ifdef _WIN32
		_putenv_s(key, value.c_str());
#else
		setenv(key, value.c_str(), 1);
#endif
	}

#ifdef _WIN32
	void prepare_windows_jvm(const jvm_installed_info& info)
	{
		if(info.home.empty())
		{
			return;
		}

		std::filesystem::path bin_path = std::filesystem::path(info.home) / "bin";
		std::filesystem::path server_path = bin_path / "server";

		std::string existing_path = get_env_var("PATH");
		std::string prefix = bin_path.string() + ";" + server_path.string();
		std::string new_path = prefix + (existing_path.empty() ? "" : ";" + existing_path);
		set_env_var("PATH", new_path);
		set_env_var("JAVA_HOME", info.home);
		set_env_var("JDK_HOME", info.home);

		// Rely on PATH and per-load search path to resolve JVM dependencies.
	}
#endif
}

namespace
{
	std::string read_release_value(const std::filesystem::path& java_home, const std::string& key)
	{
		std::filesystem::path release_path = java_home / "release";
		std::ifstream file(release_path);
		if(!file.is_open())
		{
			return "";
		}

		std::string line;
		while(std::getline(file, line))
		{
			if(line.rfind(key + "=", 0) == 0)
			{
				auto value = line.substr(key.size() + 1);
				if(value.size() >= 2 && value.front() == '"' && value.back() == '"')
				{
					value = value.substr(1, value.size() - 2);
				}
				return value;
			}
		}

		return "";
	}

	std::vector<int> parse_version_components(const std::string& version)
	{
		std::vector<int> parts;
		int current = -1;
		for(char ch : version)
		{
			if(std::isdigit(static_cast<unsigned char>(ch)))
			{
				int digit = ch - '0';
				if(current < 0)
				{
					current = digit;
				}
				else
				{
					current = current * 10 + digit;
				}
			}
			else if(current >= 0)
			{
				parts.push_back(current);
				current = -1;
			}
		}
		if(current >= 0)
		{
			parts.push_back(current);
		}

		return parts;
	}

	bool version_prefix_matches(const std::string& requested, const std::string& loaded)
	{
		auto requested_parts = parse_version_components(requested);
		auto loaded_parts = parse_version_components(loaded);
		if(requested_parts.empty() || loaded_parts.size() < requested_parts.size())
		{
			return false;
		}

		for(size_t i = 0; i < requested_parts.size(); ++i)
		{
			if(requested_parts[i] != loaded_parts[i])
			{
				return false;
			}
		}

		return true;
	}

#ifdef _WIN32
	std::string get_windows_file_version(const std::filesystem::path& dll_path)
	{
		std::wstring dll_path_w = dll_path.wstring();
		DWORD handle = 0;
		DWORD size = GetFileVersionInfoSizeW(dll_path_w.c_str(), &handle);
		if(size == 0)
		{
			return "";
		}

		std::vector<BYTE> buffer(size);
		if(!GetFileVersionInfoW(dll_path_w.c_str(), 0, size, buffer.data()))
		{
			return "";
		}

		VS_FIXEDFILEINFO* info = nullptr;
		UINT info_len = 0;
		if(!VerQueryValueW(buffer.data(), L"\\", reinterpret_cast<void**>(&info), &info_len) ||
		   info_len < sizeof(VS_FIXEDFILEINFO))
		{
			return "";
		}

		std::ostringstream oss;
		oss << HIWORD(info->dwFileVersionMS) << "."
		    << LOWORD(info->dwFileVersionMS) << "."
		    << HIWORD(info->dwFileVersionLS) << "."
		    << LOWORD(info->dwFileVersionLS);
		return oss.str();
	}
#endif

	bool paths_equal(const std::filesystem::path& left, const std::filesystem::path& right)
	{
		std::error_code ec;
		auto left_canon = std::filesystem::weakly_canonical(left, ec);
		if(ec)
		{
			left_canon = left;
		}
		ec.clear();
		auto right_canon = std::filesystem::weakly_canonical(right, ec);
		if(ec)
		{
			right_canon = right;
		}

#ifdef _WIN32
		auto left_str = left_canon.wstring();
		auto right_str = right_canon.wstring();
		std::transform(left_str.begin(), left_str.end(), left_str.begin(), ::towlower);
		std::transform(right_str.begin(), right_str.end(), right_str.begin(), ::towlower);
		return left_str == right_str;
#else
		return left_canon == right_canon;
#endif
	}

	std::filesystem::path find_java_home_from_libjvm(const std::filesystem::path& libjvm_path)
	{
		std::filesystem::path current = libjvm_path;
		if(current.has_filename())
		{
			current = current.parent_path();
		}

		for(int i = 0; i < 6; i++)
		{
			if(std::filesystem::exists(current / "release"))
			{
				return current;
			}

			if(!current.has_parent_path())
			{
				break;
			}
			current = current.parent_path();
		}

		return {};
	}

	void verify_loaded_jvm(const jvm_installed_info& requested, const std::string& loaded_path)
	{
		if(loaded_path.empty())
		{
			return;
		}

		if(!requested.libjvm_path.empty())
		{
			if(!paths_equal(requested.libjvm_path, loaded_path))
			{
				std::ostringstream oss;
				oss << "Loaded libjvm path mismatch. Requested: " << requested.libjvm_path
				    << " Loaded: " << loaded_path;
				throw std::runtime_error(oss.str());
			}
		}

		auto loaded_home = find_java_home_from_libjvm(loaded_path);
		auto requested_home = requested.home.empty() ? find_java_home_from_libjvm(requested.libjvm_path) : std::filesystem::path(requested.home);

		std::string requested_version = requested.version;
		if(requested_version.empty() && !requested_home.empty())
		{
			requested_version = read_release_value(requested_home, "JAVA_VERSION");
			if(requested_version.empty())
			{
				requested_version = read_release_value(requested_home, "JAVA_RUNTIME_VERSION");
			}
		}

#ifdef _WIN32
		if(requested_version.empty())
		{
			throw std::runtime_error("Unable to determine requested JVM version for verification");
		}

		std::string loaded_version = get_windows_file_version(loaded_path);
		if(loaded_version.empty())
		{
			throw std::runtime_error("Unable to determine loaded JVM version from file metadata");
		}

		if(!version_prefix_matches(requested_version, loaded_version))
		{
			std::ostringstream oss;
			oss << "Loaded libjvm version mismatch. Requested: " << requested_version
			    << " Loaded: " << loaded_version;
			throw std::runtime_error(oss.str());
		}
#else
		std::string loaded_version;
		if(!loaded_home.empty())
		{
			loaded_version = read_release_value(loaded_home, "JAVA_VERSION");
			if(loaded_version.empty())
			{
				loaded_version = read_release_value(loaded_home, "JAVA_RUNTIME_VERSION");
			}
		}

		if(!loaded_version.empty() && !requested_version.empty() &&
		   !version_prefix_matches(requested_version, loaded_version))
		{
			std::ostringstream oss;
			oss << "Loaded libjvm version mismatch. Requested: " << requested_version
			    << " Loaded: " << loaded_version;
			throw std::runtime_error(oss.str());
		}
#endif
	}

	std::string find_libjvm_in_home(const std::filesystem::path& java_home)
	{
#ifdef _WIN32
		std::filesystem::path dll_path = java_home / "bin" / "server" / "jvm.dll";
		if(std::filesystem::exists(dll_path))
		{
			return dll_path.string();
		}
		dll_path = java_home / "jre" / "bin" / "server" / "jvm.dll";
		if(std::filesystem::exists(dll_path))
		{
			return dll_path.string();
		}
#else
		std::filesystem::path so_path = java_home / "lib" / "server" / "libjvm.so";
		if(std::filesystem::exists(so_path))
		{
			return so_path.string();
		}
		std::filesystem::path dylib_path = java_home / "lib" / "server" / "libjvm.dylib";
		if(std::filesystem::exists(dylib_path))
		{
			return dylib_path.string();
		}
#endif

		return "";
	}

	bool is_valid_jvm_home(const std::filesystem::path& home)
	{
		if(std::filesystem::exists(home / "release"))
		{
			return true;
		}

		return !find_libjvm_in_home(home).empty();
	}

	std::vector<std::filesystem::path> find_java_from_path()
	{
		std::vector<std::filesystem::path> result;

		std::string path_env = get_env_var("PATH");
		if(path_env.empty())
		{
			return result;
		}

#ifdef _WIN32
		const char pathSeparator = ';';
		const std::vector<std::string> javaNames = {"java.exe"};
#else
		const char pathSeparator = ':';
		const std::vector<std::string> javaNames = {"java"};
#endif

		std::istringstream pathStream(path_env);
		std::string pathEntry;
		while(std::getline(pathStream, pathEntry, pathSeparator))
		{
			if(pathEntry.empty())
			{
				continue;
			}

			std::error_code ec;
			std::filesystem::path dir(pathEntry);
			if(!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec))
			{
				continue;
			}

			for(const auto& javaName : javaNames)
			{
				std::filesystem::path javaPath = dir / javaName;
				if(std::filesystem::exists(javaPath, ec))
				{
					result.push_back(javaPath);
				}
			}
		}

		return result;
	}

	std::filesystem::path resolve_jvm_home_from_executable(const std::filesystem::path& javaExe)
	{
		std::filesystem::path resolved = javaExe;
		std::error_code ec;

#ifndef _WIN32
		int maxIterations = 20;
		while(std::filesystem::is_symlink(resolved, ec) && !ec && maxIterations-- > 0)
		{
			auto target = std::filesystem::read_symlink(resolved, ec);
			if(ec)
			{
				break;
			}

			if(target.is_relative())
			{
				target = resolved.parent_path() / target;
			}

			resolved = std::filesystem::canonical(target, ec);
			if(ec)
			{
				resolved = std::filesystem::weakly_canonical(target, ec);
				if(ec)
				{
					break;
				}
			}
		}
#endif

		if(resolved.has_parent_path())
		{
			std::filesystem::path binDir = resolved.parent_path();
			if(binDir.filename() == "bin")
			{
				std::filesystem::path home = binDir.parent_path();
				if(is_valid_jvm_home(home))
				{
					return home;
				}
			}
		}

		return {};
	}

	void add_candidate_if_valid(std::vector<jvm_installed_info>& out,
	                            std::unordered_set<std::string>& seen,
	                            const std::filesystem::path& javaHome)
	{
		if(!std::filesystem::exists(javaHome))
		{
			return;
		}

		std::error_code ec;
		std::filesystem::path canonical = std::filesystem::canonical(javaHome, ec);
		if(ec)
		{
			canonical = std::filesystem::weakly_canonical(javaHome, ec);
			if(ec)
			{
				canonical = javaHome;
			}
		}

		std::string canonicalStr = canonical.string();
		if(seen.find(canonicalStr) != seen.end())
		{
			return;
		}

		auto libjvm = find_libjvm_in_home(javaHome);
		if(libjvm.empty())
		{
			return;
		}

		seen.insert(canonicalStr);

		jvm_installed_info info;
		info.home = javaHome.string();
		info.libjvm_path = libjvm;
		info.version = read_release_value(javaHome, "JAVA_VERSION");
		if(info.version.empty())
		{
			info.version = read_release_value(javaHome, "JAVA_RUNTIME_VERSION");
		}
		info.vendor = jvm_vendor::unknown;

		out.push_back(info);
	}

	std::vector<jvm_installed_info> detect_installed_jvms()
	{
		std::vector<jvm_installed_info> result;
		std::unordered_set<std::string> seen;

		std::string java_home = get_env_var("JAVA_HOME");
		if(!java_home.empty())
		{
			add_candidate_if_valid(result, seen, java_home);
		}

		std::string jdk_home = get_env_var("JDK_HOME");
		if(!jdk_home.empty())
		{
			add_candidate_if_valid(result, seen, jdk_home);
		}

		for(const auto& javaPath : find_java_from_path())
		{
			auto jvmHome = resolve_jvm_home_from_executable(javaPath);
			if(!jvmHome.empty())
			{
				add_candidate_if_valid(result, seen, jvmHome);
			}
		}

		return result;
	}

	jvm_installed_info select_default_jvm()
	{
		auto jvms = detect_installed_jvms();
		if(jvms.empty())
		{
			throw std::runtime_error("No installed JVMs detected");
		}

		auto prefer_version = [](const std::string& version, const std::string& prefix){
			return version.rfind(prefix, 0) == 0;
		};

		const std::vector<std::string> preferred_versions = {"22", "21", "17", "11"};
		for(const auto& version_prefix : preferred_versions)
		{
			for(const auto& info : jvms)
			{
				if(prefer_version(info.version, version_prefix))
				{
					return info;
				}
			}
		}

		return jvms.front();
	}
}

//--------------------------------------------------------------------
jvm::jvm()
{
#ifdef _WIN32
#define SEPARATOR ';'
#else
#define SEPARATOR ':'
#endif

	m_info = select_default_jvm();

	std::string metaffi_home = get_env_var("METAFFI_HOME");
	if(metaffi_home.empty())
	{
		throw std::runtime_error("METAFFI_HOME environment variable is not set");
	}

	std::stringstream ss;
	ss << "-Djava.class.path=." << SEPARATOR << ".." << SEPARATOR << metaffi_home << "/sdk/api/jvm/metaffi.api.jar";
	std::string classpath = get_env_var("CLASSPATH");
	if(!classpath.empty())
	{
		ss << SEPARATOR << classpath;
	}

	load_or_create_with_info(ss.str());
}
//--------------------------------------------------------------------
jvm::jvm(const jvm_installed_info& info, const std::string& classpath_option)
	: m_info(info)
{
	load_or_create_with_info(classpath_option);
}
//--------------------------------------------------------------------
void jvm::load_or_create_with_info(const std::string& classpath_option)
{
	if(m_info.libjvm_path.empty())
	{
		throw std::runtime_error("libjvm_path is empty in jvm_installed_info");
	}

#ifdef _WIN32
	prepare_windows_jvm(m_info);
#else
	if(!m_info.home.empty())
	{
		set_env_var("JAVA_HOME", m_info.home);
		set_env_var("JDK_HOME", m_info.home);
	}
#endif

	m_jni_api = std::make_shared<jni_api_wrapper>(m_info.libjvm_path);
	std::cerr << "[jvm] Verifying loaded JVM" << std::endl;
	verify_loaded_jvm(m_info, m_jni_api->get_loaded_path());

	{
		std::lock_guard<std::mutex> lock(s_jvm_mutex);
		if(s_shared_jvm)
		{
			m_jvm = s_shared_jvm;
			m_is_destroy = false;
			std::cerr << "[jvm] Reusing existing JVM instance" << std::endl;
			return;
		}

		JavaVM* existing_vms[1] = {nullptr};
		jsize existing_count = 0;
		check_throw_error(m_jni_api->get_created_java_vms(existing_vms, 1, &existing_count));
		if(existing_count > 0 && existing_vms[0])
		{
			s_shared_jvm = existing_vms[0];
			m_jvm = s_shared_jvm;
			m_is_destroy = false;
			std::cerr << "[jvm] Found existing JVM instance via JNI_GetCreatedJavaVMs" << std::endl;
			return;
		}
	}

	std::string classpath_value;
	if(!classpath_option.empty())
	{
		const std::string classpath_prefix = "-Djava.class.path=";
		if(classpath_option.rfind(classpath_prefix, 0) == 0)
		{
			classpath_value = classpath_option;
		}
		else
		{
			classpath_value = classpath_prefix + classpath_option;
		}
	}

	std::vector<JavaVMOption> default_options;
	std::vector<std::unique_ptr<char[]>> default_option_buffers;
	auto append_option = [](std::vector<JavaVMOption>& options,
	                        std::vector<std::unique_ptr<char[]>>& buffers,
	                        const char* option_string,
	                        void* extra_info)
	{
		if(!option_string)
		{
			return;
		}

		auto buffer = std::make_unique<char[]>(std::strlen(option_string) + 1);
		std::memcpy(buffer.get(), option_string, std::strlen(option_string) + 1);
		JavaVMOption option{};
		option.optionString = buffer.get();
		option.extraInfo = extra_info;
		options.push_back(option);
		buffers.push_back(std::move(buffer));
	};

	JavaVMInitArgs default_args{};
	default_args.version = JNI_VERSION_1_8;
	jint default_args_res = m_jni_api->get_default_java_vm_init_args(&default_args);
	if(default_args_res == JNI_OK)
	{
		default_options.reserve(default_args.nOptions + (classpath_value.empty() ? 0 : 1));
		default_option_buffers.reserve(default_args.nOptions + (classpath_value.empty() ? 0 : 1));
		for(jsize i = 0; i < default_args.nOptions; ++i)
		{
			append_option(default_options, default_option_buffers,
			              default_args.options[i].optionString,
			              default_args.options[i].extraInfo);
		}
		if(!classpath_value.empty())
		{
			append_option(default_options, default_option_buffers, classpath_value.c_str(), nullptr);
		}
	}

	std::vector<JavaVMOption> manual_options;
	std::vector<std::unique_ptr<char[]>> manual_option_buffers;
	if(!classpath_value.empty())
	{
		append_option(manual_options, manual_option_buffers, classpath_value.c_str(), nullptr);
	}

	auto try_create_vm = [&](JavaVMInitArgs& args)
	{
		JNIEnv* env = nullptr;
		std::cerr << "[jvm] Creating JVM (JNI_CreateJavaVM)" << std::endl;
		jint create_res = m_jni_api->create_java_vm(&m_jvm, reinterpret_cast<void**>(&env), &args);
		std::cerr << "[jvm] JNI_CreateJavaVM result: " << create_res << std::endl;
		return create_res;
	};

	JavaVMInitArgs args_to_use{};
	args_to_use.version = JNI_VERSION_1_8;
	if(default_args_res == JNI_OK)
	{
		args_to_use.nOptions = static_cast<jint>(default_options.size());
		args_to_use.options = default_options.empty() ? nullptr : default_options.data();
		args_to_use.ignoreUnrecognized = default_args.ignoreUnrecognized;
	}
	else
	{
		args_to_use.nOptions = static_cast<jint>(manual_options.size());
		args_to_use.options = manual_options.empty() ? nullptr : manual_options.data();
		args_to_use.ignoreUnrecognized = JNI_FALSE;
	}

	jint create_res = try_create_vm(args_to_use);
	if(create_res == JNI_EINVAL && default_args_res == JNI_OK)
	{
		std::cerr << "[jvm] Retrying JVM creation with manual init args" << std::endl;
		args_to_use.version = JNI_VERSION_1_8;
		args_to_use.nOptions = static_cast<jint>(manual_options.size());
		args_to_use.options = manual_options.empty() ? nullptr : manual_options.data();
		args_to_use.ignoreUnrecognized = JNI_FALSE;
		create_res = try_create_vm(args_to_use);
	}
	if(create_res == JNI_OK)
	{
		{
			std::lock_guard<std::mutex> lock(s_jvm_mutex);
			s_shared_jvm = m_jvm;
		}
		m_is_destroy = true;
		return;
	}

	if(create_res != JNI_EEXIST)
	{
		check_throw_error(create_res);
	}

	if(m_jvm)
	{
		m_is_destroy = false;
		return;
	}

	JavaVM* vms[1] = {nullptr};
	jsize n_vms = 0;
	check_throw_error(m_jni_api->get_created_java_vms(vms, 1, &n_vms));
	if(n_vms > 0 && vms[0])
	{
		{
			std::lock_guard<std::mutex> lock(s_jvm_mutex);
			s_shared_jvm = vms[0];
		}
		m_jvm = vms[0];
		m_is_destroy = false;
		return;
	}

	throw std::runtime_error("JVM already exists but could not retrieve instance");
}
//--------------------------------------------------------------------
void jvm::fini()
{
	if(m_jvm && m_is_destroy)
	{
		jint res = m_jvm->DestroyJavaVM();
		if(res != JNI_OK)
		{
			printf("Failed to destroy JVM: %ld\n", res);
		}
		m_jvm = nullptr;
	}
}
//--------------------------------------------------------------------
std::function<void()> jvm::get_environment(JNIEnv** env) const
{
	if(!m_jvm)
	{
		throw std::runtime_error("JVM is not initialized");
	}

	bool did_attach_thread = false;
	auto get_env_result = m_jvm->GetEnv(reinterpret_cast<void**>(env), JNI_VERSION_1_8);
	if(get_env_result == JNI_EDETACHED)
	{
		if(m_jvm->AttachCurrentThread(reinterpret_cast<void**>(env), nullptr) == JNI_OK)
		{
			did_attach_thread = true;
		}
		else
		{
			throw std::runtime_error("Failed to attach environment to current thread");
		}
	}
	else if(get_env_result == JNI_EVERSION)
	{
		throw std::runtime_error("Failed to get JVM environment - unsupported JNI version");
	}

	return did_attach_thread ? std::function<void()>([this](){ m_jvm->DetachCurrentThread(); }) : []() {};
}
//--------------------------------------------------------------------
void jvm::check_throw_error(jint err)
{
	if(err == JNI_OK)
	{
		return;
	}

	switch(err)
	{
		case JNI_ERR:
			throw std::runtime_error("JNI error");
		case JNI_EDETACHED:
			throw std::runtime_error("Thread detached from the JVM");
		case JNI_EVERSION:
			throw std::runtime_error("JNI version error");
		case JNI_ENOMEM:
			throw std::runtime_error("Not enough memory");
		case JNI_EEXIST:
			throw std::runtime_error("JVM already created");
		case JNI_EINVAL:
			throw std::runtime_error("Invalid JVM argument");
		default:
			throw std::runtime_error("Unknown JNI error");
	}
}
//--------------------------------------------------------------------
std::string jvm::get_exception_description(jthrowable throwable) const
{
	JNIEnv* penv = nullptr;
	auto release_env = get_environment(&penv);
	scope_guard sg_env([&](){ release_env(); });

	return get_exception_description(penv, throwable);
}
//--------------------------------------------------------------------
std::string jvm::get_exception_description(JNIEnv* penv, jthrowable throwable)
{
	if(!penv || !throwable)
	{
		return "JNI exception";
	}

	penv->ExceptionClear();

	jclass throwable_class = penv->FindClass("java/lang/Throwable");
	if(!throwable_class)
	{
		penv->ExceptionClear();
		return "Failed to find java/lang/Throwable";
	}

	jmethodID throwable_toString = penv->GetMethodID(throwable_class, "toString", "()Ljava/lang/String;");
	if(!throwable_toString)
	{
		penv->ExceptionClear();
		return "Failed to get Throwable.toString";
	}

	jobject str = penv->CallObjectMethod(throwable, throwable_toString);
	if(!str)
	{
		penv->ExceptionClear();
		return "Failed to call Throwable.toString";
	}

	scope_guard sg([&](){ penv->DeleteLocalRef(str); });

	const char* cstr = penv->GetStringUTFChars((jstring)str, nullptr);
	std::string res = cstr ? cstr : "";
	if(cstr)
	{
		penv->ReleaseStringUTFChars((jstring)str, cstr);
	}

	return res;
}
//--------------------------------------------------------------------
jvm::operator JavaVM*() const
{
	return m_jvm;
}
