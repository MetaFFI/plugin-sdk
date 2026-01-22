#include "runtime_manager.h"
#include "module.h"
#include "jni_api_wrapper.h"
#include "jni_helpers.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
	#ifdef _WIN32
	constexpr char classpath_separator = ';';
	#else
	constexpr char classpath_separator = ':';
	#endif

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

	std::string to_lower(std::string value)
	{
		std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return (char)std::tolower(c); });
		return value;
	}

	std::string trim(std::string value)
	{
		auto not_space = [](unsigned char c){ return !std::isspace(c); };
		value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
		value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
		return value;
	}

	std::string build_classpath_option()
	{
		std::ostringstream ss;
		ss << "-Djava.class.path=.";

		const char* metaffi_home = std::getenv("METAFFI_HOME");
		if(metaffi_home && std::strlen(metaffi_home) > 0)
		{
			std::filesystem::path bridge = std::filesystem::path(metaffi_home) / "jvm" / "xllr.jvm.bridge.jar";
			if(std::filesystem::exists(bridge))
			{
				ss << classpath_separator << bridge.generic_string();
			}
		}

		const char* classpath = std::getenv("CLASSPATH");
		if(classpath && std::strlen(classpath) > 0)
		{
			ss << classpath_separator << classpath;
		}

		return ss.str();
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
		// Check for release file (standard in modern JDKs)
		if(std::filesystem::exists(home / "release"))
		{
			return true;
		}

		// Check for libjvm
		if(!find_libjvm_in_home(home).empty())
		{
			return true;
		}

		return false;
	}

	std::vector<std::filesystem::path> find_java_from_path()
	{
		std::vector<std::filesystem::path> result;

		const char* pathEnv = std::getenv("PATH");
		if(!pathEnv || std::strlen(pathEnv) == 0)
		{
			return result;
		}

		std::string pathStr(pathEnv);

		#ifdef _WIN32
		const char pathSeparator = ';';
		const std::vector<std::string> javaNames = {"java.exe"};
		#else
		const char pathSeparator = ':';
		const std::vector<std::string> javaNames = {"java"};
		#endif

		std::istringstream pathStream(pathStr);
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
		// Follow symlink chain on Unix-like systems
		int maxIterations = 20; // Prevent infinite loops
		while(std::filesystem::is_symlink(resolved, ec) && !ec && maxIterations-- > 0)
		{
			auto target = std::filesystem::read_symlink(resolved, ec);
			if(ec)
			{
				break;
			}

			// Handle relative symlinks
			if(target.is_relative())
			{
				target = resolved.parent_path() / target;
			}

			resolved = std::filesystem::canonical(target, ec);
			if(ec)
			{
				// Try weakly_canonical as fallback
				resolved = std::filesystem::weakly_canonical(target, ec);
				if(ec)
				{
					break;
				}
			}
		}
		#endif

		// java executable is typically in: JAVA_HOME/bin/java
		// So we go up two levels: bin -> JAVA_HOME
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

	void add_candidate_if_valid(
		std::vector<jvm_installed_info>& out,
		std::unordered_set<std::string>& seen,
		const std::filesystem::path& javaHome)
	{
		if(!std::filesystem::exists(javaHome))
		{
			return;
		}

		// Normalize path for deduplication
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
			return; // Already processed
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

		std::string vendor = read_release_value(javaHome, "IMPLEMENTOR");
		if(vendor.empty())
		{
			vendor = read_release_value(javaHome, "JAVA_VENDOR");
		}
		info.vendor = jdk_runtime_manager::map_vendor(vendor);

		out.push_back(info);
	}

	void add_candidate(std::vector<jvm_installed_info>& out, const std::filesystem::path& java_home)
	{
		if(!std::filesystem::exists(java_home))
		{
			return;
		}

		auto libjvm = find_libjvm_in_home(java_home);
		if(libjvm.empty())
		{
			return;
		}

		jvm_installed_info info;
		info.home = java_home.string();
		info.libjvm_path = libjvm;
		info.version = read_release_value(java_home, "JAVA_VERSION");
		if(info.version.empty())
		{
			info.version = read_release_value(java_home, "JAVA_RUNTIME_VERSION");
		}
		std::string vendor = read_release_value(java_home, "IMPLEMENTOR");
		if(vendor.empty())
		{
			vendor = read_release_value(java_home, "JAVA_VENDOR");
		}
		info.vendor = jdk_runtime_manager::map_vendor(vendor);
		out.push_back(info);
	}
}

