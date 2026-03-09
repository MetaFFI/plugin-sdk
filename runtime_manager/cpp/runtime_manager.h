#pragma once

#include "cpp_abi.h"

#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>

class Module;

/**
 * C/C++ Runtime Manager
 *
 * Manages loaded C/C++ shared libraries. Unlike the JVM or CPython3 runtime
 * managers there is no external interpreter to initialise: load_runtime() is
 * essentially a no-op that sets the "loaded" flag for API consistency with
 * other MetaFFI runtime managers.
 *
 * Key responsibilities:
 *  - Maintain a per-path module cache so the same library is opened only once.
 *  - Detect the ABI of each loaded module and hard-fail on MSVC ↔ Itanium mismatch.
 *  - Provide release_runtime() to close all cached modules at once.
 *
 * Thread safety: all public methods are guarded by an internal mutex.
 */
class cpp_runtime_manager
{
public:
	/**
	 * Construct the manager. Does NOT load any runtime yet.
	 */
	cpp_runtime_manager();

	/**
	 * Destructor. Calls release_runtime() (exceptions are swallowed with a
	 * log to stderr — destructors must not throw).
	 */
	~cpp_runtime_manager();

	/**
	 * Mark the runtime as loaded.
	 * No-op beyond setting the is_runtime_loaded() flag. Idempotent.
	 */
	void load_runtime();

	/**
	 * Mark the runtime as unloaded and clear the module cache.
	 * Modules held by callers via shared_ptr remain valid until those
	 * shared_ptrs are released. Idempotent.
	 */
	void release_runtime();

	/**
	 * Load (or return a cached) C/C++ shared library module.
	 *
	 * If load_runtime() has not been called, it is called automatically.
	 *
	 * @param module_path  Path to the .dll / .so file.
	 * @return             Shared pointer to the loaded Module.
	 * @throws std::runtime_error if the file does not exist, cannot be loaded,
	 *         or has an incompatible ABI.
	 */
	std::shared_ptr<Module> load_module(const std::string& module_path);

	/**
	 * Returns true after load_runtime() has been called and before
	 * release_runtime() clears the flag.
	 */
	bool is_runtime_loaded() const;

	/**
	 * Returns the ABI this plugin binary was compiled with.
	 * (Compile-time constant — never changes at runtime.)
	 */
	cpp_abi get_abi() const;

private:
	cpp_abi    m_pluginAbi;
	bool       m_isRuntimeLoaded = false;
	mutable std::mutex m_mutex;

	// Owns strong references to all loaded modules; cleared by release_runtime().
	std::unordered_map<std::string, std::shared_ptr<Module>> m_moduleCache;
};
