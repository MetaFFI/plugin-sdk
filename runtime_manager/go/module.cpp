#include "module.h"
#include "entity.h"
#include "runtime_manager.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <boost/algorithm/string.hpp>

namespace
{
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

	// Load the shared library
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
	std::vector<std::string> parts;
	boost::split(parts, entity_path, boost::is_any_of(","));

	for (const auto& part : parts)
	{
		std::string trimmed = boost::trim_copy(part);
		if (trimmed.rfind("callable=", 0) == 0)
		{
			std::string logical_name = trimmed.substr(9);
			boost::trim(logical_name);
			if (logical_name.empty())
			{
				throw std::runtime_error("Module: callable= value is empty");
			}
			return logical_name;
		}
	}
	return entity_path;
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
	// Entity path format: "callable=LogicalName" (e.g. "HelloWorld" or "SomeClass.Print").
	// MetaFFI Go guest compiler exports C symbols as EntryPoint_<Name> (dots in Name become underscores).
	// So we map callable=X to symbol "EntryPoint_" + X with '.' replaced by '_'.

	std::string logical_name = parse_entity_path_logical_name(entity_path);
	std::string symbol = "EntryPoint_";
	for (char c : logical_name)
	{
		symbol += (c == '.') ? '_' : c;
	}
	return symbol;
}
