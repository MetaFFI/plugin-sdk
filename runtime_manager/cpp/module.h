#pragma once

#include "cpp_abi.h"
#include "entity.h"

#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <boost/dll/shared_library.hpp>

class cpp_runtime_manager;

/**
 * A loaded C/C++ shared library.
 *
 * Wraps a boost::dll::shared_library, performs ABI verification on load, and
 * provides entity resolution (callable= / global= entity paths).
 *
 * ABI verification policy (enforced in constructor):
 *   - c_only module  ↔ any plugin ABI  → allowed unconditionally
 *   - unknown module ↔ any plugin ABI  → allowed with a warning (cannot determine)
 *   - itanium module ↔ msvc   plugin   → throws (incompatible ABIs)
 *   - msvc    module ↔ itanium plugin  → throws (incompatible ABIs)
 *   - same ABI on both sides           → allowed
 *
 * Entity caching: once an entity_path has been resolved, the resulting Entity is
 * cached so that subsequent load_entity() calls for the same path are O(1).
 */
class Module
{
public:
	/**
	 * Load the shared library and verify ABI compatibility.
	 *
	 * @param manager      Owning runtime_manager (must outlive this Module).
	 * @param module_path  Path to the .dll / .so file.
	 * @throws std::runtime_error on load failure or ABI mismatch.
	 */
	Module(cpp_runtime_manager* manager, const std::string& module_path);

	/**
	 * Destructor — releases the boost::dll handle (dlclose / FreeLibrary).
	 */
	~Module();

	// Copy semantics
	Module(const Module& other);
	Module& operator=(const Module& other);

	// Move semantics
	Module(Module&& other) noexcept;
	Module& operator=(Module&& other) noexcept;

	/**
	 * Resolve an entity_path to an Entity.
	 *
	 * Supported formats:
	 *   callable=symbol_name            → CppFreeFunction
	 *   global=symbol_name,getter=true  → CppGlobalGetter
	 *   global=symbol_name,setter=true  → CppGlobalSetter
	 *
	 * The symbol_name is passed verbatim to dlsym / GetProcAddress. For C++
	 * symbols, use the already-mangled name; for extern "C" / pure C symbols,
	 * use the plain name.
	 *
	 * Results are cached: repeated calls with the same entity_path are O(1).
	 *
	 * @param entity_path  Entity path string (comma-separated key=value pairs).
	 * @return             Shared pointer to the resolved Entity.
	 * @throws std::runtime_error if the library is not loaded, the path is
	 *         malformed, or the symbol is not found.
	 */
	std::shared_ptr<Entity> load_entity(const std::string& entity_path);

	/**
	 * Unload the shared library and clear the entity cache.
	 * After calling unload(), load_entity() will throw until the module is
	 * reloaded (which requires creating a new Module).
	 */
	void unload();

	/**
	 * Check if a raw symbol name exists in the loaded library.
	 * @return true if the symbol exists; false if not found or library unloaded.
	 */
	bool has_symbol(const std::string& symbol_name) const;

	/**
	 * Get the raw address of a symbol.
	 * @return void* to the symbol, or nullptr if not found / library unloaded.
	 */
	void* get_symbol(const std::string& symbol_name) const;

	/** Path the module was loaded from. */
	const std::string& get_module_path() const;

	/** ABI detected in the module binary (set at construction time). */
	cpp_abi get_detected_abi() const;

private:
	cpp_runtime_manager*                                  m_runtimeManager = nullptr;
	std::string                                           m_modulePath;
	cpp_abi                                               m_detectedAbi    = cpp_abi::unknown;
	std::shared_ptr<boost::dll::shared_library>           m_library;
	mutable std::mutex                                    m_mutex;
	std::unordered_map<std::string, std::shared_ptr<Entity>> m_entityCache;

	// --- Entity path parsing helpers (static — no library access needed) ---

	// Returns the symbol name for callable= paths.
	static std::string parse_callable_symbol(const std::string& entity_path);

	// Returns the symbol name for global= paths.
	static std::string parse_global_symbol(const std::string& entity_path);

	// Returns true if entity_path contains "getter=true" (case-insensitive).
	static bool is_getter(const std::string& entity_path);

	// Returns true if entity_path contains "setter=true" (case-insensitive).
	static bool is_setter(const std::string& entity_path);

	// Returns a human-readable logical name from entity_path (for Entity::get_name()).
	static std::string parse_logical_name(const std::string& entity_path);
};
