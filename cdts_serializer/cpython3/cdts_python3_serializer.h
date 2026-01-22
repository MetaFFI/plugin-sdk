#pragma once

#include <runtime/cdt.h>
#include <runtime/metaffi_primitives.h>
#include <Python.h>
#include <stdexcept>
#include <string>

namespace metaffi::utils
{

/**
 * @brief CDTS Python3 Serializer - Converts between Python objects and CDTS structures
 *
 * This class provides bidirectional conversion:
 * - Serialization: Python objects → CDTS (add() with explicit type)
 * - Deserialization: CDTS → Python objects (extract_pyobject)
 *
 * IMPORTANT - Type Specification Required:
 * Python's int/float types don't specify size (int8 vs int64, float32 vs float64).
 * Therefore, serialization REQUIRES explicit type specification via the add() method.
 * This prevents information loss where the caller needs int32 but gets int64.
 *
 * Memory Management:
 * - Uses XLLR allocation for CDTS (xllr_alloc_string*, xllr_alloc_cdt_array)
 * - Returns new Python references (caller must DECREF)
 * - Manages GIL automatically via RAII
 *
 * Thread Safety:
 * - Acquires GIL in all public methods
 * - Safe to call from any thread
 */
class cdts_python3_serializer
{
private:
	cdts& data;                    // Reference to CDTS being serialized/deserialized
	metaffi_size current_index;    // Current position in CDTS

	/**
	 * @brief RAII wrapper for Python GIL (Global Interpreter Lock)
	 * Ensures GIL is acquired on construction and released on destruction
	 */
	struct gil_scoped_acquire
	{
		PyGILState_STATE state;

		gil_scoped_acquire() : state(PyGILState_Ensure()) {}
		~gil_scoped_acquire() { PyGILState_Release(state); }

		// Non-copyable, non-movable
		gil_scoped_acquire(const gil_scoped_acquire&) = delete;
		gil_scoped_acquire& operator=(const gil_scoped_acquire&) = delete;
		gil_scoped_acquire(gil_scoped_acquire&&) = delete;
		gil_scoped_acquire& operator=(gil_scoped_acquire&&) = delete;
	};

public:
	/**
	 * @brief Construct serializer for given CDTS
	 * @param pcdts CDTS reference to serialize/deserialize
	 */
	explicit cdts_python3_serializer(cdts& pcdts);

	// ===== SERIALIZATION (Python → CDTS) =====

	/**
	 * @brief Serialize Python object to CDTS at current index with explicit type
	 * @param obj Python object to serialize (borrowed reference)
	 * @param target_type MetaFFI type to serialize as (e.g., metaffi_int32_type, metaffi_float32_type)
	 * @return Reference to this serializer (for chaining)
	 * @throws std::out_of_range if CDTS is full
	 * @throws std::runtime_error if Python object cannot be converted to target_type
	 * @throws std::runtime_error if value out of range for target_type (e.g., 300 for int8)
	 *
	 * REQUIRED for ambiguous types (int, float) - no defaults to prevent information loss.
	 * Optional for unambiguous types (bool, string, None) which ignore target_type parameter.
	 *
	 * Example:
	 *   cdts data(3);
	 *   cdts_python3_serializer ser(data);
	 *   PyObject* i = PyLong_FromLong(42);
	 *   PyObject* f = PyFloat_FromDouble(3.14);
	 *   
	 *   ser.add(i, metaffi_int32_type)        // Explicit: serialize as int32
	 *      .add(f, metaffi_float32_type)      // Explicit: serialize as float32
	 *      .add(Py_True, metaffi_bool_type);  // Unambiguous: bool
	 *   
	 *   Py_DECREF(i); Py_DECREF(f);
	 */
	cdts_python3_serializer& add(PyObject* obj, metaffi_type target_type);

	// ===== DESERIALIZATION (CDTS → Python) =====

