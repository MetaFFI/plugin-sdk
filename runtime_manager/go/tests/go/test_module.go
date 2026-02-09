package main

import "C"

// EntryPoint_* names match MetaFFI Go guest compiler convention (runtime manager maps callable=X to symbol EntryPoint_X).

// Add adds two integers and returns the result
//
//export EntryPoint_Add
func EntryPoint_Add(a, b C.int) C.int {
	return a + b
}

// Subtract subtracts b from a and returns the result
//
//export EntryPoint_Subtract
func EntryPoint_Subtract(a, b C.int) C.int {
	return a - b
}

// Multiply multiplies two integers and returns the result
//
//export EntryPoint_Multiply
func EntryPoint_Multiply(a, b C.int) C.int {
	return a * b
}

// Divide divides a by b and returns the result as a float
//
//export EntryPoint_Divide
func EntryPoint_Divide(a, b C.double) C.double {
	if b == 0 {
		return 0
	}
	return a / b
}

// GetPi returns the value of Pi
//
//export EntryPoint_GetPi
func EntryPoint_GetPi() C.double {
	return 3.14159265358979
}

// IsPositive returns 1 if the number is positive, 0 otherwise
//
//export EntryPoint_IsPositive
func EntryPoint_IsPositive(n C.int) C.int {
	if n > 0 {
		return 1
	}
	return 0
}

// Max returns the maximum of two integers
//
//export EntryPoint_Max
func EntryPoint_Max(a, b C.int) C.int {
	if a > b {
		return a
	}
	return b
}

// Factorial calculates factorial of n (n!)
//
//export EntryPoint_Factorial
func EntryPoint_Factorial(n C.int) C.longlong {
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
