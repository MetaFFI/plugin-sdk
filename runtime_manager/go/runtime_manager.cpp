#include "runtime_manager.h"
#include "module.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <stdexcept>
#include <utils/safe_func.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace
{
	std::string trim(std::string value)
	{
		auto not_space = [](unsigned char c) { return !std::isspace(c); };
		value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
		value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
		return value;
	}

	// Execute a command and capture its output
	std::string execute_command(const std::string& command)
	{
		std::array<char, 128> buffer;
		std::string result;

#ifdef _WIN32
		std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
#else
		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
#endif

		if (!pipe)
		{
			return "";
		}

		while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
		{
			result += buffer.data();
		}

		return trim(result);
	}

	// Go buildinfo magic: "\xff Go buildinf:" (14 bytes)
	// This is embedded by Go compiler in all c-shared builds
	constexpr std::uint8_t kBuildInfoMagic[] = {
		0xFF, ' ', 'G', 'o', ' ', 'b', 'u', 'i', 'l', 'd', 'i', 'n', 'f', ':'
	};
	constexpr std::size_t kBuildInfoMagicSize = sizeof(kBuildInfoMagic);
	constexpr std::size_t kBuildInfoHeaderSize = 32;

	// Decode unsigned varint (Go's binary.Uvarint format - little-endian base-128)
	// Returns {value, bytes_consumed}. bytes_consumed==0 indicates failure/overflow.
	std::pair<std::uint64_t, std::size_t> decode_uvarint(const std::uint8_t* p, std::size_t n)
	{
		std::uint64_t x = 0;
		std::uint32_t s = 0;

		for (std::size_t i = 0; i < n; ++i)
		{
			std::uint8_t b = p[i];
			if (b < 0x80)
			{
				// Check for overflow
				if (i > 9 || (i == 9 && b > 1))
				{
					return {0, 0};
				}
				x |= (static_cast<std::uint64_t>(b) << s);
				return {x, i + 1};
			}
			x |= (static_cast<std::uint64_t>(b & 0x7F) << s);
			s += 7;
		}

		return {0, 0};  // Not enough bytes
	}

	// Check if a string looks like a Go version (e.g., "go1.21.5")
	bool looks_like_go_version(const std::string& v)
	{
		if (v.size() < 3)
		{
			return false;
		}
		if (v[0] != 'g' || v[1] != 'o')
		{
			return false;
		}
		// Allow "go1.x" or "go2.x" etc.
		return true;
	}
}

go_runtime_manager::go_runtime_manager(const go_installed_info& info)
	: m_info(info)
	, m_isRuntimeLoaded(false)
{
}

go_runtime_manager::~go_runtime_manager()
{
	try
	{
		release_runtime();
	}
	catch (...)
	{
		// Ignore exceptions in destructor
	}
}

std::vector<go_installed_info> go_runtime_manager::detect_installed_go()
{
	std::vector<go_installed_info> result;
	std::unordered_set<std::string> seen;

	auto add_candidate = [&](const std::string& goroot, const std::string& go_exe)
	{
		// Normalize path for deduplication
		std::error_code ec;
		std::filesystem::path normalized = std::filesystem::canonical(goroot, ec);
		if (ec)
		{
			normalized = std::filesystem::weakly_canonical(goroot, ec);
			if (ec)
			{
				return;
			}
		}

		std::string key = normalized.generic_string();
		if (seen.count(key) > 0)
		{
			return;
		}
		seen.insert(key);

		// Validate and get version
		if (!is_valid_go_installation(key))
		{
			return;
		}

		go_installed_info info;
		info.goroot = key;
		info.go_exe = go_exe;
		info.version = get_go_version(go_exe);

		if (!info.version.empty())
		{
			result.push_back(info);
		}
	};

	// 1. Check GOROOT environment variable
	char* goroot_env = metaffi_getenv_alloc("GOROOT");
	if (goroot_env && std::strlen(goroot_env) > 0)
	{
		std::filesystem::path goroot_path(goroot_env);
		std::filesystem::path go_exe_path = goroot_path / "bin" /
#ifdef _WIN32
			"go.exe";
#else
			"go";
#endif
		if (std::filesystem::exists(go_exe_path))
		{
			add_candidate(goroot_env, go_exe_path.string());
		}
	}
	metaffi_free_env(goroot_env);

	// 2. Find go from PATH
	for (const auto& go_exe : find_go_from_path())
	{
		std::string goroot = resolve_goroot_from_executable(go_exe);
		if (!goroot.empty())
		{
			add_candidate(goroot, go_exe);
		}
	}

	return result;
}

