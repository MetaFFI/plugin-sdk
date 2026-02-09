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
	logIDL("Generate IDL JSON: source=%s", g.sourcePath)
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

	// Add globals as proper GlobalDefinitions (guest template reads $m.Globals, not $m.Functions)
	for _, globalInfo := range g.extractedInfo.Globals {
		globalDef := g.createGlobalDefinition(globalInfo)
		moduleDef.AddGlobal(globalDef)
	}

	return moduleDef
}

// createFunctionDefinition creates a function definition
func (g *IDLGenerator) createFunctionDefinition(funcInfo *FunctionInfo) *IDL.FunctionDefinition {
	funcDef := IDL.NewFunctionDefinition(funcInfo.Name)
	funcDef.Comment = funcInfo.Comment

	// Set entity path
	entityPath := g.entityPathGen.CreateFunctionEntityPath(funcInfo.Name)
	g.enrichEntityPath(entityPath)
	funcDef.EntityPath = entityPath

	// Add parameters
	for _, param := range funcInfo.Parameters {
		argDef := g.createArgumentDefinition(param)
		funcDef.AddParameter(argDef)
	}

	// Add return values.
	// Go convention: if the last return value is of type `error`, it is
	// communicated through the out_err C parameter (not via CDT retval slot).
	// We strip it from the IDL return_values and set a tag so the template
	// can still generate the correct Go call capturing the error variable.
	hasErrorReturn := false
	for i, retVal := range funcInfo.ReturnValues {
		// Detect the `error` return value: TypeAlias == "error"
		if retVal.TypeAlias == "error" {
			hasErrorReturn = true
			continue // skip adding it to the IDL return values
		}
		argDef := g.createArgumentDefinition(retVal)
		// Generate name if empty
		if argDef.Name == "" {
			argDef.Name = fmt.Sprintf("ret_%d", i)
		}
		funcDef.AddReturnValues(argDef)
	}
	if hasErrorReturn {
		funcDef.SetTag("has_error_return", "true")
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
		constructorEP := g.entityPathGen.CreateConstructorEntityPath(structInfo.Constructor.Name)
		g.enrichEntityPath(constructorEP)
		funcDef.EntityPath = constructorEP
		// Constructor returns the struct instance
		if len(funcDef.ReturnValues) == 0 {
			retDef := IDL.NewArgDefinitionWithAlias("ret_0", IDL.HANDLE, structInfo.Name)
			funcDef.AddReturnValues(retDef)
		}
		// Convert to ConstructorDefinition
		constructorDef := IDL.NewConstructorDefinitionFromFunctionDefinition(funcDef)
		classDef.AddConstructor(constructorDef)
	}
	// No default constructor: if the struct has no explicit NewXxx function,
	// don't generate one because calling a non-existent function would fail.

	// Add methods
	for _, method := range structInfo.Methods {
		funcDef := g.createFunctionDefinition(method)
		methodEP := g.entityPathGen.CreateMethodEntityPath(structInfo.Name, method.Name)
		g.enrichEntityPath(methodEP)
		funcDef.EntityPath = methodEP
		// receiver_pointer tag used by guest template for *T vs T
		if method.ReceiverPtr {
			funcDef.SetTag("receiver_pointer", "true")
		} else {
			funcDef.SetTag("receiver_pointer", "false")
		}
		// Struct methods always need instance (prepends this_instance so template can index Parameters 0)
		methodDef := IDL.NewMethodDefinitionWithFunction(classDef, funcDef, true)
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
	classDef.SetTag("interface", "true") // Mark as interface so guest template skips EmptyStruct

	// Interfaces don't have constructors or fields, only methods
	for _, method := range interfaceInfo.Methods {
		funcDef := g.createFunctionDefinition(method)
		ifMethodEP := g.entityPathGen.CreateMethodEntityPath(interfaceInfo.Name, method.Name)
		g.enrichEntityPath(ifMethodEP)
		funcDef.EntityPath = ifMethodEP
		if method.ReceiverPtr {
			funcDef.SetTag("receiver_pointer", "true")
		} else {
			funcDef.SetTag("receiver_pointer", "false")
		}
		// Prepends this_instance so guest template can index Parameters 0
		methodDef := IDL.NewMethodDefinitionWithFunction(classDef, funcDef, true)
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
		fgEP := g.entityPathGen.CreateFieldGetterEntityPath(structName, field.Name)
		g.enrichEntityPath(fgEP)
		fieldDef.Getter.EntityPath = fgEP
	}
	if fieldDef.Setter != nil {
		fsEP := g.entityPathGen.CreateFieldSetterEntityPath(structName, field.Name)
		g.enrichEntityPath(fsEP)
		fieldDef.Setter.EntityPath = fsEP
	}

	return fieldDef
}

// createGlobalDefinition creates a GlobalDefinition with getter (and setter if not const).
func (g *IDLGenerator) createGlobalDefinition(globalInfo *GlobalInfo) *IDL.GlobalDefinition {
	getterName := "Get" + globalInfo.Name
	setterName := ""
	if !globalInfo.IsConst {
		setterName = "Set" + globalInfo.Name
	}

	var globalDef *IDL.GlobalDefinition
	if globalInfo.Dimensions > 0 {
		globalDef = IDL.NewGlobalDefinitionWithAlias(globalInfo.Name, globalInfo.Type, globalInfo.TypeAlias, getterName, setterName)
		globalDef.Dimensions = globalInfo.Dimensions
	} else {
		globalDef = IDL.NewGlobalDefinitionWithAlias(globalInfo.Name, globalInfo.Type, globalInfo.TypeAlias, getterName, setterName)
	}
	globalDef.Comment = globalInfo.Comment

	// Set entity paths on getter and setter
	if globalDef.Getter != nil {
		getterEP := g.entityPathGen.CreateGlobalGetterEntityPath(globalInfo.Name)
		g.enrichEntityPath(getterEP)
		globalDef.Getter.EntityPath = getterEP
	}
	if globalDef.Setter != nil {
		setterEP := g.entityPathGen.CreateGlobalSetterEntityPath(globalInfo.Name)
		g.enrichEntityPath(setterEP)
		globalDef.Setter.EntityPath = setterEP
	}

	return globalDef
}

// enrichEntityPath adds "module" (source directory) and "package" (Go import path)
// to the given entity_path so the guest compiler can set up import and replace directives.
func (g *IDLGenerator) enrichEntityPath(entityPath map[string]string) {
	if g.extractedInfo.ImportPath != "" {
		entityPath["package"] = g.extractedInfo.ImportPath
	}
	// Use the absolute source path as the module location so the guest compiler
	// can detect it as a local directory and add a replace directive.
	entityPath["module"] = g.sourcePath
}

// getNextOverloadIndex returns and increments the overload index for a function name
func (g *IDLGenerator) getNextOverloadIndex(funcName string) int {
	index := g.overloadIndexes[funcName]
	g.overloadIndexes[funcName] = index + 1
	return index
}

