#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_MSC_VER)
#define metaffi_sprintf(dest, destsz, fmt, ...) sprintf_s((dest), (destsz), (fmt), __VA_ARGS__)
#define metaffi_strncpy(dest, destsz, src, count) strncpy_s((dest), (destsz), (src), (count))
#define metaffi_strcpy(dest, destsz, src) strcpy_s((dest), (destsz), (src))

static inline char* metaffi_getenv_alloc_impl(const char* name)
{
	char* value = NULL;
	size_t value_size = 0;
	if(_dupenv_s(&value, &value_size, name) != 0 || value == NULL)
	{
		return NULL;
	}
	return value;
}

#define metaffi_getenv_alloc(name) metaffi_getenv_alloc_impl(name)
#define metaffi_free_env(ptr) free(ptr)
#else
#define metaffi_sprintf(dest, destsz, fmt, ...) snprintf((dest), (destsz), (fmt), __VA_ARGS__)
#define metaffi_strncpy(dest, destsz, src, count) strncpy((dest), (src), (count))
#define metaffi_strcpy(dest, destsz, src) strcpy((dest), (src))
#define metaffi_getenv_alloc(name) getenv(name)
#define metaffi_free_env(ptr) ((void)0)
#endif
