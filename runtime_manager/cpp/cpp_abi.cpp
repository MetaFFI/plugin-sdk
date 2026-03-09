#include "cpp_abi.h"

#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <string>

// Platform-specific includes for binary format parsing
#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <windows.h>
#elif defined(__linux__)
#   include <elf.h>
#endif


// ============================================================================
// Compile-time plugin ABI
// ============================================================================

cpp_abi get_plugin_abi()
{
#if defined(_MSC_VER)
	return cpp_abi::msvc;
#elif defined(__GNUC__) || defined(__clang__)
	return cpp_abi::itanium;
#else
	return cpp_abi::unknown;
#endif
}


// ============================================================================
// String helpers
// ============================================================================

const char* cpp_abi_name(cpp_abi abi)
{
	switch (abi)
	{
		case cpp_abi::unknown:  return "unknown";
		case cpp_abi::c_only:   return "c_only";
		case cpp_abi::itanium:  return "itanium";
		case cpp_abi::msvc:     return "msvc";
	}
	return "unknown";
}


// ============================================================================
// Binary reading helper
// ============================================================================

namespace
{
	// Read the first `max_bytes` bytes of a file (or the whole file if smaller).
	std::vector<uint8_t> read_file_head(const std::string& path, std::size_t max_bytes)
	{
		std::ifstream in(path, std::ios::binary);
		if (!in)
		{
			throw std::runtime_error("cpp_abi: cannot open file: " + path);
		}

		// Determine actual file size
		in.seekg(0, std::ios::end);
		auto file_size = static_cast<std::size_t>(in.tellg());
		in.seekg(0, std::ios::beg);

		if (file_size < 4)
		{
			throw std::runtime_error("cpp_abi: file too small to be a shared library: " + path);
		}

		std::size_t to_read = (max_bytes == 0) ? file_size : std::min(file_size, max_bytes);
		std::vector<uint8_t> buf(to_read);
		in.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(to_read));

		if (static_cast<std::size_t>(in.gcount()) != to_read)
		{
			throw std::runtime_error("cpp_abi: short read from file: " + path);
		}

		return buf;
	}

	// Case-insensitive ASCII "starts_with" check on a null-terminated string.
	bool iname_starts_with(const char* name, const char* prefix)
	{
		while (*prefix)
		{
			if (std::toupper(static_cast<unsigned char>(*name)) !=
			    std::toupper(static_cast<unsigned char>(*prefix)))
			{
				return false;
			}
			++name;
			++prefix;
		}
		return true;
	}
} // anonymous namespace


// ============================================================================
// Windows PE import-table scanner
// ============================================================================

#ifdef _WIN32
namespace
{
	// Convert a Relative Virtual Address to a file offset using the section table.
	// Returns 0 if the RVA does not fall in any section.
	uint32_t pe_rva_to_file_offset(const uint8_t*             data,
	                                std::size_t                data_size,
	                                uint32_t                   rva,
	                                const IMAGE_SECTION_HEADER* sections,
	                                uint16_t                   num_sections)
	{
		for (uint16_t i = 0; i < num_sections; ++i)
		{
			uint32_t vaddr = sections[i].VirtualAddress;
			uint32_t vsize = sections[i].Misc.VirtualSize;
			if (vsize == 0) vsize = sections[i].SizeOfRawData;

			if (rva >= vaddr && rva < vaddr + vsize)
			{
				uint32_t offset = rva - vaddr + sections[i].PointerToRawData;
				if (offset < data_size) return offset;
			}
		}
		return 0;
	}

	// Read a null-terminated ASCII string from data, up to max_len bytes.
	// Returns empty string if offset is out of range.
	std::string pe_read_string(const uint8_t* data, std::size_t data_size,
	                            uint32_t offset, std::size_t max_len = 256)
	{
		if (offset >= data_size) return {};

		std::string result;
		result.reserve(64);

		for (std::size_t i = 0; i < max_len && offset + i < data_size; ++i)
		{
			char c = static_cast<char>(data[offset + i]);
			if (c == '\0') break;
			result += c;
		}

		return result;
	}

