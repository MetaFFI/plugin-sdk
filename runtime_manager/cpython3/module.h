#pragma once

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include "python_h_declares.h"

// Forward declarations
class cpython3_runtime_manager;
class Entity;

/**
 * Python Module
 * 
 * Represents a loaded Python module. Provides thread-safe entity loading.
 * Uses RAII for PyObject* management.
 */
class Module
{
public:
	/**
	 * Constructor
	 * @param runtime_manager Pointer to cpython3_runtime_manager instance
	 * @param module_path Path to the module (file path or module name)
	 * @throws std::exception on failure
	 */
	Module(cpython3_runtime_manager* runtime_manager, const std::string& module_path);
	
	/**
	 * Destructor - releases PyObject* references
	 */
	~Module();
	
	/**
	 * Copy constructor - increments PyObject refcount
	 */
	Module(const Module& other);
	
	/**
	 * Move constructor - transfers ownership
	 */
	Module(Module&& other) noexcept;
	
	/**
	 * Copy assignment - manages refcounts
	 */
	Module& operator=(const Module& other);
	
	/**
	 * Move assignment - transfers ownership
	 */
	Module& operator=(Module&& other) noexcept;
	
	/**
	 * Get the module path
	 * @return Module path string
	 */
	const std::string& get_module_path() const;
	
	/**
	 * Load an entity from the module
	 * @param entity_path Entity path string (from idl_entities/entity_path_specs.json)
	 * @param params_types Parameter type information
	 * @param retval_types Return value type information
	 * @return Shared pointer to Entity instance
	 * @throws std::exception on failure
	 */
	std::shared_ptr<Entity> load_entity(
		const std::string& entity_path,
		const std::vector<PyObject*>& params_types,
		const std::vector<PyObject*>& retval_types
	);
	
	/**
	 * Unload the module (optional cleanup)
	 */
	void unload();

private:
	cpython3_runtime_manager* m_runtimeManager;
	std::string m_modulePath;
	PyObject* m_pyModule;  // Python module object (with refcounting)
	mutable std::mutex m_mutex;  // Thread safety
	
	// Helper methods
	void load_python_module();
	std::string check_python_error() const;
};
