#include "xllr_capi_loader.h"

#include <stdlib.h>
#ifndef WIN32
#include <stdio.h>
	#include <string.h>
#endif

// === Handlers ===
void* cdt_helper_xllr_handle = NULL;

/************************************************
*   Allocations
*************************************************/

int64_t get_cache_size(){ return cdts_cache_size; }


/************************************************
*   XLLR functions
*************************************************/
void (*pxllr_xcall_params_ret)(void*, struct cdts[2], char**, uint64_t*);
void xllr_xcall_params_ret(void* pff,
                           struct cdts params_ret[2],
                           char** out_err, uint64_t* out_err_len
)
{
	pxllr_xcall_params_ret(pff,
	                       params_ret,
	                       out_err, out_err_len);
}

void (*pxllr_xcall_no_params_ret)(void*, struct cdts[1], char**, uint64_t*);
void xllr_xcall_no_params_ret(void* pff,
                              struct cdts return_values[1],
                              char** out_err, uint64_t* out_err_len
)
{
	pxllr_xcall_no_params_ret(pff,
	                          return_values,
	                          out_err, out_err_len);
}

void (*pxllr_xcall_params_no_ret)(void*, struct cdts[1], char**, uint64_t*);
void xllr_xcall_params_no_ret(void* pff,
                              struct cdts parameters[1],
                              char** out_err, uint64_t* out_err_len
)
{
	pxllr_xcall_params_no_ret(pff,
	                          parameters,
	                          out_err, out_err_len);
}

void (*pxllr_xcall_no_params_no_ret)(void*, char**, uint64_t*);
void xllr_xcall_no_params_no_ret(void* pff,
                                 char** out_err, uint64_t* out_err_len
)
{
	pxllr_xcall_no_params_no_ret(pff, out_err, out_err_len);
}

void** (*pxllr_load_function)(const char*, uint32_t, const char*, uint32_t, const char*, uint32_t, struct metaffi_type_info*, struct metaffi_type_info*, uint8_t, uint8_t, char**, uint32_t*);
void** xllr_load_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
                          const char* module_path, uint32_t module_path_len,
                          const char* function_path, uint32_t function_path_len,
                          struct metaffi_type_info* params_types, struct metaffi_type_info* retval_types,
                          uint8_t params_count, uint8_t retval_count,
                          char** out_err, uint32_t* out_err_len)
{
	return pxllr_load_function(runtime_plugin, runtime_plugin_len,
	                            module_path, module_path_len,
						        function_path, function_path_len,
						       params_types, retval_types, params_count, retval_count,
						        out_err, out_err_len);
}
void (*pxllr_free_function)(const char*, uint32_t, void*, char**, uint32_t*);
void xllr_free_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
                        void* pff,
                        char** out_err, uint32_t* out_err_len)
{
	pxllr_free_function(runtime_plugin, runtime_plugin_len,
	                    pff,
	                    out_err, out_err_len);
}

void (*pxllr_load_runtime_plugin)(const char*, uint32_t, char**, uint32_t*);
void xllr_load_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len)
{
	pxllr_load_runtime_plugin(runtime_plugin, runtime_plugin_len, err, err_len);
}

void (*pxllr_free_runtime_plugin)(const char*, uint32_t, char**, uint32_t*);
void xllr_free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len)
{
	pxllr_free_runtime_plugin(runtime_plugin, runtime_plugin_len, err, err_len);
}

void (*pxllr_set_runtime_flag)(const char*, uint64_t);
void xllr_set_runtime_flag(const char* flag_name, uint64_t flag_name_len)
{
	pxllr_set_runtime_flag(flag_name, flag_name_len);
}

int (*pxllr_is_runtime_flag_set)(const char*, uint64_t);
int xllr_is_runtime_flag_set(const char* flag_name, uint64_t flag_name_len)
{
	return pxllr_is_runtime_flag_set(flag_name, flag_name_len);
}

struct cdts* (*palloc_cdts_buffer)(metaffi_size params, metaffi_size rets);
struct cdts* xllr_alloc_cdts_buffer(metaffi_size params, metaffi_size rets)
{
	return palloc_cdts_buffer(params, rets);
}

void (*pxllr_construct_cdts)(struct cdts*, struct construct_cdts_callbacks*);
void xllr_construct_cdts(struct cdts* pcdts, struct construct_cdts_callbacks* callbacks)
{
	pxllr_construct_cdts(pcdts, callbacks);
}

void (*pxllr_construct_cdt)(struct cdt*, struct construct_cdts_callbacks*);
void xllr_construct_cdt(struct cdt* pcdts, struct construct_cdts_callbacks* callbacks)
{
	pxllr_construct_cdt(pcdts, callbacks);
}

void (*pxllr_traverse_cdts)(struct cdts*, struct traverse_cdts_callbacks*);
void xllr_traverse_cdts(struct cdts* pcdts, struct traverse_cdts_callbacks* callbacks)
{
	pxllr_traverse_cdts(pcdts, callbacks);
}

