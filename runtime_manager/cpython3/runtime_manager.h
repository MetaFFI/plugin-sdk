#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include "python_api_wrapper.h"

// Forward declarations
class Module;
struct cdt_metaffi_handle;

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
	 * @brief RAII wrapper for Python GIL (Global Interpreter Lock)
	 * 
	 * Ensures GIL is acquired on construction and released on destruction.
	 * Uses the dynamically loaded Python API function pointers.
	 * 
	 * Usage:
	 *   auto gil = runtime_manager.acquire_gil();
	 *   // GIL is held until gil goes out of scope
	 */
	class scoped_gil
	{
	public:
		scoped_gil();
		~scoped_gil();

		// Non-copyable, non-movable
		scoped_gil(const scoped_gil&) = delete;
		scoped_gil& operator=(const scoped_gil&) = delete;
		scoped_gil(scoped_gil&&) = delete;
		scoped_gil& operator=(scoped_gil&&) = delete;

	private:
		PyGILState_STATE m_state;
	};

	/**
	 * @brief Acquire the Python GIL using RAII
	 * @return scoped_gil object that releases GIL on destruction
	 * @throws std::runtime_error if runtime is not loaded
	 */
	[[nodiscard]] scoped_gil acquire_gil() const;

	/**
	 * @brief Handle releaser callback for Python objects stored in cdt_metaffi_handle
	 * 
	 * This static method is used as a callback when a cdt_metaffi_handle containing
	 * a PyObject* needs to be released. It properly acquires the GIL before
	 * decrementing the reference count.
	 * 
	 * @param handle Handle containing PyObject* to release
	 */
	static void py_object_releaser(cdt_metaffi_handle* handle);

public:
	/**
	 * If CPython3 is already loaded in the process, returns a shared pointer to a new instance
	 * that wraps the existing Python runtime.
	 * @return Shared pointer to the cpython3_runtime_manager instance, or nullptr if Python is not loaded
	 */
	static std::shared_ptr<cpython3_runtime_manager> load_loaded_cpython3();

	/**
	 * Creates a new cpython3_runtime_manager for the specified Python version
	 * @param python_version Python version string (e.g., "3.8", "3.9", "3.13")
	 * @return Shared pointer to the cpython3_runtime_manager instance
	 * @throws std::exception on failure to load Python runtime
	 */
	static std::shared_ptr<cpython3_runtime_manager> create(const std::string& python_version);
	
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

	/**
	 * Append a path to sys.path
	 * @param path Filesystem path to add
	 * @throws std::exception on failure
	 */
	void add_sys_path(const std::string& path);

	/**
	 * Import a Python module by name
	 * @param module_name Module name to import
	 * @throws std::exception on failure
	 */
	void import_module(const std::string& module_name);

	/**
	 * Import the metaffi package into the Python runtime
	 * Sets sys.__loading_within_xllr_python3 flag during import
	 * @throws std::runtime_error if import fails
	 */
	void import_metaffi_package();

private:
	explicit cpython3_runtime_manager(const std::string& python_version);

	std::string m_python_version;
	bool m_is_runtime_loaded;
	bool m_is_embedded;  // True if we initialized the interpreter
	mutable std::mutex m_mutex;  // Thread safety
	
	// Helper methods
	void load_runtime();
	void initialize_environment();
	std::string check_python_error() const;
};
