#include "entity.h"

#include <stdexcept>

GoFunction::GoFunction(void* func_ptr, const std::string& name)
	: m_funcPtr(func_ptr)
	, m_name(name)
{
	if (m_funcPtr == nullptr)
	{
		throw std::runtime_error("GoFunction: function pointer cannot be null");
	}

	if (m_name.empty())
	{
		throw std::runtime_error("GoFunction: name cannot be empty");
	}
}

void* GoFunction::get_function_pointer() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_funcPtr;
}

const std::string& GoFunction::get_name() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_name;
}