void (*pxllr_traverse_cdt)(struct cdt*, struct traverse_cdts_callbacks*);
void xllr_traverse_cdt(struct cdt* pcdts, struct traverse_cdts_callbacks* callbacks)
{
	pxllr_traverse_cdt(pcdts, callbacks);
}

/************************************************
*   Misc
*************************************************/

// === Functions ===
#ifdef _WIN32 //// --- START WINDOWS ---
#include <Windows.h>
void get_last_error_string(DWORD err, char** out_err_str, uint64_t* out_err_size)
{
	DWORD bufLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
	                             FORMAT_MESSAGE_FROM_SYSTEM |
	                             FORMAT_MESSAGE_IGNORE_INSERTS,
	                             NULL,
	                             err,
	                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                             (LPTSTR) out_err_str,
	                             0,
	                             NULL );

	*out_err_size = bufLen;
}

void* load_library(const char* name, char** out_err, uint64_t* out_err_size)
{
	void* handle = LoadLibraryA(name);
	if(!handle)
	{
		get_last_error_string(GetLastError(), out_err, out_err_size);
		printf("Failed to load %s. Error: %s\n", name, *out_err);
	}

	return handle;
}

const char* free_library(void* lib) // return error string. null if no error.
{
	if(!lib)
	{
		return NULL;
	}

	if(!FreeLibrary(lib))
	{
		char* out_err;
		uint64_t out_err_len;
		get_last_error_string(GetLastError(), &out_err, &out_err_len);
		return out_err;
	}

	return NULL;
}

void* load_symbol(void* handle, const char* name, char** out_err, uint64_t* out_err_len)
{
	void* res = GetProcAddress(handle, name);
	if(!res)
	{
		get_last_error_string(GetLastError(), out_err, out_err_len);
		return NULL;
	}

	return res;
}

#else // ------ START POSIX ----
#include <dlfcn.h>
#include <string.h>

void* load_library(const char* name, char** out_err, uint64_t* out_err_len)
{
	void* handle = dlopen(name, RTLD_GLOBAL | RTLD_NOW);
	if(!handle)
	{
		*out_err = dlerror();
		*out_err_len = strlen(*out_err);
	}
	
	return handle;
}

const char* free_library(void* lib)
{
	if(dlclose(lib))
	{
		return dlerror();
	}
	
	return NULL;
}

void* load_symbol(void* handle, const char* name, char** out_err, uint64_t* out_err_len)
{
	void* res = dlsym(handle, name);
	if(!res)
	{
		*out_err = dlerror();
		printf("Failed to load symbol from handle. %s. %s\n", name, *out_err);
		*out_err_len = strlen(*out_err);
		return NULL;
	}
	
	return res;
}

#endif // ------- END POSIX -----

