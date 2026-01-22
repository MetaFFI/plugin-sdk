#pragma once

#include <string>
#include <memory>
#include <mutex>

#include <boost/dll/shared_library.hpp>

class go_runtime_manager;
class Entity;

/**
 * Go Module
 *
 * Represents a loaded Go shared library (.dll/.so).
 * Uses boost::dll for cross-platform dynamic library loading.
 *
 * Note: Go doesn't properly support dlclose (Go issue #11100),
 * so unload() is a no-op and modules stay loaded until process exit.
 */
class Module
{
public:
	/**
	 * Constructor - loads the shared library
	 * @param manager Pointer to the runtime manager
	 * @param module_path Path to the .dll/.so file
	 * @throws std::runtime_error if loading fails
	 */
	Module(go_runtime_manager* manager, const std::string& module_path);

	/**
	 * Destructor
	 */
	~Module();

	// Copy semantics
	Module(const Module& other);
	Module& operator=(const Module& other);

	// Move semantics
	Module(Module&& other) noexcept;
	Module& operator=(Module&& other) noexcept;

	/**
	 * Load an entity (exported function) from the module
	 * @param entity_path Entity path in format "callable=FunctionName"
	 * @return Shared pointer to Entity
	 * @throws std::runtime_error if symbol not found
	 */
	std::shared_ptr<Entity> load_entity(const std::string& entity_path);

	/**
	 * Unload the module
	 * Note: This is a no-op for Go modules due to Go limitation
	 */
	void unload();

	/**
	 * Get the module path
	 * @return Reference to the module path string
	 */
	const std::string& get_module_path() const;

	/**
	 * Check if a symbol exists in the module
	 * @param symbol_name Name of the exported symbol
	 * @return true if symbol exists
	 */
	bool has_symbol(const std::string& symbol_name) const;

private:
	go_runtime_manager* m_runtimeManager = nullptr;
	std::string m_modulePath;
	std::shared_ptr<boost::dll::shared_library> m_library;
	mutable std::mutex m_mutex;

	// Helper to parse entity_path and extract function name
	static std::string parse_entity_path(const std::string& entity_path);
};
