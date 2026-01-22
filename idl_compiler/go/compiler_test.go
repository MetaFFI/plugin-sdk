package idl_compiler

import (
	"encoding/json"
	"os"
	"path/filepath"
	"testing"

	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

// Test data file paths
var (
	testdataDir  = "./testdata"
	simpleGoFile = filepath.Join(testdataDir, "simple.go")
)

// TestTypeMapper_PrimitiveTypes tests mapping of primitive Go types
func TestTypeMapper_PrimitiveTypes(t *testing.T) {
	tm := NewTypeMapper()

	tests := []struct {
		typeName     string
		expected     IDL.MetaFFIType
		expectedName string
	}{
		{"bool", IDL.BOOL, "bool"},
		{"int8", IDL.INT8, "int8"},
		{"int16", IDL.INT16, "int16"},
		{"int32", IDL.INT32, "int32"},
		{"int64", IDL.INT64, "int64"},
		{"int", IDL.INT64, "int"},
		{"uint8", IDL.UINT8, "uint8"},
		{"uint16", IDL.UINT16, "uint16"},
		{"uint32", IDL.UINT32, "uint32"},
		{"uint64", IDL.UINT64, "uint64"},
		{"uint", IDL.UINT64, "uint"},
		{"byte", IDL.UINT8, "byte"},
		{"rune", IDL.INT32, "rune"},
		{"float32", IDL.FLOAT32, "float32"},
		{"float64", IDL.FLOAT64, "float64"},
		{"string", IDL.STRING8, "string"},
	}

	for _, tt := range tests {
		t.Run(tt.typeName, func(t *testing.T) {
			result := tm.mapPrimitiveType(tt.typeName)
			if result != tt.expected {
				t.Errorf("mapPrimitiveType(%s) = %v, want %v", tt.typeName, result, tt.expected)
			}
		})
	}
}

// TestTypeMapper_UnknownType tests fallback to HANDLE for unknown types
func TestTypeMapper_UnknownType(t *testing.T) {
	tm := NewTypeMapper()
	result := tm.mapPrimitiveType("UnknownType")
	if result != IDL.HANDLE {
		t.Errorf("mapPrimitiveType(UnknownType) = %v, want %v", result, IDL.HANDLE)
	}
}

// TestEntityPath_Function tests function entity path generation
func TestEntityPath_Function(t *testing.T) {
	epg := NewEntityPathGenerator()

	entityPath := epg.CreateFunctionEntityPath("MyFunction")

	if entityPath["callable"] != "MyFunction" {
		t.Errorf("Expected callable=MyFunction, got %v", entityPath["callable"])
	}

	if len(entityPath) != 1 {
		t.Errorf("Expected 1 key in entity_path, got %d", len(entityPath))
	}
}

// TestEntityPath_Method tests method entity path generation
func TestEntityPath_Method(t *testing.T) {
	epg := NewEntityPathGenerator()

	entityPath := epg.CreateMethodEntityPath("MyStruct", "MyMethod")

	expected := "MyStruct.MyMethod"
	if entityPath["callable"] != expected {
		t.Errorf("Expected callable=%s, got %v", expected, entityPath["callable"])
	}
}

// TestEntityPath_GlobalGetter tests global getter entity path
func TestEntityPath_GlobalGetter(t *testing.T) {
	epg := NewEntityPathGenerator()

	entityPath := epg.CreateGlobalGetterEntityPath("MyGlobal")

	if entityPath["global"] != "MyGlobal" {
		t.Errorf("Expected global=MyGlobal, got %v", entityPath["global"])
	}

	if entityPath["getter"] != "true" {
		t.Errorf("Expected getter=true, got %v", entityPath["getter"])
	}
}

// TestEntityPath_GlobalSetter tests global setter entity path
func TestEntityPath_GlobalSetter(t *testing.T) {
	epg := NewEntityPathGenerator()

	entityPath := epg.CreateGlobalSetterEntityPath("MyGlobal")

	if entityPath["global"] != "MyGlobal" {
		t.Errorf("Expected global=MyGlobal, got %v", entityPath["global"])
	}

	if entityPath["setter"] != "true" {
		t.Errorf("Expected setter=true, got %v", entityPath["setter"])
	}
}

// TestEntityPath_FieldGetter tests field getter entity path
func TestEntityPath_FieldGetter(t *testing.T) {
	epg := NewEntityPathGenerator()

	entityPath := epg.CreateFieldGetterEntityPath("MyStruct", "MyField")

	expected := "MyStruct.MyField"
	if entityPath["field"] != expected {
		t.Errorf("Expected field=%s, got %v", expected, entityPath["field"])
	}

	if entityPath["getter"] != "true" {
		t.Errorf("Expected getter=true, got %v", entityPath["getter"])
	}
}

// TestEntityPath_FieldSetter tests field setter entity path
func TestEntityPath_FieldSetter(t *testing.T) {
	epg := NewEntityPathGenerator()

	entityPath := epg.CreateFieldSetterEntityPath("MyStruct", "MyField")

	expected := "MyStruct.MyField"
	if entityPath["field"] != expected {
		t.Errorf("Expected field=%s, got %v", expected, entityPath["field"])
	}

	if entityPath["setter"] != "true" {
		t.Errorf("Expected setter=true, got %v", entityPath["setter"])
	}
}

// TestExtractor_SimpleFile tests extraction from a simple Go file
func TestExtractor_SimpleFile(t *testing.T) {
	if _, err := os.Stat(simpleGoFile); os.IsNotExist(err) {
		t.Skip("Test data file not found:", simpleGoFile)
	}

	extractor := NewExtractor(simpleGoFile, SourceTypeFile)
	info, err := extractor.Extract()

	if err != nil {
		t.Fatalf("Extract() failed: %v", err)
	}

	if info.PackageName != "testdata" {
		t.Errorf("Expected package name 'testdata', got '%s'", info.PackageName)
	}

	// Check that we extracted some functions
	if len(info.Functions) == 0 {
		t.Error("Expected at least one function to be extracted")
	}

	// Check for specific functions
	foundAdd := false
	for _, fn := range info.Functions {
		if fn.Name == "Add" {
			foundAdd = true
			if len(fn.Parameters) != 2 {
				t.Errorf("Add function should have 2 parameters, got %d", len(fn.Parameters))
			}
			if len(fn.ReturnValues) != 1 {
				t.Errorf("Add function should have 1 return value, got %d", len(fn.ReturnValues))
			}
		}
	}

	if !foundAdd {
		t.Error("Expected to find 'Add' function")
	}
}

// TestExtractor_GlobalVariables tests extraction of global variables
func TestExtractor_GlobalVariables(t *testing.T) {
	if _, err := os.Stat(simpleGoFile); os.IsNotExist(err) {
		t.Skip("Test data file not found:", simpleGoFile)
	}

	extractor := NewExtractor(simpleGoFile, SourceTypeFile)
	info, err := extractor.Extract()

	if err != nil {
		t.Fatalf("Extract() failed: %v", err)
	}

	if len(info.Globals) == 0 {
		t.Error("Expected at least one global variable to be extracted")
	}

	// Check for GlobalInt
	foundGlobalInt := false
	foundGlobalConst := false
	for _, global := range info.Globals {
		if global.Name == "GlobalInt" {
			foundGlobalInt = true
			if global.IsConst {
				t.Error("GlobalInt should not be marked as const")
			}
		}
		if global.Name == "GlobalConst" {
			foundGlobalConst = true
			if !global.IsConst {
				t.Error("GlobalConst should be marked as const")
			}
		}
	}

	if !foundGlobalInt {
		t.Error("Expected to find 'GlobalInt' global variable")
	}
	if !foundGlobalConst {
		t.Error("Expected to find 'GlobalConst' constant")
	}
}

// TestExtractor_Structs tests extraction of structs
func TestExtractor_Structs(t *testing.T) {
	if _, err := os.Stat(simpleGoFile); os.IsNotExist(err) {
		t.Skip("Test data file not found:", simpleGoFile)
	}

	extractor := NewExtractor(simpleGoFile, SourceTypeFile)
	info, err := extractor.Extract()

	if err != nil {
		t.Fatalf("Extract() failed: %v", err)
	}

	if len(info.Structs) == 0 {
		t.Error("Expected at least one struct to be extracted")
	}

	// Check for SimpleStruct
	foundSimpleStruct := false
	for _, structInfo := range info.Structs {
		if structInfo.Name == "SimpleStruct" {
			foundSimpleStruct = true

			if len(structInfo.Fields) < 2 {
				t.Errorf("SimpleStruct should have at least 2 fields, got %d", len(structInfo.Fields))
			}

			// Check for Name field
			foundNameField := false
			for _, field := range structInfo.Fields {
				if field.Name == "Name" {
					foundNameField = true
					if field.Type != IDL.STRING8 {
						t.Errorf("Name field should be STRING8, got %v", field.Type)
					}
				}
			}

			if !foundNameField {
				t.Error("Expected to find 'Name' field in SimpleStruct")
			}
		}
	}

	if !foundSimpleStruct {
		t.Error("Expected to find 'SimpleStruct' struct")
	}
}

// TestExtractor_Methods tests extraction of struct methods
func TestExtractor_Methods(t *testing.T) {
	if _, err := os.Stat(simpleGoFile); os.IsNotExist(err) {
		t.Skip("Test data file not found:", simpleGoFile)
	}

	extractor := NewExtractor(simpleGoFile, SourceTypeFile)
	info, err := extractor.Extract()

	if err != nil {
		t.Fatalf("Extract() failed: %v", err)
	}

	// Attach methods to structs
	extractor.attachMethodsToStructs(info)

	// Find SimpleStruct and check its methods
	for _, structInfo := range info.Structs {
		if structInfo.Name == "SimpleStruct" {
			if len(structInfo.Methods) == 0 {
				t.Error("SimpleStruct should have at least one method")
			}

			foundMethod := false
			for _, method := range structInfo.Methods {
				if method.Name == "Method" {
					foundMethod = true
					if method.ReceiverType != "SimpleStruct" {
						t.Errorf("Method receiver type should be SimpleStruct, got %s", method.ReceiverType)
					}
					if !method.ReceiverPtr {
						t.Error("Method should have pointer receiver")
					}
				}
			}

			if !foundMethod {
				t.Error("Expected to find 'Method' method in SimpleStruct")
			}
		}
	}
}

// TestExtractor_Interfaces tests extraction of interfaces
func TestExtractor_Interfaces(t *testing.T) {
	if _, err := os.Stat(simpleGoFile); os.IsNotExist(err) {
		t.Skip("Test data file not found:", simpleGoFile)
	}

	extractor := NewExtractor(simpleGoFile, SourceTypeFile)
	info, err := extractor.Extract()

	if err != nil {
		t.Fatalf("Extract() failed: %v", err)
	}

	if len(info.Interfaces) == 0 {
		t.Error("Expected at least one interface to be extracted")
	}

	// Check for SimpleInterface
	foundSimpleInterface := false
	for _, iface := range info.Interfaces {
		if iface.Name == "SimpleInterface" {
			foundSimpleInterface = true

			if len(iface.Methods) < 2 {
				t.Errorf("SimpleInterface should have at least 2 methods, got %d", len(iface.Methods))
			}
		}
	}

	if !foundSimpleInterface {
		t.Error("Expected to find 'SimpleInterface' interface")
	}
}

// TestCompiler_EndToEnd tests the complete compilation flow
func TestCompiler_EndToEnd(t *testing.T) {
	if _, err := os.Stat(simpleGoFile); os.IsNotExist(err) {
		t.Skip("Test data file not found:", simpleGoFile)
	}

	compiler := NewCompiler()
	jsonOutput, err := compiler.Compile(simpleGoFile)

	if err != nil {
		t.Fatalf("Compile() failed: %v", err)
	}

	if jsonOutput == "" {
		t.Error("Expected non-empty JSON output")
	}

	// Parse JSON to validate structure
	var idlDef IDL.IDLDefinition
	err = json.Unmarshal([]byte(jsonOutput), &idlDef)
	if err != nil {
		t.Fatalf("Failed to parse JSON output: %v", err)
	}

	// Validate top-level fields
	if idlDef.IDLSource == "" {
		t.Error("IDLSource should not be empty")
	}

	if idlDef.TargetLanguage != "go" {
		t.Errorf("Expected target_language='go', got '%s'", idlDef.TargetLanguage)
	}

	if len(idlDef.Modules) == 0 {
		t.Error("Expected at least one module")
	}

	// Check module contents
	module := idlDef.Modules[0]
	if len(module.Functions) == 0 {
		t.Error("Expected at least one function in module")
	}
}

// TestCompiler_NonExistentFile tests error handling for missing file
func TestCompiler_NonExistentFile(t *testing.T) {
	compiler := NewCompiler()
	_, err := compiler.Compile("/nonexistent/file.go")

	if err == nil {
		t.Error("Expected error for non-existent file")
	}
}

// TestCompiler_InvalidExtension tests error handling for wrong file extension
func TestCompiler_InvalidExtension(t *testing.T) {
	// Create a temp file with wrong extension
	tmpFile, err := os.CreateTemp("", "test*.txt")
	if err != nil {
		t.Fatal(err)
	}
	defer os.Remove(tmpFile.Name())
	tmpFile.Close()

	compiler := NewCompiler()
	_, err = compiler.Compile(tmpFile.Name())

	if err == nil {
		t.Error("Expected error for invalid file extension")
	}
}

// TestIsExported tests the IsExported function
func TestIsExported(t *testing.T) {
	tests := []struct {
		name     string
		expected bool
	}{
		{"PublicName", true},
		{"privateName", false},
		{"", false},
		{"X", true},
		{"x", false},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := IsExported(tt.name)
			if result != tt.expected {
				t.Errorf("IsExported(%s) = %v, want %v", tt.name, result, tt.expected)
			}
		})
	}
}

// Benchmark tests
func BenchmarkTypeMapper_MapPrimitiveType(b *testing.B) {
	tm := NewTypeMapper()
	for i := 0; i < b.N; i++ {
		tm.mapPrimitiveType("int32")
	}
}

func BenchmarkEntityPath_CreateFunction(b *testing.B) {
	epg := NewEntityPathGenerator()
	for i := 0; i < b.N; i++ {
		epg.CreateFunctionEntityPath("MyFunction")
	}
}

func BenchmarkCompiler_Compile(b *testing.B) {
	if _, err := os.Stat(simpleGoFile); os.IsNotExist(err) {
		b.Skip("Test data file not found:", simpleGoFile)
	}

	compiler := NewCompiler()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		_, err := compiler.Compile(simpleGoFile)
		if err != nil {
			b.Fatal(err)
		}
	}
}

