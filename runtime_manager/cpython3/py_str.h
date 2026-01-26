#pragma once

#include "py_object.h"
#include <runtime/metaffi_primitives.h>
#include <string>

/**
 * @brief Wrapper for Python str (unicode) objects
 * 
 * Provides type-safe construction and extraction of Python strings.
 * Supports UTF-8, UTF-16, and UTF-32 encodings.
 */
class py_str : public py_object
{
public:
	/**
	 * @brief Check if a PyObject is a Python str
	 * @param obj PyObject to check
	 * @return true if obj is a Python str
	 * @note Assumes GIL is held by caller
	 */
	static bool check(PyObject* obj);

public:
	/**
	 * @brief Construct empty string
	 */
	explicit py_str(cpython3_runtime_manager& rt);

	/**
	 * @brief Construct from existing PyObject
	 * @param rt Runtime manager reference
	 * @param obj PyObject* that must be a Python str
	 * @throws std::runtime_error if obj is not a Python str
	 */
	py_str(cpython3_runtime_manager& rt, PyObject* obj);

	/**
	 * @brief Construct from UTF-8 character
	 */
	py_str(cpython3_runtime_manager& rt, metaffi_char8 c);

	/**
	 * @brief Construct from UTF-16 character
	 */
	py_str(cpython3_runtime_manager& rt, metaffi_char16 c);

	/**
	 * @brief Construct from UTF-32 character
	 */
	py_str(cpython3_runtime_manager& rt, metaffi_char32 c);

	/**
	 * @brief Construct from UTF-8 string (char8_t*)
	 */
	py_str(cpython3_runtime_manager& rt, const char8_t* s);

	/**
	 * @brief Construct from UTF-8 string (const char*)
	 */
	py_str(cpython3_runtime_manager& rt, const char* s);

	/**
	 * @brief Construct from UTF-16 string
	 */
	py_str(cpython3_runtime_manager& rt, const char16_t* s);

	/**
	 * @brief Construct from UTF-32 string
	 */
	py_str(cpython3_runtime_manager& rt, const char32_t* s);

	// Move constructor
	py_str(py_str&& other) noexcept;

	// Copy assignment
	py_str& operator=(const py_str& other);

	/**
	 * @brief Get string length in code points
	 */
	[[nodiscard]] Py_ssize_t length() const;

	/**
	 * @brief Convert to UTF-8 string
	 * @return Newly allocated UTF-8 string (caller must delete[])
	 */
	[[nodiscard]] metaffi_string8 to_utf8() const;

	/**
	 * @brief Convert to UTF-16 string
	 * @return Newly allocated UTF-16 string (caller must delete[])
	 */
	[[nodiscard]] metaffi_string16 to_utf16() const;

	/**
	 * @brief Convert to UTF-32 string
	 * @return Newly allocated UTF-32 string (caller must delete[])
	 */
	[[nodiscard]] metaffi_string32 to_utf32() const;

	// Conversion operators to std::basic_string types
	explicit operator std::u8string() const;
	explicit operator std::u16string() const;
	explicit operator std::u32string() const;
	explicit operator std::string() const;
};