	cpp_abi detect_pe_abi(const std::vector<uint8_t>& data)
	{
		const std::size_t n = data.size();
		const uint8_t*    p = data.data();

		// --- DOS header ---
		if (n < sizeof(IMAGE_DOS_HEADER)) return cpp_abi::unknown;

		const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(p);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE) return cpp_abi::unknown;

		uint32_t pe_offset = static_cast<uint32_t>(dos->e_lfanew);
		if (pe_offset + 4 > n) return cpp_abi::unknown;

		// --- PE signature ---
		if (std::memcmp(p + pe_offset, "PE\0\0", 4) != 0) return cpp_abi::unknown;

		// --- COFF header ---
		const std::size_t coff_offset = pe_offset + 4;
		if (coff_offset + sizeof(IMAGE_FILE_HEADER) > n) return cpp_abi::unknown;
		const auto* coff = reinterpret_cast<const IMAGE_FILE_HEADER*>(p + coff_offset);

		// --- Optional header magic (first 2 bytes) ---
		const std::size_t opt_offset = coff_offset + sizeof(IMAGE_FILE_HEADER);
		if (coff->SizeOfOptionalHeader < 2 || opt_offset + 2 > n) return cpp_abi::unknown;
		uint16_t opt_magic;
		std::memcpy(&opt_magic, p + opt_offset, 2);

		// --- Import directory RVA and section table pointer ---
		uint32_t    import_rva    = 0;
		std::size_t section_start = opt_offset + coff->SizeOfOptionalHeader;

		if (opt_magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		{
			if (opt_offset + sizeof(IMAGE_OPTIONAL_HEADER64) > n) return cpp_abi::unknown;
			const auto* opt = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64*>(p + opt_offset);

			if (opt->NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_IMPORT)
				return cpp_abi::c_only;  // no import directory

			import_rva = opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		}
		else if (opt_magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		{
			if (opt_offset + sizeof(IMAGE_OPTIONAL_HEADER32) > n) return cpp_abi::unknown;
			const auto* opt = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32*>(p + opt_offset);

			if (opt->NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_IMPORT)
				return cpp_abi::c_only;

			import_rva = opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		}
		else
		{
			return cpp_abi::unknown;  // unrecognised optional header
		}

		if (import_rva == 0) return cpp_abi::c_only;  // no imports at all

		// --- Section table ---
		uint16_t num_sections = coff->NumberOfSections;
		if (section_start + static_cast<std::size_t>(num_sections) * sizeof(IMAGE_SECTION_HEADER) > n)
			return cpp_abi::unknown;

		const auto* sections = reinterpret_cast<const IMAGE_SECTION_HEADER*>(p + section_start);

		// --- Convert import table RVA to file offset ---
		uint32_t import_file_offset = pe_rva_to_file_offset(p, n, import_rva, sections, num_sections);
		if (import_file_offset == 0) return cpp_abi::unknown;

		// --- Walk IMAGE_IMPORT_DESCRIPTOR array ---
		// Each descriptor is 20 bytes; terminated by an all-zero entry.
		std::size_t desc_offset = import_file_offset;

		while (desc_offset + sizeof(IMAGE_IMPORT_DESCRIPTOR) <= n)
		{
			const auto* desc = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR*>(p + desc_offset);

			// The null-terminator entry has Name == 0
			if (desc->Name == 0) break;

			uint32_t name_offset = pe_rva_to_file_offset(p, n, desc->Name, sections, num_sections);
			if (name_offset != 0)
			{
				std::string dll_name = pe_read_string(p, n, name_offset);

				// MSVCP*.dll (e.g. MSVCP140.dll) is the MSVC C++ Standard Library —
				// the definitive signal that this module uses the MSVC C++ ABI.
				// VCRUNTIME*.dll and MSVCR*.dll are also linked by pure-C code
				// compiled with MSVC, so they are not reliable C++ indicators.
				if (iname_starts_with(dll_name.c_str(), "MSVCP"))
				{
					return cpp_abi::msvc;
				}

				// Check for MinGW/GCC C++ runtime DLLs → Itanium ABI on Windows
				if (iname_starts_with(dll_name.c_str(), "libstdc++") ||
				    iname_starts_with(dll_name.c_str(), "libgcc_s"))
				{
					return cpp_abi::itanium;
				}
			}

			desc_offset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
		}

		// No C++ runtime DLL found → treat as pure C
		return cpp_abi::c_only;
	}
} // anonymous namespace
#endif // _WIN32


// ============================================================================
// Linux ELF DT_NEEDED scanner
// ============================================================================

#if defined(__linux__)
namespace
{
	// Read a little-endian uint16_t at byte offset p.
	static inline uint16_t read_u16le(const uint8_t* p)
	{
		return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
	}

