#include "xllr_capi_loader.h"
#include <utils/logger_c.h>
#include <utils/safe_func.h>

#include <stdlib.h>
#ifndef WIN32
#include <stdio.h>
#include <string.h>
#include <limits.h>
#endif

#ifdef _MSC_VER
#include <corecrt.h>
#endif

// === Handlers ===
void* cdt_helper_xllr_handle = NULL;

/************************************************
*   Allocations
*************************************************/

int64_t get_cache_size(){ return cdt_cache_size; }


/************************************************
*   XLLR functions
*************************************************/

#define check_function_pointer(func, retval) \
	if(func == NULL) \
	{ \
		if(cdt_helper_xllr_handle == NULL) \
		{ \
			const char* err = load_xllr(); \
			if(err) \
			{ \
				\
				metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load xllr: %s", err); \
				return retval; \
			} \
		} \
		else \
		{ \
			const char* err = load_xllr_capi(); \
			if(err) \
			{ \
				metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load xllr_capi: %s", err); \
				return retval; \
			} \
		}\
	}

void (*pxllr_xcall_params_ret)(struct xcall*, struct cdts[2], char**);
void xllr_xcall_params_ret(struct xcall* pxcall,
                           struct cdts params_ret[2],
                           char** out_err
)
{
	check_function_pointer(pxllr_xcall_params_ret,);

	pxllr_xcall_params_ret(pxcall,
	                       params_ret,
	                       out_err);
}

void (*pxllr_xcall_no_params_ret)(struct xcall*, struct cdts[2], char**);
void xllr_xcall_no_params_ret(struct xcall* pxcall,
                              struct cdts params_ret[2],
                              char** out_err
)
{
	check_function_pointer(pxllr_xcall_no_params_ret,);

	pxllr_xcall_no_params_ret(pxcall,
	                          params_ret,
	                          out_err);
}

void (*pxllr_xcall_params_no_ret)(struct xcall*, struct cdts[2], char**);
void xllr_xcall_params_no_ret(struct xcall* pxcall,
                              struct cdts params_ret[2],
                              char** out_err
)
{
	check_function_pointer(pxllr_xcall_params_no_ret,);

	pxllr_xcall_params_no_ret(pxcall,
	                          params_ret,
	                          out_err);
}

void (*pxllr_xcall_no_params_no_ret)(struct xcall*, char**);
void xllr_xcall_no_params_no_ret(struct xcall* pxcall,
                                 char** out_err
)
{
	check_function_pointer(pxllr_xcall_no_params_no_ret,);
	pxllr_xcall_no_params_no_ret(pxcall, out_err);
}

struct xcall* (*pxllr_load_entity)(const char*, const char*, const char*, struct metaffi_type_info*, int8_t, struct metaffi_type_info*, int8_t, char**);
struct xcall* xllr_load_entity(const char* runtime_plugin,
                          const char* module_path,
                          const char* entity_path,
                          struct metaffi_type_info* params_types, int8_t params_count,
                          struct metaffi_type_info* retval_types, int8_t retval_count,
                          char** out_err)
{
	check_function_pointer(pxllr_load_entity, NULL);
	return pxllr_load_entity(runtime_plugin,
	                            module_path,
						        entity_path,
						       params_types, params_count,
	                           retval_types, retval_count,
						        out_err);
}
void (*pxllr_free_xcall)(const char*, void*, char**);
void xllr_free_xcall(const char* runtime_plugin,
                        struct xcall* pxcall,
                        char** out_err)
{
	check_function_pointer(pxllr_free_xcall,);
	pxllr_free_xcall(runtime_plugin,
	                    pxcall,
	                    out_err);
}

struct cdt* (*pxllr_alloc_cdt_array)(uint64_t);
struct cdt* xllr_alloc_cdt_array(uint64_t size)
{
	check_function_pointer(pxllr_alloc_cdt_array, NULL);
	return pxllr_alloc_cdt_array(size);
}

void (*pxllr_free_cdt_array)(struct cdt*);
void xllr_free_cdt_array(struct cdt* pcdt)
{
	check_function_pointer(pxllr_free_cdt_array,);
	pxllr_free_cdt_array(pcdt);
}

void* (*pxllr_alloc_memory)(uint64_t);
void* xllr_alloc_memory(uint64_t size)
{
	check_function_pointer(pxllr_alloc_memory, NULL);
	return pxllr_alloc_memory(size);
}

void (*pxllr_free_memory)(void*);
void xllr_free_memory(void* ptr)
{
	check_function_pointer(pxllr_free_memory,);
	pxllr_free_memory(ptr);
}

