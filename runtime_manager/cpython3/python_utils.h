#pragma once
#include "python_api_wrapper.h"
#include <string>

namespace metaffi::runtime::cpython3
{

/**
 * @brief Get current Python error message and clear error state
 * @return Error message string, or empty if no error
 */
std::string check_python_error();

}
