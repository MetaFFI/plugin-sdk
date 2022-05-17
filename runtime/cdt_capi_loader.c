#include "cdt_capi_loader.h"

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
void set_##type##_element(type* arr, int index, type val){ return pset_##type##_element(arr, index, val); }

#define set_string_element_impl_fptr(type) \
typedef void (*pset_##type##_element_t) (type*, metaffi_size*, int, type, metaffi_size); \
pset_##type##_element_t pset_##type##_element; \
void set_##type##_element(type* arr, metaffi_size* sizes_array, int index, type str, metaffi_size str_size){ return pset_##type##_element(arr, sizes_array, index, str, str_size); }

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
void (*pxllr_call)(const char*, uint32_t, int64_t, struct cdt*, uint64_t, struct cdt*, uint64_t, char**, uint64_t*);
void xllr_call(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len,
				int64_t function_id,
				struct cdt* parameters, uint64_t parameters_length,
				struct cdt* return_values, uint64_t return_values_length,
				char** out_err, uint64_t* out_err_len
)
{
	pxllr_call(runtime_plugin_name, runtime_plugin_name_len,
		  function_id,
		  parameters, parameters_length,
		  return_values, return_values_length,
		  out_err, out_err_len);
}

int64_t (*pxllr_load_function)(const char*, uint32_t, const char*, uint32_t, int64_t, char**, uint32_t*);
int64_t xllr_load_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
							 const char* function_path, uint32_t function_path_len,
							 int64_t function_id_opt,
							 char** out_err, uint32_t* out_err_len)
{
	return pxllr_load_function(runtime_plugin, runtime_plugin_len,
						  function_path, function_path_len,
						  function_id_opt,
						  out_err, out_err_len);
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

// === Handlers ===
void* cdt_helper_xllr_handle = NULL;

/************************************************
*   Misc
*************************************************/

// === Functions ===
#ifdef _WIN32 //// --- START WINDOWS ---
#include <Windows.h>
void get_last_error_string(DWORD err, char** out_err_str)
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

	// TODO: out_err_str should get cleaned up!
}

void* load_library(const char* name, char** out_err)
{
	void* handle = LoadLibraryA(name);
	if(!handle)
	{
		get_last_error_string(GetLastError(), out_err);
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
		get_last_error_string(GetLastError(), out_err);
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
		printf("Failed to load symbol from handle. %s. %s\n", name, *out_err);
		return NULL;
	}
	
	return res;
}

#endif // ------- END POSIX -----

const char* load_xllr_api()
{
	char* out_err = NULL;
	pxllr_call = (void (*)(const char*, uint32_t, int64_t, struct cdt*, uint64_t, struct cdt*, uint64_t, char**, uint64_t*)) load_symbol(cdt_helper_xllr_handle, "xcall", &out_err);
	if(!pxllr_call)
	{
		return out_err;
	}
	
	pxllr_load_function = (int64_t (*)(const char*, uint32_t, const char*, uint32_t, int64_t, char**, uint32_t*))load_symbol(cdt_helper_xllr_handle, "load_function", &out_err);
	if(!pxllr_load_function)
	{
		return out_err;
	}

	pxllr_free_runtime_plugin = (void (*)(const char*, uint32_t, char**, uint32_t*))load_symbol(cdt_helper_xllr_handle, "free_runtime_plugin", &out_err);
	if(!pxllr_free_runtime_plugin)
	{
		return out_err;
	}
	
	pxllr_is_runtime_flag_set = (int (*)(const char*, uint64_t))load_symbol(cdt_helper_xllr_handle, "is_runtime_flag_set", &out_err);
	if(!pxllr_is_runtime_flag_set)
	{
		return out_err;
	}
	
	pxllr_set_runtime_flag = (void (*)(const char*, uint64_t))load_symbol(cdt_helper_xllr_handle, "set_runtime_flag", &out_err);
	if(!pxllr_set_runtime_flag)
	{
		return out_err;
	}
	
	return NULL;
}

const char* load_xllr()
{
	if(cdt_helper_xllr_handle){
		return NULL;
	}
	
	const char* metaffi_home = getenv("METAFFI_HOME");
	if(!metaffi_home)
	{
		return "METAFFI_HOME is not set";
	}

#ifdef _WIN32
	const char* ext = ".dll";
#elif __APPLE__
	const char* ext = ".dylib";
#else
	const char* ext = ".so";
#endif
	
	char xllr_full_path[300] = {0};
	sprintf(xllr_full_path, "%s/xllr%s", metaffi_home, ext);
	
	char* out_err;
	cdt_helper_xllr_handle = load_library(xllr_full_path, &out_err);
	if(!cdt_helper_xllr_handle)
	{
		// error has occurred
		printf("Failed to load XLLR: %s", out_err);
		return "Failed to load XLLR";
	}
	
	const char* err = load_xllr_api();
	if(err){
		printf("Failed to load xcall function: %s", err);
		return "Failed to load xcall function";
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

#define load_helper_function(name) \
	p##name = (p##name##_t)load_symbol(cdt_helper_xllr_handle, #name, &err); \
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