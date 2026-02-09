package idl_compiler

import (
	"fmt"
	"go/ast"
	"strings"

	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

// TypeMapper maps Go types to MetaFFI types
type TypeMapper struct{}

// NewTypeMapper creates a new TypeMapper instance
func NewTypeMapper() *TypeMapper {
	return &TypeMapper{}
}

// MapType maps a Go type expression to MetaFFI type and dimensions
// Returns (metaffiType, dimensions, typeAlias)
func (tm *TypeMapper) MapType(typeExpr ast.Expr, typeName string) (IDL.MetaFFIType, int, string) {
	// Handle nil types (void/untyped)
	if typeExpr == nil {
		return IDL.NULL, 0, ""
	}

	// Array/slice types - check dimensions first
	dimensions := 0
	currentType := typeExpr

	for {
		switch t := currentType.(type) {
		case *ast.ArrayType:
			dimensions++
			currentType = t.Elt
		default:
			goto done_counting
		}
	}
done_counting:

	// Map the base type
	metaffiType, baseTypeAlias := tm.mapBaseType(currentType, typeName)

	// Construct full type alias with array dimensions
	typeAlias := baseTypeAlias
	if dimensions > 0 {
		typeAlias = strings.Repeat("[]", dimensions) + baseTypeAlias
	}

	return metaffiType, dimensions, typeAlias
}

// mapBaseType maps a base Go type to MetaFFI type
func (tm *TypeMapper) mapBaseType(typeExpr ast.Expr, typeName string) (IDL.MetaFFIType, string) {
	// If typeName is provided, use it for mapping
	if typeName != "" {
		return tm.mapPrimitiveType(typeName), typeName
	}

	// Extract type from AST
	switch t := typeExpr.(type) {
	case *ast.Ident:
		// Simple identifier (int, string, CustomType, etc.)
		return tm.mapPrimitiveType(t.Name), t.Name

	case *ast.SelectorExpr:
		// Qualified identifier (pkg.Type)
		pkgName := ""
		if pkg, ok := t.X.(*ast.Ident); ok {
			pkgName = pkg.Name
		}
		fullName := pkgName + "." + t.Sel.Name
		return IDL.HANDLE, fullName

	case *ast.StarExpr:
		// Pointer type (*T) - map the underlying type, but preserve * in alias
		baseType, baseAlias := tm.mapBaseType(t.X, "")
		return baseType, "*" + baseAlias

	case *ast.InterfaceType:
		// Interface type
		if t.Methods == nil || len(t.Methods.List) == 0 {
			// Empty interface (interface{})
			return IDL.ANY, "interface{}"
		}
		// Non-empty interface
		return IDL.HANDLE, "interface"

	case *ast.StructType:
		// Anonymous struct
		return IDL.HANDLE, "struct"

	case *ast.MapType:
		// Map type (map[K]V) - render full type
		_, keyAlias := tm.mapBaseType(t.Key, "")
		_, valAlias := tm.mapBaseType(t.Value, "")
		return IDL.HANDLE, "map[" + keyAlias + "]" + valAlias

	case *ast.ChanType:
		// Channel type - render full type (chan T, chan<- T, <-chan T)
		_, elemAlias := tm.mapBaseType(t.Value, "")
		switch t.Dir {
		case ast.SEND:
			return IDL.HANDLE, "chan<- " + elemAlias
		case ast.RECV:
			return IDL.HANDLE, "<-chan " + elemAlias
		default:
			return IDL.HANDLE, "chan " + elemAlias
		}

	case *ast.FuncType:
		// Function type - render full signature as callable
		return IDL.CALLABLE, renderFuncType(t, tm)

	default:
		// Unknown type - fallback to handle
		return IDL.HANDLE, fmt.Sprintf("%T", typeExpr)
	}
}

// mapPrimitiveType maps Go primitive type names to MetaFFI types
func (tm *TypeMapper) mapPrimitiveType(typeName string) IDL.MetaFFIType {
	switch typeName {
	// Boolean
	case "bool":
		return IDL.BOOL

	// Signed integers
	case "int":
		return IDL.INT64 // Platform-dependent, use int64 for consistency
	case "int8":
		return IDL.INT8
	case "int16":
		return IDL.INT16
	case "int32", "rune": // rune is alias for int32
		return IDL.INT32
	case "int64":
		return IDL.INT64

	// Unsigned integers
	case "uint":
		return IDL.UINT64 // Platform-dependent, use uint64 for consistency
	case "uint8", "byte": // byte is alias for uint8
		return IDL.UINT8
	case "uint16":
		return IDL.UINT16
	case "uint32":
		return IDL.UINT32
	case "uint64":
		return IDL.UINT64
	case "uintptr":
		return IDL.SIZE

	// Floating point
	case "float32":
		return IDL.FLOAT32
	case "float64":
		return IDL.FLOAT64

	// Complex (not directly supported - use handle)
	case "complex64", "complex128":
		return IDL.HANDLE

	// String
	case "string":
		return IDL.STRING8

	// Special types
	case "any":
		return IDL.ANY // any is alias for interface{} (Go 1.18+)
	case "error":
		return IDL.HANDLE // error is an interface

	// Unsafe pointer
	case "unsafe.Pointer":
		return IDL.HANDLE

	// Default: custom type
	default:
		return IDL.HANDLE
	}
}

// GetArrayElementType returns the MetaFFI type for array elements
// Example: []int32 with dimensions=1 → returns INT32
func (tm *TypeMapper) GetArrayElementType(metaffiType IDL.MetaFFIType) IDL.MetaFFIType {
	// Already handled in MapType - dimensions are separate
	return metaffiType
}

// renderFuncType renders a full Go function type signature from its AST node.
// Example: func(string, int) (string, error)
func renderFuncType(ft *ast.FuncType, tm *TypeMapper) string {
	renderFieldTypes := func(fl *ast.FieldList) string {
		if fl == nil || len(fl.List) == 0 {
			return ""
		}
		var parts []string
		for _, f := range fl.List {
			_, alias := tm.mapBaseType(f.Type, "")
			if len(f.Names) <= 1 {
				parts = append(parts, alias)
			} else {
				for range f.Names {
					parts = append(parts, alias)
				}
			}
		}
		return strings.Join(parts, ", ")
	}

	params := renderFieldTypes(ft.Params)
	result := ""
	if ft.Results != nil && len(ft.Results.List) > 0 {
		resultTypes := renderFieldTypes(ft.Results)
		if len(ft.Results.List) > 1 {
			result = " (" + resultTypes + ")"
		} else {
			result = " " + resultTypes
		}
	}
	return "func(" + params + ")" + result
}

// IsExportedType checks if a type name is exported (public)
func IsExportedType(name string) bool {
	if name == "" {
		return false
	}
	// Check if first character is uppercase
	return name[0] >= 'A' && name[0] <= 'Z'
}

