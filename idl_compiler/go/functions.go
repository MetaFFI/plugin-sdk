package idl_compiler

import (
	"go/ast"
	"strings"
)

// parseFunctionSignature parses a function declaration into FunctionInfo
func (e *Extractor) parseFunctionSignature(funcDecl *ast.FuncDecl) *FunctionInfo {
	funcInfo := &FunctionInfo{
		Name:    funcDecl.Name.Name,
		Comment: extractComment(funcDecl.Doc),
	}

	// Parse receiver (for methods)
	if funcDecl.Recv != nil && len(funcDecl.Recv.List) > 0 {
		recvField := funcDecl.Recv.List[0]
		funcInfo.ReceiverType = e.getTypeName(recvField.Type)

		// Check if receiver is pointer
		if _, isPtr := recvField.Type.(*ast.StarExpr); isPtr {
			funcInfo.ReceiverPtr = true
		}
	}

	// Parse parameters
	if funcDecl.Type.Params != nil {
		for _, field := range funcDecl.Type.Params.List {
			// Each field can have multiple names (e.g., a, b int)
			if len(field.Names) == 0 {
				// Unnamed parameter
				param := e.parseParameter("", field.Type, field.Comment)
				funcInfo.Parameters = append(funcInfo.Parameters, param)
			} else {
				for _, name := range field.Names {
					param := e.parseParameter(name.Name, field.Type, field.Comment)
					funcInfo.Parameters = append(funcInfo.Parameters, param)
				}
			}
		}
	}

	// Check for variadic function
	if funcDecl.Type.Params != nil && len(funcDecl.Type.Params.List) > 0 {
		lastParam := funcDecl.Type.Params.List[len(funcDecl.Type.Params.List)-1]
		if _, ok := lastParam.Type.(*ast.Ellipsis); ok {
			funcInfo.IsVariadic = true
		}
	}

	// Parse return values
	if funcDecl.Type.Results != nil {
		for _, field := range funcDecl.Type.Results.List {
			// Generate name for unnamed returns
			names := field.Names
			if len(names) == 0 {
				// Unnamed return - generate name
				retName := ""
				if len(funcDecl.Type.Results.List) == 1 {
					retName = "ret"
				} else {
					retName = "" // Will be auto-generated as ret_0, ret_1, etc.
				}
				param := e.parseParameter(retName, field.Type, field.Comment)
				funcInfo.ReturnValues = append(funcInfo.ReturnValues, param)
			} else {
				for _, name := range names {
					param := e.parseParameter(name.Name, field.Type, field.Comment)
					funcInfo.ReturnValues = append(funcInfo.ReturnValues, param)
				}
			}
		}
	}

	return funcInfo
}

// parseParameter parses a function parameter or return value
func (e *Extractor) parseParameter(name string, typeExpr ast.Expr, comment *ast.CommentGroup) *ParameterInfo {
	// Handle ellipsis (variadic) parameters
	if ellipsis, ok := typeExpr.(*ast.Ellipsis); ok {
		typeExpr = ellipsis.Elt // Get the element type
		// The variadic nature is tracked in FunctionInfo.IsVariadic
	}

	// Map the type
	metaffiType, dimensions, typeAlias := e.typeMapper.MapType(typeExpr, "")

	return &ParameterInfo{
		Name:       name,
		Type:       metaffiType,
		TypeAlias:  typeAlias,
		Dimensions: dimensions,
		Comment:    extractComment(comment),
	}
}

// getTypeName extracts the type name from a type expression
func (e *Extractor) getTypeName(typeExpr ast.Expr) string {
	switch t := typeExpr.(type) {
	case *ast.Ident:
		return t.Name
	case *ast.StarExpr:
		// Pointer type - get the underlying type
		return e.getTypeName(t.X)
	case *ast.SelectorExpr:
		// Qualified type (pkg.Type)
		if pkg, ok := t.X.(*ast.Ident); ok {
			return pkg.Name + "." + t.Sel.Name
		}
		return t.Sel.Name
	default:
		return ""
	}
}

// findConstructor tries to find a constructor function for a struct
// Constructor is typically a function named "New{StructName}" that returns the struct type
func (e *Extractor) findConstructor(structName string, functions []*FunctionInfo) *FunctionInfo {
	// Look for NewXxx or new naming patterns
	patterns := []string{
		"New" + structName,
		"new" + structName,
		"New" + strings.ToLower(structName),
	}

	for _, fn := range functions {
		for _, pattern := range patterns {
			if strings.EqualFold(fn.Name, pattern) {
				// Check if it returns the struct type
				if len(fn.ReturnValues) > 0 {
					returnType := fn.ReturnValues[0].TypeAlias
					// Check if return type matches struct name (with or without pointer)
					if strings.Contains(returnType, structName) {
						return fn
					}
				}
			}
		}
	}

	return nil
}
