#pragma once

// ============================================================================
// metaffi.h — Single header-only MetaFFI C++ API
//
// Usage:
//   #include <metaffi/metaffi.h>
//   Compile with: -std=c++17 -I$METAFFI_HOME/include -I$METAFFI_HOME/cpp/api
//
// This header replaces metaffi.api.cpp.dll — no DLL linking required.
// All cross-DLL memory goes through xllr's own alloc/free (loaded from xllr.dll).
// ============================================================================

// Prevent Windows.h from defining min/max macros (conflicts with std::numeric_limits etc.)
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

// --- Existing declaration headers ---
// These provide struct/class declarations and type definitions.
// Our header adds inline implementations below.
#include <metaffi/api/metaffi_api.h>
// metaffi_api.h transitively includes:
//   runtime/metaffi_primitives.h, runtime/cdt.h, runtime/xcall.h,
//   runtime/xllr_capi_loader.h, utils/env_utils.h, utils/expand_env.h,
//   utils/entity_path_parser.h, cdts_serializer/cpp/cdts_cpp_serializer.h

// Runtime ID for C++ handles
#ifndef CPP_RUNTIME_ID
#define CPP_RUNTIME_ID 5423631039
#endif

// --- Standard library (for inline implementations) ---
#include <algorithm>
#include <cstring>
#include <iostream>
#include <mutex>
#include <regex>

// --- Platform (for DLL loading) ---
#ifdef _WIN32
// Windows.h already included by xllr_capi_loader.h
#else
#include <dlfcn.h>
#include <unistd.h>
#endif


// ============================================================================
// Section 0: Inline cdts/cdt method implementations
//
// These are declared in cdt.h but defined in cdt.cpp.
// For header-only usage, we provide inline definitions here.
// ============================================================================

inline cdts::cdts(metaffi_size length, metaffi_int64 fixed_dimensions)
	: arr(nullptr), length(length), fixed_dimensions(fixed_dimensions), allocated_on_cache(0)
{
	if(length > 0)
	{
		arr = new cdt[length]{};
	}
}

inline cdt& cdts::operator[](metaffi_size index) const
{
	return arr[index];
}

inline cdt& cdts::at(metaffi_size index) const
{
	return arr[index];
}

inline void cdts::set(metaffi_size index, cdt&& val) const
{
	arr[index].type = val.type;
	arr[index].free_required = val.free_required;
	arr[index].cdt_val = val.cdt_val;

	val.type = metaffi_null_type;
	val.free_required = 0;
	std::memset(&val.cdt_val, 0, sizeof(val.cdt_val));
}

inline void cdts::free()
{
	if(!arr)
	{
		return;
	}

	// if NOT allocated on cache (i.e. array)
	if(!this->allocated_on_cache)
	{
		delete[] arr;
		arr = nullptr;
	}
	else
	{
		for(metaffi_size i = 0; i < length; i++)
		{
			arr[i].~cdt();
		}
	}

	arr = nullptr;
}

inline cdts::~cdts()
{
	free();
}


// ============================================================================
// Section 1: Inline XLLR Loader
//
// Replaces xllr_capi_loader.c for C++ consumers.
// Uses C++17 inline variables for single-instance state across TUs.
// ============================================================================

namespace metaffi::detail
{

// Platform-specific DLL loading helpers (internal)

inline void* platform_load_library(const char* path, std::string& out_err)
{
#ifdef _WIN32
	void* handle = LoadLibraryA(path);
	if(!handle)
	{
		DWORD err_code = GetLastError();
		char* msg = nullptr;
		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPSTR>(&msg), 0, nullptr);
		out_err = msg ? msg : "Unknown error loading library";
		if(msg) LocalFree(msg);
	}
	return handle;
#else
	void* handle = dlopen(path, RTLD_GLOBAL | RTLD_NOW);
	if(!handle)
	{
		const char* err = dlerror();
		out_err = err ? err : "Unknown dlopen error";
	}
	return handle;
#endif
}

inline const char* platform_free_library(void* handle)
{
	if(!handle) return nullptr;
#ifdef _WIN32
	if(!FreeLibrary(static_cast<HMODULE>(handle)))
	{
		return "Failed to free library";
	}
	return nullptr;
#else
	if(dlclose(handle))
	{
		return dlerror();
	}
	return nullptr;
#endif
}

inline void* platform_load_symbol(void* handle, const char* name, std::string& out_err)
{
#ifdef _WIN32
	void* sym = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle), name));
	if(!sym)
	{
		DWORD err_code = GetLastError();
		char* msg = nullptr;
		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPSTR>(&msg), 0, nullptr);
		out_err = "Failed to load symbol '";
		out_err += name;
		out_err += "': ";
		out_err += msg ? msg : "unknown error";
		if(msg) LocalFree(msg);
	}
	return sym;
#else
	void* sym = dlsym(handle, name);
	if(!sym)
	{
		const char* err = dlerror();
		out_err = "Failed to load symbol '";
		out_err += name;
		out_err += "': ";
		out_err += err ? err : "unknown error";
	}
	return sym;
#endif
}

// XLLR function pointer table — inline variable ensures single instance across all TUs
struct xllr_api
{
	std::once_flag init_flag;
	void* lib_handle = nullptr;

	// xcall functions
	void (*p_xcall_params_ret)(struct xcall*, struct cdts[2], char**) = nullptr;
	void (*p_xcall_no_params_ret)(struct xcall*, struct cdts[2], char**) = nullptr;
	void (*p_xcall_params_no_ret)(struct xcall*, struct cdts[2], char**) = nullptr;
	void (*p_xcall_no_params_no_ret)(struct xcall*, char**) = nullptr;

	// Entity loading
	struct xcall* (*p_load_entity)(const char*, const char*, const char*,
	                               struct metaffi_type_info*, int8_t,
	                               struct metaffi_type_info*, int8_t,
	                               char**) = nullptr;
	struct xcall* (*p_load_callable)(void*,
	                                 struct metaffi_type_info*, int8_t,
	                                 struct metaffi_type_info*, int8_t,
	                                 char**) = nullptr;
	void (*p_free_xcall)(const char*, void*, char**) = nullptr;

	// Memory management
	void* (*p_alloc_memory)(uint64_t) = nullptr;
	void (*p_free_memory)(void*) = nullptr;
	struct cdt* (*p_alloc_cdt_array)(uint64_t) = nullptr;
	void (*p_free_cdt_array)(struct cdt*) = nullptr;

	// String allocation
	char* (*p_alloc_string)(const char*, uint64_t) = nullptr;
	char8_t* (*p_alloc_string8)(const char8_t*, uint64_t) = nullptr;
	char16_t* (*p_alloc_string16)(const char16_t*, uint64_t) = nullptr;
	char32_t* (*p_alloc_string32)(const char32_t*, uint64_t) = nullptr;
	void (*p_free_string)(char*) = nullptr;

	// Runtime plugin management
	void (*p_load_runtime_plugin)(const char*, char**) = nullptr;
	void (*p_free_runtime_plugin)(const char*, char**) = nullptr;

	// Runtime flags
	void (*p_set_runtime_flag)(const char*) = nullptr;
	int (*p_is_runtime_flag_set)(const char*) = nullptr;

	// CDTS buffer management
	struct cdts* (*p_alloc_cdts_buffer)(metaffi_size, metaffi_size) = nullptr;
	void (*p_free_cdts_buffer)(struct cdts*) = nullptr;

