package main

import (
	"fmt"
	"os"

	"github.com/MetaFFI/sdk/compiler/go/capi"
)

const usage = `MetaFFI Go Compiler - compiles IDL JSON to Go guest or host code.

Usage:
  go_compiler guest <idl_json_file> <output_path> [options]
  go_compiler host  <idl_json_file> <output_path> [options]

Arguments:
  idl_json_file  Path to file containing MetaFFI IDL JSON
  output_path    Output directory (or "dir;filename" for explicit filename)
  options        (Optional) Comma-separated key=value options

Output:
  - Exit code 0 = success
  - Exit code 1 = error (message on STDERR)
`

func main() {
	if len(os.Args) < 4 {
		fmt.Fprint(os.Stderr, usage)
		os.Exit(1)
	}
	mode := os.Args[1]
	idlFile := os.Args[2]
	outputPath := os.Args[3]
	options := ""
	if len(os.Args) >= 5 {
		options = os.Args[4]
	}
	fmt.Fprintf(os.Stderr, "[go_compiler] mode=%s idl_file=%s output=%s options=%q\n", mode, idlFile, outputPath, options)

	idlJSON, err := os.ReadFile(idlFile)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error reading IDL file: %v\n", err)
		os.Exit(1)
	}
	fmt.Fprintf(os.Stderr, "[go_compiler] read IDL JSON (%d bytes)\n", len(idlJSON))

	switch mode {
	case "guest":
		if errMsg := capi.CompileToGuest(string(idlJSON), outputPath, options); errMsg != "" {
			fmt.Fprintf(os.Stderr, "%s\n", errMsg)
			os.Exit(1)
		}
	case "host":
		if errMsg := capi.CompileFromHost(string(idlJSON), outputPath, options); errMsg != "" {
			fmt.Fprintf(os.Stderr, "%s\n", errMsg)
			os.Exit(1)
		}
	default:
		fmt.Fprintf(os.Stderr, "Unknown mode %q (use guest or host)\n", mode)
		os.Exit(1)
	}
	os.Exit(0)
}