	// Read a little-endian uint32_t.
	static inline uint32_t read_u32le(const uint8_t* p)
	{
		return (static_cast<uint32_t>(p[0]))       |
		       (static_cast<uint32_t>(p[1]) << 8)  |
		       (static_cast<uint32_t>(p[2]) << 16) |
		       (static_cast<uint32_t>(p[3]) << 24);
	}

	// Read a little-endian uint64_t.
	static inline uint64_t read_u64le(const uint8_t* p)
	{
		uint64_t v = 0;
		for (int i = 7; i >= 0; --i)
			v = (v << 8) | static_cast<uint64_t>(p[i]);
		return v;
	}

	// Find the file offset and size of the PT_DYNAMIC segment for 64-bit ELF.
	// Returns {offset, size} = {0,0} if not found.
	std::pair<uint64_t, uint64_t> elf64_find_dynamic(const uint8_t* data, std::size_t n)
	{
		// ELF64 header: e_phoff at offset 32, e_phentsize at 54, e_phnum at 56
		if (n < 64) return {0, 0};

		uint64_t phoff    = read_u64le(data + 32);
		uint16_t phentsize = read_u16le(data + 54);
		uint16_t phnum    = read_u16le(data + 56);

		for (uint16_t i = 0; i < phnum; ++i)
		{
			std::size_t phdr_off = static_cast<std::size_t>(phoff) + i * phentsize;
			if (phdr_off + 56 > n) break;

			uint32_t p_type   = read_u32le(data + phdr_off);      // Elf64_Phdr.p_type
			uint64_t p_offset = read_u64le(data + phdr_off + 8);  // Elf64_Phdr.p_offset
			uint64_t p_filesz = read_u64le(data + phdr_off + 32); // Elf64_Phdr.p_filesz

			if (p_type == PT_DYNAMIC)
			{
				return {p_offset, p_filesz};
			}
		}

		return {0, 0};
	}

	// Find the file offset for a load VA in a 64-bit ELF using PT_LOAD segments.
	// Returns 0 if not found.
	uint64_t elf64_va_to_offset(const uint8_t* data, std::size_t n, uint64_t va)
	{
		if (n < 64) return 0;

		uint64_t phoff     = read_u64le(data + 32);
		uint16_t phentsize = read_u16le(data + 54);
		uint16_t phnum     = read_u16le(data + 56);

		for (uint16_t i = 0; i < phnum; ++i)
		{
			std::size_t off = static_cast<std::size_t>(phoff) + i * phentsize;
			if (off + 56 > n) break;

			uint32_t p_type   = read_u32le(data + off);        // p_type
			uint64_t p_offset = read_u64le(data + off + 8);    // p_offset
			uint64_t p_vaddr  = read_u64le(data + off + 16);   // p_vaddr
			uint64_t p_filesz = read_u64le(data + off + 32);   // p_filesz

			if (p_type == PT_LOAD && va >= p_vaddr && va < p_vaddr + p_filesz)
			{
				return p_offset + (va - p_vaddr);
			}
		}

		return 0;
	}

