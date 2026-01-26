#pragma once

#include "py_object.h"
#include <cstdint>

/**
 * @brief Wrapper for Python int objects
 * 
 * Provides type-safe construction and extraction of Python integers.
 * Supports all standard C++ integer types (int8_t through uint64_t).
 */
class py_int : public py_object
{
public:
	/**
	 * @brief Check if a PyObject is a Python int
	 * @param obj PyObject to check
	 * @return true if obj is a Python int
	 * @note Assumes GIL is held by caller
	 */
	static bool check(PyObject* obj);

public:
	// Constructors from C++ integer types
	py_int(cpython3_runtime_manager& rt, int8_t val);
	py_int(cpython3_runtime_manager& rt, int16_t val);
	py_int(cpython3_runtime_manager& rt, int32_t val);
	py_int(cpython3_runtime_manager& rt, int64_t val);
	py_int(cpython3_runtime_manager& rt, uint8_t val);
	py_int(cpython3_runtime_manager& rt, uint16_t val);
	py_int(cpython3_runtime_manager& rt, uint32_t val);
	py_int(cpython3_runtime_manager& rt, uint64_t val);

	/**
	 * @brief Construct from existing PyObject
	 * @param rt Runtime manager reference
	 * @param obj PyObject* that must be a Python int
	 * @throws std::runtime_error if obj is not a Python int
	 */
	py_int(cpython3_runtime_manager& rt, PyObject* obj);

	/**
	 * @brief Construct from void pointer (for handle conversion)
	 * @param rt Runtime manager reference
	 * @param ptr Pointer to convert to Python int
	 */
	py_int(cpython3_runtime_manager& rt, void* ptr);

	// Move constructor
	py_int(py_int&& other) noexcept;

	// Copy assignment
	py_int& operator=(const py_int& other);

	// Conversion operators to C++ integer types
	explicit operator int8_t() const;
	explicit operator int16_t() const;
	explicit operator int32_t() const;
	explicit operator int64_t() const;
	explicit operator uint8_t() const;
	explicit operator uint16_t() const;
	explicit operator uint32_t() const;
	explicit operator uint64_t() const;
};
