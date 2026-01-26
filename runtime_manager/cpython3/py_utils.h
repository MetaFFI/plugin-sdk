#pragma once

#include <string>

/**
 * @brief Check for Python errors and return error message
 * 
 * Checks if a Python error has occurred and returns a formatted error message.
 * Clears the Python error state after fetching.
 * 
 * @return Error message string, or empty string if no error occurred
 * @note Assumes GIL is held by caller
 */
std::string check_python_error();
