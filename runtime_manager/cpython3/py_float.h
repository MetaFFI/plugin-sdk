#pragma once

#include "py_object.h"

/**
 * @brief Wrapper for Python float objects
 * 
 * Provides type-safe construction and extraction of Python floats.
 * Supports both float and double precision.
 */
class py_float : public py_object
{
public:
	/**
	 * @brief Check if a PyObject is a Python float
	 * @param obj PyObject to check
	 * @return true if obj is a Python float
	 * @note Assumes GIL is held by caller
	 */
	static bool check(PyObject* obj);

public:
	/**
	 * @brief Construct from float value
	 */
	py_float(cpython3_runtime_manager& rt, float val);

	/**
	 * @brief Construct from double value
	 */
	py_float(cpython3_runtime_manager& rt, double val);

	/**
	 * @brief Construct from existing PyObject
	 * @param rt Runtime manager reference
	 * @param obj PyObject* that must be a Python float
	 * @throws std::runtime_error if obj is not a Python float
	 */
	py_float(cpython3_runtime_manager& rt, PyObject* obj);

	// Move constructor
	py_float(py_float&& other) noexcept;

	// Copy assignment
	py_float& operator=(const py_float& other);

	// Conversion operators
	explicit operator float() const;
	explicit operator double() const;
};
