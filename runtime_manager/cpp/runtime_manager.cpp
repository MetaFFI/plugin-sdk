#include "runtime_manager.h"
#include "module.h"

#include <filesystem>
#include <stdexcept>
#include <iostream>


cpp_runtime_manager::cpp_runtime_manager()
	: m_pluginAbi(get_plugin_abi())
	, m_isRuntimeLoaded(false)
{
}

cpp_runtime_manager::~cpp_runtime_manager()
{
	try
	{
		release_runtime();
	}
	catch (...)
	{
		std::cerr << "cpp_runtime_manager destructor: exception during release_runtime()\n";
	}
}

void cpp_runtime_manager::load_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	// No external interpreter to start — just set the flag.
	if (!m_isRuntimeLoaded)
	{
		m_isRuntimeLoaded = true;
	}
}

void cpp_runtime_manager::release_runtime()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_isRuntimeLoaded)
	{
		return;  // Idempotent
	}

	// Drop all cached module references.
	// Modules still held by callers via shared_ptr continue to live until those
	// shared_ptrs are released; this is intentional RAII behaviour.
	m_moduleCache.clear();

	m_isRuntimeLoaded = false;
}

std::shared_ptr<Module> cpp_runtime_manager::load_module(const std::string& module_path)
{
	// Fail-fast: check file existence before acquiring the heavy lock.
	std::error_code ec;
	if (!std::filesystem::exists(module_path, ec))
	{
		throw std::runtime_error(
			"cpp_runtime_manager::load_module: file not found: " + module_path);
	}

	std::lock_guard<std::mutex> lock(m_mutex);

	// Auto-load the runtime if the caller skipped load_runtime().
	if (!m_isRuntimeLoaded)
	{
		m_isRuntimeLoaded = true;  // Same as load_runtime() body without re-locking
	}

	// Return cached module if already loaded.
	auto it = m_moduleCache.find(module_path);
	if (it != m_moduleCache.end())
	{
		return it->second;
	}

	// Load a new module — Module constructor does ABI detection + verification.
	auto module = std::make_shared<Module>(this, module_path);

	m_moduleCache[module_path] = module;
	return module;
}

bool cpp_runtime_manager::is_runtime_loaded() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_isRuntimeLoaded;
}

cpp_abi cpp_runtime_manager::get_abi() const
{
	return m_pluginAbi;  // compile-time constant — no lock needed
}
