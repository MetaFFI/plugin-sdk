package idl_compiler

import (
	"go/ast"
	"strings"
)

// parseStruct parses a struct type declaration into StructInfo
func (e *Extractor) parseStruct(name string, structType *ast.StructType, doc *ast.CommentGroup) *StructInfo {
	structInfo := &StructInfo{
		Name:    name,
		Comment: extractComment(doc),
		Fields:  []*FieldInfo{},
		Methods: []*FunctionInfo{},
	}

	// Parse struct fields
	if structType.Fields != nil {
		for _, field := range structType.Fields.List {
			// Check if field has names (embedded fields don't)
			if len(field.Names) == 0 {
				// Embedded field
				fieldInfo := e.parseField("", field.Type, field.Tag, field.Comment, true)
				if fieldInfo != nil && IsExported(fieldInfo.Name) {
					structInfo.Fields = append(structInfo.Fields, fieldInfo)
				}
			} else {
				// Named fields
				for _, fieldName := range field.Names {
					// Only include exported fields
					if !IsExported(fieldName.Name) {
						continue
					}

					fieldInfo := e.parseField(fieldName.Name, field.Type, field.Tag, field.Comment, false)
					if fieldInfo != nil {
						structInfo.Fields = append(structInfo.Fields, fieldInfo)
					}
				}
			}
		}
	}

	return structInfo
}

// parseField parses a struct field
func (e *Extractor) parseField(name string, typeExpr ast.Expr, tag *ast.BasicLit, comment *ast.CommentGroup, isEmbedded bool) *FieldInfo {
	// For embedded fields, extract the type name
	if isEmbedded {
		name = e.getTypeName(typeExpr)
	}

	// Map the type
	metaffiType, dimensions, typeAlias := e.typeMapper.MapType(typeExpr, "")

	fieldInfo := &FieldInfo{
		Name:       name,
		Type:       metaffiType,
		TypeAlias:  typeAlias,
		Dimensions: dimensions,
		Comment:    extractComment(comment),
		IsEmbedded: isEmbedded,
	}

	// Extract struct tag if present
	if tag != nil {
		fieldInfo.Tag = strings.Trim(tag.Value, "`")
	}

	return fieldInfo
}

// parseInterface parses an interface type declaration into InterfaceInfo
func (e *Extractor) parseInterface(name string, interfaceType *ast.InterfaceType, doc *ast.CommentGroup) *InterfaceInfo {
	interfaceInfo := &InterfaceInfo{
		Name:    name,
		Comment: extractComment(doc),
		Methods: []*FunctionInfo{},
	}

	// Parse interface methods
	if interfaceType.Methods != nil {
		for _, method := range interfaceType.Methods.List {
			// Each method should have a name
			if len(method.Names) == 0 {
				// Embedded interface - skip for now
				continue
			}

			for _, methodName := range method.Names {
				// Only exported methods
				if !IsExported(methodName.Name) {
					continue
				}

				// Parse method signature
				if funcType, ok := method.Type.(*ast.FuncType); ok {
					methodInfo := e.parseFunctionType(methodName.Name, funcType, method.Comment)
					if methodInfo != nil {
						interfaceInfo.Methods = append(interfaceInfo.Methods, methodInfo)
					}
				}
			}
		}
	}

	return interfaceInfo
}

// parseFunctionType parses a function type (used for interface methods)
func (e *Extractor) parseFunctionType(name string, funcType *ast.FuncType, comment *ast.CommentGroup) *FunctionInfo {
	funcInfo := &FunctionInfo{
		Name:    name,
		Comment: extractComment(comment),
	}

	// Parse parameters
	if funcType.Params != nil {
		for _, field := range funcType.Params.List {
			if len(field.Names) == 0 {
				// Unnamed parameter
				param := e.parseParameter("", field.Type, field.Comment)
				funcInfo.Parameters = append(funcInfo.Parameters, param)
			} else {
				for _, paramName := range field.Names {
					param := e.parseParameter(paramName.Name, field.Type, field.Comment)
					funcInfo.Parameters = append(funcInfo.Parameters, param)
				}
			}
		}
	}

	// Check for variadic
	if funcType.Params != nil && len(funcType.Params.List) > 0 {
		lastParam := funcType.Params.List[len(funcType.Params.List)-1]
		if _, ok := lastParam.Type.(*ast.Ellipsis); ok {
			funcInfo.IsVariadic = true
		}
	}

	// Parse return values
	if funcType.Results != nil {
		for _, field := range funcType.Results.List {
			if len(field.Names) == 0 {
				// Unnamed return
				param := e.parseParameter("", field.Type, field.Comment)
				funcInfo.ReturnValues = append(funcInfo.ReturnValues, param)
			} else {
				for _, retName := range field.Names {
					param := e.parseParameter(retName.Name, field.Type, field.Comment)
					funcInfo.ReturnValues = append(funcInfo.ReturnValues, param)
				}
			}
		}
	}

	return funcInfo
}

// extractMethodsForStruct finds all methods for a given struct
func (e *Extractor) extractMethodsForStruct(structName string, file *ast.File) []*FunctionInfo {
	methods := []*FunctionInfo{}

	// Walk through all function declarations
	for _, decl := range file.Decls {
		funcDecl, ok := decl.(*ast.FuncDecl)
		if !ok {
			continue
		}

		// Check if it's a method with receiver
		if funcDecl.Recv == nil || len(funcDecl.Recv.List) == 0 {
			continue
		}

		// Get receiver type
		recvField := funcDecl.Recv.List[0]
		recvTypeName := e.getTypeName(recvField.Type)

		// Check if receiver type matches our struct
		if recvTypeName == structName {
			// Only include exported methods
			if IsExported(funcDecl.Name.Name) {
				methodInfo := e.parseFunctionSignature(funcDecl)
				if methodInfo != nil {
					methods = append(methods, methodInfo)
				}
			}
		}
	}

	return methods
}

// attachMethodsToStructs attaches methods to their corresponding structs
func (e *Extractor) attachMethodsToStructs(info *ExtractedInfo) {
	// For each parsed file, find methods and attach to structs
	for _, file := range info.ParsedFiles {
		for _, structInfo := range info.Structs {
			methods := e.extractMethodsForStruct(structInfo.Name, file)
			structInfo.Methods = append(structInfo.Methods, methods...)
		}
	}
}

// attachConstructorsToStructs finds and attaches constructor functions to structs
func (e *Extractor) attachConstructorsToStructs(info *ExtractedInfo) {
	for _, structInfo := range info.Structs {
		constructor := e.findConstructor(structInfo.Name, info.Functions)
		if constructor != nil {
			structInfo.Constructor = constructor
		}
	}
}
