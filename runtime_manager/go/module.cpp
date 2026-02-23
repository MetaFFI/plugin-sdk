#include "module.h"
#include "entity.h"
#include "runtime_manager.h"
#include <utils/entity_path_parser.h>

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <boost/algorithm/string.hpp>

namespace
{
	// Go doesn't support dlclose (Go issue #11100).  Calling FreeLibrary on a Go
	// shared library causes a crash during Go runtime teardown.  We keep every
	// loaded library alive for the lifetime of the process by storing an extra
	// shared_ptr in this static map.  The map is keyed by canonical module path
	// so the same library is only loaded once.
	std::mutex g_loaded_libraries_mutex;
	std::unordered_map<std::string, std::shared_ptr<boost::dll::shared_library>>& get_loaded_libraries()
	{
		static std::unordered_map<std::string, std::shared_ptr<boost::dll::shared_library>> s_map;
		return s_map;
	}

	bool go_module_log_enabled()
	{
		static const bool enabled = []() -> bool
		{
			const char* raw = std::getenv("METAFFI_GO_PLUGIN_DEBUG_LOG");
			if(!raw)
			{
				return false;
			}

			std::string val(raw);
			boost::algorithm::to_lower(val);
			return val == "1" || val == "true" || val == "yes" || val == "on";
		}();
		return enabled;
	}
}

#define GO_MODULE_LOG(msg) do { if(go_module_log_enabled()) { std::cerr << "[go_module] " << msg << std::endl; } } while(0)

Module::Module(go_runtime_manager* manager, const std::string& module_path)
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

	// Load the shared library (or reuse a previously loaded one).
	// Go doesn't support dlclose, so loaded libraries are cached for the
	// process lifetime to prevent boost::dll's destructor from calling
	// FreeLibrary and crashing Go's runtime teardown.
	{
		std::lock_guard<std::mutex> lib_lock(g_loaded_libraries_mutex);
		auto& cache = get_loaded_libraries();
		auto it = cache.find(m_modulePath);
		if (it != cache.end())
		{
			m_library = it->second;
		}
		else
		{
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
				throw std::runtime_error("Module: failed to load shared library '" + m_modulePath + "': " + e.what());
			}

			cache[m_modulePath] = m_library;
		}
	}
}

Module::~Module()
{
	// Note: We don't explicitly unload due to Go's dlclose limitation
	// The library will be unloaded when the process exits
}

Module::Module(const Module& other)
{
	std::lock_guard<std::mutex> lock(other.m_mutex);
	m_runtimeManager = other.m_runtimeManager;
	m_modulePath = other.m_modulePath;
	m_library = other.m_library;  // shared_ptr copy
}

Module& Module::operator=(const Module& other)
{
	if (this != &other)
	{
		std::scoped_lock lock(m_mutex, other.m_mutex);
		m_runtimeManager = other.m_runtimeManager;
		m_modulePath = other.m_modulePath;
		m_library = other.m_library;
	}
	return *this;
}

Module::Module(Module&& other) noexcept
{
	std::lock_guard<std::mutex> lock(other.m_mutex);
	m_runtimeManager = other.m_runtimeManager;
	m_modulePath = std::move(other.m_modulePath);
	m_library = std::move(other.m_library);
	other.m_runtimeManager = nullptr;
}

Module& Module::operator=(Module&& other) noexcept
{
	if (this != &other)
	{
		std::scoped_lock lock(m_mutex, other.m_mutex);
		m_runtimeManager = other.m_runtimeManager;
		m_modulePath = std::move(other.m_modulePath);
		m_library = std::move(other.m_library);
		other.m_runtimeManager = nullptr;
	}
	return *this;
}

