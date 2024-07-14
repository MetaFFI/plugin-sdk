#pragma once

#define trace_line printf("+++ %s: %s:%d\n", __FILE__, __FUNCTION__, __LINE__)

#ifdef __GNUC__
#define DLL_PRIVATE __attribute__ ((visibility ("hidden")))
#else
#define DLL_PRIVATE
#endif
