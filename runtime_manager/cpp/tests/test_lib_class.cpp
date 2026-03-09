/**
 * test_lib_class.cpp — C++ class test library for Phase 2 runtime_manager tests.
 *
 * Exports:
 *   Namespace math:
 *     int math::multiply(int, int)   — namespaced free function
 *     int math::g_scale              — namespaced global variable
 *
 *   Class Point:
 *     int    x      — public field at offset 0
 *     double y      — public field (aligned offset, typically 8)
 *     Point(int x, double y)         — constructor
 *     ~Point()                       — destructor
 *     double sum() const             — instance method: x + y
 *     void   set_x(int v)            — instance method: set x
 *     static int static_add(int, int) — static method (free function semantics)
 *
 * All symbols exported via WINDOWS_EXPORT_ALL_SYMBOLS / visibility=default.
 * The calling test uses platform-specific mangled names via #ifdef _MSC_VER.
 */

#include <cstddef>
#include <cstdlib>

#ifdef _WIN32
#   define EXPORT __declspec(dllexport)
#else
#   define EXPORT __attribute__((visibility("default")))
#endif


// ============================================================================
// Namespaced free function and global
// ============================================================================

namespace math
{
	EXPORT int g_scale = 10;

	EXPORT int multiply(int a, int b)
	{
		return a * b;
	}
} // namespace math


// ============================================================================
// Class Point
// ============================================================================

class EXPORT Point
{
public:
	int    x;
	double y;

	Point(int x_val, double y_val)
		: x(x_val)
		, y(y_val)
	{
	}

	// Explicitly defined (not defaulted) so the symbol is always exported.
	~Point()
	{
		x = 0;
		y = 0.0;
	}

	double sum() const
	{
		return static_cast<double>(x) + y;
	}

	void set_x(int v)
	{
		x = v;
	}

	static int static_add(int a, int b)
	{
		return a + b;
	}
};


// ============================================================================
// Exported factory helpers (extern "C") for regression / portability tests.
// These allow tests to verify the class works without relying on mangled names.
// ============================================================================

extern "C"
{
	EXPORT Point* point_create(int x, double y)
	{
		return new Point(x, y);
	}

	EXPORT void point_destroy(Point* p)
	{
		delete p;
	}

	EXPORT double point_sum(Point* p)
	{
		return p->sum();
	}

	EXPORT int point_get_x(Point* p)
	{
		return p->x;
	}

	EXPORT double point_get_y(Point* p)
	{
		return p->y;
	}

	EXPORT std::size_t point_sizeof()
	{
		return sizeof(Point);
	}

	EXPORT std::size_t point_offset_x()
	{
		return offsetof(Point, x);
	}

	EXPORT std::size_t point_offset_y()
	{
		return offsetof(Point, y);
	}
}
