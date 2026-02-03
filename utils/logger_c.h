#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define METAFFI_LOG_LEVEL_TRACE 0
#define METAFFI_LOG_LEVEL_DEBUG 1
#define METAFFI_LOG_LEVEL_INFO 2
#define METAFFI_LOG_LEVEL_WARN 3
#define METAFFI_LOG_LEVEL_ERROR 4
#define METAFFI_LOG_LEVEL_CRITICAL 5
#define METAFFI_LOG_LEVEL_OFF 6

void metaffi_log(const char* component, int level, const char* message);
void metaffi_logf(const char* component, int level, const char* fmt, ...);
void metaffi_logfv(const char* component, int level, const char* fmt, va_list args);

#ifdef __cplusplus
}
#endif