void go_runtime_manager::load_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_isRuntimeLoaded)
	{
		return;  // Already loaded (idempotent)
	}

	// For Go, "loading runtime" means validating that Go is available
	// The actual Go runtime is embedded in each shared library

	if (!m_info.go_exe.empty())
	{
		std::error_code ec;
		if (!std::filesystem::exists(m_info.go_exe, ec))
		{
			throw std::runtime_error("Go executable not found: " + m_info.go_exe);
		}
	}

	if (!m_info.goroot.empty())
	{
		if (!is_valid_go_installation(m_info.goroot))
		{
			throw std::runtime_error("Invalid Go installation: " + m_info.goroot);
		}
	}

	m_isRuntimeLoaded = true;
}

void go_runtime_manager::release_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_isRuntimeLoaded)
	{
		return;  // Already released (idempotent)
	}

	// For Go, there's nothing to release
	// Loaded Go shared libraries cannot be properly unloaded (Go limitation)
	// They will be cleaned up when the process exits

	m_isRuntimeLoaded = false;
}

std::shared_ptr<Module> go_runtime_manager::load_module(const std::string& module_path)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	// Auto-load runtime if needed
	if (!m_isRuntimeLoaded)
	{
		m_mutex.unlock();
		load_runtime();
		m_mutex.lock();
	}

	return std::make_shared<Module>(this, module_path);
}

bool go_runtime_manager::is_runtime_loaded() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_isRuntimeLoaded;
}

const go_installed_info& go_runtime_manager::get_go_info() const
{
	return m_info;
}

std::vector<std::string> go_runtime_manager::find_go_from_path()
{
	std::vector<std::string> result;

	char* pathEnv = metaffi_getenv_alloc("PATH");
	if (!pathEnv || std::strlen(pathEnv) == 0)
	{
		metaffi_free_env(pathEnv);
		return result;
	}

	std::string pathStr(pathEnv);
	metaffi_free_env(pathEnv);

#ifdef _WIN32
	const char pathSeparator = ';';
	const std::string goName = "go.exe";
#else
	const char pathSeparator = ':';
	const std::string goName = "go";
#endif

	std::istringstream pathStream(pathStr);
	std::string pathEntry;
	std::unordered_set<std::string> seen;

	while (std::getline(pathStream, pathEntry, pathSeparator))
	{
		if (pathEntry.empty())
		{
			continue;
		}

		std::error_code ec;
		std::filesystem::path dir(pathEntry);
		if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec))
		{
			continue;
		}

		std::filesystem::path goPath = dir / goName;
		if (std::filesystem::exists(goPath, ec))
		{
			std::string goPathStr = goPath.string();
			if (seen.count(goPathStr) == 0)
			{
				seen.insert(goPathStr);
				result.push_back(goPathStr);
			}
		}
	}

	return result;
}

