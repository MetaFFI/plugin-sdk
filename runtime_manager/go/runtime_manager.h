#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <optional>
#include <cstdint>

class Module;

/**
 * Confidence level for Go shared library detection
 */
enum class go_detect_confidence
{
	no,       // Not a Go shared library
	probable, // Found valid-looking buildinfo header but couldn't fully validate
	yes       // Found header and decoded inline Go version string (Go 1.18+)
};

/**
 * Result of Go shared library detection
 */
struct go_detect_result
{
	go_detect_confidence confidence = go_detect_confidence::no;
	std::string reason;
	std::optional<std::string> go_version;  // Go version if detected (e.g., "go1.21.5")
	std::uint64_t file_offset = 0;          // Offset where buildinfo magic was found
};

/**
 * Go Runtime Manager
 *
 * Manages Go shared libraries (.dll/.so) compiled with -buildmode=c-shared.
 * Go compiles to standalone native shared libraries with embedded runtime;
 * there is no external interpreter to load. This manager only loads modules
 * and resolves entities. Go installation detection belongs in the compiler.
 */
class go_runtime_manager
{
public:
	/**
	 * Default constructor
	 */
	go_runtime_manager();

	/**
	 * Destructor
	 */
	~go_runtime_manager();

	/**
	 * Check if a shared library is a Go shared library
	 *
	 * Detects Go shared libraries by scanning for the Go buildinfo magic bytes
	 * ("\xff Go buildinf:") which are embedded by the Go compiler in all
	 * c-shared builds. This is the same method Go's debug/buildinfo package uses.
	 *
	 * @param library_path Path to the shared library (.dll/.so/.dylib)
	 * @return Detection result with confidence level and optional Go version
	 * @throws std::runtime_error if file doesn't exist or can't be opened
	 */
	static go_detect_result is_go_shared_library(const std::string& library_path);

	/**
	 * Load runtime
	 * No-op for Go: there is no external runtime to load (Go compiles to standalone binaries).
	 * Exists for API consistency with other runtime managers.
	 */
	void load_runtime();

	/**
	 * Release runtime
	 * Marks as unloaded. Note: Go doesn't support proper dlclose.
	 */
	void release_runtime();

	/**
	 * Load a Go shared library module.
	 * Validates the file is a Go shared library via is_go_shared_library() before loading.
	 * @param module_path Path to the .dll/.so file
	 * @return Shared pointer to Module instance
	 * @throws std::runtime_error if file is not a Go shared library or loading fails
	 */
	std::shared_ptr<Module> load_module(const std::string& module_path);

	/**
	 * Check if runtime is loaded
	 * @return true if runtime is loaded
	 */
	bool is_runtime_loaded() const;

private:
	bool m_isRuntimeLoaded = false;
	mutable std::mutex m_mutex;
};
