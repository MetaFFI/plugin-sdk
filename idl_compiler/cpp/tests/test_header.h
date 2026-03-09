/**
 * test_header.h — Input fixture for C++ IDL compiler tests.
 *
 * Contains:
 *   - Global namespace free function and global variable
 *   - Namespaced free function and global variable (namespace math)
 *   - Class Point with constructor, destructor, instance methods, and fields
 */

#pragma once

// Global namespace function and global
int add(int a, int b);
extern int g_counter;

// Namespaced function and global
namespace math
{
	int multiply(int a, int b);
	extern int g_scale;
}

// Class with constructor, destructor, method, field
class Point
{
public:
	int    x;
	double y;

	Point(int x, double y);
	~Point();

	double sum() const;
	void   set_x(int v);

	static int static_add(int a, int b);
};
