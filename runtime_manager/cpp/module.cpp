#include "module.h"
#include "runtime_manager.h"

#include <utils/entity_path_parser.h>

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <utils/safe_func.h>

#include <boost/algorithm/string.hpp>

namespace
{
	// Determine whether logging is enabled via environment variable.
	bool cpp_module_log_enabled()
	{
		static const bool enabled = []() -> bool
		{
			char* raw = metaffi_getenv_alloc("METAFFI_CPP_PLUGIN_DEBUG_LOG");
			if (!raw) return false;

			std::string val(raw);
			metaffi_free_env(raw);
			boost::algorithm::to_lower(val);
			return val == "1" || val == "true" || val == "yes" || val == "on";
		}();
		return enabled;
	}
} // anonymous namespace

#define CPP_MODULE_LOG(msg) \
	do { if (cpp_module_log_enabled()) { std::cerr << "[cpp_module] " << msg << std::endl; } } while (0)


// ============================================================================
// ABI compatibility verification
// ============================================================================

namespace
{
	// Enforce the ABI compatibility table defined in the design doc.
	// Throws std::runtime_error on hard mismatch; warns (stderr) on unknown.
	void verify_abi_compatibility(cpp_abi plugin_abi,
	                               cpp_abi module_abi,
	                               const std::string& module_path)
	{
		// Pure-C module: compatible with any plugin ABI
		if (module_abi == cpp_abi::c_only) return;

		// Unknown module ABI: cannot determine — warn but allow
		if (module_abi == cpp_abi::unknown)
		{
			std::cerr << "[cpp_module] WARNING: Cannot determine ABI of '"
			          << module_path
			          << "'. Proceeding without verification; "
			             "crashes may occur if ABIs are incompatible.\n";
			return;
		}

		// Hard mismatch: MSVC vs Itanium
		if (plugin_abi == cpp_abi::msvc && module_abi == cpp_abi::itanium)
		{
			throw std::runtime_error(
				"ABI mismatch: plugin is MSVC ABI but module '" + module_path +
				"' uses Itanium ABI (e.g. MinGW/GCC). "
				"These ABIs are incompatible; rebuild the module with MSVC.");
		}

		if (plugin_abi == cpp_abi::itanium && module_abi == cpp_abi::msvc)
		{
			throw std::runtime_error(
				"ABI mismatch: plugin is Itanium ABI but module '" + module_path +
				"' uses MSVC ABI. "
				"These ABIs are incompatible; rebuild the module with GCC or Clang.");
		}

		// Same ABI family: allow (within-family version incompatibility is a
		// known limitation — cannot be detected at the binary level).
	}
} // anonymous namespace


// ============================================================================
// Constructor / Destructor
// ============================================================================

Module::Module(cpp_runtime_manager* manager, const std::string& module_path)
	: m_runtimeManager(manager)
	, m_modulePath(module_path)
{
	if (m_runtimeManager == nullptr)
	{
		throw std::runtime_error("Module: runtime manager cannot be null");
	}

	if (m_modulePath.empty())
	{
		throw std::runtime_error("Module: module path cannot be empty");
	}

	// Detect the module's ABI before loading (uses file inspection only).
	CPP_MODULE_LOG("Detecting ABI for: " << m_modulePath);
	m_detectedAbi = detect_module_abi(m_modulePath);
	CPP_MODULE_LOG("Detected ABI: " << cpp_abi_name(m_detectedAbi));

	// Enforce compatibility between plugin ABI and module ABI.
	cpp_abi plugin_abi = get_plugin_abi();
	verify_abi_compatibility(plugin_abi, m_detectedAbi, m_modulePath);

	// Load the shared library via boost::dll.
	m_library = std::make_shared<boost::dll::shared_library>();

#ifdef _WIN32
	auto load_mode = boost::dll::load_mode::default_mode;
#else
	auto load_mode = boost::dll::load_mode::rtld_now | boost::dll::load_mode::rtld_global;
#endif

	try
	{
		m_library->load(m_modulePath, load_mode);
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error(
			"Module: failed to load shared library '" + m_modulePath + "': " + e.what());
	}

	CPP_MODULE_LOG("Loaded: " << m_modulePath);
}

Module::~Module()
{
	// m_library's destructor calls dlclose / FreeLibrary automatically.
	// Entity cache is cleared first (shared_ptrs released).
}


// ============================================================================
// Copy / Move semantics
// ============================================================================

Module::Module(const Module& other)
{
	std::lock_guard<std::mutex> lock(other.m_mutex);
	m_runtimeManager = other.m_runtimeManager;
	m_modulePath     = other.m_modulePath;
	m_detectedAbi    = other.m_detectedAbi;
	m_library        = other.m_library;
	m_entityCache    = other.m_entityCache;
}