std::shared_ptr<Entity> Module::load_entity(const std::string& entity_path)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	GO_MODULE_LOG("load_entity: entity_path=" << entity_path << " module=" << m_modulePath);

	if (!m_library || !m_library->is_loaded())
	{
		throw std::runtime_error("Module: library not loaded");
	}

	// Parse entity_path to get symbol name (EntryPoint_*) and logical name (for get_name())
	std::string symbol = parse_entity_path(entity_path);
	std::string logical_name = parse_entity_path_logical_name(entity_path);
	GO_MODULE_LOG("load_entity: symbol=" << symbol << " logical_name=" << logical_name);

	// Check if symbol exists
	bool has = m_library->has(symbol);
	GO_MODULE_LOG("load_entity: has(symbol)=" << (has ? "true" : "false"));
	if (!has)
	{
		throw std::runtime_error("Module: symbol '" + symbol + "' not found in '" + m_modulePath + "'");
	}

	// Get raw function pointer
	// We use get<void*> and reinterpret to avoid type signature requirements
	GO_MODULE_LOG("load_entity: getting function pointer...");
	void* func_ptr = reinterpret_cast<void*>(m_library->get<void()>(symbol));
	GO_MODULE_LOG("load_entity: func_ptr=" << func_ptr << " returning Entity");

	return std::make_shared<GoFunction>(func_ptr, logical_name);
}

void Module::unload()
{
	// No-op: Go doesn't support proper dlclose (Go issue #11100)
	// Calling dlclose on a Go shared library can cause crashes
	// The library will be unloaded when the process exits
}

const std::string& Module::get_module_path() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_modulePath;
}

bool Module::has_symbol(const std::string& symbol_name) const
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_library || !m_library->is_loaded())
	{
		return false;
	}

	return m_library->has(symbol_name);
}

std::string Module::parse_entity_path_logical_name(const std::string& entity_path)
{
	metaffi::utils::entity_path_parser fpp(entity_path);

	if (fpp.contains("callable"))
	{
		std::string logical_name = boost::trim_copy(fpp["callable"]);
		if (logical_name.empty())
		{
			throw std::runtime_error("Module: callable= value is empty");
		}
		return logical_name;
	}

	if (fpp.contains("global"))
	{
		std::string logical_name = boost::trim_copy(fpp["global"]);
		if (logical_name.empty())
		{
			throw std::runtime_error("Module: global= value is empty");
		}
		return logical_name;
	}

	if (fpp.contains("field"))
	{
		std::string logical_name = boost::trim_copy(fpp["field"]);
		if (logical_name.empty())
		{
			throw std::runtime_error("Module: field= value is empty");
		}
		return logical_name;
	}

	std::string logical_name = boost::trim_copy(entity_path);
	if (logical_name.empty())
	{
		throw std::runtime_error("Module: entity_path is empty");
	}

	return logical_name;
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

std::string Module::parse_entity_path(const std::string& entity_path)
{
	metaffi::utils::entity_path_parser fpp(entity_path);
	std::string symbol = "EntryPoint_";

	if (fpp.contains("callable"))
	{
		std::string callable_name = boost::trim_copy(fpp["callable"]);
		if (callable_name.empty())
		{
			throw std::runtime_error("Module: callable= value is empty");
		}

		boost::replace_all(callable_name, ".", "_");
		symbol += callable_name;
		if (callable_name.size() >= 12 && callable_name.substr(callable_name.size() - 12) == "_EmptyStruct")
		{
			symbol += "_MetaFFI";
		}
		return symbol;
	}

	if (fpp.contains("global"))
	{
		std::string global_name = boost::trim_copy(fpp["global"]);
		if (global_name.empty())
		{
			throw std::runtime_error("Module: global= value is empty");
		}

		if (fpp.contains("getter"))
		{
			symbol += "Get";
		}
		else if (fpp.contains("setter"))
		{
			symbol += "Set";
		}
		else
		{
			throw std::runtime_error("Module: global action is not specified (getter/setter)");
		}

		symbol += global_name;
		return symbol;
	}

	if (fpp.contains("field"))
	{
		std::string field_name = boost::trim_copy(fpp["field"]);
		if (field_name.empty())
		{
			throw std::runtime_error("Module: field= value is empty");
		}

		std::string action = fpp.contains("getter") ? "_Get" : fpp.contains("setter") ? "_Set" : "";
		if (action.empty())
		{
			throw std::runtime_error("Module: field action is not specified (getter/setter)");
		}

		boost::replace_all(field_name, ".", action);
		symbol += field_name;
		return symbol;
	}

	std::string logical_name = parse_entity_path_logical_name(entity_path);
	boost::replace_all(logical_name, ".", "_");
	symbol += logical_name;
	if (logical_name.size() >= 12 && logical_name.substr(logical_name.size() - 12) == "_EmptyStruct")
	{
		symbol += "_MetaFFI";
	}

	return symbol;
}
