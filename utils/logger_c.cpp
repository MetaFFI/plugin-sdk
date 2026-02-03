#include "logger_c.h"
#include "logger.hpp"

#include <cstdarg>
#include <cstdio>
#include <vector>

namespace
{
spdlog::level::level_enum to_spdlog_level(int level)
{
	switch(level)
	{
		case METAFFI_LOG_LEVEL_TRACE:
			return spdlog::level::trace;
		case METAFFI_LOG_LEVEL_DEBUG:
			return spdlog::level::debug;
		case METAFFI_LOG_LEVEL_INFO:
			return spdlog::level::info;
		case METAFFI_LOG_LEVEL_WARN:
			return spdlog::level::warn;
		case METAFFI_LOG_LEVEL_ERROR:
			return spdlog::level::err;
		case METAFFI_LOG_LEVEL_CRITICAL:
			return spdlog::level::critical;
		case METAFFI_LOG_LEVEL_OFF:
			return spdlog::level::off;
		default:
			return spdlog::level::info;
	}
}

const char* normalize_component(const char* component)
{
	return component ? component : "metaffi";
}
}

extern "C" void metaffi_log(const char* component, int level, const char* message)
{
	auto logger = metaffi::get_logger(normalize_component(component));
	logger->log(to_spdlog_level(level), "{}", message ? message : "");
}

extern "C" void metaffi_logfv(const char* component, int level, const char* fmt, va_list args)
{
	if(!fmt)
	{
		metaffi_log(component, level, "");
		return;
	}

	va_list args_copy;
	va_copy(args_copy, args);
	int size = std::vsnprintf(nullptr, 0, fmt, args_copy);
	va_end(args_copy);

	if(size < 0)
	{
		metaffi_log(component, level, "Log formatting failed");
		return;
	}

	std::vector<char> buffer(static_cast<size_t>(size) + 1);
	std::vsnprintf(buffer.data(), buffer.size(), fmt, args);
	metaffi_log(component, level, buffer.data());
}

extern "C" void metaffi_logf(const char* component, int level, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	metaffi_logfv(component, level, fmt, args);
	va_end(args);
}