jdk_runtime_manager::jdk_runtime_manager(const jvm_installed_info& info)
	: m_info(info)
{
}

jdk_runtime_manager::~jdk_runtime_manager()
{
	if(m_isRuntimeLoaded)
	{
		try
		{
			release_runtime();
		}
		catch(const std::exception&)
		{
		}
	}
}

std::vector<jvm_installed_info> jdk_runtime_manager::detect_installed_jvms()
{
	std::vector<jvm_installed_info> result;
	std::unordered_set<std::string> seen;

	// Priority 1: JAVA_HOME environment variable
	const char* javaHome = std::getenv("JAVA_HOME");
	if(javaHome && std::strlen(javaHome) > 0)
	{
		std::filesystem::path homePath(javaHome);
		if(std::filesystem::exists(homePath))
		{
			add_candidate_if_valid(result, seen, homePath);
		}
	}

	// Priority 2: JDK_HOME environment variable (alternative)
	const char* jdkHome = std::getenv("JDK_HOME");
	if(jdkHome && std::strlen(jdkHome) > 0)
	{
		std::filesystem::path homePath(jdkHome);
		if(std::filesystem::exists(homePath))
		{
			add_candidate_if_valid(result, seen, homePath);
		}
	}

	// Priority 3: Find java executable in PATH and resolve to JVM home
	auto javaFromPath = find_java_from_path();
	for(const auto& javaPath : javaFromPath)
	{
		auto jvmHome = resolve_jvm_home_from_executable(javaPath);
		if(!jvmHome.empty())
		{
			add_candidate_if_valid(result, seen, jvmHome);
		}
	}

	return result;
}

void jdk_runtime_manager::load_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(m_isRuntimeLoaded)
	{
		return;
	}

	ensure_jvm_loaded();
	m_isRuntimeLoaded = true;
}

void jdk_runtime_manager::release_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(!m_isRuntimeLoaded)
	{
		return;
	}

	// Note: We intentionally do NOT call DestroyJavaVM() because:
	// 1. HotSpot JVM only allows one JVM per process lifetime
	// 2. Once destroyed, JNI_CreateJavaVM fails with JNI_EEXIST
	// 3. JNI_GetCreatedJavaVMs returns 0 after destruction, but creation still fails
	// Instead, we just mark as unloaded and leave the JVM running.
	// The JVM will be cleaned up when the process exits.

	m_isRuntimeLoaded = false;
	// Don't clear m_jvm - other managers might still reference it
}

std::shared_ptr<Module> jdk_runtime_manager::load_module(const std::string& module_path)
{
	if(!m_isRuntimeLoaded)
	{
		load_runtime();
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	return std::make_shared<Module>(this, module_path);
}

bool jdk_runtime_manager::is_runtime_loaded() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_isRuntimeLoaded;
}

const jvm_installed_info& jdk_runtime_manager::get_jvm_info() const
{
	return m_info;
}

std::function<void()> jdk_runtime_manager::get_env(JNIEnv** env) const
{
	if(!env)
	{
		throw std::runtime_error("JNIEnv output pointer is null");
	}

	if(!m_jvm)
	{
		throw std::runtime_error("JVM is not initialized");
	}

	bool attached = false;
	jint res = m_jvm->GetEnv(reinterpret_cast<void**>(env), JNI_VERSION_1_8);
	if(res == JNI_EDETACHED)
	{
		if(m_jvm->AttachCurrentThread(reinterpret_cast<void**>(env), nullptr) != JNI_OK)
		{
			throw std::runtime_error("Failed to attach current thread to JVM");
		}
		attached = true;
	}
	else if(res == JNI_EVERSION)
	{
		throw std::runtime_error("Unsupported JNI version");
	}

	return attached ? std::function<void()>([this](){ m_jvm->DetachCurrentThread(); }) : [](){};
}

