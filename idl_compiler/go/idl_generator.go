package idl_compiler

import (
	"encoding/json"
	"fmt"
	"path/filepath"
	"strings"

	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

// IDLGenerator generates MetaFFI IDL JSON from extracted Go information
type IDLGenerator struct {
	extractedInfo   *ExtractedInfo
	sourcePath      string
	entityPathGen   *EntityPathGenerator
	overloadIndexes map[string]int // Track overload indexes for same-named functions
}

// NewIDLGenerator creates a new IDL generator
func NewIDLGenerator(extractedInfo *ExtractedInfo, sourcePath string) *IDLGenerator {
	return &IDLGenerator{
		extractedInfo:   extractedInfo,
		sourcePath:      sourcePath,
		entityPathGen:   NewEntityPathGenerator(),
		overloadIndexes: make(map[string]int),
	}
}

// Generate generates the IDL definition and returns it as JSON string
func (g *IDLGenerator) Generate() (string, error) {
	// Create IDL definition
	idlDef := IDL.NewIDLDefinition(g.sourcePath, "go")

	// Set basic metadata (additional fields beyond constructor)
	g.setMetadata(idlDef)

	// Create module definition
	moduleDef := g.createModuleDefinition()

	// Add module to IDL
	idlDef.AddModule(moduleDef)

	// Finalize construction
	idlDef.FinalizeConstruction()

	// Marshal to JSON
	jsonBytes, err := json.MarshalIndent(idlDef, "", "  ")
	if err != nil {
		return "", fmt.Errorf("failed to marshal IDL to JSON: %w", err)
	}

	return string(jsonBytes), nil
}

// setMetadata sets the top-level IDL metadata
func (g *IDLGenerator) setMetadata(idlDef *IDL.IDLDefinition) {
	// Get file name and extension
	fileName := filepath.Base(g.sourcePath)
	ext := filepath.Ext(fileName)
	fileNameNoExt := strings.TrimSuffix(fileName, ext)

	// Set metadata
	idlDef.IDLSource = g.extractedInfo.PackageName
	idlDef.IDLExtension = ext
	idlDef.IDLFilenameWithExtension = fileName
	idlDef.IDLFullPath = g.sourcePath
	idlDef.TargetLanguage = "go"

	// Set MetaFFI guest library name
	idlDef.MetaFFIGuestLib = fmt.Sprintf("%s_MetaFFIGuest", fileNameNoExt)
}

// createModuleDefinition creates a module definition from extracted info
func (g *IDLGenerator) createModuleDefinition() *IDL.ModuleDefinition {
	moduleDef := IDL.NewModuleDefinition(g.extractedInfo.PackageName)

	// Set entity path for module
	moduleDef.SetEntityPath("package", g.extractedInfo.PackageName)
	if g.extractedInfo.ImportPath != "" {
		moduleDef.SetEntityPath("module", g.extractedInfo.ImportPath)
	}

	// Add functions
	for _, funcInfo := range g.extractedInfo.Functions {
		funcDef := g.createFunctionDefinition(funcInfo)
		moduleDef.AddFunction(funcDef)
	}

	// Add structs as classes
	for _, structInfo := range g.extractedInfo.Structs {
		classDef := g.createClassDefinition(structInfo)
		moduleDef.AddClass(classDef)
	}

	// Add interfaces as classes (with only methods, no fields)
	for _, interfaceInfo := range g.extractedInfo.Interfaces {
		classDef := g.createInterfaceClassDefinition(interfaceInfo)
		moduleDef.AddClass(classDef)
	}

	// Add globals as getter/setter function pairs
	for _, globalInfo := range g.extractedInfo.Globals {
		getterDef, setterDef := g.createGlobalAccessors(globalInfo)
		moduleDef.AddFunction(getterDef)
		if setterDef != nil && !globalInfo.IsConst {
			moduleDef.AddFunction(setterDef)
		}
	}

	return moduleDef
}

// createFunctionDefinition creates a function definition
func (g *IDLGenerator) createFunctionDefinition(funcInfo *FunctionInfo) *IDL.FunctionDefinition {
	funcDef := IDL.NewFunctionDefinition(funcInfo.Name)
	funcDef.Comment = funcInfo.Comment

	// Set entity path
	entityPath := g.entityPathGen.CreateFunctionEntityPath(funcInfo.Name)
	funcDef.EntityPath = entityPath

	// Add parameters
	for _, param := range funcInfo.Parameters {
		argDef := g.createArgumentDefinition(param)
		funcDef.AddParameter(argDef)
	}

	// Add return values
	for i, retVal := range funcInfo.ReturnValues {
		argDef := g.createArgumentDefinition(retVal)
		// Generate name if empty
		if argDef.Name == "" {
			argDef.Name = fmt.Sprintf("ret_%d", i)
		}
		funcDef.AddReturnValues(argDef)
	}

	// Set overload index
	funcDef.OverloadIndex = int32(g.getNextOverloadIndex(funcInfo.Name))

	return funcDef
}

// createArgumentDefinition creates an argument definition from parameter info
func (g *IDLGenerator) createArgumentDefinition(param *ParameterInfo) *IDL.ArgDefinition {
	var argDef *IDL.ArgDefinition
	if param.Dimensions > 0 {
		argDef = IDL.NewArgArrayDefinitionWithAlias(param.Name, param.Type, param.Dimensions, param.TypeAlias)
	} else {
		argDef = IDL.NewArgDefinitionWithAlias(param.Name, param.Type, param.TypeAlias)
	}
	argDef.Comment = param.Comment

	return argDef
}

// createClassDefinition creates a class definition from struct info
func (g *IDLGenerator) createClassDefinition(structInfo *StructInfo) *IDL.ClassDefinition {
	classDef := IDL.NewClassDefinition(structInfo.Name)
	classDef.Comment = structInfo.Comment

	// Add constructor if found
	if structInfo.Constructor != nil {
		funcDef := g.createFunctionDefinition(structInfo.Constructor)
		funcDef.EntityPath = g.entityPathGen.CreateConstructorEntityPath(structInfo.Constructor.Name)
		// Constructor returns the struct instance
		if len(funcDef.ReturnValues) == 0 {
			retDef := IDL.NewArgDefinitionWithAlias("ret_0", IDL.HANDLE, structInfo.Name)
			funcDef.AddReturnValues(retDef)
		}
		// Convert to ConstructorDefinition
		constructorDef := IDL.NewConstructorDefinitionFromFunctionDefinition(funcDef)
		classDef.AddConstructor(constructorDef)
	} else {
		// Create default constructor
		defaultConstructor := IDL.NewConstructorDefinition("New" + structInfo.Name)
		defaultConstructor.EntityPath = g.entityPathGen.CreateConstructorEntityPath("New" + structInfo.Name)
		// Update return value type alias
		if len(defaultConstructor.ReturnValues) > 0 {
			defaultConstructor.ReturnValues[0].TypeAlias = structInfo.Name
		}
		classDef.AddConstructor(defaultConstructor)
	}

	// Add methods
	for _, method := range structInfo.Methods {
		funcDef := g.createFunctionDefinition(method)
		funcDef.EntityPath = g.entityPathGen.CreateMethodEntityPath(structInfo.Name, method.Name)
		// Convert to MethodDefinition
		instanceRequired := method.ReceiverPtr
		methodDef := IDL.NewMethodDefinitionWithFunction(classDef, funcDef, instanceRequired)
		classDef.AddMethod(methodDef)
	}

	// Add fields as getter/setter pairs
	for _, field := range structInfo.Fields {
		// Skip embedded fields for now
		if field.IsEmbedded {
			continue
		}

		fieldDef := g.createFieldDefinition(field, structInfo.Name, classDef)
		classDef.AddField(fieldDef)
	}

	return classDef
}

// createInterfaceClassDefinition creates a class definition from interface info
func (g *IDLGenerator) createInterfaceClassDefinition(interfaceInfo *InterfaceInfo) *IDL.ClassDefinition {
	classDef := IDL.NewClassDefinition(interfaceInfo.Name)
	classDef.Comment = interfaceInfo.Comment

	// Interfaces don't have constructors or fields, only methods
	for _, method := range interfaceInfo.Methods {
		funcDef := g.createFunctionDefinition(method)
		funcDef.EntityPath = g.entityPathGen.CreateMethodEntityPath(interfaceInfo.Name, method.Name)
		// Convert to MethodDefinition (interface methods don't require instance)
		methodDef := IDL.NewMethodDefinitionWithFunction(classDef, funcDef, false)
		classDef.AddMethod(methodDef)
	}

	return classDef
}

// createFieldDefinition creates a field definition with getter/setter
func (g *IDLGenerator) createFieldDefinition(field *FieldInfo, structName string, parentClass *IDL.ClassDefinition) *IDL.FieldDefinition {
	var fieldDef *IDL.FieldDefinition

	getterName := "Get" + field.Name
	setterName := "Set" + field.Name

	// Create field definition with getters and setters
	if field.Dimensions > 0 {
		fieldDef = IDL.NewFieldArrayDefinitionWithAlias(
			parentClass,
			field.Name,
			field.Type,
			field.Dimensions,
			field.TypeAlias,
			getterName,
			setterName,
			true, // instance required for struct fields
		)
	} else {
		fieldDef = IDL.NewFieldDefinitionWithAlias(
			parentClass,
			field.Name,
			field.Type,
			field.TypeAlias,
			getterName,
			setterName,
			true, // instance required for struct fields
		)
	}

	fieldDef.Comment = field.Comment

	// Set entity paths for getter and setter
	if fieldDef.Getter != nil {
		fieldDef.Getter.EntityPath = g.entityPathGen.CreateFieldGetterEntityPath(structName, field.Name)
	}
	if fieldDef.Setter != nil {
		fieldDef.Setter.EntityPath = g.entityPathGen.CreateFieldSetterEntityPath(structName, field.Name)
	}

	return fieldDef
}

// createGlobalAccessors creates getter and setter functions for a global variable
func (g *IDLGenerator) createGlobalAccessors(globalInfo *GlobalInfo) (*IDL.FunctionDefinition, *IDL.FunctionDefinition) {
	// Create getter
	getter := IDL.NewFunctionDefinition("Get" + globalInfo.Name)
	getter.Comment = globalInfo.Comment
	getter.EntityPath = g.entityPathGen.CreateGlobalGetterEntityPath(globalInfo.Name)

	var retDef *IDL.ArgDefinition
	if globalInfo.Dimensions > 0 {
		retDef = IDL.NewArgArrayDefinitionWithAlias("ret_0", globalInfo.Type, globalInfo.Dimensions, globalInfo.TypeAlias)
	} else {
		retDef = IDL.NewArgDefinitionWithAlias("ret_0", globalInfo.Type, globalInfo.TypeAlias)
	}
	getter.AddReturnValues(retDef)

	// Create setter (not for constants)
	var setter *IDL.FunctionDefinition
	if !globalInfo.IsConst {
		setter = IDL.NewFunctionDefinition("Set" + globalInfo.Name)
		setter.Comment = "Set " + globalInfo.Name
		setter.EntityPath = g.entityPathGen.CreateGlobalSetterEntityPath(globalInfo.Name)

		var paramDef *IDL.ArgDefinition
		if globalInfo.Dimensions > 0 {
			paramDef = IDL.NewArgArrayDefinitionWithAlias("value", globalInfo.Type, globalInfo.Dimensions, globalInfo.TypeAlias)
		} else {
			paramDef = IDL.NewArgDefinitionWithAlias("value", globalInfo.Type, globalInfo.TypeAlias)
		}
		setter.AddParameter(paramDef)
	}

	return getter, setter
}

// getNextOverloadIndex returns and increments the overload index for a function name
func (g *IDLGenerator) getNextOverloadIndex(funcName string) int {
	index := g.overloadIndexes[funcName]
	g.overloadIndexes[funcName] = index + 1
	return index
}

