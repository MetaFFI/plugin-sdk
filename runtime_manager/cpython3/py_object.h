#pragma once

#include "python_api_wrapper.h"
#include <runtime/metaffi_primitives.h>
#include <stdexcept>
#include <string>

// Forward declaration
class cpython3_runtime_manager;

/**
 * @brief Base class for Python object wrappers
 * 
 * Provides reference counting, type detection, and attribute access.
 * All derived classes must hold a reference to cpython3_runtime_manager.
 * 
 * GIL Management:
 * - All public methods acquire the GIL via m_runtime
 * - Protected/private methods assume GIL is already held
 */
class py_object
{
protected:
	cpython3_runtime_manager& m_runtime;
	PyObject* instance = nullptr;

public:
	/**
	 * @brief Constructor for derived classes or direct use
	 * @param runtime Reference to the runtime manager
	 */
	explicit py_object(cpython3_runtime_manager& runtime);

	/**
	 * @brief Constructor that takes ownership of a PyObject
	 * @param runtime Reference to the runtime manager
	 * @param obj PyObject* to wrap (borrowed reference, will be INCREF'd)
	 * @note Assumes GIL is held by caller
	 */
	py_object(cpython3_runtime_manager& runtime, PyObject* obj);

public:
	/**
	 * @brief Get the Python type name of a PyObject
	 * @param obj PyObject to check (must not be null)
	 * @return Type name as string
	 * @throws std::runtime_error if obj is null or type cannot be determined
	 * @note Assumes GIL is held by caller (static method)
	 */
	static std::string get_object_type(PyObject* obj);

	/**
	 * @brief Get the MetaFFI type for a PyObject
	 * @param obj PyObject to check
	 * @return Corresponding metaffi_type
	 * @note Assumes GIL is held by caller (static method)
	 */
	static metaffi_type get_metaffi_type(PyObject* obj);

public:
	/**
	 * @brief Move constructor
	 */
	py_object(py_object&& other) noexcept;

	/**
	 * @brief Destructor - decrements reference count
	 */
	virtual ~py_object();

	/**
	 * @brief Copy assignment operator
	 */
	py_object& operator=(const py_object& other);

	/**
	 * @brief Conversion operator to raw PyObject*
	 * @return The wrapped PyObject* (borrowed reference)
	 */
	explicit operator PyObject*() const;

	/**
	 * @brief Get the Python type name of this object
	 * @return Type name as string
	 */
	[[nodiscard]] std::string get_type() const;

	/**
	 * @brief Increment reference count
	 */
	void inc_ref();

	/**
	 * @brief Decrement reference count
	 */
	void dec_ref();

	/**
	 * @brief Get an attribute of this object
	 * @param name Attribute name
	 * @return New reference to attribute value
	 */
	PyObject* get_attribute(const char* name) const;

	/**
	 * @brief Set an attribute of this object
	 * @param name Attribute name
	 * @param val Value to set (borrowed reference)
	 */
	void set_attribute(const char* name, PyObject* val);

	/**
	 * @brief Release ownership of the internal PyObject
	 * 
	 * Returns the internal PyObject* without decrementing its reference count
	 * and sets the internal pointer to nullptr. The caller takes ownership.
	 * 
	 * @return The internal PyObject* (caller owns reference)
	 */
	[[nodiscard]] PyObject* detach();

	/**
	 * @brief Get the internal PyObject without releasing ownership
	 * @return The internal PyObject* (borrowed reference)
	 */
	[[nodiscard]] PyObject* get() const { return instance; }

	/**
	 * @brief Check if the wrapper holds a valid (non-null) PyObject
	 * @return true if instance is not null
	 */
	[[nodiscard]] bool is_valid() const { return instance != nullptr; }

	/**
	 * @brief Check if the wrapped object is Python None
	 * @return true if instance is Py_None
	 */
	[[nodiscard]] bool is_none() const;

	// Prevent copying (use move semantics)
	py_object(const py_object&) = delete;
};
