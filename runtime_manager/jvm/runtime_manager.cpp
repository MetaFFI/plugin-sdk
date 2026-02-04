#include "runtime_manager.h"
#include "module.h"
#include "jvm.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <utils/env_utils.h>
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

		std::string metaffi_home = get_env_var("METAFFI_HOME");
		if(!metaffi_home.empty())
		{
			std::filesystem::path bridge = std::filesystem::path(metaffi_home) / "jvm" / "xllr.jvm.bridge.jar";
			if(std::filesystem::exists(bridge))
			{
				ss << classpath_separator << bridge.generic_string();
			}
		}

		std::string classpath = get_env_var("CLASSPATH");
		if(!classpath.empty())
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

		std::string path_env = get_env_var("PATH");
		if(path_env.empty())
		{
			return result;
		}

		std::string pathStr(path_env);

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
		info.vendor = jvm_runtime_manager::map_vendor(vendor);

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
		info.vendor = jvm_runtime_manager::map_vendor(vendor);
		out.push_back(info);
	}
}

jvm_runtime_manager::jvm_runtime_manager(const jvm_installed_info& info)
	: m_info(info)
{
}

jvm_runtime_manager::~jvm_runtime_manager()
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

std::vector<jvm_installed_info> jvm_runtime_manager::detect_installed_jvms()
{
	std::vector<jvm_installed_info> result;
	std::unordered_set<std::string> seen;

	// Priority 1: JAVA_HOME environment variable
	std::string java_home = get_env_var("JAVA_HOME");
	if(!java_home.empty())
	{
		std::filesystem::path homePath(java_home);
		if(std::filesystem::exists(homePath))
		{
			add_candidate_if_valid(result, seen, homePath);
		}
	}

	// Priority 2: JDK_HOME environment variable (alternative)
	std::string jdk_home = get_env_var("JDK_HOME");
	if(!jdk_home.empty())
	{
		std::filesystem::path homePath(jdk_home);
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

void jvm_runtime_manager::load_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(m_isRuntimeLoaded)
	{
		return;
	}

	ensure_jvm_loaded();
	m_isRuntimeLoaded = true;
}

void jvm_runtime_manager::release_runtime()
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

std::shared_ptr<Module> jvm_runtime_manager::load_module(const std::string& module_path)
{
	if(!m_isRuntimeLoaded)
	{
		load_runtime();
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	return std::make_shared<Module>(this, module_path);
}

bool jvm_runtime_manager::is_runtime_loaded() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_isRuntimeLoaded;
}

const jvm_installed_info& jvm_runtime_manager::get_jvm_info() const
{
	return m_info;
}

std::function<void()> jvm_runtime_manager::get_env(JNIEnv** env) const
{
	if(!env)
	{
		throw std::runtime_error("JNIEnv output pointer is null");
	}

	if(!m_jvm)
	{
		throw std::runtime_error("JVM is not initialized");
	}

	return m_jvm->get_environment(env);
}

void jvm_runtime_manager::ensure_jvm_loaded()
{
	if(m_jvm)
	{
		return;
	}

	std::string classpath_option = build_classpath_option();
	m_jvm = std::make_shared<jvm>(m_info, classpath_option);
}

std::string jvm_runtime_manager::normalize_vendor(const std::string& vendor)
{
	return to_lower(trim(vendor));
}

jvm_vendor jvm_runtime_manager::map_vendor(const std::string& vendor)
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
