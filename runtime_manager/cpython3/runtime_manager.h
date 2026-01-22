#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>

// Forward declarations
class Module;

/**
 * CPython3 Runtime Manager
 * 
 * Manages the Python 3.8-3.13 runtime environment.
 * Provides thread-safe runtime initialization, module loading, and cleanup.
 */
class cpython3_runtime_manager
{
public:
	/**
	 * Constructor
	 * @param python_version Python version string (e.g., "3.8", "3.9", "3.13")
	 */
	explicit cpython3_runtime_manager(const std::string& python_version);
	
	/**
	 * Destructor - automatically releases runtime if still loaded
	 */
	~cpython3_runtime_manager();
	
	/**
	 * Detect installed Python 3 versions (3.8-3.13)
	 * @return Vector of version strings (e.g., ["3.8", "3.9", "3.11"])
	 */
	static std::vector<std::string> detect_installed_python3();
	
	/**
	 * Load and initialize the Python runtime
	 * @throws std::exception on failure
	 */
	void load_runtime();
	
	/**
	 * Release and finalize the Python runtime
	 * @throws std::exception on failure
	 */
	void release_runtime();
	
	/**
	 * Load a Python module
	 * @param module_path Path to the module (file path or module name)
	 * @return Shared pointer to Module instance
	 * @throws std::exception on failure
	 */
	std::shared_ptr<Module> load_module(const std::string& module_path);
	
	/**
	 * Check if runtime is currently loaded
	 * @return true if runtime is loaded, false otherwise
	 */
	bool is_runtime_loaded() const;
	
	/**
	 * Get the Python version this cpython3_runtime_manager is configured for
	 * @return Python version string
	 */
	const std::string& get_python_version() const;

private:
	std::string m_pythonVersion;
	bool m_isRuntimeLoaded;
	bool m_isEmbedded;  // True if we initialized the interpreter
	mutable std::mutex m_mutex;  // Thread safety
	
	// Helper methods
	void initialize_environment();
	std::string check_python_error() const;
};
