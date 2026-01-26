#pragma once

#include "py_object.h"
#include <runtime/metaffi_primitives.h>

/**
 * @brief Wrapper for Python list objects
 * 
 * Provides type-safe construction and manipulation of Python lists.
 */
class py_list : public py_object
{
public:
	/**
	 * @brief Check if a PyObject is a Python list
	 * @param obj PyObject to check
	 * @return true if obj is a Python list
	 * @note Assumes GIL is held by caller
	 */
	static bool check(PyObject* obj);

	/**
	 * @brief Get metadata about a list (dimensions, common type, etc.)
	 * @param obj Python list object
	 * @param out_is_1d_array Set to true if list is 1-dimensional
	 * @param out_is_fixed_dimension Set to true if all sublists have same nesting
	 * @param out_size Set to the size of the list
	 * @param out_common_type Set to common element type (0 if mixed)
	 * @note Assumes GIL is held by caller
	 */
	static void get_metadata(PyObject* obj, bool& out_is_1d_array, bool& out_is_fixed_dimension, 
	                        Py_ssize_t& out_size, metaffi_type& out_common_type);

public:
	/**
	 * @brief Construct empty list of given size
	 * @param rt Runtime manager reference
	 * @param size Initial size (elements are NULL)
	 */
	explicit py_list(cpython3_runtime_manager& rt, Py_ssize_t size = 0);

	/**
	 * @brief Construct from existing PyObject
	 * @param rt Runtime manager reference
	 * @param obj PyObject* that must be a Python list
	 * @throws std::runtime_error if obj is not a Python list
	 */
	py_list(cpython3_runtime_manager& rt, PyObject* obj);

	// Copy constructor
	py_list(py_list& other) noexcept;

	// Move constructor
	py_list(py_list&& other) noexcept;

	// Copy assignment
	py_list& operator=(const py_list& other);

	// Assignment from PyObject*
	py_list& operator=(PyObject* other);

	/**
	 * @brief Get item at index (borrowed reference)
	 */
	PyObject* operator[](int index);

	/**
	 * @brief Append item to list
	 * @param obj Item to append (borrowed reference)
	 */
	void append(PyObject* obj);

	/**
	 * @brief Set item at index
	 * @param index Index to set
	 * @param obj Item to set (steals reference)
	 */
	void set_item(Py_ssize_t index, PyObject* obj);

	/**
	 * @brief Get list length
	 */
	[[nodiscard]] Py_ssize_t length() const;
};