	// Construct/traverse
	void (*p_construct_cdts)(struct cdts*, struct construct_cdts_callbacks*, char**) = nullptr;
	void (*p_construct_cdt)(struct cdt*, struct construct_cdts_callbacks*, char**) = nullptr;
	void (*p_traverse_cdts)(struct cdts*, struct traverse_cdts_callbacks*, char**) = nullptr;
	void (*p_traverse_cdt)(struct cdt*, struct traverse_cdts_callbacks*, char**) = nullptr;

	void ensure_loaded()
	{
		std::call_once(init_flag, [this]()
		{
			// Get METAFFI_HOME
#ifdef _WIN32
			char* home_tmp = nullptr;
			size_t home_size = 0;
			if(_dupenv_s(&home_tmp, &home_size, "METAFFI_HOME") != 0 || home_tmp == nullptr)
			{
				throw std::runtime_error("METAFFI_HOME environment variable is not set");
			}
			std::string metaffi_home(home_tmp);
			free(home_tmp);
#else
			const char* home_tmp = std::getenv("METAFFI_HOME");
			if(!home_tmp)
			{
				throw std::runtime_error("METAFFI_HOME environment variable is not set");
			}
			std::string metaffi_home(home_tmp);
#endif

			// Build xllr library path
#ifdef _WIN32
			std::string xllr_path = metaffi_home + "\\xllr.dll";
#elif defined(__APPLE__)
			std::string xllr_path = metaffi_home + "/xllr.dylib";
#else
			std::string xllr_path = metaffi_home + "/xllr.so";
#endif

			// Load the library
			std::string load_err;
			lib_handle = platform_load_library(xllr_path.c_str(), load_err);
			if(!lib_handle)
			{
				throw std::runtime_error("Failed to load xllr library (" + xllr_path + "): " + load_err);
			}

			// Helper: load a required symbol or throw
			auto load = [this](const char* name) -> void*
			{
				std::string err;
				void* sym = platform_load_symbol(lib_handle, name, err);
				if(!sym)
				{
					throw std::runtime_error(err);
				}
				return sym;
			};

			// Load all symbols (names match xllr.dll exports)
			p_xcall_params_ret = reinterpret_cast<decltype(p_xcall_params_ret)>(load("xcall_params_ret"));
			p_xcall_no_params_ret = reinterpret_cast<decltype(p_xcall_no_params_ret)>(load("xcall_no_params_ret"));
			p_xcall_params_no_ret = reinterpret_cast<decltype(p_xcall_params_no_ret)>(load("xcall_params_no_ret"));
			p_xcall_no_params_no_ret = reinterpret_cast<decltype(p_xcall_no_params_no_ret)>(load("xcall_no_params_no_ret"));

			p_load_entity = reinterpret_cast<decltype(p_load_entity)>(load("load_entity"));
			p_free_xcall = reinterpret_cast<decltype(p_free_xcall)>(load("free_xcall"));

			p_alloc_cdt_array = reinterpret_cast<decltype(p_alloc_cdt_array)>(load("alloc_cdt_array"));
			p_free_cdt_array = reinterpret_cast<decltype(p_free_cdt_array)>(load("free_cdt_array"));
			p_alloc_memory = reinterpret_cast<decltype(p_alloc_memory)>(load("alloc_memory"));
			p_free_memory = reinterpret_cast<decltype(p_free_memory)>(load("free_memory"));

			p_free_string = reinterpret_cast<decltype(p_free_string)>(load("free_string"));
			p_alloc_string = reinterpret_cast<decltype(p_alloc_string)>(load("alloc_string"));
			p_alloc_string8 = reinterpret_cast<decltype(p_alloc_string8)>(load("alloc_string8"));
			p_alloc_string16 = reinterpret_cast<decltype(p_alloc_string16)>(load("alloc_string16"));
			p_alloc_string32 = reinterpret_cast<decltype(p_alloc_string32)>(load("alloc_string32"));

			p_load_runtime_plugin = reinterpret_cast<decltype(p_load_runtime_plugin)>(load("load_runtime_plugin"));
			p_free_runtime_plugin = reinterpret_cast<decltype(p_free_runtime_plugin)>(load("free_runtime_plugin"));

			p_is_runtime_flag_set = reinterpret_cast<decltype(p_is_runtime_flag_set)>(load("is_runtime_flag_set"));
			p_set_runtime_flag = reinterpret_cast<decltype(p_set_runtime_flag)>(load("set_runtime_flag"));

			p_alloc_cdts_buffer = reinterpret_cast<decltype(p_alloc_cdts_buffer)>(load("alloc_cdts_buffer"));
			p_free_cdts_buffer = reinterpret_cast<decltype(p_free_cdts_buffer)>(load("free_cdts_buffer"));

			p_construct_cdts = reinterpret_cast<decltype(p_construct_cdts)>(load("construct_cdts"));
			p_construct_cdt = reinterpret_cast<decltype(p_construct_cdt)>(load("construct_cdt"));
			p_traverse_cdts = reinterpret_cast<decltype(p_traverse_cdts)>(load("traverse_cdts"));
			p_traverse_cdt = reinterpret_cast<decltype(p_traverse_cdt)>(load("traverse_cdt"));

			// load_callable is optional (may not be in all xllr versions)
			std::string ignored_err;
			p_load_callable = reinterpret_cast<decltype(p_load_callable)>(
				platform_load_symbol(lib_handle, "load_callable", ignored_err));
		});
	}
};

inline xllr_api g_xllr;

} // namespace metaffi::detail


// Inline implementations for all extern "C" functions declared in xllr_capi_loader.h.
// These replace the implementations in xllr_capi_loader.c for header-only consumers.

extern "C"
{

inline int64_t get_cache_size()
{
	return cdt_cache_size;
}

inline void xllr_xcall_params_ret(struct xcall* pxcall, struct cdts params_ret[2], char** out_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_xcall_params_ret(pxcall, params_ret, out_err);
}

inline void xllr_xcall_no_params_ret(struct xcall* pxcall, struct cdts params_ret[2], char** out_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_xcall_no_params_ret(pxcall, params_ret, out_err);
}

inline void xllr_xcall_params_no_ret(struct xcall* pxcall, struct cdts params_ret[2], char** out_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_xcall_params_no_ret(pxcall, params_ret, out_err);
}

inline void xllr_xcall_no_params_no_ret(struct xcall* pxcall, char** out_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_xcall_no_params_no_ret(pxcall, out_err);
}

inline struct xcall* xllr_load_entity(const char* runtime_plugin,
                                      const char* module_path,
                                      const char* entity_path,
                                      struct metaffi_type_info* params_types, int8_t params_count,
                                      struct metaffi_type_info* retvals_types, int8_t retval_count,
                                      char** out_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	return metaffi::detail::g_xllr.p_load_entity(runtime_plugin, module_path, entity_path,
	                                              params_types, params_count,
	                                              retvals_types, retval_count, out_err);
}

inline struct xcall* xllr_load_callable(void* make_callable_context,
                                        struct metaffi_type_info* params_types, int8_t params_count,
                                        struct metaffi_type_info* retvals_types, int8_t retval_count,
                                        char** out_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	if(!metaffi::detail::g_xllr.p_load_callable)
	{
		if(out_err)
		{
			static const char msg[] = "load_callable not available in this xllr version";
			*out_err = const_cast<char*>(msg);
		}
		return nullptr;
	}
	return metaffi::detail::g_xllr.p_load_callable(make_callable_context,
	                                                params_types, params_count,
	                                                retvals_types, retval_count, out_err);
}

inline void xllr_free_xcall(const char* runtime_plugin, struct xcall* pxcall, char** out_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_free_xcall(runtime_plugin, pxcall, out_err);
}

inline char* xllr_alloc_string(const char* err_message, uint64_t length)
{
	metaffi::detail::g_xllr.ensure_loaded();
	return metaffi::detail::g_xllr.p_alloc_string(err_message, length);
}

inline char8_t* xllr_alloc_string8(const char8_t* err_message, uint64_t length)
{
	metaffi::detail::g_xllr.ensure_loaded();
	return metaffi::detail::g_xllr.p_alloc_string8(err_message, length);
}

inline char16_t* xllr_alloc_string16(const char16_t* err_message, uint64_t length)
{
	metaffi::detail::g_xllr.ensure_loaded();
	return metaffi::detail::g_xllr.p_alloc_string16(err_message, length);
}

inline char32_t* xllr_alloc_string32(const char32_t* err_message, uint64_t length)
{
	metaffi::detail::g_xllr.ensure_loaded();
	return metaffi::detail::g_xllr.p_alloc_string32(err_message, length);
}

inline void xllr_free_string(char* err_to_free)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_free_string(err_to_free);
}