void (*pxllr_free_string)(char*);
void xllr_free_string(char* err_to_free)
{
	check_function_pointer(pxllr_free_string,);
	pxllr_free_string(err_to_free);
}

char* (*palloc_string)(const char*, uint64_t);
char* xllr_alloc_string(const char* err_message, uint64_t length)
{
	check_function_pointer(palloc_string, NULL);
	return palloc_string(err_message, length);
}

char8_t* (*pxllr_alloc_string8)(const char8_t*, uint64_t);
char8_t* xllr_alloc_string8(const char8_t* err_message, uint64_t length)
{
	check_function_pointer(pxllr_alloc_string8, NULL);
	return pxllr_alloc_string8(err_message, length);
}

char16_t* (*pxllr_alloc_string16)(const char16_t*, uint64_t);
char16_t* xllr_alloc_string16(const char16_t* err_message, uint64_t length)
{
	check_function_pointer(pxllr_alloc_string16, NULL);
	return pxllr_alloc_string16(err_message, length);
}

char32_t* (*pxllr_alloc_string32)(const char32_t*, uint64_t);
char32_t* xllr_alloc_string32(const char32_t* err_message, uint64_t length)
{
	check_function_pointer(pxllr_alloc_string32, NULL);
	return pxllr_alloc_string32(err_message, length);
}

void (*pxllr_load_runtime_plugin)(const char*, char**);
void xllr_load_runtime_plugin(const char* runtime_plugin, char** err)
{
	check_function_pointer(pxllr_load_runtime_plugin,);

	pxllr_load_runtime_plugin(runtime_plugin, err);
}

void (*pxllr_free_runtime_plugin)(const char*, char**);
void xllr_free_runtime_plugin(const char* runtime_plugin, char** err)
{
	check_function_pointer(pxllr_free_runtime_plugin,);

	pxllr_free_runtime_plugin(runtime_plugin, err);
}

void (*pxllr_set_runtime_flag)(const char*);
void xllr_set_runtime_flag(const char* flag_name)
{
	check_function_pointer(pxllr_set_runtime_flag,);

	pxllr_set_runtime_flag(flag_name);
}

int (*pxllr_is_runtime_flag_set)(const char*);
int xllr_is_runtime_flag_set(const char* flag_name)
{
	check_function_pointer(pxllr_is_runtime_flag_set, 0);

	return pxllr_is_runtime_flag_set(flag_name);
}

struct cdts* (*palloc_cdts_buffer)(metaffi_size params, metaffi_size rets);
struct cdts* xllr_alloc_cdts_buffer(metaffi_size params, metaffi_size rets)
{
	check_function_pointer(palloc_cdts_buffer, NULL);

	return palloc_cdts_buffer(params, rets);
}

void (*pfree_cdts_buffer)(struct cdts* pcdts);
void xllr_free_cdts_buffer(struct cdts* pcdts)
{
	check_function_pointer(pfree_cdts_buffer,);

	pfree_cdts_buffer(pcdts);
}

void (*pxllr_construct_cdts)(struct cdts*, struct construct_cdts_callbacks*, char** out_nul_term_err);
void xllr_construct_cdts(struct cdts* pcdts, struct construct_cdts_callbacks* callbacks, char** out_nul_term_err)
{
	check_function_pointer(pxllr_construct_cdts,);

	pxllr_construct_cdts(pcdts, callbacks, out_nul_term_err);
}

void (*pxllr_construct_cdt)(struct cdt*, struct construct_cdts_callbacks*, char** out_nul_term_err);
void xllr_construct_cdt(struct cdt* pcdts, struct construct_cdts_callbacks* callbacks, char** out_nul_term_err)
{
	check_function_pointer(pxllr_construct_cdt,);

	pxllr_construct_cdt(pcdts, callbacks, out_nul_term_err);
}

void (*pxllr_traverse_cdts)(struct cdts*, struct traverse_cdts_callbacks*, char** out_nul_term_err);
void xllr_traverse_cdts(struct cdts* pcdts, struct traverse_cdts_callbacks* callbacks, char** out_nul_term_err)
{
	check_function_pointer(pxllr_traverse_cdts,);

	pxllr_traverse_cdts(pcdts, callbacks, out_nul_term_err);
}

void (*pxllr_traverse_cdt)(struct cdt*, struct traverse_cdts_callbacks*, char** out_nul_term_err);
void xllr_traverse_cdt(struct cdt* pcdts, struct traverse_cdts_callbacks* callbacks, char** out_nul_term_err)
{
	check_function_pointer(pxllr_traverse_cdt,);

	pxllr_traverse_cdt(pcdts, callbacks, out_nul_term_err);
}

