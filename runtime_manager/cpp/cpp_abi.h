#pragma once

#include <string>

/**
 * C++ ABI variants known to the runtime manager.
 *
 * Two ABIs exist in the wild:
 *   - Itanium: used by GCC and Clang on Linux/macOS; also MinGW on Windows.
 *   - MSVC: used by the Microsoft compiler on Windows.
 *
 * They are name-mangling compatible within each family but have incompatible
 * std::string / std::vector memory layouts, so mixing them at runtime crashes.
 */
enum class cpp_abi
{
	unknown,  // Cannot determine ABI from binary
	c_only,   // No C++ runtime dependency — pure C, ABI-neutral
	itanium,  // Itanium C++ ABI (GCC/Clang; also MinGW on Windows)
	msvc      // MSVC ABI (Visual C++ compiler)
};

/**
 * Return the ABI this plugin binary was compiled with.
 * This is a compile-time constant determined by preprocessor macros.
 */
cpp_abi get_plugin_abi();

/**
 * Detect the C++ ABI of a shared library by inspecting its binary.
 *
 * Detection strategy:
 *   Windows / PE  : scan the import directory for VCRUNTIME*.dll / MSVCP*.dll
 *                   (→ msvc) or libstdc++*.dll / libgcc*.dll (→ itanium).
 *                   No C++ runtime import → c_only.
 *   Linux  / ELF  : scan DT_NEEDED entries for libstdc++.so (→ itanium) or
 *                   libc++.so (→ itanium). Neither → c_only.
 *   Other         : cpp_abi::unknown.
 *
 * @param path   Path to a shared library (.dll / .so / .dylib)
 * @return       Detected ABI
 * @throws       std::runtime_error if the file cannot be opened or is too small
 */
cpp_abi detect_module_abi(const std::string& path);

/**
 * Return a human-readable name for a cpp_abi value.
 * Never returns nullptr.
 */
const char* cpp_abi_name(cpp_abi abi);
