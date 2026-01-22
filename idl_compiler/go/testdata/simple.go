package testdata

// Simple test data for IDL compiler tests

// SimpleFunction is a basic function
func SimpleFunction(x int, y string) bool {
	return true
}

// Add adds two integers
func Add(a int32, b int32) int32 {
	return a + b
}

// Greet returns a greeting
func Greet(name string) string {
	return "Hello, " + name
}

// NoParams has no parameters
func NoParams() {
}

// MultiReturn returns multiple values
func MultiReturn(x int) (int, error) {
	return x * 2, nil
}

// VariadicFunc accepts variable arguments
func VariadicFunc(prefix string, values ...int) string {
	return prefix
}

// Global variables
var GlobalInt int = 42
var GlobalString string = "test"

const GlobalConst int = 100

// SimpleStruct is a basic struct
type SimpleStruct struct {
	Name  string
	Value int
}

// Method is an instance method
func (s *SimpleStruct) Method(x int) int {
	return x + s.Value
}

// NewSimpleStruct is a constructor
func NewSimpleStruct(name string, value int) *SimpleStruct {
	return &SimpleStruct{Name: name, Value: value}
}

// SimpleInterface is a basic interface
type SimpleInterface interface {
	DoSomething() error
	GetValue() int
}