void jdk_runtime_manager::ensure_jvm_loaded()
{
	if(m_info.libjvm_path.empty())
	{
		throw std::runtime_error("libjvm_path is empty in jvm_installed_info");
	}

#ifdef _WIN32
	// Setup DLL search paths for Windows
	if(!m_info.home.empty())
	{
		std::filesystem::path bin_path = std::filesystem::path(m_info.home) / "bin";
		std::filesystem::path server_path = bin_path / "server";

		std::wstring wbin = bin_path.wstring();
		SetDllDirectoryW(wbin.c_str());
		SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
		AddDllDirectory(bin_path.wstring().c_str());
		AddDllDirectory(server_path.wstring().c_str());

		std::string existing_path = std::getenv("PATH") ? std::getenv("PATH") : "";
		std::string prefix = bin_path.string() + ";" + server_path.string();
		std::string new_path = prefix + (existing_path.empty() ? "" : ";" + existing_path);
		_putenv_s("PATH", new_path.c_str());
		_putenv_s("JAVA_HOME", m_info.home.c_str());
		_putenv_s("JDK_HOME", m_info.home.c_str());
	}

	// Pre-load jvm.dll to ensure it's found
	if(!m_info.libjvm_path.empty())
	{
		std::wstring wjvm = std::filesystem::path(m_info.libjvm_path).wstring();
		LoadLibraryExW(wjvm.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
	}
#endif

	// Check for existing JVM first
	jsize n_vms = 0;
	jint get_vms_result = JNI_GetCreatedJavaVMs(nullptr, 0, &n_vms);
	if(get_vms_result == JNI_OK && n_vms > 0)
	{
		check_jni_error(JNI_GetCreatedJavaVMs(&m_jvm, 1, &n_vms));
		m_isEmbedded = false;
		return;
	}

	// Create new JVM
	JavaVMOption options[1];
	std::string classpath_option = build_classpath_option();
	options[0].optionString = const_cast<char*>(classpath_option.c_str());
	options[0].extraInfo = nullptr;

	JavaVMInitArgs vm_args{};
	vm_args.version = JNI_VERSION_21;
	vm_args.nOptions = 1;
	vm_args.options = options;
	vm_args.ignoreUnrecognized = JNI_FALSE;

	if(!m_info.home.empty())
	{
		#ifdef _WIN32
		_putenv_s("JAVA_HOME", m_info.home.c_str());
		_putenv_s("JDK_HOME", m_info.home.c_str());
		#else
		setenv("JAVA_HOME", m_info.home.c_str(), 1);
		setenv("JDK_HOME", m_info.home.c_str(), 1);
		#endif
	}

	JNIEnv* env = nullptr;
	check_jni_error(JNI_CreateJavaVM(&m_jvm, reinterpret_cast<void**>(&env), &vm_args));
	m_isEmbedded = true;
}

void jdk_runtime_manager::check_jni_error(jint err)
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
			throw std::runtime_error("Thread detached from JVM");
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

std::string jdk_runtime_manager::normalize_vendor(const std::string& vendor)
{
	return to_lower(trim(vendor));
}

jvm_vendor jdk_runtime_manager::map_vendor(const std::string& vendor)
{
	std::string val = normalize_vendor(vendor);
	if(val.find("openjdk") != std::string::npos)
		return jvm_vendor::openjdk;
	if(val.find("oracle") != std::string::npos)
		return jvm_vendor::oracle;
	if(val.find("microsoft") != std::string::npos)
		return jvm_vendor::microsoft;
	if(val.find("adoptium") != std::string::npos || val.find("temurin") != std::string::npos || val.find("eclipse") != std::string::npos)
		return jvm_vendor::adoptium;
	if(val.find("amazon") != std::string::npos || val.find("corretto") != std::string::npos)
		return jvm_vendor::amazon;
	if(val.find("azul") != std::string::npos || val.find("zulu") != std::string::npos)
		return jvm_vendor::azul;
	if(val.find("graalvm") != std::string::npos)
		return jvm_vendor::graalvm;
	if(val.find("ibm") != std::string::npos || val.find("openj9") != std::string::npos)
		return jvm_vendor::ibm;
	if(val.find("sap") != std::string::npos || val.find("sapmachine") != std::string::npos)
		return jvm_vendor::sap;
	if(val.find("bellsoft") != std::string::npos || val.find("liberica") != std::string::npos)
		return jvm_vendor::bellsoft;
	if(val.find("red hat") != std::string::npos || val.find("redhat") != std::string::npos)
		return jvm_vendor::redhat;
	if(val.find("alibaba") != std::string::npos || val.find("dragonwell") != std::string::npos)
		return jvm_vendor::alibaba;
	if(val.find("tencent") != std::string::npos)
		return jvm_vendor::tencent;
	if(val.find("huawei") != std::string::npos)
		return jvm_vendor::huawei;
	if(val.find("jetbrains") != std::string::npos || val.find("jbr") != std::string::npos)
		return jvm_vendor::jetbrains;
	if(val.find("apple") != std::string::npos)
		return jvm_vendor::apple;
	return jvm_vendor::unknown;
}