inline void* xllr_alloc_memory(uint64_t size)
{
	metaffi::detail::g_xllr.ensure_loaded();
	return metaffi::detail::g_xllr.p_alloc_memory(size);
}

inline void xllr_free_memory(void* ptr)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_free_memory(ptr);
}

inline struct cdt* xllr_alloc_cdt_array(uint64_t count)
{
	metaffi::detail::g_xllr.ensure_loaded();
	return metaffi::detail::g_xllr.p_alloc_cdt_array(count);
}

inline void xllr_free_cdt_array(struct cdt* arr)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_free_cdt_array(arr);
}

inline void xllr_load_runtime_plugin(const char* runtime_plugin, char** err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_load_runtime_plugin(runtime_plugin, err);
}

inline void xllr_free_runtime_plugin(const char* runtime_plugin, char** err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_free_runtime_plugin(runtime_plugin, err);
}

inline void xllr_set_runtime_flag(const char* flag_name)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_set_runtime_flag(flag_name);
}

inline int xllr_is_runtime_flag_set(const char* flag_name)
{
	metaffi::detail::g_xllr.ensure_loaded();
	return metaffi::detail::g_xllr.p_is_runtime_flag_set(flag_name);
}

inline struct cdts* xllr_alloc_cdts_buffer(metaffi_size params, metaffi_size rets)
{
	metaffi::detail::g_xllr.ensure_loaded();
	return metaffi::detail::g_xllr.p_alloc_cdts_buffer(params, rets);
}

inline void xllr_free_cdts_buffer(struct cdts* pcdts)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_free_cdts_buffer(pcdts);
}

inline void xllr_construct_cdts(struct cdts* pcdts, struct construct_cdts_callbacks* callbacks, char** out_nul_term_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_construct_cdts(pcdts, callbacks, out_nul_term_err);
}

inline void xllr_construct_cdt(struct cdt* pcdt, struct construct_cdts_callbacks* callbacks, char** out_nul_term_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_construct_cdt(pcdt, callbacks, out_nul_term_err);
}

inline void xllr_traverse_cdts(struct cdts* pcdts, struct traverse_cdts_callbacks* callbacks, char** out_nul_term_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_traverse_cdts(pcdts, callbacks, out_nul_term_err);
}

inline void xllr_traverse_cdt(struct cdt* pcdt, struct traverse_cdts_callbacks* callbacks, char** out_nul_term_err)
{
	metaffi::detail::g_xllr.ensure_loaded();
	metaffi::detail::g_xllr.p_traverse_cdt(pcdt, callbacks, out_nul_term_err);
}

// Loader management functions

#ifdef _WIN32
inline void get_last_error_string(DWORD err, char** out_err_str)
{
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	              NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	              (LPTSTR)out_err_str, 0, NULL);
}
#endif

inline void* load_library(const char* name, char** out_err)
{
	std::string err;
	void* handle = metaffi::detail::platform_load_library(name, err);
	if(!handle && out_err)
	{
		// Allocate error string the caller can free
		*out_err = static_cast<char*>(malloc(err.size() + 1));
		if(*out_err) std::memcpy(*out_err, err.c_str(), err.size() + 1);
	}
	return handle;
}

inline const char* free_library(void* lib)
{
	return metaffi::detail::platform_free_library(lib);
}

inline void* load_symbol(void* handle, const char* name, char** out_err)
{
	std::string err;
	void* sym = metaffi::detail::platform_load_symbol(handle, name, err);
	if(!sym && out_err)
	{
		*out_err = static_cast<char*>(malloc(err.size() + 1));
		if(*out_err) std::memcpy(*out_err, err.c_str(), err.size() + 1);
	}
	return sym;
}

inline const char* load_xllr_capi()
{
	// In header-only mode, ensure_loaded() handles symbol loading
	try
	{
		metaffi::detail::g_xllr.ensure_loaded();
		return nullptr;
	}
	catch(const std::exception& e)
	{
		// Return static error — caller must not free
		static thread_local std::string err_storage;
		err_storage = e.what();
		return err_storage.c_str();
	}
}

inline const char* load_xllr()
{
	return load_xllr_capi();
}

inline const char* free_xllr()
{
	auto& api = metaffi::detail::g_xllr;
	if(api.lib_handle)
	{
		const char* err = metaffi::detail::platform_free_library(api.lib_handle);
		api.lib_handle = nullptr;
		return err;
	}
	return nullptr;
}

inline const char* load_metaffi_library(const char* path_within_metaffi_home, void** out_handle)
{
#ifdef _WIN32
	char* home_tmp = nullptr;
	size_t home_size = 0;
	if(_dupenv_s(&home_tmp, &home_size, "METAFFI_HOME") != 0 || home_tmp == nullptr)
	{
		return "Failed getting METAFFI_HOME. Is it set?";
	}
	std::string metaffi_home(home_tmp);
	free(home_tmp);
	const char* ext = ".dll";
	const char sep = '\\';
#elif defined(__APPLE__)
	const char* home_tmp = std::getenv("METAFFI_HOME");
	if(!home_tmp) return "Failed getting METAFFI_HOME. Is it set?";
	std::string metaffi_home(home_tmp);
	const char* ext = ".dylib";
	const char sep = '/';
#else
	const char* home_tmp = std::getenv("METAFFI_HOME");
	if(!home_tmp) return "Failed getting METAFFI_HOME. Is it set?";
	std::string metaffi_home(home_tmp);
	const char* ext = ".so";
	const char sep = '/';
#endif

	std::string full_path = metaffi_home + sep + path_within_metaffi_home + ext;

	std::string err;
	*out_handle = metaffi::detail::platform_load_library(full_path.c_str(), err);
	if(!*out_handle)
	{
		static thread_local std::string err_storage;
		err_storage = err;
		return err_storage.c_str();
	}
	return nullptr;
}

} // extern "C"


// ============================================================================
// Section 2: Inline Utility Implementations
// Replaces env_utils.cpp, expand_env.cpp, entity_path_parser.cpp (no Boost)
// ============================================================================

// --- get_env_var (replaces env_utils.cpp) ---

inline std::string get_env_var(const char* name)
{
#ifdef _WIN32
	char* value = nullptr;
	size_t value_size = 0;
	if(_dupenv_s(&value, &value_size, name) != 0 || value == nullptr)
	{
		return {};
	}
	std::string result(value);
	free(value);
	return result;
#else
	const char* val = std::getenv(name);
	return val ? std::string(val) : std::string();
#endif
}