std::string go_runtime_manager::resolve_goroot_from_executable(const std::string& go_exe)
{
	std::filesystem::path resolved(go_exe);
	std::error_code ec;

#ifndef _WIN32
	// Follow symlink chain on Unix-like systems
	int maxIterations = 20;
	while (std::filesystem::is_symlink(resolved, ec) && !ec && maxIterations-- > 0)
	{
		auto target = std::filesystem::read_symlink(resolved, ec);
		if (ec)
		{
			break;
		}

		// Handle relative symlinks
		if (target.is_relative())
		{
			target = resolved.parent_path() / target;
		}

		resolved = std::filesystem::canonical(target, ec);
		if (ec)
		{
			resolved = std::filesystem::weakly_canonical(target, ec);
			if (ec)
			{
				break;
			}
		}
	}
#endif

	// go executable is typically in: GOROOT/bin/go
	// So we go up two levels: bin -> GOROOT
	if (resolved.has_parent_path())
	{
		std::filesystem::path binDir = resolved.parent_path();
		if (binDir.filename() == "bin")
		{
			std::filesystem::path goroot = binDir.parent_path();
			if (is_valid_go_installation(goroot.string()))
			{
				return goroot.string();
			}
		}
	}

	return "";
}

std::string go_runtime_manager::get_go_version(const std::string& go_exe)
{
	// Run "go version" and parse output
	// Output format: "go version go1.21.5 windows/amd64"

	std::string cmd = "\"" + go_exe + "\" version 2>&1";
	std::string output = execute_command(cmd);

	if (output.empty())
	{
		return "";
	}

	// Parse: "go version go1.21.5 windows/amd64"
	// Extract the version part (e.g., "go1.21.5")

	const std::string prefix = "go version ";
	if (output.rfind(prefix, 0) != 0)
	{
		return "";
	}

	std::string rest = output.substr(prefix.length());

	// Find the version token (starts with "go")
	std::istringstream iss(rest);
	std::string token;
	if (iss >> token)
	{
		if (token.rfind("go", 0) == 0)
		{
			return token;  // e.g., "go1.21.5"
		}
	}

	return "";
}

bool go_runtime_manager::is_valid_go_installation(const std::string& goroot)
{
	std::error_code ec;
	std::filesystem::path root(goroot);

	if (!std::filesystem::exists(root, ec))
	{
		return false;
	}

	// Check for go executable in bin/
	std::filesystem::path bin_dir = root / "bin";
	if (!std::filesystem::exists(bin_dir, ec))
	{
		return false;
	}

#ifdef _WIN32
	std::filesystem::path go_exe = bin_dir / "go.exe";
#else
	std::filesystem::path go_exe = bin_dir / "go";
#endif

	if (!std::filesystem::exists(go_exe, ec))
	{
		return false;
	}

	// Check for pkg directory (standard in Go installations)
	std::filesystem::path pkg_dir = root / "pkg";
	if (std::filesystem::exists(pkg_dir, ec))
	{
		return true;
	}

	// Check for src directory (standard in Go installations)
	std::filesystem::path src_dir = root / "src";
	if (std::filesystem::exists(src_dir, ec))
	{
		return true;
	}

	// If we have the go executable, that's enough
	return true;
}

