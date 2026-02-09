// logger_c_stub.c — C-only stub for metaffi_logf / metaffi_log / metaffi_logfv.
//
// The production xllr_capi_loader.c (included via CGo's <include/xllr_capi_loader.c>)
// calls metaffi_logf which is normally provided by the C++ logger_c.cpp (via spdlog).
// CGo cannot link C++, so we provide lightweight C-only implementations that forward
// to stderr.  This file is compiled automatically by CGo because it lives in the
// package directory.

#include <stdio.h>
#include <stdarg.h>

#ifndef METAFFI_LOG_LEVEL_ERROR
#define METAFFI_LOG_LEVEL_ERROR 4
#endif

void metaffi_log(const char* component, int level, const char* message)
{
	if (level >= METAFFI_LOG_LEVEL_ERROR) {
		fprintf(stderr, "[%s] %s\n", component ? component : "?", message ? message : "");
	}
}

void metaffi_logf(const char* component, int level, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (level >= METAFFI_LOG_LEVEL_ERROR) {
		fprintf(stderr, "[%s] ", component ? component : "?");
		vfprintf(stderr, fmt ? fmt : "", args);
		fprintf(stderr, "\n");
	}
	va_end(args);
}

void metaffi_logfv(const char* component, int level, const char* fmt, va_list args)
{
	if (level >= METAFFI_LOG_LEVEL_ERROR) {
		fprintf(stderr, "[%s] ", component ? component : "?");
		vfprintf(stderr, fmt ? fmt : "", args);
		fprintf(stderr, "\n");
	}
}
