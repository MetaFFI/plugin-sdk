package main

import (
	"fmt"
	"os"

	idl_compiler "github.com/MetaFFI/sdk/idl_compiler/go"
)

const usage = `MetaFFI Go IDL Compiler
Extracts interface definitions from Go source code and generates MetaFFI IDL JSON.

Usage:
  go_idl_compiler <source_path> [output_path]

Arguments:
  source_path   Path to .go file or directory containing Go source files
  output_path   (Optional) Path to output JSON file. If omitted, outputs to STDOUT

Examples:
  go_idl_compiler mypackage.go
  go_idl_compiler mypackage.go mypackage.json
  go_idl_compiler ./mypackage

Output:
  - JSON is written to STDOUT (if no output_path)
  - JSON is written to file (if output_path provided)
  - Errors are written to STDERR
  - Exit code 0 = success, non-zero = error
`

func main() {
	// Check arguments
	if len(os.Args) < 2 {
		fmt.Fprint(os.Stderr, usage)
		os.Exit(1)
	}

	sourcePath := os.Args[1]

	// Check for help flag
	if sourcePath == "-h" || sourcePath == "--help" || sourcePath == "help" {
		fmt.Print(usage)
		os.Exit(0)
	}

	// Create compiler
	compiler := idl_compiler.NewCompiler()

	// Check if output path is provided
	if len(os.Args) >= 3 {
		// Write to file
		outputPath := os.Args[2]
		err := compiler.CompileToFile(sourcePath, outputPath)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error: %v\n", err)
			os.Exit(1)
		}
		// Success message to STDERR (so STDOUT is clean)
		fmt.Fprintf(os.Stderr, "IDL generated successfully: %s\n", outputPath)
	} else {
		// Write to STDOUT
		jsonOutput, err := compiler.Compile(sourcePath)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error: %v\n", err)
			os.Exit(1)
		}
		// Output JSON to STDOUT
		fmt.Println(jsonOutput)
	}

	os.Exit(0)
}
