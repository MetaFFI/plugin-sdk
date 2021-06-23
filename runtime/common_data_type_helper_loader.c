#include "common_data_type_helper_loader.h"

// == Defines ==
#define get_arg_type_str_impl_fptr(type) \
typedef int (*pget_arg_##type##_t) (void**, int, type*, openffi_size*); \
pget_arg_##type##_t pget_arg_##type; \
int get_arg_##type(void** data_array, int index, type* out_res, openffi_size* length){ return pget_arg_##type(data_array, index, out_res, length); }

#define get_arg_type_impl_fptr(type) \
typedef int (*pget_arg_##type##_t)(void**, int, type*); \
pget_arg_##type##_t pget_arg_##type;\
int get_arg_##type(void** data_array, int index, type* out_res){ return pget_arg_##type(data_array, index, out_res); }

#define get_arg_type_str_array_impl_fptr(type) \
typedef int (*pget_arg_##type##_array_t) (void**, int, type**, openffi_size**, openffi_size**, openffi_size*); \
pget_arg_##type##_array_t pget_arg_##type##_array; \
int get_arg_##type##_array(void** data_array, int index, type** array, openffi_size** sizes_array, openffi_size** dimensions_lengths, openffi_size* dimensions){ \
	return pget_arg_##type##_array(data_array, index, array, sizes_array, dimensions_lengths, dimensions); \
}

#define get_arg_type_array_impl_fptr(type) \
typedef int(*pget_arg_##type##_array_t)(void**, int, type**, openffi_size**, openffi_size*); \
pget_arg_##type##_array_t pget_arg_##type##_array; \
int get_arg_##type##_array(void** data_array, int index, type** out_res, openffi_size** dimensions_lengths, openffi_size* dimensions){ \
	return pget_arg_##type##_array(data_array, index, out_res, dimensions_lengths, dimensions); \
}

#define set_arg_openffi_str_impl_fptr(type)\
typedef int (*pset_arg_##type##_t) (void**, int, openffi_string, openffi_size*); \
pset_arg_##type##_t pset_arg_##type; \
int set_arg_##type(void** data_array, int index, openffi_string val, openffi_size* string_length){ return pset_arg_##type(data_array, index, val, string_length); }

#define set_arg_type_impl_fptr(type) \
typedef int (*pset_arg_##type##_t) (void**, int, type*); \
pset_arg_##type##_t pset_arg_##type; \
int set_arg_##type(void** data_array, int index, type* val){ return pset_arg_##type(data_array, index, val); }

#define set_arg_openffi_str_array_impl_fptr(type)\
typedef int (*pset_arg_##type##_array_t) (void**, int, type*, openffi_size*, openffi_size*, openffi_size*); \
pset_arg_##type##_array_t pset_arg_##type##_array; \
int set_arg_##type##_array(void** data_array, int index, type* array, openffi_size* string_sizes, openffi_size* dimensions_lengths, openffi_size* dimensions){ return pset_arg_##type##_array(data_array, index, array, string_sizes, dimensions_lengths, dimensions); }

#define set_arg_type_array_impl_fptr(type) \
typedef int (*pset_arg_##type##_array_t) (void**, int, type*, openffi_size*, openffi_size*); \
pset_arg_##type##_array_t pset_arg_##type##_array; \
int set_arg_##type##_array(void** data_array, int index, type* array, openffi_size* dimensions_lengths, openffi_size* dimensions){ return pset_arg_##type##_array(data_array, index, array, dimensions_lengths, dimensions); }

#define get_numeric_element_impl_fptr(type) \
typedef type (*pget_##type##_element_t) (type*, int); \
pget_##type##_element_t pget_##type##_element; \
type get_##type##_element(type* arr, int index){ return pget_##type##_element(arr, index); }

#define set_numeric_element_impl_fptr(type) \
typedef void (*pset_##type##_element_t) (type*, int, type); \
pset_##type##_element_t pset_##type##_element; \
void set_##type##_element(type* arr, int index, type val){ return pset_##type##_element(arr, index, val); }

#define alloc_numeric_on_heap_impl_fptr(type) \
typedef type* (*palloc_##type##_on_heap_t)(type val); \
palloc_##type##_on_heap_t palloc_##type##_on_heap; \
type* alloc_##type##_on_heap(type val){ return palloc_##type##_on_heap(val); }

#define alloc_str_on_heap_impl_fptr(type) \
typedef type (*palloc_##type##_on_heap_t)(type val, openffi_size str_size); \
palloc_##type##_on_heap_t palloc_##type##_on_heap; \
type alloc_##type##_on_heap(type val, openffi_size str_size){ return palloc_##type##_on_heap(val, str_size); }


// === arg helpers pointer to functions & wrapper functions ===

typedef int8_t (*pis_arg_overflow_t) (uint64_t*, int);
pis_arg_overflow_t pis_arg_overflow;
int8_t is_arg_overflow(uint64_t* size_left, int size){ return pis_arg_overflow(size_left, size); }

typedef int (*pget_type_t)(void** data_array, int index, openffi_type* out_type);
pget_type_t pget_type;
int get_type(void** data_array, int index, openffi_type* out_type){ return pget_type(data_array, index, out_type); }

typedef void** (*palloc_args_buffer_t)(int size);
palloc_args_buffer_t palloc_args_buffer;
void** alloc_args_buffer(int size){ return palloc_args_buffer(size); }

get_arg_type_str_impl_fptr(openffi_string);
get_arg_type_str_impl_fptr(openffi_string8);
get_arg_type_str_impl_fptr(openffi_string16);
get_arg_type_str_impl_fptr(openffi_string32);

typedef int (*pget_arg_pointer_type_t) (void**, int, void**);
pget_arg_pointer_type_t pget_arg_pointer_type;
int get_arg_pointer_type(void** data_array, int index, void** out_res){ return pget_arg_pointer_type(data_array, index, out_res); }

get_arg_type_impl_fptr(openffi_float64);
get_arg_type_impl_fptr(openffi_float32);
get_arg_type_impl_fptr(openffi_int64);
get_arg_type_impl_fptr(openffi_int32);
get_arg_type_impl_fptr(openffi_int16);
get_arg_type_impl_fptr(openffi_int8);
get_arg_type_impl_fptr(openffi_uint64);
get_arg_type_impl_fptr(openffi_uint32);
get_arg_type_impl_fptr(openffi_uint16);
get_arg_type_impl_fptr(openffi_uint8);
get_arg_type_impl_fptr(openffi_size);
get_arg_type_impl_fptr(openffi_bool);

get_arg_type_str_array_impl_fptr(openffi_string);
get_arg_type_str_array_impl_fptr(openffi_string8);
get_arg_type_str_array_impl_fptr(openffi_string16);
get_arg_type_str_array_impl_fptr(openffi_string32);

get_arg_type_array_impl_fptr(openffi_float64);
get_arg_type_array_impl_fptr(openffi_float32);
get_arg_type_array_impl_fptr(openffi_int64);
get_arg_type_array_impl_fptr(openffi_int32);
get_arg_type_array_impl_fptr(openffi_int16);
get_arg_type_array_impl_fptr(openffi_int8);
get_arg_type_array_impl_fptr(openffi_uint64);
get_arg_type_array_impl_fptr(openffi_uint32);
get_arg_type_array_impl_fptr(openffi_uint16);
get_arg_type_array_impl_fptr(openffi_uint8);
get_arg_type_array_impl_fptr(openffi_size);
get_arg_type_array_impl_fptr(openffi_bool);

typedef int (*pset_arg_t)(void**, int, void*);
pset_arg_t pset_arg;
int set_arg(void** data_array, int index, void* val){ return pset_arg(data_array, index, val); }

set_arg_openffi_str_impl_fptr(openffi_string);
set_arg_openffi_str_impl_fptr(openffi_string8);
set_arg_openffi_str_impl_fptr(openffi_string16);
set_arg_openffi_str_impl_fptr(openffi_string32);

set_arg_type_impl_fptr(openffi_float64);
set_arg_type_impl_fptr(openffi_float32);
set_arg_type_impl_fptr(openffi_int64);
set_arg_type_impl_fptr(openffi_int32);
set_arg_type_impl_fptr(openffi_int16);
set_arg_type_impl_fptr(openffi_int8);
set_arg_type_impl_fptr(openffi_uint64);
set_arg_type_impl_fptr(openffi_uint32);
set_arg_type_impl_fptr(openffi_uint16);
set_arg_type_impl_fptr(openffi_uint8);
set_arg_type_impl_fptr(openffi_size);
set_arg_type_impl_fptr(openffi_bool);

set_arg_openffi_str_array_impl_fptr(openffi_string);
set_arg_openffi_str_array_impl_fptr(openffi_string8);
set_arg_openffi_str_array_impl_fptr(openffi_string16);
set_arg_openffi_str_array_impl_fptr(openffi_string32);

set_arg_type_array_impl_fptr(openffi_float64);
set_arg_type_array_impl_fptr(openffi_float32);
set_arg_type_array_impl_fptr(openffi_int64);
set_arg_type_array_impl_fptr(openffi_int32);
set_arg_type_array_impl_fptr(openffi_int16);
set_arg_type_array_impl_fptr(openffi_int8);
set_arg_type_array_impl_fptr(openffi_uint64);
set_arg_type_array_impl_fptr(openffi_uint32);
set_arg_type_array_impl_fptr(openffi_uint16);
set_arg_type_array_impl_fptr(openffi_uint8);
set_arg_type_array_impl_fptr(openffi_size);
set_arg_type_array_impl_fptr(openffi_bool);

typedef const char* (*pget_openffi_string_element_t)(int index, openffi_string *str, const openffi_size *sizes, openffi_size *out_size);
pget_openffi_string_element_t pget_openffi_string_element;
const char* get_openffi_string_element(int index, openffi_string *str, const openffi_size *sizes, openffi_size *out_size){ return pget_openffi_string_element(index, str, sizes, out_size); }

get_numeric_element_impl_fptr(openffi_float64);
get_numeric_element_impl_fptr(openffi_float32);
get_numeric_element_impl_fptr(openffi_int64);
get_numeric_element_impl_fptr(openffi_int32);
get_numeric_element_impl_fptr(openffi_int16);
get_numeric_element_impl_fptr(openffi_int8);
get_numeric_element_impl_fptr(openffi_uint64);
get_numeric_element_impl_fptr(openffi_uint32);
get_numeric_element_impl_fptr(openffi_uint16);
get_numeric_element_impl_fptr(openffi_uint8);
get_numeric_element_impl_fptr(openffi_size);

typedef void (*pset_openffi_string_element_t)(int index, openffi_string *str_array, openffi_size *sizes_array, openffi_string str, openffi_size str_size);
pset_openffi_string_element_t pset_openffi_string_element;
void set_openffi_string_element(int index, openffi_string *str_array, openffi_size *sizes_array, openffi_string str, openffi_size str_size){ pset_openffi_string_element(index, str_array, sizes_array, str, str_size); }

set_numeric_element_impl_fptr(openffi_float64);
set_numeric_element_impl_fptr(openffi_float32);
set_numeric_element_impl_fptr(openffi_int64);
set_numeric_element_impl_fptr(openffi_int32);
set_numeric_element_impl_fptr(openffi_int16);
set_numeric_element_impl_fptr(openffi_int8);
set_numeric_element_impl_fptr(openffi_uint64);
set_numeric_element_impl_fptr(openffi_uint32);
set_numeric_element_impl_fptr(openffi_uint16);
set_numeric_element_impl_fptr(openffi_uint8);
set_numeric_element_impl_fptr(openffi_size);


alloc_numeric_on_heap_impl_fptr(openffi_float64);
alloc_numeric_on_heap_impl_fptr(openffi_float32);
alloc_numeric_on_heap_impl_fptr(openffi_int64);
alloc_numeric_on_heap_impl_fptr(openffi_int32);
alloc_numeric_on_heap_impl_fptr(openffi_int16);
alloc_numeric_on_heap_impl_fptr(openffi_int8);
alloc_numeric_on_heap_impl_fptr(openffi_uint64);
alloc_numeric_on_heap_impl_fptr(openffi_uint32);
alloc_numeric_on_heap_impl_fptr(openffi_uint16);
alloc_numeric_on_heap_impl_fptr(openffi_uint8);
alloc_numeric_on_heap_impl_fptr(openffi_size);
alloc_numeric_on_heap_impl_fptr(openffi_bool);

alloc_str_on_heap_impl_fptr(openffi_string);
alloc_str_on_heap_impl_fptr(openffi_string8);
alloc_str_on_heap_impl_fptr(openffi_string16);
alloc_str_on_heap_impl_fptr(openffi_string32);


// === XLLR function pointers ===
void (*pxllr_call)(const char*, uint32_t, int64_t, void**, uint64_t, void**, uint64_t, char**, uint64_t*);
void xllr_call(const char* runtime_plugin_name, uint32_t runtime_plugin_name_len,
				int64_t function_id,
				void** parameters, uint64_t parameters_length,
				void** return_values, uint64_t return_values_length,
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

// === Handlers ===
void* cdt_helper_xllr_handle = NULL;

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
	pxllr_call = (void (*)(const char*, uint32_t, int64_t, void**, uint64_t, void**, uint64_t, char**, uint64_t*)) load_symbol(cdt_helper_xllr_handle, "call", &out_err);
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
	if(!pxllr_load_function)
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
	
	const char* openffi_home = getenv("OPENFFI_HOME");
	if(!openffi_home)
	{
		return "OPENFFI_HOME is not set";
	}

#ifdef _WIN32
	const char* ext = ".dll";
#elif __APPLE__
	const char* ext = ".dylib";
#else
	const char* ext = ".so";
#endif
	
	char xllr_full_path[300] = {0};
	sprintf(xllr_full_path, "%s/xllr%s", openffi_home, ext);
	
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
		printf("Failed to load call function: %s", err);
		return "Failed to load call function";
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
const char* load_args_helpers()
{
	if(pis_arg_overflow){
		return NULL;
	}

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
	
	load_helper_function(is_arg_overflow);
	load_helper_function(alloc_args_buffer);
	load_helper_function(get_arg_pointer_type);
	load_helper_function(set_arg);
	load_helper_function(get_openffi_string_element);
	load_helper_function(set_openffi_string_element);
	load_helper_function(get_type);

#define get_arg_type_str_impl_fptr_assign(type) \
	load_helper_function(get_arg_##type)
	
	get_arg_type_str_impl_fptr_assign(openffi_string);
	get_arg_type_str_impl_fptr_assign(openffi_string8);
	get_arg_type_str_impl_fptr_assign(openffi_string16);
	get_arg_type_str_impl_fptr_assign(openffi_string32);

#define get_arg_type_impl_fptr_assign(type) \
	load_helper_function(get_arg_##type)
	
	get_arg_type_impl_fptr_assign(openffi_float64);
	get_arg_type_impl_fptr_assign(openffi_float32);
	get_arg_type_impl_fptr_assign(openffi_int64);
	get_arg_type_impl_fptr_assign(openffi_int32);
	get_arg_type_impl_fptr_assign(openffi_int16);
	get_arg_type_impl_fptr_assign(openffi_int8);
	get_arg_type_impl_fptr_assign(openffi_uint64);
	get_arg_type_impl_fptr_assign(openffi_uint32);
	get_arg_type_impl_fptr_assign(openffi_uint16);
	get_arg_type_impl_fptr_assign(openffi_uint8);
	get_arg_type_impl_fptr_assign(openffi_size);
	get_arg_type_impl_fptr_assign(openffi_bool);

#define get_arg_type_str_array_impl_fptr_assign(type) \
	load_helper_function(get_arg_##type##_array)
	
	get_arg_type_str_array_impl_fptr_assign(openffi_string);
	get_arg_type_str_array_impl_fptr_assign(openffi_string8);
	get_arg_type_str_array_impl_fptr_assign(openffi_string16);
	get_arg_type_str_array_impl_fptr_assign(openffi_string32);

#define get_arg_type_array_impl_fptr_assign(type) \
	load_helper_function(get_arg_##type##_array);
	
	get_arg_type_array_impl_fptr_assign(openffi_float64);
	get_arg_type_array_impl_fptr_assign(openffi_float32);
	get_arg_type_array_impl_fptr_assign(openffi_int64);
	get_arg_type_array_impl_fptr_assign(openffi_int32);
	get_arg_type_array_impl_fptr_assign(openffi_int16);
	get_arg_type_array_impl_fptr_assign(openffi_int8);
	get_arg_type_array_impl_fptr_assign(openffi_uint64);
	get_arg_type_array_impl_fptr_assign(openffi_uint32);
	get_arg_type_array_impl_fptr_assign(openffi_uint16);
	get_arg_type_array_impl_fptr_assign(openffi_uint8);
	get_arg_type_array_impl_fptr_assign(openffi_size);
	get_arg_type_array_impl_fptr_assign(openffi_bool);

#define set_arg_openffi_str_impl_fptr_assign(type) \
	load_helper_function(set_arg_##type);
	
	set_arg_openffi_str_impl_fptr_assign(openffi_string);
	set_arg_openffi_str_impl_fptr_assign(openffi_string8);
	set_arg_openffi_str_impl_fptr_assign(openffi_string16);
	set_arg_openffi_str_impl_fptr_assign(openffi_string32);

#define set_arg_type_impl_fptr_assign(type) \
	load_helper_function(set_arg_##type);
	
	set_arg_type_impl_fptr_assign(openffi_float64);
	set_arg_type_impl_fptr_assign(openffi_float32);
	set_arg_type_impl_fptr_assign(openffi_int64);
	set_arg_type_impl_fptr_assign(openffi_int32);
	set_arg_type_impl_fptr_assign(openffi_int16);
	set_arg_type_impl_fptr_assign(openffi_int8);
	set_arg_type_impl_fptr_assign(openffi_uint64);
	set_arg_type_impl_fptr_assign(openffi_uint32);
	set_arg_type_impl_fptr_assign(openffi_uint16);
	set_arg_type_impl_fptr_assign(openffi_uint8);
	set_arg_type_impl_fptr_assign(openffi_size);
	set_arg_type_impl_fptr_assign(openffi_bool);

#define set_arg_openffi_str_array_impl_fptr_assign(type) \
	load_helper_function(set_arg_##type##_array);
	
	set_arg_openffi_str_array_impl_fptr_assign(openffi_string);
	set_arg_openffi_str_array_impl_fptr_assign(openffi_string8);
	set_arg_openffi_str_array_impl_fptr_assign(openffi_string16);
	set_arg_openffi_str_array_impl_fptr_assign(openffi_string32);

#define set_arg_type_array_impl_fptr_assign(type) \
	load_helper_function(set_arg_##type##_array);
	
	set_arg_type_array_impl_fptr_assign(openffi_float64);
	set_arg_type_array_impl_fptr_assign(openffi_float32);
	set_arg_type_array_impl_fptr_assign(openffi_int64);
	set_arg_type_array_impl_fptr_assign(openffi_int32);
	set_arg_type_array_impl_fptr_assign(openffi_int16);
	set_arg_type_array_impl_fptr_assign(openffi_int8);
	set_arg_type_array_impl_fptr_assign(openffi_uint64);
	set_arg_type_array_impl_fptr_assign(openffi_uint32);
	set_arg_type_array_impl_fptr_assign(openffi_uint16);
	set_arg_type_array_impl_fptr_assign(openffi_uint8);
	set_arg_type_array_impl_fptr_assign(openffi_size);
	set_arg_type_array_impl_fptr_assign(openffi_bool);

#define get_numeric_element_impl_fptr_assign(type) \
	load_helper_function(get_##type##_element);
	
	get_numeric_element_impl_fptr_assign(openffi_float64);
	get_numeric_element_impl_fptr_assign(openffi_float32);
	get_numeric_element_impl_fptr_assign(openffi_int64);
	get_numeric_element_impl_fptr_assign(openffi_int32);
	get_numeric_element_impl_fptr_assign(openffi_int16);
	get_numeric_element_impl_fptr_assign(openffi_int8);
	get_numeric_element_impl_fptr_assign(openffi_uint64);
	get_numeric_element_impl_fptr_assign(openffi_uint32);
	get_numeric_element_impl_fptr_assign(openffi_uint16);
	get_numeric_element_impl_fptr_assign(openffi_uint8);
	get_numeric_element_impl_fptr_assign(openffi_size);

#define set_numeric_element_impl_fptr_assign(type) \
	load_helper_function(set_##type##_element);
	
	set_numeric_element_impl_fptr_assign(openffi_float64);
	set_numeric_element_impl_fptr_assign(openffi_float32);
	set_numeric_element_impl_fptr_assign(openffi_int64);
	set_numeric_element_impl_fptr_assign(openffi_int32);
	set_numeric_element_impl_fptr_assign(openffi_int16);
	set_numeric_element_impl_fptr_assign(openffi_int8);
	set_numeric_element_impl_fptr_assign(openffi_uint64);
	set_numeric_element_impl_fptr_assign(openffi_uint32);
	set_numeric_element_impl_fptr_assign(openffi_uint16);
	set_numeric_element_impl_fptr_assign(openffi_uint8);
	set_numeric_element_impl_fptr_assign(openffi_size);
	
#define alloc_numeric_on_heap_impl_fptr_assign(type) \
	load_helper_function(alloc_##type##_on_heap)
	
	alloc_numeric_on_heap_impl_fptr_assign(openffi_float64);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_float32);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_int64);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_int32);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_int16);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_int8);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_uint64);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_uint32);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_uint16);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_uint8);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_size);
	alloc_numeric_on_heap_impl_fptr_assign(openffi_bool);

#define alloc_str_on_heap_impl_fptr_assign(type) \
	load_helper_function(alloc_##type##_on_heap)
	
	alloc_str_on_heap_impl_fptr_assign(openffi_string);
	alloc_str_on_heap_impl_fptr_assign(openffi_string8);
	alloc_str_on_heap_impl_fptr_assign(openffi_string16);
	alloc_str_on_heap_impl_fptr_assign(openffi_string32);

	return NULL;
}

#undef common_data_type_helper_loader_c