	/**
	 * @brief Extract single PyObject from CDTS at current index
	 * @return New Python reference (caller must DECREF)
	 * @throws std::out_of_range if no more elements
	 * @throws std::runtime_error if CDTS type cannot be converted
	 *
	 * Example:
	 *   PyObject* obj = ser.extract_pyobject();
	 *   // Use obj...
	 *   Py_DECREF(obj);
	 */
	PyObject* extract_pyobject();

	/**
	 * @brief Extract all remaining elements as Python tuple
	 * @return New Python tuple reference (caller must DECREF)
	 * @throws std::runtime_error if any element cannot be converted
	 *
	 * Example:
	 *   PyObject* tuple = ser.extract_as_tuple();
	 *   // Use tuple...
	 *   Py_DECREF(tuple);
	 */
	PyObject* extract_as_tuple();

	// ===== TYPE INTROSPECTION =====

	/**
	 * @brief Peek at type of current element without extracting
	 * @return MetaFFI type of current element
	 * @throws std::out_of_range if no more elements
	 */
	metaffi_type peek_type() const;

	/**
	 * @brief Check if current element is null
	 * @return true if current element is metaffi_null_type
	 * @throws std::out_of_range if no more elements
	 */
	bool is_null() const;

	// ===== UTILITIES =====

	/**
	 * @brief Reset index to beginning
	 */
	void reset();

	/**
	 * @brief Get current index position
	 * @return Current index
	 */
	metaffi_size get_index() const;

	/**
	 * @brief Set index position
	 * @param index New index position
	 * @throws std::out_of_range if index >= size
	 */
	void set_index(metaffi_size index);

	/**
	 * @brief Get total size of CDTS
	 * @return Number of elements in CDTS
	 */
	metaffi_size size() const;

	/**
	 * @brief Check if there are more elements to process
	 * @return true if current_index < size
	 */
	bool has_more() const;

	/**
	 * @brief Handle releaser callback for Python objects in handles
	 * @param handle Handle containing PyObject*
	 *
	 * This is public so it can be used in tests and when manually creating handles
	 */
	static void py_object_releaser(cdt_metaffi_handle* handle);

private:
	// ===== CONVERSION HELPERS =====

	/**
	 * @brief Convert Python object to CDT with explicit type
	 * @param obj Python object (borrowed reference)
	 * @param target Target CDT to fill
	 * @param target_type MetaFFI type to serialize as
	 * @throws std::runtime_error on conversion error or value out of range
	 *
	 * Assumes GIL is held
	 */
	void pyobject_to_cdt(PyObject* obj, cdt& target, metaffi_type target_type);

	/**
	 * @brief Convert CDT to Python object
	 * @param source Source CDT to convert
	 * @return New Python reference
	 * @throws std::runtime_error on conversion error
	 *
	 * Assumes GIL is held
	 */
	PyObject* cdt_to_pyobject(const cdt& source);

	/**
	 * @brief Convert Python list/tuple to CDT array with explicit element type
	 * @param list Python list or tuple (borrowed reference)
	 * @param target Target CDT to fill with array
	 * @param element_type MetaFFI type for array elements
	 * @throws std::runtime_error on conversion error
	 *
	 * Assumes GIL is held
	 */
	void pylist_to_cdt_array(PyObject* list, cdt& target, metaffi_type element_type);

	/**
	 * @brief Convert CDT array to Python list
	 * @param arr Source CDTS array
	 * @return New Python list reference
	 * @throws std::runtime_error on conversion error
	 *
	 * Assumes GIL is held
	 */
	PyObject* cdt_array_to_pylist(const cdts& arr);

	/**
	 * @brief Validate that a Python integer fits in the target type
	 * @param value Python long long value
	 * @param target_type Target MetaFFI integer type
	 * @throws std::runtime_error if value out of range
	 *
	 * Assumes GIL is held
	 */
	void validate_int_range(long long value, metaffi_type target_type);

	/**
	 * @brief Check if index is within bounds
	 * @param index Index to check
	 * @throws std::out_of_range if index >= size
	 */
	void check_bounds(metaffi_size index) const;
};

} // namespace metaffi::utils
