#pragma once

#include "py_object.h"
#include <runtime/cdt.h>

/**
 * @brief Wrapper for MetaFFI handle objects (cross-language handles)
 * 
 * MetaFFI handles wrap objects from other runtimes (Python, JVM, Go, etc.)
 * and provide a way to pass them across language boundaries.
 */
class py_metaffi_handle : public py_object
{
public:
	/**
	 * @brief Check if a PyObject is a MetaFFI handle
	 * @param obj PyObject to check
	 * @return true if obj is a MetaFFIHandle object
	 * @note Assumes GIL is held by caller
	 */
	static bool check(PyObject* obj);

	/**
	 * @brief Extract PyObject from a cdt_metaffi_handle
	 * 
	 * If the handle is from Python runtime, returns the wrapped PyObject.
	 * If from another runtime, creates a Python MetaFFIHandle wrapper object.
	 * 
	 * @param rt Runtime manager reference
	 * @param cdt_handle Handle to extract from
	 * @return py_object wrapping the extracted object
	 * @note Assumes GIL is held by caller
	 */
	static py_object extract_pyobject_from_handle(cpython3_runtime_manager& rt, const cdt_metaffi_handle& cdt_handle);

public:
	/**
	 * @brief Construct from existing PyObject (must be MetaFFIHandle)
	 * @param rt Runtime manager reference
	 * @param obj PyObject* that must be a MetaFFIHandle
	 * @throws std::runtime_error if obj is not a MetaFFIHandle
	 */
	py_metaffi_handle(cpython3_runtime_manager& rt, PyObject* obj);

	// Move constructor
	py_metaffi_handle(py_metaffi_handle&& other) noexcept;

	// Copy assignment
	py_metaffi_handle& operator=(const py_metaffi_handle& other);

	/**
	 * @brief Convert to cdt_metaffi_handle
	 * @return Newly allocated cdt_metaffi_handle (caller must delete)
	 * @throws std::runtime_error if conversion fails
	 */
	[[nodiscard]] cdt_metaffi_handle* as_cdt_metaffi_handle() const;
};