// --- expand_env (replaces expand_env.cpp, no Boost) ---

namespace metaffi::utils
{

namespace detail_expand
{

inline bool iequals(const std::string& a, const std::string& b)
{
	return std::equal(a.begin(), a.end(), b.begin(), b.end(),
	                  [](char x, char y) { return std::tolower(x) == std::tolower(y); });
}

// Simple find-and-replace-all without Boost
inline void replace_all(std::string& str, const std::string& from, const std::string& to)
{
	if(from.empty()) return;
	std::string::size_type pos = 0;
	while((pos = str.find(from, pos)) != std::string::npos)
	{
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
}

} // namespace detail_expand

inline std::string expand_env(const std::string& str)
{
	static std::regex re(
		R"(%([A-Z0-9_\(\){}\[\]\$\*\+\\\/\"#',;\.!@\?-]+)%|\$ENV:([A-Z0-9_]+)|\$([A-Z_]{1}[A-Z0-9_]+)|\$\{([^}]+)\})",
		std::regex::icase);
	static std::string cur_dir_win("CD");
	static std::string cur_dir_nix("PWD");

	std::string working_copy_str = str;
	std::string res = str;

	for(std::smatch m; std::regex_search(working_copy_str, m, re); working_copy_str = m.suffix())
	{
		std::string env_var_name;
		if(m[1].matched) env_var_name = m[1].str();
		else if(m[2].matched) env_var_name = m[2].str();
		else if(m[3].matched) env_var_name = m[3].str();
		else if(m[4].matched) env_var_name = m[4].str();

		if((env_var_name.length() == 2 && detail_expand::iequals(env_var_name, cur_dir_win)) ||
		   (env_var_name.length() == 3 && detail_expand::iequals(env_var_name, cur_dir_nix)))
		{
			char cwd[1024] = {0};
#ifdef _WIN32
			GetCurrentDirectory(1024, cwd);
#else
			getcwd(cwd, 1024);
#endif
			detail_expand::replace_all(res, m.str(), cwd);
		}
		else
		{
			std::string tmp = get_env_var(env_var_name.c_str());
			if(!tmp.empty())
			{
				detail_expand::replace_all(res, m.str(), tmp);
			}
		}
	}

	return res;
}

// --- entity_path_parser (replaces entity_path_parser.cpp, no Boost) ---

namespace detail_parser
{

// Simple split by delimiter without Boost
inline std::vector<std::string> split(const std::string& str, char delimiter)
{
	std::vector<std::string> result;
	std::string::size_type start = 0;
	std::string::size_type end = str.find(delimiter);

	while(end != std::string::npos)
	{
		result.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(delimiter, start);
	}
	result.push_back(str.substr(start));
	return result;
}

} // namespace detail_parser

inline entity_path_parser::entity_path_parser(const std::string& entity_path)
{
	auto items = detail_parser::split(entity_path, ',');

	for(const std::string& item : items)
	{
		if(item.empty()) continue;

		auto keyval = detail_parser::split(item, '=');

		if(keyval.size() > 2)
		{
			throw std::runtime_error("function path is invalid, too many '='");
		}

		if(keyval.size() == 1)
		{
			entity_path_items[keyval[0]] = "";
		}
		else
		{
			// Expand environment variables in value
			std::string exp = expand_env(keyval[1]);
			entity_path_items[keyval[0]] = exp;
		}
	}
}

inline std::string entity_path_parser::operator[](const std::string& key) const
{
	auto i = entity_path_items.find(key);
	if(i == entity_path_items.end())
	{
		return "";
	}
	return i->second;
}

inline bool entity_path_parser::contains(const std::string& key) const
{
	return entity_path_items.find(key) != entity_path_items.end();
}

} // namespace metaffi::utils


// ============================================================================
// Section 3: Inline CDTS C++ Serializer
// Replaces cdts_cpp_serializer.cpp
// ============================================================================

namespace metaffi::utils
{

// --- Constructor ---

inline cdts_cpp_serializer::cdts_cpp_serializer(cdts& pcdts)
	: data(pcdts), current_index(0)
{
}

// --- Helper ---

inline void cdts_cpp_serializer::check_bounds(metaffi_size index) const
{
	if(index >= data.length)
	{
		std::stringstream ss;
		ss << "Index out of bounds: " << index << " >= " << data.length;
		throw std::out_of_range(ss.str());
	}
}

// --- Serialization (C++ -> CDT) ---

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(int8_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(int16_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(int32_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(int64_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(uint8_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(uint16_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(uint32_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(uint64_t val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(float val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(double val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(bool val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

#if defined(__linux__)
inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(long long val)
{
	return (*this) << static_cast<int64_t>(val);
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(unsigned long long val)
{
	return (*this) << static_cast<uint64_t>(val);
}
#endif

// Strings

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const std::string& val)
{
	check_bounds(current_index);
	data[current_index].set_string((const char8_t*)val.c_str(), true);
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const std::u16string& val)
{
	check_bounds(current_index);
	data[current_index].set_string(val.c_str(), true);
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const std::u32string& val)
{
	check_bounds(current_index);
	data[current_index].set_string(val.c_str(), true);
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const char* val)
{
	check_bounds(current_index);
	data[current_index].set_string((const char8_t*)val, true);
	current_index++;
	return *this;
}

// Characters

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const metaffi_char8& val)
{
	check_bounds(current_index);
	data[current_index] = val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const metaffi_char16& val)
{
	check_bounds(current_index);
	data[current_index].type = metaffi_char16_type;
	data[current_index].cdt_val.char16_val = val;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const metaffi_char32& val)
{
	check_bounds(current_index);
	data[current_index].type = metaffi_char32_type;
	data[current_index].cdt_val.char32_val = val;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

// Handles

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(metaffi_handle val)
{
	check_bounds(current_index);

	void* handle_mem = xllr_alloc_memory(sizeof(cdt_metaffi_handle));
	if(!handle_mem)
	{
		throw std::runtime_error("Failed to allocate memory for cdt_metaffi_handle");
	}

	cdt_metaffi_handle* handle = new (handle_mem) cdt_metaffi_handle();
	handle->handle = val;
	handle->runtime_id = CPP_RUNTIME_ID;
	handle->release = nullptr;

	data[current_index].type = metaffi_handle_type;
	data[current_index].cdt_val.handle_val = handle;
	data[current_index].free_required = true;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const cdt_metaffi_handle& handle)
{
	check_bounds(current_index);
	data[current_index].set_handle(&handle);
	current_index++;
	return *this;
}

// Callables

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const cdt_metaffi_callable& callable)
{
	check_bounds(current_index);

	// Create a copy with xllr-allocated arrays
	void* callable_mem = xllr_alloc_memory(sizeof(cdt_metaffi_callable));
	if(!callable_mem)
	{
		throw std::runtime_error("Failed to allocate callable memory");
	}
	cdt_metaffi_callable* callable_copy = new (callable_mem) cdt_metaffi_callable();
	callable_copy->val = callable.val;
	callable_copy->params_types_length = callable.params_types_length;
	callable_copy->retval_types_length = callable.retval_types_length;

	// Allocate and copy parameter types
	if(callable.params_types_length > 0)
	{
		void* params_mem = xllr_alloc_memory(sizeof(metaffi_type) * callable.params_types_length);
		if(!params_mem)
		{
			xllr_free_memory(callable_mem);
			throw std::runtime_error("Failed to allocate callable parameters memory");
		}
		callable_copy->parameters_types = static_cast<metaffi_type*>(params_mem);
		std::memcpy(callable_copy->parameters_types, callable.parameters_types,
		            sizeof(metaffi_type) * callable.params_types_length);
	}
	else
	{
		callable_copy->parameters_types = nullptr;
	}

	// Allocate and copy return value types
	if(callable.retval_types_length > 0)
	{
		void* ret_mem = xllr_alloc_memory(sizeof(metaffi_type) * callable.retval_types_length);
		if(!ret_mem)
		{
			if(callable_copy->parameters_types) xllr_free_memory(callable_copy->parameters_types);
			xllr_free_memory(callable_mem);
			throw std::runtime_error("Failed to allocate callable return types memory");
		}
		callable_copy->retval_types = static_cast<metaffi_type*>(ret_mem);
		std::memcpy(callable_copy->retval_types, callable.retval_types,
		            sizeof(metaffi_type) * callable.retval_types_length);
	}
	else
	{
		callable_copy->retval_types = nullptr;
	}

	data[current_index].type = metaffi_callable_type;
	data[current_index].cdt_val.callable_val = callable_copy;
	data[current_index].free_required = true;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(std::nullptr_t)
{
	check_bounds(current_index);
	data[current_index].type = metaffi_null_type;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator<<(const metaffi_variant& val)
{
	check_bounds(current_index);

	std::visit([this](auto&& v)
	{
		using V = std::decay_t<decltype(v)>;
		if constexpr (std::is_same_v<V, metaffi_float32> || std::is_same_v<V, metaffi_float64> ||
		              std::is_same_v<V, metaffi_int8> || std::is_same_v<V, metaffi_uint8> ||
		              std::is_same_v<V, metaffi_int16> || std::is_same_v<V, metaffi_uint16> ||
		              std::is_same_v<V, metaffi_int32> || std::is_same_v<V, metaffi_uint32> ||
		              std::is_same_v<V, metaffi_int64> || std::is_same_v<V, metaffi_uint64> ||
		              std::is_same_v<V, metaffi_char8> || std::is_same_v<V, metaffi_char16> ||
		              std::is_same_v<V, metaffi_char32> || std::is_same_v<V, cdt_metaffi_handle> ||
		              std::is_same_v<V, cdt_metaffi_callable>)
		{
			*this << v;
		}
		else if constexpr (std::is_same_v<V, metaffi_string8>)
		{
			if(v == nullptr) { this->null(); }
			else { *this << std::string(reinterpret_cast<const char*>(v)); }
		}
		else if constexpr (std::is_same_v<V, metaffi_string16>)
		{
			if(v == nullptr) { this->null(); }
			else { *this << std::u16string(reinterpret_cast<const char16_t*>(v)); }
		}
		else if constexpr (std::is_same_v<V, metaffi_string32>)
		{
			if(v == nullptr) { this->null(); }
			else { *this << std::u32string(reinterpret_cast<const char32_t*>(v)); }
		}
		else
		{
			this->null();
		}
	}, val);

	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::null()
{
	check_bounds(current_index);
	data[current_index].type = metaffi_null_type;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

// --- Deserialization (CDT -> C++) ---

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(int8_t& val)     { validate_type_at<int8_t>(current_index);  val = static_cast<int8_t>(data[current_index]);  current_index++; return *this; }
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(int16_t& val)    { validate_type_at<int16_t>(current_index); val = static_cast<int16_t>(data[current_index]); current_index++; return *this; }
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(int32_t& val)    { validate_type_at<int32_t>(current_index); val = static_cast<int32_t>(data[current_index]); current_index++; return *this; }
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(int64_t& val)    { validate_type_at<int64_t>(current_index); val = static_cast<int64_t>(data[current_index]); current_index++; return *this; }
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(uint8_t& val)    { validate_type_at<uint8_t>(current_index);  val = static_cast<uint8_t>(data[current_index]);  current_index++; return *this; }
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(uint16_t& val)   { validate_type_at<uint16_t>(current_index); val = static_cast<uint16_t>(data[current_index]); current_index++; return *this; }
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(uint32_t& val)   { validate_type_at<uint32_t>(current_index); val = static_cast<uint32_t>(data[current_index]); current_index++; return *this; }
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(uint64_t& val)   { validate_type_at<uint64_t>(current_index); val = static_cast<uint64_t>(data[current_index]); current_index++; return *this; }
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(float& val)      { validate_type_at<float>(current_index);    val = static_cast<float>(data[current_index]);    current_index++; return *this; }
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(double& val)     { validate_type_at<double>(current_index);   val = static_cast<double>(data[current_index]);   current_index++; return *this; }
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(bool& val)       { validate_type_at<bool>(current_index);     val = static_cast<bool>(data[current_index]);     current_index++; return *this; }

#if defined(__linux__)
inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(long long& val)
{
	int64_t tmp = 0;
	(*this) >> tmp;
	val = static_cast<long long>(tmp);
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(unsigned long long& val)
{
	uint64_t tmp = 0;
	(*this) >> tmp;
	val = static_cast<unsigned long long>(tmp);
	return *this;
}
#endif

// Strings

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(std::string& val)
{
	validate_type_at<std::string>(current_index);
	val = std::string((const char*)data[current_index].cdt_val.string8_val);
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(std::u16string& val)
{
	validate_type_at<std::u16string>(current_index);
	val = std::u16string(data[current_index].cdt_val.string16_val);
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(std::u32string& val)
{
	validate_type_at<std::u32string>(current_index);
	val = std::u32string(data[current_index].cdt_val.string32_val);
	current_index++;
	return *this;
}

// Characters

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(metaffi_char8& val)
{
	validate_type_at<metaffi_char8>(current_index);
	val = data[current_index].cdt_val.char8_val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(metaffi_char16& val)
{
	validate_type_at<metaffi_char16>(current_index);
	val = data[current_index].cdt_val.char16_val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(metaffi_char32& val)
{
	validate_type_at<metaffi_char32>(current_index);
	val = data[current_index].cdt_val.char32_val;
	current_index++;
	return *this;
}

// Handles

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(cdt_metaffi_handle& handle)
{
	validate_type_at<cdt_metaffi_handle>(current_index);
	handle = *data[current_index].cdt_val.handle_val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(cdt_metaffi_handle*& handle)
{
	check_bounds(current_index);

	if(data[current_index].type == metaffi_null_type)
	{
		handle = nullptr;
		current_index++;
		return *this;
	}

	if(data[current_index].type != metaffi_handle_type)
	{
		std::stringstream ss;
		ss << "(cdts_cpp_serializer) Type mismatch at index " << current_index
		   << ": expected handle, got type " << data[current_index].type;
		throw std::runtime_error(ss.str());
	}
	handle = data[current_index].cdt_val.handle_val;
	data[current_index].cdt_val.handle_val = nullptr;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(metaffi_handle& val)
{
	check_bounds(current_index);

	if(data[current_index].type == metaffi_null_type)
	{
		val = nullptr;
		current_index++;
		return *this;
	}

	if(data[current_index].type != metaffi_handle_type)
	{
		std::stringstream ss;
		ss << "(cdts_cpp_serializer) Type mismatch at index " << current_index
		   << ": expected handle, got type " << data[current_index].type;
		throw std::runtime_error(ss.str());
	}

	cdt_metaffi_handle* handle = data[current_index].cdt_val.handle_val;
	if(handle == nullptr || handle->handle == nullptr)
	{
		val = nullptr;
		current_index++;
		return *this;
	}

	val = handle->handle;
	current_index++;
	return *this;
}

// Callables

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(cdt_metaffi_callable& callable)
{
	validate_type_at<cdt_metaffi_callable>(current_index);
	callable = *data[current_index].cdt_val.callable_val;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(cdt_metaffi_callable*& callable)
{
	check_bounds(current_index);
	if(data[current_index].type != metaffi_callable_type)
	{
		std::stringstream ss;
		ss << "(cdts_cpp_serializer) Type mismatch at index " << current_index
		   << ": expected callable, got type " << data[current_index].type;
		throw std::runtime_error(ss.str());
	}
	callable = data[current_index].cdt_val.callable_val;
	data[current_index].cdt_val.callable_val = nullptr;
	data[current_index].free_required = false;
	current_index++;
	return *this;
}

inline cdts_cpp_serializer& cdts_cpp_serializer::operator>>(metaffi_variant& val)
{
	val = extract_any();
	return *this;
}

// --- ANY type support ---

inline cdts_cpp_serializer::cdts_any_variant cdts_cpp_serializer::extract_any()
{
	check_bounds(current_index);
	metaffi_type type = data[current_index].type;

	if(type == metaffi_null_type)
	{
		current_index++;
		return cdt_metaffi_handle{};
	}

	if(type & metaffi_array_type)
	{
		throw std::runtime_error("Array extraction via extract_any() not yet implemented - use operator>> with vector type");
	}

	switch(type)
	{
		case metaffi_int8_type:    { auto v = data[current_index].cdt_val.int8_val;    current_index++; return v; }
		case metaffi_int16_type:   { auto v = data[current_index].cdt_val.int16_val;   current_index++; return v; }
		case metaffi_int32_type:   { auto v = data[current_index].cdt_val.int32_val;   current_index++; return v; }
		case metaffi_int64_type:   { auto v = data[current_index].cdt_val.int64_val;   current_index++; return v; }
		case metaffi_uint8_type:   { auto v = data[current_index].cdt_val.uint8_val;   current_index++; return v; }
		case metaffi_uint16_type:  { auto v = data[current_index].cdt_val.uint16_val;  current_index++; return v; }
		case metaffi_uint32_type:  { auto v = data[current_index].cdt_val.uint32_val;  current_index++; return v; }
		case metaffi_uint64_type:  { auto v = data[current_index].cdt_val.uint64_val;  current_index++; return v; }
		case metaffi_float32_type: { auto v = data[current_index].cdt_val.float32_val; current_index++; return v; }
		case metaffi_float64_type: { auto v = data[current_index].cdt_val.float64_val; current_index++; return v; }
		case metaffi_bool_type:    { metaffi_uint8 v = data[current_index].cdt_val.bool_val ? 1 : 0; current_index++; return v; }
		case metaffi_char8_type:   { auto v = data[current_index].cdt_val.char8_val;   current_index++; return v; }
		case metaffi_char16_type:  { auto v = data[current_index].cdt_val.char16_val;  current_index++; return v; }
		case metaffi_char32_type:  { auto v = data[current_index].cdt_val.char32_val;  current_index++; return v; }
		case metaffi_string8_type:
		{
			cdt& source = data[current_index];
			metaffi_string8 v = source.cdt_val.string8_val;
			source.free_required = false;
			current_index++;
			return v;
		}
		case metaffi_string16_type:
		{
			cdt& source = data[current_index];
			metaffi_string16 v = source.cdt_val.string16_val;
			source.free_required = false;
			current_index++;
			return v;
		}
		case metaffi_string32_type:
		{
			cdt& source = data[current_index];
			metaffi_string32 v = source.cdt_val.string32_val;
			source.free_required = false;
			current_index++;
			return v;
		}
		case metaffi_handle_type:
		{
			cdt_metaffi_handle v;
			*this >> v;
			return v;
		}
		case metaffi_callable_type:
		{
			cdt_metaffi_callable v;
			*this >> v;
			return v;
		}
		default:
		{
			std::stringstream ss;
			ss << "Unknown type at index " << current_index << ": " << type;
			throw std::runtime_error(ss.str());
		}
	}
}

inline metaffi_type cdts_cpp_serializer::peek_type() const  { check_bounds(current_index); return data[current_index].type; }
inline bool cdts_cpp_serializer::is_null() const            { check_bounds(current_index); return data[current_index].type == metaffi_null_type; }
inline void cdts_cpp_serializer::reset()                    { current_index = 0; }
inline metaffi_size cdts_cpp_serializer::get_index() const  { return current_index; }
inline void cdts_cpp_serializer::set_index(metaffi_size i)  { current_index = i; }
inline metaffi_size cdts_cpp_serializer::size() const       { return data.length; }
inline bool cdts_cpp_serializer::has_more() const           { return current_index < data.length; }

} // namespace metaffi::utils


// ============================================================================
// Section 4: Inline MetaFFI API
// Replaces metaffi_api.cpp (no spdlog dependency)
// ============================================================================

namespace metaffi::api
{

namespace detail_api
{

constexpr const char* kRuntimePluginPrefix = "xllr.";

inline std::string normalize_runtime_plugin(std::string runtime_plugin)
{
	if(runtime_plugin.empty())
	{
		throw std::invalid_argument("runtime_plugin must not be empty");
	}

	if(runtime_plugin.rfind(kRuntimePluginPrefix, 0) == 0)
	{
		return runtime_plugin;
	}

	return std::string(kRuntimePluginPrefix) + runtime_plugin;
}

inline void ensure_count_fits_int8(std::size_t count, const char* label)
{
	if(count > static_cast<std::size_t>((std::numeric_limits<int8_t>::max)()))
	{
		std::ostringstream ss;
		ss << label << " count exceeds int8_t max: " << count;
		throw std::invalid_argument(ss.str());
	}
}

inline void throw_if_err(char* err, const char* context)
{
	if(err == nullptr) return;

	std::string err_text(err);
	xllr_free_string(err);

	std::string msg = context;
	msg += ": ";
	msg += err_text;
	throw std::runtime_error(msg);
}

inline bool has_any_flag(metaffi_type type)        { return (type & metaffi_any_type) == metaffi_any_type; }
inline bool is_array_type(metaffi_type type)       { return (type & metaffi_array_type) == metaffi_array_type; }
inline bool is_packed_array_type(metaffi_type type) { return (type & metaffi_packed_type) == metaffi_packed_type; }

inline metaffi_type base_type(metaffi_type type)
{
	return is_array_type(type) ? (type & ~(metaffi_array_type | metaffi_packed_type)) : type;
}

inline metaffi_int64 get_actual_fixed_dimensions(const cdt& actual)
{
	if(!is_array_type(actual.type)) return MIXED_OR_UNKNOWN_DIMENSIONS;
	if(is_packed_array_type(actual.type)) return 1;
	if(actual.cdt_val.array_val != nullptr) return actual.cdt_val.array_val->fixed_dimensions;
	return MIXED_OR_UNKNOWN_DIMENSIONS;
}

inline bool matches_expected_type(const MetaFFITypeInfo& expected, const cdt& actual)
{
	const metaffi_type expected_type = expected.type;
	const metaffi_type actual_type = actual.type;

	if(expected_type == metaffi_array_type) return is_array_type(actual_type);
	if(has_any_flag(expected_type))
	{
		if(is_array_type(expected_type)) return is_array_type(actual_type);
		return true;
	}

	if(is_array_type(expected_type))
	{
		if(!is_array_type(actual_type)) return false;
		if(base_type(expected_type) != base_type(actual_type)) return false;
		if(expected.fixed_dimensions != MIXED_OR_UNKNOWN_DIMENSIONS)
		{
			if(get_actual_fixed_dimensions(actual) != expected.fixed_dimensions) return false;
		}
		return true;
	}

	return expected_type == actual_type;
}

inline const char* safe_alias(const MetaFFITypeInfo& ti) { return ti.alias ? ti.alias : ""; }

inline const char* metaffi_type_name_str(metaffi_type type)
{
	const char* name = nullptr;
	metaffi_type_to_str(type, name);
	return name ? name : "Unknown type";
}

inline std::string format_type_diagnostics(const MetaFFITypeInfo& expected, const cdt& actual)
{
	std::ostringstream ss;
	ss << "expected type=" << static_cast<unsigned long long>(expected.type)
	   << " (" << metaffi_type_name_str(expected.type) << ")"
	   << ", actual type=" << static_cast<unsigned long long>(actual.type)
	   << " (" << metaffi_type_name_str(actual.type) << ")"
	   << ", expected alias=\"" << safe_alias(expected) << "\""
	   << ", expected fixed_dimensions=" << static_cast<long long>(expected.fixed_dimensions)
	   << ", actual fixed_dimensions=" << static_cast<long long>(get_actual_fixed_dimensions(actual));
	return ss.str();
}

inline void validate_cdts_types(const std::vector<MetaFFITypeInfo>& expected,
                                const cdts& actual,
                                const char* what)
{
	if(actual.length != expected.size())
	{
		std::ostringstream ss;
		ss << what << " count mismatch. expected=" << expected.size()
		   << ", actual=" << actual.length;
		throw std::invalid_argument(ss.str());
	}

	for(std::size_t i = 0; i < expected.size(); ++i)
	{
		if(!matches_expected_type(expected[i], actual.arr[i]))
		{
			const std::string details = format_type_diagnostics(expected[i], actual.arr[i]);
			std::cerr << "+++ " << what << " type mismatch at index " << i << ": " << details << std::endl;
			std::ostringstream ss;
			ss << what << " type mismatch at index " << i << ". " << details;
			throw std::invalid_argument(ss.str());
		}
	}
}

} // namespace detail_api


// --- MetaFFIRuntime ---

inline MetaFFIRuntime::MetaFFIRuntime(std::string runtime_plugin)
	: _runtime_plugin(detail_api::normalize_runtime_plugin(std::move(runtime_plugin)))
{
}

inline const std::string& MetaFFIRuntime::runtime_plugin() const { return _runtime_plugin; }

inline void MetaFFIRuntime::load_runtime_plugin() const
{
	if(get_env_var("METAFFI_HOME").empty())
	{
		throw std::runtime_error("METAFFI_HOME environment variable is not set");
	}

	char* err = nullptr;
	xllr_load_runtime_plugin(_runtime_plugin.c_str(), &err);
	detail_api::throw_if_err(err, "Failed to load runtime plugin");
}

inline void MetaFFIRuntime::release_runtime_plugin() const
{
	char* err = nullptr;
	xllr_free_runtime_plugin(_runtime_plugin.c_str(), &err);
	detail_api::throw_if_err(err, "Failed to release runtime plugin");
}

inline MetaFFIModule MetaFFIRuntime::load_module(std::string module_path) const
{
	return MetaFFIModule(_runtime_plugin, std::move(module_path));
}

// --- MetaFFIModule ---

inline MetaFFIModule::MetaFFIModule(std::string runtime_plugin, std::string module_path)
	: _runtime_plugin(detail_api::normalize_runtime_plugin(std::move(runtime_plugin))),
	  _module_path(std::move(module_path))
{
}

inline const std::string& MetaFFIModule::module_path() const     { return _module_path; }
inline const std::string& MetaFFIModule::runtime_plugin() const  { return _runtime_plugin; }

inline MetaFFIEntity MetaFFIModule::load_entity(const std::string& entity_path,
                                                std::initializer_list<metaffi_type> params_types,
                                                std::initializer_list<metaffi_type> retvals_types) const
{
	std::vector<MetaFFITypeInfo> params;
	params.reserve(params_types.size());
	for(auto t : params_types) params.emplace_back(t);

	std::vector<MetaFFITypeInfo> retvals;
	retvals.reserve(retvals_types.size());
	for(auto t : retvals_types) retvals.emplace_back(t);

	return load_entity_with_info(entity_path, params, retvals);
}

inline MetaFFIEntity MetaFFIModule::load_entity_with_info(const std::string& entity_path,
                                                          const std::vector<MetaFFITypeInfo>& params_types,
                                                          const std::vector<MetaFFITypeInfo>& retvals_types) const
{
	if(entity_path.empty())
	{
		throw std::invalid_argument("entity_path must not be empty");
	}

	// Validate entity path format early (fail-fast)
	metaffi::utils::entity_path_parser parser(entity_path);
	(void)parser;

	detail_api::ensure_count_fits_int8(params_types.size(), "params_types");
	detail_api::ensure_count_fits_int8(retvals_types.size(), "retvals_types");

	std::vector<MetaFFITypeInfo> params_copy = params_types;
	std::vector<MetaFFITypeInfo> retvals_copy = retvals_types;

	char* err = nullptr;
	xcall* pxcall = xllr_load_entity(
		_runtime_plugin.c_str(),
		_module_path.c_str(),
		entity_path.c_str(),
		params_copy.empty() ? nullptr : params_copy.data(),
		static_cast<int8_t>(params_copy.size()),
		retvals_copy.empty() ? nullptr : retvals_copy.data(),
		static_cast<int8_t>(retvals_copy.size()),
		&err);

	detail_api::throw_if_err(err, "Failed to load entity");

	if(pxcall == nullptr)
	{
		throw std::runtime_error("xllr_load_entity returned null xcall");
	}

	return MetaFFIEntity(_runtime_plugin, pxcall, std::move(params_copy), std::move(retvals_copy));
}

// --- MetaFFIEntity ---

inline MetaFFIEntity::MetaFFIEntity(std::string runtime_plugin,
                                    xcall* pxcall,
                                    std::vector<MetaFFITypeInfo> params_types,
                                    std::vector<MetaFFITypeInfo> retvals_types)
	: MetaFFIEntity(std::move(runtime_plugin), pxcall,
	                std::move(params_types), std::move(retvals_types), true)
{
}

inline MetaFFIEntity::MetaFFIEntity(std::string runtime_plugin,
                                    xcall* pxcall,
                                    std::vector<MetaFFITypeInfo> params_types,
                                    std::vector<MetaFFITypeInfo> retvals_types,
                                    bool owns_xcall)
	: _runtime_plugin(detail_api::normalize_runtime_plugin(std::move(runtime_plugin))),
	  _pxcall(pxcall),
	  _params_types(std::move(params_types)),
	  _retvals_types(std::move(retvals_types)),
	  _owns_xcall(owns_xcall)
{
	if(_pxcall == nullptr)
	{
		throw std::invalid_argument("pxcall must not be null");
	}
}

inline MetaFFIEntity::MetaFFIEntity() noexcept
	: _pxcall(nullptr), _owns_xcall(false)
{
}

inline MetaFFIEntity::MetaFFIEntity(MetaFFIEntity&& other) noexcept
	: _runtime_plugin(std::move(other._runtime_plugin)),
	  _pxcall(other._pxcall),
	  _params_types(std::move(other._params_types)),
	  _retvals_types(std::move(other._retvals_types)),
	  _owns_xcall(other._owns_xcall)
{
	other._pxcall = nullptr;
	other._owns_xcall = false;
}

inline MetaFFIEntity& MetaFFIEntity::operator=(MetaFFIEntity&& other) noexcept
{
	if(this != &other)
	{
		if(_pxcall != nullptr && _owns_xcall)
		{
			char* err = nullptr;
			xllr_free_xcall(_runtime_plugin.c_str(), _pxcall, &err);
			if(err)
			{
				std::cerr << "MetaFFI: Failed to free xcall in move assignment: " << err << std::endl;
				xllr_free_string(err);
			}
		}

		_runtime_plugin = std::move(other._runtime_plugin);
		_pxcall = other._pxcall;
		_params_types = std::move(other._params_types);
		_retvals_types = std::move(other._retvals_types);
		_owns_xcall = other._owns_xcall;
		other._pxcall = nullptr;
		other._owns_xcall = false;
	}
	return *this;
}

inline MetaFFIEntity::~MetaFFIEntity()
{
	if(_pxcall != nullptr && _owns_xcall)
	{
		char* err = nullptr;
		xllr_free_xcall(_runtime_plugin.c_str(), _pxcall, &err);
		if(err)
		{
			std::cerr << "MetaFFI: Failed to free xcall: " << err << std::endl;
			xllr_free_string(err);
		}
		_pxcall = nullptr;
	}
}

inline const std::vector<MetaFFITypeInfo>& MetaFFIEntity::parameters_types() const { return _params_types; }
inline const std::vector<MetaFFITypeInfo>& MetaFFIEntity::retval_types() const     { return _retvals_types; }

inline void MetaFFIEntity::ensure_params_count(std::size_t count) const
{
	if(count != _params_types.size())
	{
		std::ostringstream ss;
		ss << "Parameters count mismatch. expected=" << _params_types.size() << ", actual=" << count;
		throw std::invalid_argument(ss.str());
	}
}

inline void MetaFFIEntity::ensure_retvals_count(std::size_t count) const
{
	if(count != _retvals_types.size())
	{
		std::ostringstream ss;
		ss << "Return values count mismatch. expected=" << _retvals_types.size() << ", actual=" << count;
		throw std::invalid_argument(ss.str());
	}
}

inline void MetaFFIEntity::validate_params(const cdts& params) const
{
	detail_api::validate_cdts_types(_params_types, params, "Parameter");
}

inline void MetaFFIEntity::validate_retvals(const cdts& retvals) const
{
	if(_retvals_types.empty()) return;
	detail_api::validate_cdts_types(_retvals_types, retvals, "Return value");
}

inline cdts MetaFFIEntity::call_with_cdts(cdts&& params)
{
	if(_pxcall == nullptr)
	{
		throw std::runtime_error("xcall is null");
	}

	const metaffi_size params_count = params.length;
	const metaffi_size retvals_count = static_cast<metaffi_size>(_retvals_types.size());
	cdts retvals(retvals_count);

	char* err = nullptr;

	if(params_count == 0 && retvals_count == 0)
	{
		xllr_xcall_no_params_no_ret(_pxcall, &err);
		detail_api::throw_if_err(err, "xcall invocation failed");
		return cdts();
	}

	if(params_count > 0 && retvals_count > 0)
	{
		std::array<cdts, 2> params_ret{};
		params_ret[0] = std::move(params);
		params_ret[1] = std::move(retvals);

		xllr_xcall_params_ret(_pxcall, params_ret.data(), &err);
		detail_api::throw_if_err(err, "xcall invocation failed");

		retvals = std::move(params_ret[1]);
		validate_retvals(retvals);
		return retvals;
	}

	if(params_count > 0)
	{
		std::array<cdts, 2> params_ret{};
		params_ret[0] = std::move(params);
		params_ret[1] = cdts(0);

		xllr_xcall_params_no_ret(_pxcall, params_ret.data(), &err);
		detail_api::throw_if_err(err, "xcall invocation failed");
		return cdts();
	}

	// Only retvals
	std::array<cdts, 2> params_ret{};
	params_ret[0] = cdts(0);
	params_ret[1] = std::move(retvals);

	xllr_xcall_no_params_ret(_pxcall, params_ret.data(), &err);
	detail_api::throw_if_err(err, "xcall invocation failed");

	retvals = std::move(params_ret[1]);
	validate_retvals(retvals);
	return retvals;
}

inline cdts MetaFFIEntity::call_raw(cdts&& params)
{
	ensure_params_count(params.length);
	validate_params(params);
	return call_with_cdts(std::move(params));
}

// --- MetaFFICallable ---

inline MetaFFICallable::MetaFFICallable(cdt_metaffi_callable* callable, std::string runtime_plugin)
	: _callable(callable),
	  _runtime_plugin(detail_api::normalize_runtime_plugin(std::move(runtime_plugin)))
{
}

inline MetaFFICallable::MetaFFICallable(MetaFFICallable&& other) noexcept
	: _callable(other._callable),
	  _runtime_plugin(std::move(other._runtime_plugin))
{
	other._callable = nullptr;
}

inline MetaFFICallable& MetaFFICallable::operator=(MetaFFICallable&& other) noexcept
{
	if(this != &other)
	{
		if(_callable)
		{
			if(_callable->parameters_types) xllr_free_memory(_callable->parameters_types);
			if(_callable->retval_types) xllr_free_memory(_callable->retval_types);
			xllr_free_memory(_callable);
		}

		_callable = other._callable;
		_runtime_plugin = std::move(other._runtime_plugin);
		other._callable = nullptr;
	}
	return *this;
}

inline MetaFFICallable::~MetaFFICallable()
{
	if(_callable)
	{
		if(_callable->parameters_types) xllr_free_memory(_callable->parameters_types);
		if(_callable->retval_types) xllr_free_memory(_callable->retval_types);
		xllr_free_memory(_callable);
		_callable = nullptr;
	}
}

inline cdt_metaffi_callable* MetaFFICallable::get() const   { return _callable; }
inline bool MetaFFICallable::is_null() const                { return !_callable || !_callable->val; }

inline cdt_metaffi_callable* MetaFFICallable::release()
{
	cdt_metaffi_callable* released = _callable;
	_callable = nullptr;
	return released;
}

inline MetaFFIEntity MetaFFICallable::as_entity() const
{
	if(!_callable || !_callable->val)
	{
		throw std::invalid_argument("callable must not be null");
	}

	std::vector<MetaFFITypeInfo> params;
	params.reserve(_callable->params_types_length);
	for(int i = 0; i < _callable->params_types_length; ++i)
	{
		const metaffi_type type = _callable->parameters_types ? _callable->parameters_types[i] : metaffi_any_type;
		params.emplace_back(type);
	}

	std::vector<MetaFFITypeInfo> retvals;
	retvals.reserve(_callable->retval_types_length);
	for(int i = 0; i < _callable->retval_types_length; ++i)
	{
		const metaffi_type type = _callable->retval_types ? _callable->retval_types[i] : metaffi_any_type;
		retvals.emplace_back(type);
	}

	return MetaFFIEntity(_runtime_plugin,
	                     reinterpret_cast<xcall*>(_callable->val),
	                     std::move(params),
	                     std::move(retvals),
	                     false);
}

} // namespace metaffi::api
