#pragma once

#define trace_line printf("+++ %s: %s:%d\n", __FILE__, __FUNCTION__, __LINE__)

#ifdef __GNUC__
#define DLL_PRIVATE __attribute__ ((visibility ("hidden")))
#else
#define DLL_PRIVATE
#endif



#ifdef __GNUC__
#define DYN_LIB_EVENTS(on_load_code, on_unload_code) \
__attribute__((constructor)) void LibraryLoad(void)\
{\
	on_load_code\
}\
__attribute__((destructor)) void LibraryUnload(void)\
{\
    on_unload_code\
}
#elif defined(_MSC_VER)
#define DYN_LIB_EVENTS(on_load_code, on_unload_code) \
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)\
{\
	switch (ul_reason_for_call)\
	{\
		case DLL_PROCESS_ATTACH:\
		{\
			on_load_code\
		}break;\
		case DLL_PROCESS_DETACH:\
		{\
			on_unload_code\
		}break;\
	}\
	return TRUE;\
}
#endif
