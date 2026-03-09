#include "entity.h"

#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <new>


// ============================================================================
// CppFreeFunction
// ============================================================================

CppFreeFunction::CppFreeFunction(void* func_ptr, const std::string& name)
	: m_funcPtr(func_ptr)
	, m_name(name)
{
	if (m_funcPtr == nullptr)
	{
		throw std::runtime_error("CppFreeFunction: function pointer cannot be null");
	}

	if (m_name.empty())
	{
		throw std::runtime_error("CppFreeFunction: name cannot be empty");
	}
}

void* CppFreeFunction::get_function_pointer() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_funcPtr;
}

const std::string& CppFreeFunction::get_name() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_name;
}


// ============================================================================
// CppGlobalGetter
// ============================================================================

CppGlobalGetter::CppGlobalGetter(void* var_ptr, const std::string& name)
	: m_varPtr(var_ptr)
	, m_name(name)
{
	if (m_varPtr == nullptr)
	{
		throw std::runtime_error("CppGlobalGetter: variable pointer cannot be null");
	}

	if (m_name.empty())
	{
		throw std::runtime_error("CppGlobalGetter: name cannot be empty");
	}
}

void* CppGlobalGetter::get_function_pointer() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_varPtr;
}

const std::string& CppGlobalGetter::get_name() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_name;
}

void* CppGlobalGetter::get() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_varPtr;
}


// ============================================================================
// CppGlobalSetter
// ============================================================================

CppGlobalSetter::CppGlobalSetter(void* var_ptr, const std::string& name)
	: m_varPtr(var_ptr)
	, m_name(name)
{
	if (m_varPtr == nullptr)
	{
		throw std::runtime_error("CppGlobalSetter: variable pointer cannot be null");
	}

	if (m_name.empty())
	{
		throw std::runtime_error("CppGlobalSetter: name cannot be empty");
	}
}

void* CppGlobalSetter::get_function_pointer() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_varPtr;
}

const std::string& CppGlobalSetter::get_name() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_name;
}

void CppGlobalSetter::set(const void* value, std::size_t size) const
{
	if (value == nullptr)
	{
		throw std::runtime_error("CppGlobalSetter::set: value cannot be null");
	}

	if (size == 0)
	{
		throw std::runtime_error("CppGlobalSetter::set: size cannot be zero");
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	std::memcpy(m_varPtr, value, size);
}


// ============================================================================
// CppInstanceMethod
// ============================================================================

CppInstanceMethod::CppInstanceMethod(void* func_ptr, const std::string& name)
	: m_funcPtr(func_ptr)
	, m_name(name)
{
	if (m_funcPtr == nullptr)
	{
		throw std::runtime_error("CppInstanceMethod: function pointer cannot be null");
	}

	if (m_name.empty())
	{
		throw std::runtime_error("CppInstanceMethod: name cannot be empty");
	}
}

void* CppInstanceMethod::get_function_pointer() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_funcPtr;
}

const std::string& CppInstanceMethod::get_name() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_name;
}


// ============================================================================
// CppConstructor
// ============================================================================

CppConstructor::CppConstructor(void* func_ptr, const std::string& name, std::size_t class_size)
	: m_funcPtr(func_ptr)
	, m_name(name)
	, m_classSize(class_size)
{
	if (m_funcPtr == nullptr)
	{
		throw std::runtime_error("CppConstructor: function pointer cannot be null");
	}

	if (m_name.empty())
	{
		throw std::runtime_error("CppConstructor: name cannot be empty");
	}

	if (m_classSize == 0)
	{
		throw std::runtime_error("CppConstructor: class_size cannot be zero");
	}
}

void* CppConstructor::allocate() const
{
	std::lock_guard<std::mutex> lock(m_mutex);

	void* ptr = std::malloc(m_classSize);
	if (ptr == nullptr)
	{
		throw std::bad_alloc();
	}

	return ptr;
}

void* CppConstructor::get_function_pointer() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_funcPtr;
}

const std::string& CppConstructor::get_name() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_name;
}


// ============================================================================
// CppDestructor
// ============================================================================

CppDestructor::CppDestructor(void* func_ptr, const std::string& name)
	: m_funcPtr(func_ptr)
	, m_name(name)
{
	if (m_funcPtr == nullptr)
	{
		throw std::runtime_error("CppDestructor: function pointer cannot be null");
	}

	if (m_name.empty())
	{
		throw std::runtime_error("CppDestructor: name cannot be empty");
	}
}

void CppDestructor::destroy(void* instance) const
{
	if (instance == nullptr)
	{
		throw std::runtime_error("CppDestructor::destroy: instance cannot be null");
	}

	std::lock_guard<std::mutex> lock(m_mutex);

	// Call the destructor function with instance as this*
	auto dtor_fn = reinterpret_cast<void(*)(void*)>(m_funcPtr);
	dtor_fn(instance);

	// Free the heap-allocated memory
	std::free(instance);
}

void* CppDestructor::get_function_pointer() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_funcPtr;
}

const std::string& CppDestructor::get_name() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_name;
}


// ============================================================================
// CppFieldGetter
// ============================================================================

CppFieldGetter::CppFieldGetter(const std::string& field_name, std::size_t field_offset)
	: m_name(field_name)
	, m_fieldOffset(field_offset)
{
	if (m_name.empty())
	{
		throw std::runtime_error("CppFieldGetter: field_name cannot be empty");
	}
}

void* CppFieldGetter::get(void* instance) const
{
	if (instance == nullptr)
	{
		throw std::runtime_error("CppFieldGetter::get: instance cannot be null");
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	return static_cast<char*>(instance) + m_fieldOffset;
}

void* CppFieldGetter::get_function_pointer() const
{
	// Field access uses offset arithmetic — no function pointer.
	return nullptr;
}

const std::string& CppFieldGetter::get_name() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_name;
}


// ============================================================================
// CppFieldSetter
// ============================================================================

CppFieldSetter::CppFieldSetter(const std::string& field_name, std::size_t field_offset, std::size_t field_size)
	: m_name(field_name)
	, m_fieldOffset(field_offset)
	, m_fieldSize(field_size)
{
	if (m_name.empty())
	{
		throw std::runtime_error("CppFieldSetter: field_name cannot be empty");
	}
}

void CppFieldSetter::set(void* instance, const void* value, std::size_t sz) const
{
	if (instance == nullptr)
	{
		throw std::runtime_error("CppFieldSetter::set: instance cannot be null");
	}

	if (value == nullptr)
	{
		throw std::runtime_error("CppFieldSetter::set: value cannot be null");
	}

	if (sz == 0)
	{
		throw std::runtime_error("CppFieldSetter::set: size cannot be zero");
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	std::memcpy(static_cast<char*>(instance) + m_fieldOffset, value, sz);
}

void* CppFieldSetter::get_function_pointer() const
{
	// Field access uses offset arithmetic — no function pointer.
	return nullptr;
}

const std::string& CppFieldSetter::get_name() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_name;
}