Module& Module::operator=(const Module& other)
{
	if (this != &other)
	{
		std::scoped_lock lock(m_mutex, other.m_mutex);
		m_runtimeManager = other.m_runtimeManager;
		m_modulePath     = other.m_modulePath;
		m_detectedAbi    = other.m_detectedAbi;
		m_library        = other.m_library;
		m_entityCache    = other.m_entityCache;
	}
	return *this;
}

Module::Module(Module&& other) noexcept
{
	std::lock_guard<std::mutex> lock(other.m_mutex);
	m_runtimeManager         = other.m_runtimeManager;
	m_modulePath             = std::move(other.m_modulePath);
	m_detectedAbi            = other.m_detectedAbi;
	m_library                = std::move(other.m_library);
	m_entityCache            = std::move(other.m_entityCache);
	other.m_runtimeManager   = nullptr;
}

Module& Module::operator=(Module&& other) noexcept
{
	if (this != &other)
	{
		std::scoped_lock lock(m_mutex, other.m_mutex);
		m_runtimeManager         = other.m_runtimeManager;
		m_modulePath             = std::move(other.m_modulePath);
		m_detectedAbi            = other.m_detectedAbi;
		m_library                = std::move(other.m_library);
		m_entityCache            = std::move(other.m_entityCache);
		other.m_runtimeManager   = nullptr;
	}
	return *this;
}


// ============================================================================
// Entity resolution
// ============================================================================

std::shared_ptr<Entity> Module::load_entity(const std::string& entity_path)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	CPP_MODULE_LOG("load_entity: entity_path=" << entity_path);

	if (!m_library || !m_library->is_loaded())
	{
		throw std::runtime_error("Module::load_entity: library is not loaded");
	}

	// Return cached entity if available
	auto it = m_entityCache.find(entity_path);
	if (it != m_entityCache.end())
	{
		CPP_MODULE_LOG("load_entity: returning cached entity");
		return it->second;
	}

	// Parse entity_path to determine entity type and symbol name
	metaffi::utils::entity_path_parser fpp(entity_path);

	std::shared_ptr<Entity> entity;

	if (fpp.contains("callable"))
	{
		// --- Callable: free function, static method, instance method, constructor, or destructor ---
		std::string symbol = boost::trim_copy(fpp["callable"]);
		if (symbol.empty())
		{
			throw std::runtime_error("Module::load_entity: callable= value is empty");
		}

		CPP_MODULE_LOG("load_entity: looking up symbol '" << symbol << "'");

		if (!m_library->has(symbol))
		{
			throw std::runtime_error(
				"Module::load_entity: symbol '" + symbol +
				"' not found in '" + m_modulePath + "'");
		}

		void* func_ptr = reinterpret_cast<void*>(m_library->get<void()>(symbol));

		// Determine subtype from flags
		bool is_constructor    = fpp.contains("constructor");
		bool is_destructor     = fpp.contains("destructor");
		bool instance_required = fpp.contains("instance_required");

		if (is_constructor)
		{
			// callable=…,constructor=true,class_size=N
			std::string class_size_str = boost::trim_copy(fpp["class_size"]);
			if (class_size_str.empty())
			{
				throw std::runtime_error(
					"Module::load_entity: constructor entity path must contain 'class_size': " +
					entity_path);
			}

			std::size_t class_size = std::stoull(class_size_str);
			entity = std::make_shared<CppConstructor>(func_ptr, symbol, class_size);
			CPP_MODULE_LOG("load_entity: CppConstructor resolved, ptr=" << func_ptr
			               << " class_size=" << class_size);
		}
		else if (is_destructor)
		{
			// callable=…,destructor=true
			entity = std::make_shared<CppDestructor>(func_ptr, symbol);
			CPP_MODULE_LOG("load_entity: CppDestructor resolved, ptr=" << func_ptr);
		}
		else if (instance_required)
		{
			// callable=…,instance_required=true
			entity = std::make_shared<CppInstanceMethod>(func_ptr, symbol);
			CPP_MODULE_LOG("load_entity: CppInstanceMethod resolved, ptr=" << func_ptr);
		}
		else
		{
			// callable=… (free function or static method)
			entity = std::make_shared<CppFreeFunction>(func_ptr, symbol);
			CPP_MODULE_LOG("load_entity: CppFreeFunction resolved, ptr=" << func_ptr);
		}
	}
	else if (fpp.contains("field"))
	{
		// --- Field access: getter or setter via byte offset (no symbol lookup) ---
		std::string field_name = boost::trim_copy(fpp["field"]);
		if (field_name.empty())
		{
			throw std::runtime_error("Module::load_entity: field= value is empty");
		}

		bool getter = fpp.contains("getter");
		bool setter = fpp.contains("setter");

		if (!getter && !setter)
		{
			throw std::runtime_error(
				"Module::load_entity: field entity path must contain 'getter' or 'setter' key: " +
				entity_path);
		}

		std::string offset_str = boost::trim_copy(fpp["field_offset"]);
		if (offset_str.empty())
		{
			throw std::runtime_error(
				"Module::load_entity: field entity path must contain 'field_offset': " +
				entity_path);
		}

		std::size_t field_offset = std::stoull(offset_str);

		if (getter)
		{
			entity = std::make_shared<CppFieldGetter>(field_name, field_offset);
			CPP_MODULE_LOG("load_entity: CppFieldGetter, field='" << field_name
			               << "' offset=" << field_offset);
		}
		else
		{
			std::string size_str = boost::trim_copy(fpp["field_size"]);
			if (size_str.empty())
			{
				throw std::runtime_error(
					"Module::load_entity: field setter entity path must contain 'field_size': " +
					entity_path);
			}

			std::size_t field_size = std::stoull(size_str);
			entity = std::make_shared<CppFieldSetter>(field_name, field_offset, field_size);
			CPP_MODULE_LOG("load_entity: CppFieldSetter, field='" << field_name
			               << "' offset=" << field_offset << " size=" << field_size);
		}
	}
	else if (fpp.contains("global"))
	{
		// --- Global variable: getter or setter ---
		std::string symbol = boost::trim_copy(fpp["global"]);
		if (symbol.empty())
		{
			throw std::runtime_error("Module::load_entity: global= value is empty");
		}

		bool getter = fpp.contains("getter");
		bool setter = fpp.contains("setter");

		if (!getter && !setter)
		{
			throw std::runtime_error(
				"Module::load_entity: global entity path must contain 'getter' or 'setter' key: " +
				entity_path);
		}

		CPP_MODULE_LOG("load_entity: looking up global symbol '" << symbol << "'");

		if (!m_library->has(symbol))
		{
			throw std::runtime_error(
				"Module::load_entity: global symbol '" + symbol +
				"' not found in '" + m_modulePath + "'");
		}

		// dlsym / GetProcAddress returns the address of the global variable.
		void* var_ptr = reinterpret_cast<void*>(m_library->get<void()>(symbol));

		if (getter)
		{
			entity = std::make_shared<CppGlobalGetter>(var_ptr, symbol);
			CPP_MODULE_LOG("load_entity: CppGlobalGetter resolved, ptr=" << var_ptr);
		}
		else
		{
			entity = std::make_shared<CppGlobalSetter>(var_ptr, symbol);
			CPP_MODULE_LOG("load_entity: CppGlobalSetter resolved, ptr=" << var_ptr);
		}
	}
	else
	{
		throw std::runtime_error(
			"Module::load_entity: unsupported entity path format. "
			"Expected 'callable=...', 'field=...', or 'global=...,getter=true/setter=true'. "
			"Got: " + entity_path);
	}

	// Cache for future calls with the same entity_path
	m_entityCache[entity_path] = entity;
	return entity;
}