/************************************************
*   Misc
*************************************************/

// === Functions ===
#ifdef _WIN32 //// --- START WINDOWS ---
#include <Windows.h>
void get_last_error_string(DWORD err, char** out_err_str)
{
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
	                 FORMAT_MESSAGE_FROM_SYSTEM |
	                 FORMAT_MESSAGE_IGNORE_INSERTS,
	                 NULL,
	                 err,
	                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                 (LPTSTR) out_err_str,
	                 0,
	                 NULL );

}

void* load_library(const char* name, char** out_err)
{
	void* handle = LoadLibraryA(name);
	if(!handle)
	{
		get_last_error_string(GetLastError(), out_err);
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load %s. Error: %s", name, *out_err);
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
		get_last_error_string(GetLastError(), &out_err);
		return out_err;
	}

	return NULL;
}

void* load_symbol(void* handle, const char* name, char** out_err)
{
	void* res = GetProcAddress(handle, name);
	if(!res)
	{
		char* win_out_err = NULL;
		get_last_error_string(GetLastError(), &win_out_err);
		
		// write to out_err "Failed to load symbol %s. win error: %s"
		int err_len = snprintf(NULL, 0, "Failed to load symbol %s. win error: %s", name, win_out_err);
		*out_err = calloc(1, err_len+1);
		metaffi_sprintf(*out_err, (size_t)(err_len + 1), "Failed to load symbol %s. win error: %s", name, win_out_err);
		
		return NULL;
	}

	return res;
}

#else // ------ START POSIX ----
#include <dlfcn.h>

