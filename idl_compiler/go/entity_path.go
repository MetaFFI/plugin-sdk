package idl_compiler

import (
	"fmt"
)

// EntityPathGenerator generates entity_path structures for Go entities
// according to entity_path_specs.json
type EntityPathGenerator struct{}

// NewEntityPathGenerator creates a new EntityPathGenerator
func NewEntityPathGenerator() *EntityPathGenerator {
	return &EntityPathGenerator{}
}

// CreateFunctionEntityPath creates entity_path for a package-level function
// Format: {"callable": "FunctionName"}
func (epg *EntityPathGenerator) CreateFunctionEntityPath(functionName string) map[string]string {
	return map[string]string{
		"callable": functionName,
	}
}

// CreateMethodEntityPath creates entity_path for a struct method
// Format: {"callable": "StructName.MethodName"}
func (epg *EntityPathGenerator) CreateMethodEntityPath(structName, methodName string) map[string]string {
	return map[string]string{
		"callable": fmt.Sprintf("%s.%s", structName, methodName),
	}
}

// CreateConstructorEntityPath creates entity_path for a constructor function
// Format: {"callable": "ConstructorFunctionName"}
// Note: Go doesn't have special constructor syntax, so NewXxx functions are treated as regular functions
func (epg *EntityPathGenerator) CreateConstructorEntityPath(constructorName string) map[string]string {
	return map[string]string{
		"callable": constructorName,
	}
}

// CreateGlobalGetterEntityPath creates entity_path for a global variable getter
// Format: {"global": "VarName", "getter": "true"}
func (epg *EntityPathGenerator) CreateGlobalGetterEntityPath(globalName string) map[string]string {
	return map[string]string{
		"global": globalName,
		"getter": "true",
	}
}

// CreateGlobalSetterEntityPath creates entity_path for a global variable setter
// Format: {"global": "VarName", "setter": "true"}
func (epg *EntityPathGenerator) CreateGlobalSetterEntityPath(globalName string) map[string]string {
	return map[string]string{
		"global": globalName,
		"setter": "true",
	}
}

// CreateFieldGetterEntityPath creates entity_path for a struct field getter
// Format: {"field": "StructName.FieldName", "getter": "true"}
func (epg *EntityPathGenerator) CreateFieldGetterEntityPath(structName, fieldName string) map[string]string {
	return map[string]string{
		"field":  fmt.Sprintf("%s.%s", structName, fieldName),
		"getter": "true",
	}
}

// CreateFieldSetterEntityPath creates entity_path for a struct field setter
// Format: {"field": "StructName.FieldName", "setter": "true"}
func (epg *EntityPathGenerator) CreateFieldSetterEntityPath(structName, fieldName string) map[string]string {
	return map[string]string{
		"field":  fmt.Sprintf("%s.%s", structName, fieldName),
		"setter": "true",
	}
}