// ============================================================================
// Unload / query
// ============================================================================

void Module::unload()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	// Clear entity cache — function pointers into the unloaded library become invalid
	m_entityCache.clear();

	if (m_library && m_library->is_loaded())
	{
		m_library->unload();
	}

	CPP_MODULE_LOG("unload: " << m_modulePath);
}

const std::string& Module::get_module_path() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_modulePath;
}

cpp_abi Module::get_detected_abi() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_detectedAbi;
}

bool Module::has_symbol(const std::string& symbol_name) const
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_library || !m_library->is_loaded()) return false;

	return m_library->has(symbol_name);
}

void* Module::get_symbol(const std::string& symbol_name) const
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_library || !m_library->is_loaded() || !m_library->has(symbol_name))
	{
		return nullptr;
	}

	return reinterpret_cast<void*>(m_library->get<void()>(symbol_name));
}


// ============================================================================
// Static entity path parsing helpers
// (Defined for interface completeness; actual parsing is inlined in load_entity)
// ============================================================================

std::string Module::parse_callable_symbol(const std::string& entity_path)
{
	metaffi::utils::entity_path_parser fpp(entity_path);
	if (fpp.contains("callable"))
	{
		return boost::trim_copy(fpp["callable"]);
	}
	return boost::trim_copy(entity_path);
}

std::string Module::parse_global_symbol(const std::string& entity_path)
{
	metaffi::utils::entity_path_parser fpp(entity_path);
	if (fpp.contains("global"))
	{
		return boost::trim_copy(fpp["global"]);
	}
	return {};
}

bool Module::is_getter(const std::string& entity_path)
{
	metaffi::utils::entity_path_parser fpp(entity_path);
	return fpp.contains("getter");
}

bool Module::is_setter(const std::string& entity_path)
{
	metaffi::utils::entity_path_parser fpp(entity_path);
	return fpp.contains("setter");
}

std::string Module::parse_logical_name(const std::string& entity_path)
{
	metaffi::utils::entity_path_parser fpp(entity_path);

	if (fpp.contains("callable")) return boost::trim_copy(fpp["callable"]);
	if (fpp.contains("global"))   return boost::trim_copy(fpp["global"]);

	return boost::trim_copy(entity_path);
}