void* load_library(const char* name, char** out_err)
{
	void* handle = dlopen(name, RTLD_GLOBAL | RTLD_NOW);
	if(!handle)
	{
		*out_err = dlerror();
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

void* load_symbol(void* handle, const char* name, char** out_err)
{
	void* res = dlsym(handle, name);
	if(!res)
	{
		*out_err = dlerror();
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load symbol from handle. %s. %s", name, *out_err);
		return NULL;
	}
	
	return res;
}

#endif //_WIN32 ------- END POSIX else block -----

const char* load_xllr_capi()
{
	char* out_err = NULL;

	pxllr_xcall_params_ret = (void (*)(struct xcall*, struct cdts[2], char**)) load_symbol(cdt_helper_xllr_handle, "xcall_params_ret", &out_err);
	if(!pxllr_xcall_params_ret)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load xllr_xcall_params_ret: %s", out_err);
		return out_err;
	}

	pxllr_xcall_no_params_ret = (void (*)(struct xcall*, struct cdts[2], char**)) load_symbol(cdt_helper_xllr_handle, "xcall_no_params_ret", &out_err);
	if(!pxllr_xcall_no_params_ret)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load xllr_xcall_no_params_ret: %s", out_err);
		return out_err;
	}

	pxllr_xcall_params_no_ret = (void (*)(struct xcall*, struct cdts[2], char**)) load_symbol(cdt_helper_xllr_handle, "xcall_params_no_ret", &out_err);
	if(!pxllr_xcall_params_no_ret)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load xllr_xcall_params_no_ret: %s", out_err);
		return out_err;
	}

	pxllr_xcall_no_params_no_ret = (void (*)(struct xcall*, char**)) load_symbol(cdt_helper_xllr_handle, "xcall_no_params_no_ret", &out_err);
	if(!pxllr_xcall_no_params_no_ret)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load xllr_xcall_no_params_no_ret: %s", out_err);
		return out_err;
	}
	
	pxllr_load_entity = (struct xcall* (*)(const char*, const char*, const char*, struct metaffi_type_info*, int8_t, struct metaffi_type_info*, int8_t, char**))load_symbol(cdt_helper_xllr_handle, "load_entity", &out_err);
	if(!pxllr_load_entity)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load load_function: %s", out_err);
		return out_err;
	}

	pxllr_free_xcall = (void (*)(const char*, void*, char**))load_symbol(cdt_helper_xllr_handle, "free_xcall", &out_err);
	if(!pxllr_free_xcall)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load free_and_remove_xcall: %s", out_err);
		return out_err;
	}
	
	pxllr_alloc_cdt_array = (struct cdt* (*)(uint64_t))load_symbol(cdt_helper_xllr_handle, "alloc_cdt_array", &out_err);
	if(!pxllr_alloc_cdt_array)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load alloc_cdt_array: %s", out_err);
		return out_err;
	}
	
	pxllr_free_cdt_array = (void (*)(struct cdt*))load_symbol(cdt_helper_xllr_handle, "free_cdt_array", &out_err);
	if(!pxllr_free_cdt_array)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load free_cdt_array: %s", out_err);
		return out_err;
	}
	
	pxllr_alloc_memory = (void* (*)(uint64_t))load_symbol(cdt_helper_xllr_handle, "alloc_memory", &out_err);
	if(!pxllr_alloc_memory)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load alloc_memory: %s", out_err);
		return out_err;
	}
	
	pxllr_free_memory = (void (*)(void*))load_symbol(cdt_helper_xllr_handle, "free_memory", &out_err);
	if(!pxllr_free_memory)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load free_memory: %s", out_err);
		return out_err;
	}
	
	pxllr_free_string = (void (*)(char*))load_symbol(cdt_helper_xllr_handle, "free_string", &out_err);
	if(!pxllr_free_string)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load free_string: %s", out_err);
		return out_err;
	}
	
	palloc_string = (char* (*)(const char*, uint64_t))load_symbol(cdt_helper_xllr_handle, "alloc_string", &out_err);
	if(!palloc_string)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load alloc_string: %s", out_err);
		return out_err;
	}
	
	pxllr_alloc_string8 = (char8_t* (*)(const char8_t*, uint64_t))load_symbol(cdt_helper_xllr_handle, "alloc_string8", &out_err);
	if(!pxllr_alloc_string8)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load alloc_string8: %s", out_err);
		return out_err;
	}
	
	pxllr_alloc_string16 = (char16_t* (*)(const char16_t*, uint64_t))load_symbol(cdt_helper_xllr_handle, "alloc_string16", &out_err);
	if(!pxllr_alloc_string16)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load alloc_string16: %s", out_err);
		return out_err;
	}
	
	pxllr_alloc_string32 = (char32_t* (*)(const char32_t*, uint64_t))load_symbol(cdt_helper_xllr_handle, "alloc_string32", &out_err);
	if(!pxllr_alloc_string32)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load alloc_string32: %s", out_err);
		return out_err;
	}
	
	pxllr_load_runtime_plugin = (void (*)(const char*, char**))load_symbol(cdt_helper_xllr_handle, "load_runtime_plugin", &out_err);
	if(!pxllr_load_runtime_plugin)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load load_runtime_plugin: %s", out_err);
		return out_err;
	}

	pxllr_free_runtime_plugin = (void (*)(const char*, char**))load_symbol(cdt_helper_xllr_handle, "free_runtime_plugin", &out_err);
	if(!pxllr_free_runtime_plugin)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load free_runtime_plugin: %s", out_err);
		return out_err;
	}

	pxllr_is_runtime_flag_set = (int (*)(const char*))load_symbol(cdt_helper_xllr_handle, "is_runtime_flag_set", &out_err);
	if(!pxllr_is_runtime_flag_set)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load is_runtime_flag_set: %s", out_err);
		return out_err;
	}

	pxllr_set_runtime_flag = (void (*)(const char*))load_symbol(cdt_helper_xllr_handle, "set_runtime_flag", &out_err);
	if(!pxllr_set_runtime_flag)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load set_runtime_flag: %s", out_err);
		return out_err;
	}

	palloc_cdts_buffer = (struct cdts* (*)(metaffi_size, metaffi_size))load_symbol(cdt_helper_xllr_handle, "alloc_cdts_buffer", &out_err);
	if(!palloc_cdts_buffer)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load alloc_cdts_buffer: %s", out_err);
		return out_err;
	}
	
	pfree_cdts_buffer = (void (*)(struct cdts*))load_symbol(cdt_helper_xllr_handle, "free_cdts_buffer", &out_err);
	if(!pfree_cdts_buffer)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load free_cdts_buffer: %s", out_err);
		return out_err;
	}
	
	pxllr_construct_cdts = (void (*)(struct cdts*, struct construct_cdts_callbacks*, char** out_nul_term_err))load_symbol(cdt_helper_xllr_handle, "construct_cdts", &out_err);
	if(!pxllr_construct_cdts)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load construct_cdts: %s", out_err);
		return out_err;
	}
	
	pxllr_construct_cdt = (void (*)(struct cdt*, struct construct_cdts_callbacks*, char** out_nul_term_err))load_symbol(cdt_helper_xllr_handle, "construct_cdt", &out_err);
	if(!pxllr_construct_cdt)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load construct_cdt: %s", out_err);
		return out_err;
	}
	
	pxllr_traverse_cdts = (void (*)(struct cdts*, struct traverse_cdts_callbacks*, char** out_nul_term_err))load_symbol(cdt_helper_xllr_handle, "traverse_cdts", &out_err);
	if(!pxllr_traverse_cdts)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load traverse_cdts: %s", out_err);
		return out_err;
	}
	
	pxllr_traverse_cdt = (void (*)(struct cdt*, struct traverse_cdts_callbacks*, char** out_nul_term_err))load_symbol(cdt_helper_xllr_handle, "traverse_cdt", &out_err);
	if(!pxllr_traverse_cdt)
	{
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load traverse_cdt: %s", out_err);
		return out_err;
	}

	return NULL;
}

