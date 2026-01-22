#include "module.h"
#include "entity.h"
#include "runtime_manager.h"

#include <stdexcept>
#include <sstream>
#include <boost/algorithm/string.hpp>

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

	if (!m_library || !m_library->is_loaded())
	{
		throw std::runtime_error("Module: library not loaded");
	}

	// Parse entity_path to get function name
	std::string function_name = parse_entity_path(entity_path);

	// Check if symbol exists
	if (!m_library->has(function_name))
	{
		throw std::runtime_error("Module: symbol '" + function_name + "' not found in '" + m_modulePath + "'");
	}

	// Get raw function pointer
	// We use get<void*> and reinterpret to avoid type signature requirements
	void* func_ptr = reinterpret_cast<void*>(m_library->get<void()>(function_name));

	return std::make_shared<GoFunction>(func_ptr, function_name);
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

std::string Module::parse_entity_path(const std::string& entity_path)
{
	// Entity path format: "callable=FunctionName"
	// Simple key=value parsing

	std::vector<std::string> parts;
	boost::split(parts, entity_path, boost::is_any_of(","));

	for (const auto& part : parts)
	{
		std::string trimmed = boost::trim_copy(part);

		if (trimmed.rfind("callable=", 0) == 0)
		{
			// Found "callable=..." - extract function name
			return trimmed.substr(9);  // Length of "callable="
		}
	}

	// If no "callable=" prefix, assume the whole string is the function name
	return entity_path;
}
