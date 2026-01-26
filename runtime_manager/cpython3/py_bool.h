#pragma once

#include "py_object.h"
#include <runtime/metaffi_primitives.h>

/**
 * @brief Wrapper for Python bool objects
 * 
 * Provides type-safe construction and extraction of Python booleans.
 */
class py_bool : public py_object
{
public:
	/**
	 * @brief Check if a PyObject is a Python bool
	 * @param obj PyObject to check
	 * @return true if obj is a Python bool (Py_True or Py_False)
	 * @note Assumes GIL is held by caller
	 */
	static bool check(PyObject* obj);

public:
	/**
	 * @brief Construct from bool value
	 */
	py_bool(cpython3_runtime_manager& rt, bool val);

	/**
	 * @brief Construct from existing PyObject
	 * @param rt Runtime manager reference
	 * @param obj PyObject* that must be a Python bool
	 * @throws std::runtime_error if obj is not a Python bool
	 */
	py_bool(cpython3_runtime_manager& rt, PyObject* obj);

	// Move constructor
	py_bool(py_bool&& other) noexcept;

	// Copy assignment
	py_bool& operator=(const py_bool& other);

	// Conversion operators
	explicit operator bool() const;
	explicit operator metaffi_bool() const;
};
