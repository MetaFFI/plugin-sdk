package idl_compiler

import (
	"fmt"
	"os"
	"path/filepath"
)

// Compiler is the main orchestrator for Go IDL compilation
type Compiler struct {
	sourcePath string
	sourceType SourceType
}

// NewCompiler creates a new Compiler instance
func NewCompiler() *Compiler {
	return &Compiler{
		sourceType: SourceTypeAuto,
	}
}

// Compile compiles Go source to MetaFFI IDL JSON
// sourcePath can be a .go file, directory, or module path
func (c *Compiler) Compile(sourcePath string) (string, error) {
	c.sourcePath = sourcePath

	// Validate source path
	if err := c.validateSourcePath(); err != nil {
		return "", err
	}

	// Extract information from Go source
	extractor := NewExtractor(c.sourcePath, c.sourceType)
	extractedInfo, err := extractor.Extract()
	if err != nil {
		return "", fmt.Errorf("extraction failed: %w", err)
	}

	// Post-processing: attach methods to structs
	extractor.attachMethodsToStructs(extractedInfo)

	// Post-processing: attach constructors to structs
	extractor.attachConstructorsToStructs(extractedInfo)

	// Generate IDL JSON
	generator := NewIDLGenerator(extractedInfo, c.sourcePath)
	jsonOutput, err := generator.Generate()
	if err != nil {
		return "", fmt.Errorf("IDL generation failed: %w", err)
	}

	return jsonOutput, nil
}

// CompileToFile compiles and writes output to a file
func (c *Compiler) CompileToFile(sourcePath string, outputPath string) error {
	// Compile to JSON string
	jsonOutput, err := c.Compile(sourcePath)
	if err != nil {
		return err
	}

	// Write to file
	err = os.WriteFile(outputPath, []byte(jsonOutput), 0644)
	if err != nil {
		return fmt.Errorf("failed to write output file: %w", err)
	}

	return nil
}

// validateSourcePath validates and normalizes the source path
func (c *Compiler) validateSourcePath() error {
	// Check if path exists
	info, err := os.Stat(c.sourcePath)
	if err != nil {
		if os.IsNotExist(err) {
			return fmt.Errorf("source path does not exist: %s", c.sourcePath)
		}
		return fmt.Errorf("failed to stat source path: %w", err)
	}

	// Determine source type
	if c.sourceType == SourceTypeAuto {
		if info.IsDir() {
			c.sourceType = SourceTypeDir
		} else {
			c.sourceType = SourceTypeFile

			// Validate file extension
			ext := filepath.Ext(c.sourcePath)
			if ext != ".go" {
				return fmt.Errorf("source file must have .go extension, got: %s", ext)
			}
		}
	}

	return nil
}

// SetSourceType explicitly sets the source type
func (c *Compiler) SetSourceType(sourceType SourceType) {
	c.sourceType = sourceType
}
