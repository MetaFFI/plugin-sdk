/**
 * test_lib_cpp.cpp — Minimal C++ shared library for ABI detection tests.
 *
 * Uses std::string / std::to_string to force a C++ runtime dependency in
 * the import table (VCRUNTIME140.dll on MSVC, libstdc++.so on GCC).
 * detect_module_abi() should therefore classify this library as 'msvc' or
 * 'itanium', matching the plugin ABI used at build time.
 */

#include <string>

#ifdef _WIN32
#   define EXPORT __declspec(dllexport)
#else
#   define EXPORT __attribute__((visibility("default")))
#endif

extern "C"
{
    EXPORT int cpp_identity(int x)
    {
        // std::string / std::to_string pull in the C++ runtime, forcing a
        // non-c_only ABI classification by detect_module_abi().
        std::string s = std::to_string(x);
        volatile int len = static_cast<int>(s.size());
        (void)len;
        return x;
    }
}
