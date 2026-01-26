#pragma once

#include "py_object.h"
#include <runtime/metaffi_primitives.h>

/**
 * @brief Wrapper for Python tuple objects
 * 
 * Provides type-safe construction and manipulation of Python tuples.
 */
class py_tuple : public py_object
{
public:
	/**
	 * @brief Check if a PyObject is a Python tuple
	 * @param obj PyObject to check
	 * @return true if obj is a Python tuple
	 * @note Assumes GIL is held by caller
	 */
	static bool check(PyObject* obj);

	/**
	 * @brief Get metadata about a tuple (dimensions, common type, etc.)
	 * @param obj Python tuple object
	 * @param out_is_1d_array Set to true if tuple is 1-dimensional
	 * @param out_is_fixed_dimension Set to true if all sublists have same nesting
	 * @param out_size Set to the size of the tuple
	 * @param out_common_type Set to common element type (0 if mixed)
	 * @note Assumes GIL is held by caller
	 */
	static void get_metadata(PyObject* obj, bool& out_is_1d_array, bool& out_is_fixed_dimension, 
	                        Py_ssize_t& out_size, metaffi_type& out_common_type);

	/**
	 * @brief Get tuple size
	 * @note Assumes GIL is held by caller
	 */
	static Py_ssize_t get_size(PyObject* obj);

public:
	/**
	 * @brief Construct empty tuple of given size
	 * @param rt Runtime manager reference
	 * @param size Tuple size
	 */
	explicit py_tuple(cpython3_runtime_manager& rt, Py_ssize_t size);

	/**
	 * @brief Construct from array of PyObjects
	 * @param rt Runtime manager reference
	 * @param objects Array of PyObject pointers
	 * @param object_count Number of objects
	 */
	py_tuple(cpython3_runtime_manager& rt, PyObject** objects, int object_count);

	/**
	 * @brief Construct from existing PyObject
	 * @param rt Runtime manager reference
	 * @param existingTuple PyObject* that must be a Python tuple
	 * @throws std::runtime_error if obj is not a Python tuple
	 */
	py_tuple(cpython3_runtime_manager& rt, PyObject* existingTuple);

	// Copy constructor
	py_tuple(const py_tuple& other);

	// Move constructor
	py_tuple(py_tuple&& other) noexcept;

	// Copy assignment
	py_tuple& operator=(const py_tuple& other);

	// Move assignment
	py_tuple& operator=(py_tuple&& other) noexcept;

	/**
	 * @brief Get item at index (borrowed reference)
	 */
	PyObject* operator[](int index) const;

	/**
	 * @brief Get tuple size
	 */
	[[nodiscard]] Py_ssize_t size() const;

	/**
	 * @brief Set item at index
	 * @param index Index to set
	 * @param value Item to set (steals reference)
	 */
	void set_item(Py_ssize_t index, PyObject* value);
};
