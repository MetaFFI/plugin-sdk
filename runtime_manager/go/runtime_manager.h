#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>
#include <cstdint>

class Module;

/**
 * Information about an installed Go toolchain
 */
struct go_installed_info
{
	std::string version;   // e.g., "go1.21.5"
	std::string goroot;    // GOROOT path
	std::string go_exe;    // Path to go executable
};

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
 * Unlike JDK/CPython, Go compiles to native shared libraries with embedded runtime,
 * so there's no interpreter to manage - just library loading and symbol resolution.
 */
class go_runtime_manager
{
public:
	/**
	 * Constructor
	 * @param info Go installation information
	 */
	explicit go_runtime_manager(const go_installed_info& info);

	/**
	 * Destructor
	 */
	~go_runtime_manager();

	/**
	 * Detect installed Go toolchains
	 * Searches GOROOT environment variable and PATH for go executable
	 * @return Vector of detected Go installations
	 */
	static std::vector<go_installed_info> detect_installed_go();

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
	 * Load/validate runtime
	 * For Go, this just validates the Go installation exists
	 */
	void load_runtime();

	/**
	 * Release runtime
	 * Marks as unloaded. Note: Go doesn't support proper dlclose.
	 */
	void release_runtime();

	/**
	 * Load a Go shared library module
	 * @param module_path Path to the .dll/.so file
	 * @return Shared pointer to Module instance
	 */
	std::shared_ptr<Module> load_module(const std::string& module_path);

	/**
	 * Check if runtime is loaded
	 * @return true if runtime is loaded
	 */
	bool is_runtime_loaded() const;

	/**
	 * Get Go installation info
	 * @return Reference to go_installed_info
	 */
	const go_installed_info& get_go_info() const;

private:
	go_installed_info m_info;
	bool m_isRuntimeLoaded = false;
	mutable std::mutex m_mutex;

	// Helper functions for detection
	static std::vector<std::string> find_go_from_path();
	static std::string resolve_goroot_from_executable(const std::string& go_exe);
	static std::string get_go_version(const std::string& go_exe);
	static bool is_valid_go_installation(const std::string& goroot);
};