const char* load_xllr()
{
	if(cdt_helper_xllr_handle){
		return NULL;
	}

	
#ifndef _WIN32
	#define MAX_PATH PATH_MAX
#endif

	char metaffi_home[MAX_PATH+1] = {0};
	char* metaffi_home_tmp = metaffi_getenv_alloc("METAFFI_HOME");
	if(!metaffi_home_tmp)
	{
	    return "Failed getting METAFFI_HOME. Is it set?";
	}

	size_t metaffi_home_len = strlen(metaffi_home_tmp);
	if(metaffi_home_len > MAX_PATH-1)
	{
		metaffi_free_env(metaffi_home_tmp);
		return "METAFFI_HOME is larger than MAX_PATH";
	}

	metaffi_strncpy(metaffi_home, sizeof(metaffi_home), metaffi_home_tmp, MAX_PATH);
	metaffi_free_env(metaffi_home_tmp);

#ifdef _WIN32
	const char* ext = ".dll";
#elif __APPLE__
	const char* ext = ".dylib";
#else
	const char* ext = ".so";
#endif


#ifdef _WIN32
	char xllr_full_path[MAX_PATH] = {0};
	metaffi_sprintf(xllr_full_path, sizeof(xllr_full_path), "%s\\xllr%s", metaffi_home, ext);
#else
	char xllr_full_path[PATH_MAX+6] = {0};
	metaffi_sprintf(xllr_full_path, sizeof(xllr_full_path), "%s/xllr%s", metaffi_home, ext);
#endif

	char* out_err;
	cdt_helper_xllr_handle = load_library(xllr_full_path, &out_err);
	if(!cdt_helper_xllr_handle)
	{
		// error has occurred
		//char* err_buf = calloc(1, (23+out_err_size+1)*sizeof(char));
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to load XLLR: %s", out_err);
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
		metaffi_logf("sdk.runtime", METAFFI_LOG_LEVEL_ERROR, "Failed to free XLLR: %s", err);
		return "Failed to free XLLR: %s";
	}

	return NULL;
}

const char* load_metaffi_library(const char* path_within_metaffi_home, void** out_handle)
{
#ifndef _WIN32
	#define MAX_PATH PATH_MAX
#endif

	char metaffi_home[MAX_PATH+1] = {0};
	char* metaffi_home_tmp = metaffi_getenv_alloc("METAFFI_HOME");
	if(!metaffi_home_tmp)
	{
	    return "Failed getting METAFFI_HOME. Is it set?";
	}

	size_t metaffi_home_len = strlen(metaffi_home_tmp);
	if(metaffi_home_len > MAX_PATH-1)
	{
		metaffi_free_env(metaffi_home_tmp);
		return "METAFFI_HOME is larger than MAX_PATH";
	}

	metaffi_strncpy(metaffi_home, sizeof(metaffi_home), metaffi_home_tmp, MAX_PATH);
	metaffi_free_env(metaffi_home_tmp);
	
	// get correct extension
#ifdef _WIN32
	const char* ext = ".dll";
#elif __APPLE__
	const char* ext = ".dylib";
#else
	const char* ext = ".so";
#endif
	
	// load $METAFFI_HOME/path_within_metaffi_home
#ifdef _WIN32
	char full_path[MAX_PATH] = {0};
	metaffi_sprintf(full_path, sizeof(full_path), "%s\\%s.%s", metaffi_home, path_within_metaffi_home, ext);
#else
	char full_path[PATH_MAX*2] = {0};
	metaffi_sprintf(full_path, sizeof(full_path), "%s/%s.%s", metaffi_home, path_within_metaffi_home, ext);
#endif
	
	char* out_err;
	*out_handle = load_library(full_path, &out_err);
	if(!*out_handle)
	{
		return out_err;
	}
	
	return NULL;
}
