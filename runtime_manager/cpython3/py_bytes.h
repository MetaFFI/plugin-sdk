#pragma once

#include "py_object.h"
#include <cstdint>

/**
 * @brief Wrapper for Python bytes objects
 * 
 * Provides type-safe construction and extraction of Python bytes.
 */
class py_bytes : public py_object
{
public:
	/**
	 * @brief Check if a PyObject is a Python bytes
	 * @param obj PyObject to check
	 * @return true if obj is a Python bytes
	 * @note Assumes GIL is held by caller
	 */
	static bool check(PyObject* obj);

public:
	/**
	 * @brief Construct from raw data
	 * @param rt Runtime manager reference
	 * @param val Pointer to byte data
	 * @param size Size in bytes
	 */
	py_bytes(cpython3_runtime_manager& rt, const char* val, Py_ssize_t size);

	/**
	 * @brief Construct from uint8_t data
	 * @param rt Runtime manager reference
	 * @param val Pointer to byte data
	 * @param size Size in bytes
	 */
	py_bytes(cpython3_runtime_manager& rt, const uint8_t* val, Py_ssize_t size);

	/**
	 * @brief Construct from existing PyObject
	 * @param rt Runtime manager reference
	 * @param obj PyObject* that must be a Python bytes
	 * @throws std::runtime_error if obj is not a Python bytes
	 */
	py_bytes(cpython3_runtime_manager& rt, PyObject* obj);

	// Move constructor
	py_bytes(py_bytes&& other) noexcept;

	// Copy assignment
	py_bytes& operator=(const py_bytes& other);

	/**
	 * @brief Get size in bytes
	 */
	[[nodiscard]] Py_ssize_t size() const;

	/**
	 * @brief Get byte at index
	 * @param i Index
	 * @return Byte value at index
	 */
	uint8_t operator[](int i) const;

	/**
	 * @brief Get pointer to internal buffer (borrowed)
	 */
	explicit operator const uint8_t*() const;

	/**
	 * @brief Get copy of buffer (caller must delete[])
	 */
	explicit operator uint8_t*() const;
};
