#pragma once
#include "python_api_wrapper.h"

/**
 * @brief RAII guard for Python GIL (Global Interpreter Lock)
 *
 * Acquires GIL on construction, releases on destruction.
 * Non-copyable, non-movable.
 */
class gil_guard
{
public:
	gil_guard(): state_(pPyGILState_Ensure()) {}
	~gil_guard() { pPyGILState_Release(state_); }

	gil_guard(const gil_guard&) = delete;
	gil_guard& operator=(const gil_guard&) = delete;
	gil_guard(gil_guard&&) = delete;
	gil_guard& operator=(gil_guard&&) = delete;

private:
	PyGILState_STATE state_;
};
