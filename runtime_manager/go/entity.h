#pragma once

#include <string>
#include <mutex>

/**
 * Abstract base class for Go entities
 *
 * Represents an exported symbol from a Go shared library.
 * Since Go exports C functions directly, entities just wrap function pointers.
 */
class Entity
{
public:
	virtual ~Entity() = default;

	/**
	 * Get the raw function pointer
	 * Caller is responsible for casting to the correct signature
	 * @return Function pointer as void*
	 */
	virtual void* get_function_pointer() const = 0;

	/**
	 * Get the entity name (exported symbol name)
	 * @return Reference to the name string
	 */
	virtual const std::string& get_name() const = 0;
};

/**
 * Concrete implementation for Go exported functions
 *
 * Wraps a function pointer exported from a Go shared library.
 * The function was exported using cgo's //export directive.
 */
class GoFunction : public Entity
{
public:
	/**
	 * Constructor
	 * @param func_ptr The function pointer from boost::dll
	 * @param name The exported symbol name
	 */
	GoFunction(void* func_ptr, const std::string& name);

	/**
	 * Destructor
	 */
	~GoFunction() override = default;

	// No copy/move - entities are managed via shared_ptr
	GoFunction(const GoFunction&) = delete;
	GoFunction& operator=(const GoFunction&) = delete;
	GoFunction(GoFunction&&) = delete;
	GoFunction& operator=(GoFunction&&) = delete;

	/**
	 * Get the function pointer
	 * @return Function pointer as void*
	 */
	void* get_function_pointer() const override;

	/**
	 * Get the function name
	 * @return Reference to the name string
	 */
	const std::string& get_name() const override;

private:
	void* m_funcPtr = nullptr;
	std::string m_name;
	mutable std::mutex m_mutex;
};