const char* load_xllr_capi()
{
	char* out_err = NULL;
	uint64_t out_err_len;

	pxllr_xcall_params_ret = (void (*)(void*, struct cdts[2], char**, uint64_t*)) load_symbol(cdt_helper_xllr_handle, "xcall_params_ret", &out_err, &out_err_len);
	if(!pxllr_xcall_params_ret)
	{
		printf("Failed to load xllr_xcall_params_ret: %s\n", out_err);
		return out_err;
	}

	pxllr_xcall_no_params_ret = (void (*)(void*, struct cdts[1], char**, uint64_t*)) load_symbol(cdt_helper_xllr_handle, "xcall_no_params_ret", &out_err, &out_err_len);
	if(!pxllr_xcall_no_params_ret)
	{
		printf("Failed to load xllr_xcall_no_params_ret: %s\n", out_err);
		return out_err;
	}

	pxllr_xcall_params_no_ret = (void (*)(void*, struct cdts[1], char**, uint64_t*)) load_symbol(cdt_helper_xllr_handle, "xcall_params_no_ret", &out_err, &out_err_len);
	if(!pxllr_xcall_params_no_ret)
	{
		printf("Failed to load xllr_xcall_params_no_ret: %s\n", out_err);
		return out_err;
	}

	pxllr_xcall_no_params_no_ret = (void (*)(void*, char**, uint64_t*)) load_symbol(cdt_helper_xllr_handle, "xcall_no_params_no_ret", &out_err, &out_err_len);
	if(!pxllr_xcall_no_params_no_ret)
	{
		printf("Failed to load xllr_xcall_no_params_no_ret: %s\n", out_err);
		return out_err;
	}
	
	pxllr_load_function = (void** (*)(const char*, uint32_t, const char*, uint32_t, const char*, uint32_t, struct metaffi_type_info*, struct metaffi_type_info*, uint8_t, uint8_t, char**, uint32_t*))load_symbol(cdt_helper_xllr_handle, "load_function", &out_err, &out_err_len);
	if(!pxllr_load_function)
	{
		printf("Failed to load load_function: %s\n", out_err);
		return out_err;
	}

	pxllr_free_function = (void (*)(const char*, uint32_t, void*, char**, uint32_t*))load_symbol(cdt_helper_xllr_handle, "free_function", &out_err, &out_err_len);
	if(!pxllr_free_function)
	{
		printf("Failed to load free_function: %s\n", out_err);
		return out_err;
	}

	pxllr_load_runtime_plugin = (void (*)(const char*, uint32_t, char**, uint32_t*))load_symbol(cdt_helper_xllr_handle, "load_runtime_plugin", &out_err, &out_err_len);
	if(!pxllr_load_runtime_plugin)
	{
		printf("Failed to load load_runtime_plugin: %s\n", out_err);
		return out_err;
	}

	pxllr_free_runtime_plugin = (void (*)(const char*, uint32_t, char**, uint32_t*))load_symbol(cdt_helper_xllr_handle, "free_runtime_plugin", &out_err, &out_err_len);
	if(!pxllr_free_runtime_plugin)
	{
		printf("Failed to load free_runtime_plugin: %s\n", out_err);
		return out_err;
	}

	pxllr_is_runtime_flag_set = (int (*)(const char*, uint64_t))load_symbol(cdt_helper_xllr_handle, "is_runtime_flag_set", &out_err, &out_err_len);
	if(!pxllr_is_runtime_flag_set)
	{
		printf("Failed to load is_runtime_flag_set: %s\n", out_err);
		return out_err;
	}

	pxllr_set_runtime_flag = (void (*)(const char*, uint64_t))load_symbol(cdt_helper_xllr_handle, "set_runtime_flag", &out_err, &out_err_len);
	if(!pxllr_set_runtime_flag)
	{
		printf("Failed to load set_runtime_flag: %s\n", out_err);
		return out_err;
	}

	palloc_cdts_buffer = (struct cdts* (*)(metaffi_size, metaffi_size))load_symbol(cdt_helper_xllr_handle, "alloc_cdts_buffer", &out_err, &out_err_len);
	if(!palloc_cdts_buffer)
	{
		printf("Failed to load alloc_cdts_buffer: %s\n", out_err);
		return out_err;
	}
	
	pxllr_construct_cdts = (void (*)(struct cdts*, struct construct_cdts_callbacks*))load_symbol(cdt_helper_xllr_handle, "construct_cdts", &out_err, &out_err_len);
	if(!pxllr_construct_cdts)
	{
		printf("Failed to load construct_cdts: %s\n", out_err);
		return out_err;
	}
	
	pxllr_construct_cdt = (void (*)(struct cdt*, struct construct_cdts_callbacks*))load_symbol(cdt_helper_xllr_handle, "construct_cdt", &out_err, &out_err_len);
	if(!pxllr_construct_cdt)
	{
		printf("Failed to load construct_cdt: %s\n", out_err);
		return out_err;
	}
	
	pxllr_traverse_cdts = (void (*)(struct cdts*, struct traverse_cdts_callbacks*))load_symbol(cdt_helper_xllr_handle, "traverse_cdts", &out_err, &out_err_len);
	if(!pxllr_traverse_cdts)
	{
		printf("Failed to load traverse_cdts: %s\n", out_err);
		return out_err;
	}

	return NULL;
}

const char* load_xllr()
{
	if(cdt_helper_xllr_handle){
		return NULL;
	}

	size_t metaffi_home_size = 320;
	char metaffi_home[320] = {0};
	const char* metaffi_home_tmp = getenv("METAFFI_HOME");
	if(metaffi_home_tmp)
	{
		strcpy(metaffi_home, metaffi_home_tmp);
	}
	else
	{
		return "Failed getting METAFFI_HOME. Is it set?";
	}

#ifdef _WIN32
	const char* ext = ".dll";
#elif __APPLE__
	const char* ext = ".dylib";
#else
	const char* ext = ".so";
#endif

	char xllr_full_path[400] = {0};
#ifdef _WIN32
	sprintf_s(xllr_full_path, sizeof(xllr_full_path), "%s\\xllr%s", metaffi_home, ext);
#else
	sprintf(xllr_full_path, "%s/xllr%s", metaffi_home, ext);
#endif

	char* out_err;
	uint64_t out_err_size;
	cdt_helper_xllr_handle = load_library(xllr_full_path, &out_err, &out_err_size);
	if(!cdt_helper_xllr_handle)
	{
		// error has occurred
		//char* err_buf = calloc(1, (23+out_err_size+1)*sizeof(char));
		printf("Failed to load XLLR: %s", out_err);
#ifdef _WIN32
		LocalFree(out_err);
#else
		free(out_err);
#endif

		// Returning non-allocated string so caller does not have to free anything
		// Error is printed to "printf"
		return "Failed to load XLLR";
	}

	const char* err = load_xllr_capi();
	if(err){
		return err;
	}

	return NULL;
}

const char* free_xllr()
{
	const char* err = free_library(cdt_helper_xllr_handle);

	if(err)
	{
		printf("Failed to free XLLR: %s", err);
		return "Failed to free XLLR: %s";
	}

	return NULL;
}
