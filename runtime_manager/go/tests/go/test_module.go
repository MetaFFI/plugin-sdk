package main

import "C"

// Add adds two integers and returns the result
//
//export Add
func Add(a, b C.int) C.int {
	return a + b
}

// Subtract subtracts b from a and returns the result
//
//export Subtract
func Subtract(a, b C.int) C.int {
	return a - b
}

// Multiply multiplies two integers and returns the result
//
//export Multiply
func Multiply(a, b C.int) C.int {
	return a * b
}

// Divide divides a by b and returns the result as a float
//
//export Divide
func Divide(a, b C.double) C.double {
	if b == 0 {
		return 0
	}
	return a / b
}

// GetPi returns the value of Pi
//
//export GetPi
func GetPi() C.double {
	return 3.14159265358979
}

// IsPositive returns 1 if the number is positive, 0 otherwise
//
//export IsPositive
func IsPositive(n C.int) C.int {
	if n > 0 {
		return 1
	}
	return 0
}

// Max returns the maximum of two integers
//
//export Max
func Max(a, b C.int) C.int {
	if a > b {
		return a
	}
	return b
}

// Factorial calculates factorial of n (n!)
//
//export Factorial
func Factorial(n C.int) C.longlong {
	if n <= 1 {
		return 1
	}
	result := C.longlong(1)
	for i := C.int(2); i <= n; i++ {
		result *= C.longlong(i)
	}
	return result
}

// Required for c-shared build mode
func main() {}
