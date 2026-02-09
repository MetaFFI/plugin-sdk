// Package capi provides Go compiler entrypoints used by the go_compiler CLI (cmd/go_compiler).
// The C++ compiler plugin runs the go_compiler executable and passes IDL via a temp file.
package capi

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	compiler "github.com/MetaFFI/sdk/compiler/go"
	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

func logCompile(format string, args ...interface{}) {
	fmt.Fprintf(os.Stderr, "[go_compiler] "+format+"\n", args...)
}

var (
	guestCompiler compiler.GuestCompiler
	hostCompiler  compiler.HostCompiler
)

func init() {
	guestCompiler = compiler.NewGuestCompiler()
	hostCompiler = compiler.NewHostCompiler()
}

func parseOptions(optionsStr string) map[string]string {
	m := make(map[string]string)
	if strings.TrimSpace(optionsStr) == "" {
		return m
	}
	for _, option := range strings.Split(optionsStr, ",") {
		keyval := strings.SplitN(option, "=", 2)
		if len(keyval) != 2 {
			continue
		}
		m[strings.TrimSpace(keyval[0])] = strings.TrimSpace(keyval[1])
	}
	return m
}

func parseOutPath(outputPath string) (dir, filename string) {
	dir = outputPath
	filename = ""
	outPathStat, err := os.Stat(outputPath)
	if (err != nil && os.IsNotExist(err)) || (err == nil && !outPathStat.IsDir()) {
		splitted := filepath.SplitList(outputPath)
		if len(splitted) < 2 {
			return outputPath, ""
		}
		dir = filepath.Join(splitted[:len(splitted)-1]...)
		filename = splitted[len(splitted)-1]
	}
	return dir, filename
}

// CompileToGuest compiles IDL JSON to guest code. Returns error message or "" on success.
func CompileToGuest(idlJSON, outputPath, guestOptions string) string {
	logCompile("CompileToGuest: idl=%d bytes output=%s", len(idlJSON), outputPath)
	def, err := IDL.NewIDLDefinitionFromJSON(idlJSON)
	if err != nil {
		return err.Error()
	}
	dir, outFilename := parseOutPath(outputPath)
	if outFilename == "" {
		outFilename = def.IDLSource
	}
	logCompile("Guest output: dir=%s filename=%s", dir, outFilename)
	opts := parseOptions(guestOptions)
	if err := guestCompiler.Compile(def, dir, outFilename, opts); err != nil {
		return err.Error()
	}
	logCompile("CompileToGuest done")
	return ""
}

// CompileFromHost compiles IDL JSON to host code. Returns error message or "" on success.
func CompileFromHost(idlJSON, outputPath, hostOptions string) string {
	logCompile("CompileFromHost: idl=%d bytes output=%s", len(idlJSON), outputPath)
	def, err := IDL.NewIDLDefinitionFromJSON(idlJSON)
	if err != nil {
		return err.Error()
	}
	dir, outFilename := parseOutPath(outputPath)
	if outFilename == "" {
		outFilename = def.IDLSource
	}
	logCompile("Host output: dir=%s filename=%s", dir, outFilename)
	opts := parseOptions(hostOptions)
	if err := hostCompiler.Compile(def, dir, outFilename, opts); err != nil {
		return err.Error()
	}
	logCompile("CompileFromHost done")
	return ""
}
