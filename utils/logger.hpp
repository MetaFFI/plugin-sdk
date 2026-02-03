#pragma once

// ---------------------------------------------------------------------------
// Compile-time level. Default INFO. Override via CMake (see Logging.cmake).
// Must be defined BEFORE spdlog is included. The compiler -D flag always wins
// over this #ifndef, so this is just a safe fallback.
// ---------------------------------------------------------------------------
#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL 2  // INFO
#endif

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>  // stderr_sink_mt (non-color)
#include <string>
#include <memory>

namespace metaffi {

// ---------------------------------------------------------------------------
// get_logger — returns (or creates) a named component logger.
// The first call creates a shared stderr_sink_mt with the standard pattern.
// Subsequent calls for the same name return the cached logger.
// Thread-safe.
// ---------------------------------------------------------------------------
inline std::shared_ptr<spdlog::logger> get_logger(const std::string& name)
{
    // Shared sink: stderr, no color, standard pattern, flush on error+
    static auto shared_sink = []() {
        auto sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
        sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%s:%#] [%n] [%l] - %v");
        return sink;
    }();

    // Fast path: logger already registered
    auto existing = spdlog::get(name);
    if (existing) return existing;

    // Slow path: create and register (thread-safe via try/catch on duplicate)
    try
    {
        auto logger = std::make_shared<spdlog::logger>(name, shared_sink);
        logger->flush_on(spdlog::level::err);
        spdlog::register_logger(logger);
        return logger;
    }
    catch(...)
    {
        // Another thread registered it first — just return it
        return spdlog::get(name);
    }
}

// ---------------------------------------------------------------------------
// Runtime level setters
// ---------------------------------------------------------------------------
inline void set_log_level(spdlog::level::level_enum level)
{
    spdlog::set_level(level);
}

inline void set_log_level(const std::string& component, spdlog::level::level_enum level)
{
    auto logger = spdlog::get(component);
    if (logger) logger->set_level(level);
}

} // namespace metaffi

// ---------------------------------------------------------------------------
// Logging macros. Wrap spdlog's SPDLOG_LOGGER_* which capture __FILE__ /
// __LINE__ at the expansion site (i.e. the caller's source location).
// ---------------------------------------------------------------------------
#define METAFFI_TRACE(logger, ...)     SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__)
#define METAFFI_DEBUG(logger, ...)     SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__)
#define METAFFI_INFO(logger, ...)      SPDLOG_LOGGER_INFO(logger, __VA_ARGS__)
#define METAFFI_WARN(logger, ...)      SPDLOG_LOGGER_WARN(logger, __VA_ARGS__)
#define METAFFI_ERROR(logger, ...)     SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__)
#define METAFFI_CRITICAL(logger, ...)  SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__)

// Exception-aware variants. Append exception.what() to the message.
// Usage: catch(const std::exception& e) { METAFFI_ERROR_EX(LOG, "context", e); }
#define METAFFI_ERROR_EX(logger, msg, ex)     do { (void)(ex); METAFFI_ERROR(logger, "{}: {}", msg, (ex).what()); } while(0)
#define METAFFI_CRITICAL_EX(logger, msg, ex)  do { (void)(ex); METAFFI_CRITICAL(logger, "{}: {}", msg, (ex).what()); } while(0)
#define METAFFI_WARN_EX(logger, msg, ex)      do { (void)(ex); METAFFI_WARN(logger, "{}: {}", msg, (ex).what()); } while(0)
