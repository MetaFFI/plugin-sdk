#include "cdt_capi_loader.h"

#ifndef WIN32
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
#endif
/************************************************
*   Allocations
*************************************************/

#define alloc_numeric_on_heap_impl_fptr(type) \
typedef type* (*palloc_##type##_on_heap_t)(type val); \
palloc_##type##_on_heap_t palloc_##type##_on_heap; \
type* alloc_##type##_on_heap(type val){ return palloc_##type##_on_heap(val); } \
typedef type* (*palloc_##type##_array_on_heap_t)(metaffi_size length); \
palloc_##type##_array_on_heap_t palloc_##type##_array_on_heap; \
type* alloc_##type##_array_on_heap(metaffi_size length){ return palloc_##type##_array_on_heap(length); }

#define alloc_string_on_heap_impl_fptr(type) \
typedef type (*palloc_##type##_on_heap_t)(type val, metaffi_size str_size); \
palloc_##type##_on_heap_t palloc_##type##_on_heap; \
type alloc_##type##_on_heap(type val, metaffi_size str_size){ return palloc_##type##_on_heap(val, str_size); } \
typedef type* (*palloc_##type##_array_on_heap_t)(metaffi_size length); \
palloc_##type##_array_on_heap_t palloc_##type##_array_on_heap; \
type* alloc_##type##_array_on_heap(metaffi_size length){ return palloc_##type##_array_on_heap(length); }

alloc_numeric_on_heap_impl_fptr(metaffi_float64);
alloc_numeric_on_heap_impl_fptr(metaffi_float32);
alloc_numeric_on_heap_impl_fptr(metaffi_int64);
alloc_numeric_on_heap_impl_fptr(metaffi_int32);
alloc_numeric_on_heap_impl_fptr(metaffi_int16);
alloc_numeric_on_heap_impl_fptr(metaffi_int8);
alloc_numeric_on_heap_impl_fptr(metaffi_uint64);
alloc_numeric_on_heap_impl_fptr(metaffi_uint32);
alloc_numeric_on_heap_impl_fptr(metaffi_uint16);
alloc_numeric_on_heap_impl_fptr(metaffi_uint8);
alloc_numeric_on_heap_impl_fptr(metaffi_size);
alloc_numeric_on_heap_impl_fptr(metaffi_bool);
alloc_string_on_heap_impl_fptr(metaffi_string8);
alloc_string_on_heap_impl_fptr(metaffi_string16);
alloc_string_on_heap_impl_fptr(metaffi_string32);


//====================================================================

/************************************************
*   Getters
*************************************************/

typedef metaffi_type (*pget_type_t)(struct cdt* data_array, int index);
pget_type_t pget_type;
metaffi_type get_type(struct cdt* data_array, int index){ return pget_type(data_array, index); }

typedef struct cdt* (*pget_cdt_t)(struct cdt* data_array, int index);
pget_cdt_t pget_cdt;
struct cdt* get_cdt(struct cdt* data_array, int index){ return pget_cdt(data_array, index); }


//====================================================================

/************************************************
*   Array Elements Getters
*************************************************/

#define get_numeric_element_impl_fptr(type) \
typedef type (*pget_##type##_element_t) (type*, int); \
pget_##type##_element_t pget_##type##_element; \
type get_##type##_element(type* arr, int index){ return pget_##type##_element(arr, index); }

#define get_string_element_impl_fptr(type) \
typedef type (*pget_##type##_element_t)(type*, int, const metaffi_size*, metaffi_size*);\
pget_##type##_element_t pget_##type##_element;\
type get_##type##_element(type* arr, int index, const metaffi_size* sizes, metaffi_size* out_size){ return pget_##type##_element(arr, index, sizes, out_size); }

get_numeric_element_impl_fptr(metaffi_float64);
get_numeric_element_impl_fptr(metaffi_float32);
get_numeric_element_impl_fptr(metaffi_int64);
get_numeric_element_impl_fptr(metaffi_int32);
get_numeric_element_impl_fptr(metaffi_int16);
get_numeric_element_impl_fptr(metaffi_int8);
get_numeric_element_impl_fptr(metaffi_uint64);
get_numeric_element_impl_fptr(metaffi_uint32);
get_numeric_element_impl_fptr(metaffi_uint16);
get_numeric_element_impl_fptr(metaffi_uint8);
get_numeric_element_impl_fptr(metaffi_size);
get_numeric_element_impl_fptr(metaffi_handle);
get_numeric_element_impl_fptr(metaffi_bool);
get_string_element_impl_fptr(metaffi_string8);
get_string_element_impl_fptr(metaffi_string16);
get_string_element_impl_fptr(metaffi_string32);


/************************************************
*   Array Elements Setters
*************************************************/

#define set_numeric_element_impl_fptr(type) \
typedef void (*pset_##type##_element_t) (type*, int, type); \
pset_##type##_element_t pset_##type##_element; \
void set_##type##_element(type* arr, int index, type val){ pset_##type##_element(arr, index, val); }

#define set_string_element_impl_fptr(type) \
typedef void (*pset_##type##_element_t) (type*, metaffi_size*, int, type, metaffi_size); \
pset_##type##_element_t pset_##type##_element; \
void set_##type##_element(type* arr, metaffi_size* sizes_array, int index, type str, metaffi_size str_size){ pset_##type##_element(arr, sizes_array, index, str, str_size); }

set_numeric_element_impl_fptr(metaffi_float64);
set_numeric_element_impl_fptr(metaffi_float32);
set_numeric_element_impl_fptr(metaffi_int64);
set_numeric_element_impl_fptr(metaffi_int32);
set_numeric_element_impl_fptr(metaffi_int16);
set_numeric_element_impl_fptr(metaffi_int8);
set_numeric_element_impl_fptr(metaffi_uint64);
set_numeric_element_impl_fptr(metaffi_uint32);
set_numeric_element_impl_fptr(metaffi_uint16);
set_numeric_element_impl_fptr(metaffi_uint8);
set_numeric_element_impl_fptr(metaffi_size);
set_numeric_element_impl_fptr(metaffi_handle);
set_numeric_element_impl_fptr(metaffi_bool);
set_string_element_impl_fptr(metaffi_string8);
set_string_element_impl_fptr(metaffi_string16);
set_string_element_impl_fptr(metaffi_string32);



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

void* (*pxllr_load_function)(const char*, uint32_t, const char*, uint32_t, void*, int8_t, int8_t, char**, uint32_t*);
void* xllr_load_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
							 const char* function_path, uint32_t function_path_len,
							 void* pff,
	                           int8_t params_count, int8_t retval_count,
	                           char** out_err, uint32_t* out_err_len)
{
	return pxllr_load_function(runtime_plugin, runtime_plugin_len,
						  function_path, function_path_len,
					       pff, params_count, retval_count,
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

// === Handlers ===
void* cdt_helper_xllr_handle = NULL;

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

const char* load_xllr_api()
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
	
	pxllr_load_function = (void* (*)(const char*, uint32_t, const char*, uint32_t, void*, int8_t, int8_t, char**, uint32_t*))load_symbol(cdt_helper_xllr_handle, "load_function", &out_err, &out_err_len);
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
	
	
	return NULL;
}

const char* load_xllr()
{
	if(cdt_helper_xllr_handle){
		return NULL;
	}
	
	size_t metaffi_home_size = 320;
	char metaffi_home[320] = {0};
#ifdef _WIN32
	if(getenv_s(&metaffi_home_size, metaffi_home, metaffi_home_size, "METAFFI_HOME") != 0)
#else
	const char* metaffi_home_tmp = getenv("METAFFI_HOME");
	if(metaffi_home_tmp){ strcpy(metaffi_home, metaffi_home_tmp); }
	else
#endif
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
	sprintf_s(xllr_full_path, sizeof(xllr_full_path), "%s/xllr%s", metaffi_home, ext);
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
	
	const char* err = load_xllr_api();
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

// load args helpers functions
// if failed, returned error message, otherwise return NULL
const char* load_cdt_capi()
{
	if(!cdt_helper_xllr_handle)
	{
		const char* err = load_xllr();
		if(err){
			return err;
		}
	}
	
	char* err = NULL;
	uint64_t out_err_len = 0;

#define load_helper_function(name) \
	p##name = (p##name##_t)load_symbol(cdt_helper_xllr_handle, #name, &err, &out_err_len); \
	if(err){ return err; }

	load_helper_function(get_type);
	load_helper_function(get_cdt);
	
#define get_numeric_element_impl_fptr_assign(type) \
	load_helper_function(get_##type##_element);
	
	get_numeric_element_impl_fptr_assign(metaffi_float64);
	get_numeric_element_impl_fptr_assign(metaffi_float32);
	get_numeric_element_impl_fptr_assign(metaffi_int64);
	get_numeric_element_impl_fptr_assign(metaffi_int32);
	get_numeric_element_impl_fptr_assign(metaffi_int16);
	get_numeric_element_impl_fptr_assign(metaffi_int8);
	get_numeric_element_impl_fptr_assign(metaffi_uint64);
	get_numeric_element_impl_fptr_assign(metaffi_uint32);
	get_numeric_element_impl_fptr_assign(metaffi_uint16);
	get_numeric_element_impl_fptr_assign(metaffi_uint8);
	get_numeric_element_impl_fptr_assign(metaffi_size);
	get_numeric_element_impl_fptr_assign(metaffi_bool);
	get_numeric_element_impl_fptr_assign(metaffi_handle);
	
	load_helper_function(get_metaffi_string8_element);
	load_helper_function(get_metaffi_string16_element);
	load_helper_function(get_metaffi_string32_element);

#define set_numeric_element_impl_fptr_assign(type) \
	load_helper_function(set_##type##_element);
	
	set_numeric_element_impl_fptr_assign(metaffi_float64);
	set_numeric_element_impl_fptr_assign(metaffi_float32);
	set_numeric_element_impl_fptr_assign(metaffi_int64);
	set_numeric_element_impl_fptr_assign(metaffi_int32);
	set_numeric_element_impl_fptr_assign(metaffi_int16);
	set_numeric_element_impl_fptr_assign(metaffi_int8);
	set_numeric_element_impl_fptr_assign(metaffi_uint64);
	set_numeric_element_impl_fptr_assign(metaffi_uint32);
	set_numeric_element_impl_fptr_assign(metaffi_uint16);
	set_numeric_element_impl_fptr_assign(metaffi_uint8);
	set_numeric_element_impl_fptr_assign(metaffi_size);
	set_numeric_element_impl_fptr_assign(metaffi_bool);
	
	load_helper_function(set_metaffi_string8_element);
	load_helper_function(set_metaffi_string16_element);
	load_helper_function(set_metaffi_string32_element);
	
#define alloc_numeric_on_heap_impl_fptr_assign(type) \
	load_helper_function(alloc_##type##_on_heap)
	
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_float64);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_float32);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_int64);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_int32);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_int16);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_int8);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_uint64);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_uint32);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_uint16);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_uint8);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_size);
	alloc_numeric_on_heap_impl_fptr_assign(metaffi_bool);

#define alloc_str_on_heap_impl_fptr_assign(type) \
	load_helper_function(alloc_##type##_on_heap)
	
	alloc_str_on_heap_impl_fptr_assign(metaffi_string8);
	alloc_str_on_heap_impl_fptr_assign(metaffi_string16);
	alloc_str_on_heap_impl_fptr_assign(metaffi_string32);

	return NULL;
}

#undef common_data_type_helper_loader_c