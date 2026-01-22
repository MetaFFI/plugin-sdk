package idl_compiler

import (
	"fmt"
	"go/ast"
	"go/parser"
	"go/token"
	"os"
	"path/filepath"
	"strings"

	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

// SourceType represents the type of source input
type SourceType int

const (
	SourceTypeFile SourceType = iota // Single .go file
	SourceTypeDir                     // Directory containing .go files
	SourceTypeAuto                    // Auto-detect
)

// ExtractedInfo holds all extracted information from Go source
type ExtractedInfo struct {
	PackageName      string
	ImportPath       string
	Functions        []*FunctionInfo
	Structs          []*StructInfo
	Interfaces       []*InterfaceInfo
	Globals          []*GlobalInfo
	FileSet          *token.FileSet
	ParsedFiles      []*ast.File
}

// Extractor extracts interface definitions from Go source code
type Extractor struct {
	typeMapper      *TypeMapper
	entityPathGen   *EntityPathGenerator
	sourcePath      string
	sourceType      SourceType
}

// NewExtractor creates a new Extractor instance
func NewExtractor(sourcePath string, sourceType SourceType) *Extractor {
	return &Extractor{
		typeMapper:    NewTypeMapper(),
		entityPathGen: NewEntityPathGenerator(),
		sourcePath:    sourcePath,
		sourceType:    sourceType,
	}
}

// Extract performs the extraction from the configured source
func (e *Extractor) Extract() (*ExtractedInfo, error) {
	// Determine source type if auto
	sourceType := e.sourceType
	if sourceType == SourceTypeAuto {
		sourceType = e.detectSourceType()
	}

	// Parse based on source type
	switch sourceType {
	case SourceTypeFile:
		return e.extractFromFile(e.sourcePath)
	case SourceTypeDir:
		return e.extractFromDir(e.sourcePath)
	default:
		return nil, fmt.Errorf("unknown source type: %v", sourceType)
	}
}

// detectSourceType auto-detects whether source is file or directory
func (e *Extractor) detectSourceType() SourceType {
	info, err := os.Stat(e.sourcePath)
	if err != nil {
		// Default to file
		return SourceTypeFile
	}

	if info.IsDir() {
		return SourceTypeDir
	}
	return SourceTypeFile
}

// extractFromFile extracts from a single Go file
func (e *Extractor) extractFromFile(filePath string) (*ExtractedInfo, error) {
	fset := token.NewFileSet()

	// Parse the file
	file, err := parser.ParseFile(fset, filePath, nil, parser.ParseComments)
	if err != nil {
		return nil, fmt.Errorf("failed to parse file %s: %w", filePath, err)
	}

	// Create extracted info
	info := &ExtractedInfo{
		PackageName: file.Name.Name,
		FileSet:     fset,
		ParsedFiles: []*ast.File{file},
	}

	// Extract entities from the single file
	e.extractEntitiesFromFile(file, info)

	return info, nil
}

// extractFromDir extracts from all .go files in a directory
func (e *Extractor) extractFromDir(dirPath string) (*ExtractedInfo, error) {
	fset := token.NewFileSet()

	// Parse all Go files in directory
	pkgs, err := parser.ParseDir(fset, dirPath, func(fi os.FileInfo) bool {
		// Filter: include only .go files, exclude _test.go files
		name := fi.Name()
		return !fi.IsDir() &&
			filepath.Ext(name) == ".go" &&
			!strings.HasSuffix(name, "_test.go")
	}, parser.ParseComments)

	if err != nil {
		return nil, fmt.Errorf("failed to parse directory %s: %w", dirPath, err)
	}

	// We expect exactly one package per directory (standard Go convention)
	if len(pkgs) == 0 {
		return nil, fmt.Errorf("no Go packages found in directory %s", dirPath)
	}

	if len(pkgs) > 1 {
		return nil, fmt.Errorf("multiple packages found in directory %s, expected one", dirPath)
	}

	// Get the single package
	var pkg *ast.Package
	var pkgName string
	for name, p := range pkgs {
		pkgName = name
		pkg = p
		break
	}

	// Create extracted info
	info := &ExtractedInfo{
		PackageName: pkgName,
		FileSet:     fset,
	}

	// Collect all files
	for _, file := range pkg.Files {
		info.ParsedFiles = append(info.ParsedFiles, file)
	}

	// Extract entities from all files
	for _, file := range pkg.Files {
		e.extractEntitiesFromFile(file, info)
	}

	return info, nil
}

// extractEntitiesFromFile extracts all entities from a single AST file
func (e *Extractor) extractEntitiesFromFile(file *ast.File, info *ExtractedInfo) {
	// Walk through all declarations in the file
	for _, decl := range file.Decls {
		switch d := decl.(type) {
		case *ast.FuncDecl:
			// Function or method declaration
			e.extractFunction(d, info)

		case *ast.GenDecl:
			// General declaration (import, const, type, var)
			e.extractGenDecl(d, info)
		}
	}
}

// extractFunction extracts a function or method declaration
func (e *Extractor) extractFunction(funcDecl *ast.FuncDecl, info *ExtractedInfo) {
	// Only extract exported functions/methods
	if !ast.IsExported(funcDecl.Name.Name) {
		return
	}

	// Check if it's a method (has receiver) or function
	if funcDecl.Recv != nil && len(funcDecl.Recv.List) > 0 {
		// It's a method - will be extracted by struct extractor
		return
	}

	// It's a package-level function
	funcInfo := e.parseFunctionSignature(funcDecl)
	if funcInfo != nil {
		info.Functions = append(info.Functions, funcInfo)
	}
}

// extractGenDecl extracts from a general declaration (const, type, var)
func (e *Extractor) extractGenDecl(genDecl *ast.GenDecl, info *ExtractedInfo) {
	for _, spec := range genDecl.Specs {
		switch s := spec.(type) {
		case *ast.TypeSpec:
			// Type declaration (struct, interface, alias, etc.)
			if !ast.IsExported(s.Name.Name) {
				continue
			}
			e.extractTypeSpec(s, genDecl.Doc, info)

		case *ast.ValueSpec:
			// Variable or constant declaration
			e.extractValueSpec(s, genDecl.Tok, genDecl.Doc, info)
		}
	}
}

// extractTypeSpec extracts a type declaration
func (e *Extractor) extractTypeSpec(typeSpec *ast.TypeSpec, doc *ast.CommentGroup, info *ExtractedInfo) {
	switch t := typeSpec.Type.(type) {
	case *ast.StructType:
		// Struct type
		structInfo := e.parseStruct(typeSpec.Name.Name, t, doc)
		if structInfo != nil {
			info.Structs = append(info.Structs, structInfo)
		}

	case *ast.InterfaceType:
		// Interface type
		interfaceInfo := e.parseInterface(typeSpec.Name.Name, t, doc)
		if interfaceInfo != nil {
			info.Interfaces = append(info.Interfaces, interfaceInfo)
		}

	// Other types (aliases, function types, etc.) are not extracted as separate entities
	}
}

// extractValueSpec extracts a variable or constant declaration
func (e *Extractor) extractValueSpec(valueSpec *ast.ValueSpec, tok token.Token, doc *ast.CommentGroup, info *ExtractedInfo) {
	// Only extract exported names
	for _, name := range valueSpec.Names {
		if !ast.IsExported(name.Name) {
			continue
		}

		// Create global info
		globalInfo := &GlobalInfo{
			Name:    name.Name,
			IsConst: tok == token.CONST,
			Comment: extractComment(doc),
		}

		// Try to get type from type expression
		if valueSpec.Type != nil {
			metaffiType, dimensions, typeAlias := e.typeMapper.MapType(valueSpec.Type, "")
			globalInfo.Type = metaffiType
			globalInfo.Dimensions = dimensions
			globalInfo.TypeAlias = typeAlias
		} else {
			// No explicit type - infer from value or default to any
			globalInfo.Type = IDL.ANY
			globalInfo.TypeAlias = "any"
		}

		info.Globals = append(info.Globals, globalInfo)
	}
}

// parseFunctionSignature, parseStruct, and parseInterface are implemented in their respective files

// extractComment extracts comment text from a comment group
func extractComment(commentGroup *ast.CommentGroup) string {
	if commentGroup == nil {
		return ""
	}
	return strings.TrimSpace(commentGroup.Text())
}

// IsExported checks if a name is exported (public)
func IsExported(name string) bool {
	return ast.IsExported(name)
}
