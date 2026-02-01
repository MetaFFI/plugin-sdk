package guest

/*
#cgo !windows LDFLAGS: -L. -ldl
#cgo windows LDFLAGS: -L.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../../utils/safe_func.h"

#ifndef _WIN32
#include <dlfcn.h>
int call_guest_test()
{
	char* metaffi_home = metaffi_getenv_alloc("METAFFI_HOME");
	if(!metaffi_home)
	{
		printf("METAFFI_HOME is not set\n");
		return -1;
	}
	char lib_dir[260] = {0};
	metaffi_sprintf(lib_dir, sizeof(lib_dir), "%s/xllr.test.so", metaffi_home);
	metaffi_free_env(metaffi_home);

	void* lib_handle = dlopen(lib_dir, RTLD_NOW);
	if(!lib_handle)
	{
		printf("Failed loading library - %s\n", dlerror());
		return -1;
	}

	void* res = dlsym(lib_handle, "test_guest");
	if(!res)
	{
		printf("Failed loading symbol test_guest from xllr.test.so - %s\n", dlerror());
		return -1;
	}

	return ((int (*) (const char*, const char*))res)("xllr.go", "module=$PWD/temp,package=GoFuncs,function=F1,metaffi_guest_lib=$PWD/temp/test_MetaFFIGuest,entrypoint_function=EntryPoint_F1");
}
#else
#include <windows.h>
int call_guest_test()
{
	char* metaffi_home = metaffi_getenv_alloc("METAFFI_HOME");
	if(!metaffi_home)
	{
		printf("METAFFI_HOME is not set\n");
		return -1;
	}
	char lib_dir[260] = {0};
	metaffi_sprintf(lib_dir, sizeof(lib_dir), "%s/xllr.test.dll", metaffi_home);
	metaffi_free_env(metaffi_home);

	void* lib_handle = LoadLibraryA(lib_dir);
	if(!lib_handle)
	{
		printf("Failed loading library %s - 0x%x\n", lib_dir, GetLastError());
		return -1;
	}

	void* res = GetProcAddress(lib_handle, "test_guest");
	if(!res)
	{
		printf("Failed loading symbol test_guest from xllr.test.dll - 0x%x\n", GetLastError());
		return -1;
	}

	return ((int (*) (const char*, const char*))res)("xllr.go", "module=$PWD/temp,package=GoFuncs,function=F1,metaffi_guest_lib=$PWD/temp/test_MetaFFIGuest,entrypoint_function=EntryPoint_F1");
}
#endif
*/
import "C"

func CallHostMock() int{
	return int(C.call_guest_test())
}