go_detect_result go_runtime_manager::is_go_shared_library(const std::string& library_path)
{
	std::error_code ec;
	if (!std::filesystem::exists(library_path, ec))
	{
		throw std::runtime_error("File does not exist: " + library_path);
	}

	std::ifstream in(library_path, std::ios::binary);
	if (!in)
	{
		throw std::runtime_error("Failed to open file: " + library_path);
	}

	// Chunked scan to handle large files efficiently
	constexpr std::size_t kChunkSize = 1 << 20;  // 1 MiB
	const std::size_t overlap = kBuildInfoMagicSize + kBuildInfoHeaderSize + 64;

	std::vector<std::uint8_t> buffer(kChunkSize + overlap);
	std::vector<std::uint8_t> tail;

	std::uint64_t globalOffset = 0;

	while (true)
	{
		// Prepend tail from previous chunk to handle boundary matches
		std::size_t prefix = 0;
		if (!tail.empty())
		{
			prefix = tail.size();
			std::copy(tail.begin(), tail.end(), buffer.begin());
		}

		in.read(reinterpret_cast<char*>(buffer.data() + prefix), static_cast<std::streamsize>(kChunkSize));
		const std::size_t bytesRead = static_cast<std::size_t>(in.gcount());

		if (bytesRead == 0 && prefix == 0)
		{
			break;
		}

		const std::size_t total = prefix + bytesRead;
		const std::uint8_t* data = buffer.data();

		// Scan for magic bytes
		for (std::size_t i = 0; i + kBuildInfoMagicSize <= total; ++i)
		{
			if (std::memcmp(data + i, kBuildInfoMagic, kBuildInfoMagicSize) != 0)
			{
				continue;
			}

			// Found magic!
			const std::uint64_t foundAt = globalOffset + i - prefix;

			// Need at least 32 bytes header available
			if (i + kBuildInfoHeaderSize > total)
			{
				return {
					go_detect_confidence::probable,
					"Found Go buildinfo magic near end of buffer; insufficient bytes to validate full header.",
					std::nullopt,
					foundAt
				};
			}

			const std::uint8_t* header = data + i;

			// Header layout (from Go source):
			// magic[14], ptrSize[1], flags[1], then 2 pointers (or inline strings after header)
			constexpr std::size_t ptrSizeOffset = 14;
			constexpr std::size_t flagsOffset = 15;

			const std::uint8_t ptrSize = header[ptrSizeOffset];
			const std::uint8_t flags = header[flagsOffset];

			// flagsVersionInl = 0x2 means inline strings (Go 1.18+)
			const bool inlineStrings = (flags & 0x2) == 0x2;

			if (!inlineStrings)
			{
				// Pre-1.18 pointer format: sanity-check ptrSize
				if (ptrSize != 4 && ptrSize != 8)
				{
					// Likely false positive, continue scanning
					continue;
				}

				return {
					go_detect_confidence::probable,
					"Found valid-looking Go buildinfo header (pre-1.18 pointer format).",
					std::nullopt,
					foundAt
				};
			}

			// Go 1.18+ inline format: version string is varint-length-prefixed right after header
			std::size_t p = i + kBuildInfoHeaderSize;
			if (p >= total)
			{
				return {
					go_detect_confidence::probable,
					"Found Go buildinfo header (inline format) but no room to decode strings.",
					std::nullopt,
					foundAt
				};
			}

			// Decode version string length
			auto [versionLength, bytesConsumed] = decode_uvarint(data + p, total - p);
			if (bytesConsumed == 0)
			{
				return {
					go_detect_confidence::probable,
					"Found Go buildinfo header (inline format) but failed decoding version length.",
					std::nullopt,
					foundAt
				};
			}

			p += bytesConsumed;
			if (p + versionLength > total)
			{
				return {
					go_detect_confidence::probable,
					"Found Go buildinfo header (inline format) but version string spans beyond current buffer.",
					std::nullopt,
					foundAt
				};
			}

			// Extract version string
			std::string version(reinterpret_cast<const char*>(data + p), static_cast<std::size_t>(versionLength));

			if (!looks_like_go_version(version))
			{
				return {
					go_detect_confidence::probable,
					"Found Go buildinfo header (inline format) but decoded version does not look like a Go version.",
					std::nullopt,
					foundAt
				};
			}

			return {
				go_detect_confidence::yes,
				"Found Go buildinfo header and decoded inline Go version string.",
				version,
				foundAt
			};
		}

		// Save tail for overlap to handle magic spanning chunk boundaries
		// Use parentheses to avoid conflict with Windows min macro
		const std::size_t keep = (std::min)(overlap, total);
		tail.assign(buffer.begin() + (total - keep), buffer.begin() + total);

		if (!in)
		{
			break;
		}

		globalOffset += bytesRead;
	}

	return {
		go_detect_confidence::no,
		"No Go buildinfo magic found.",
		std::nullopt,
		0
	};
}
