/**
 * test_lib.c — Pure-C test library for the cpp_runtime_manager tests.
 *
 * Compiled as a shared library (.dll / .so). Because this is pure C with no
 * C++ runtime dependency, detect_module_abi() should classify it as c_only.
 *
 * Exported symbols:
 *   Functions : add, subtract, multiply, divide, get_pi, is_positive, max, factorial
 *   Globals   : g_counter (int), g_message (const char*)
 */

#ifdef _WIN32
#   define EXPORT __declspec(dllexport)
#else
#   define EXPORT __attribute__((visibility("default")))
#endif

#include <stdlib.h>   /* abs */
#include <string.h>   /* strlen */


/* -------------------------------------------------------------------------
 * Free functions
 * ---------------------------------------------------------------------- */

EXPORT int add(int a, int b)
{
	return a + b;
}

EXPORT int subtract(int a, int b)
{
	return a - b;
}

EXPORT int multiply(int a, int b)
{
	return a * b;
}

EXPORT double divide(double a, double b)
{
	if (b == 0.0) return 0.0;
	return a / b;
}

EXPORT double get_pi(void)
{
	return 3.14159265358979;
}

EXPORT int is_positive(int n)
{
	return (n > 0) ? 1 : 0;
}

EXPORT int max_of(int a, int b)
{
	return (a > b) ? a : b;
}

EXPORT long long factorial(int n)
{
	if (n <= 1) return 1LL;
	long long result = 1LL;
	int i;
	for (i = 2; i <= n; ++i) result *= (long long)i;
	return result;
}


/* -------------------------------------------------------------------------
 * Global variables
 * ---------------------------------------------------------------------- */

EXPORT int g_counter = 42;

EXPORT double g_ratio = 2.71828;

EXPORT const char* g_message = "hello from test_lib";