	cpp_abi detect_elf_abi(const std::vector<uint8_t>& data)
	{
		const std::size_t n = data.size();
		const uint8_t*    p = data.data();

		// --- ELF magic ---
		static constexpr uint8_t kElfMagic[4] = {0x7f, 'E', 'L', 'F'};
		if (n < 16 || std::memcmp(p, kElfMagic, 4) != 0) return cpp_abi::unknown;

		uint8_t ei_class = p[4]; // 1 = ELFCLASS32, 2 = ELFCLASS64
		uint8_t ei_data  = p[5]; // 1 = ELFDATA2LSB (LE), 2 = ELFDATA2MSB (BE)

		// Only support 64-bit little-endian ELF (x86_64, aarch64, riscv64).
		// 32-bit or big-endian ELF returns unknown rather than a false negative.
		if (ei_class != ELFCLASS64 || ei_data != ELFDATA2LSB)
		{
			return cpp_abi::unknown;
		}

		// --- Find PT_DYNAMIC segment ---
		auto [dyn_offset, dyn_size] = elf64_find_dynamic(p, n);
		if (dyn_offset == 0 || dyn_size == 0) return cpp_abi::c_only;
		if (dyn_offset + dyn_size > n)        return cpp_abi::unknown;

		// --- Walk Elf64_Dyn entries (each 16 bytes: int64 tag + int64 val) ---
		// First pass: collect DT_NEEDED string offsets and DT_STRTAB address.
		std::vector<uint64_t> needed_offsets;
		uint64_t strtab_va = 0;

		for (std::size_t pos = dyn_offset; pos + 16 <= dyn_offset + dyn_size && pos + 16 <= n; pos += 16)
		{
			int64_t  d_tag = static_cast<int64_t>(read_u64le(p + pos));
			uint64_t d_val = read_u64le(p + pos + 8);

			if (d_tag == DT_NULL) break;
			if (d_tag == DT_NEEDED) needed_offsets.push_back(d_val);
			if (d_tag == DT_STRTAB) strtab_va = d_val;
		}

		if (needed_offsets.empty()) return cpp_abi::c_only;

		// --- Resolve DT_STRTAB virtual address to file offset ---
		uint64_t strtab_off = elf64_va_to_offset(p, n, strtab_va);
		if (strtab_off == 0) return cpp_abi::unknown;

		// --- Check each DT_NEEDED name ---
		for (uint64_t str_idx : needed_offsets)
		{
			uint64_t name_off = strtab_off + str_idx;
			if (name_off >= n) continue;

			// Read null-terminated library name
			std::string lib_name;
			lib_name.reserve(64);
			for (std::size_t i = 0; i < 256 && name_off + i < n; ++i)
			{
				char c = static_cast<char>(p[name_off + i]);
				if (c == '\0') break;
				lib_name += c;
			}

			// libstdc++ (GCC) and libc++ (Clang) both use Itanium ABI
			if (lib_name.find("libstdc++") != std::string::npos ||
			    lib_name.find("libc++")    != std::string::npos)
			{
				return cpp_abi::itanium;
			}
		}

		return cpp_abi::c_only;
	}
} // anonymous namespace
#endif // __linux__


// ============================================================================
// Public API
// ============================================================================

cpp_abi detect_module_abi(const std::string& path)
{
	// Read enough of the file to cover headers and import/dynamic sections.
	// 4 MB is generous for metadata; most shared libraries fit entirely within this.
	constexpr std::size_t kMaxBytes = 4u * 1024u * 1024u;
	std::vector<uint8_t> data = read_file_head(path, kMaxBytes);

#ifdef _WIN32
	return detect_pe_abi(data);
#elif defined(__linux__)
	return detect_elf_abi(data);
#else
	(void)data;
	return cpp_abi::unknown;
#endif
}
