#include "runtime_manager.h"
#include "module.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <array>
#include <stdexcept>
#include <iostream>

namespace
{
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

go_runtime_manager::go_runtime_manager()
	: m_isRuntimeLoaded(false)
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
		// Log and rethrow or swallow: plan says at worst log to cerr
		std::cerr << "go_runtime_manager destructor: exception during release_runtime()" << std::endl;
	}
}

void go_runtime_manager::load_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	// No-op for Go: there is no external runtime to load (Go compiles to standalone binaries).
	if (!m_isRuntimeLoaded)
	{
		m_isRuntimeLoaded = true;
	}
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

	// Auto-load runtime if needed (no-op for Go, but sets flag for API consistency)
	if (!m_isRuntimeLoaded)
	{
		m_mutex.unlock();
		load_runtime();
		m_mutex.lock();
	}

	// Fail-fast: ensure the file is a Go shared library before loading
	go_detect_result detect = is_go_shared_library(module_path);
	if (detect.confidence == go_detect_confidence::no)
	{
		throw std::runtime_error("Not a Go shared library: " + module_path + " (" + detect.reason + ")");
	}

	return std::make_shared<Module>(this, module_path);
}

bool go_runtime_manager::is_runtime_loaded() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_isRuntimeLoaded;
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
