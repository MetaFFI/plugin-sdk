package common

import (
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
)

// GoInstalledInfo holds information about an installed Go toolchain.
// Used by the compiler to run "go build" and related commands.
type GoInstalledInfo struct {
	Version string // e.g. "go1.21.5"
	Goroot  string // GOROOT path
	GoExe   string // Path to go executable
}

// DetectInstalledGo searches GOROOT and PATH for Go installations
// and returns a slice of detected toolchains. Used at compile time
// to locate the Go toolchain for building guest shared libraries.
// Returns an empty slice if none found.
func DetectInstalledGo() []GoInstalledInfo {
	var result []GoInstalledInfo
	seen := make(map[string]bool)

	addCandidate := func(goroot, goExe string) {
		norm, err := filepath.Abs(goroot)
		if err != nil {
			norm = filepath.Clean(goroot)
		}
		if seen[norm] {
			return
		}
		if !isValidGoInstallation(norm) {
			return
		}
		version := getGoVersion(goExe)
		if version == "" {
			return
		}
		seen[norm] = true
		result = append(result, GoInstalledInfo{
			Version: version,
			Goroot:  norm,
			GoExe:   goExe,
		})
	}

	// 1. GOROOT
	if goroot, ok := os.LookupEnv("GOROOT"); ok && goroot != "" {
		goExe := filepath.Join(goroot, "bin", goExeName())
		if fi, err := os.Stat(goExe); err == nil && fi.Mode().IsRegular() {
			addCandidate(goroot, goExe)
		}
	}

	// 2. PATH
	for _, goExe := range findGoFromPath() {
		goroot := resolveGorootFromExecutable(goExe)
		if goroot != "" {
			addCandidate(goroot, goExe)
		}
	}

	return result
}

func goExeName() string {
	if runtime.GOOS == "windows" {
		return "go.exe"
	}
	return "go"
}

func findGoFromPath() []string {
	pathEnv := os.Getenv("PATH")
	if pathEnv == "" {
		return nil
	}
	var sep string
	if runtime.GOOS == "windows" {
		sep = ";"
	} else {
		sep = ":"
	}
	seen := make(map[string]bool)
	var result []string
	for _, dir := range strings.Split(pathEnv, sep) {
		dir = strings.TrimSpace(dir)
		if dir == "" {
			continue
		}
		goPath := filepath.Join(dir, goExeName())
		if fi, err := os.Stat(goPath); err == nil && fi.Mode().IsRegular() {
			abs, _ := filepath.Abs(goPath)
			if abs == "" {
				abs = goPath
			}
			if !seen[abs] {
				seen[abs] = true
				result = append(result, abs)
			}
		}
	}
	return result
}

func resolveGorootFromExecutable(goExe string) string {
	resolved, err := filepath.EvalSymlinks(goExe)
	if err != nil {
		resolved = goExe
	}
	// go is typically in GOROOT/bin/go -> parent of bin is GOROOT
	dir := filepath.Dir(resolved)
	if filepath.Base(dir) == "bin" {
		goroot := filepath.Dir(dir)
		if isValidGoInstallation(goroot) {
			return goroot
		}
	}
	return ""
}

func getGoVersion(goExe string) string {
	cmd := exec.Command(goExe, "version")
	cmd.Env = os.Environ()
	out, err := cmd.Output()
	if err != nil {
		return ""
	}
	// "go version go1.21.5 windows/amd64"
	s := strings.TrimSpace(string(out))
	const prefix = "go version "
	if !strings.HasPrefix(s, prefix) {
		return ""
	}
	s = s[len(prefix):]
	fields := strings.Fields(s)
	if len(fields) == 0 {
		return ""
	}
	tok := fields[0]
	if strings.HasPrefix(tok, "go") {
		return tok
	}
	return ""
}

func isValidGoInstallation(goroot string) bool {
	if goroot == "" {
		return false
	}
	fi, err := os.Stat(goroot)
	if err != nil || !fi.IsDir() {
		return false
	}
	binDir := filepath.Join(goroot, "bin")
	if _, err := os.Stat(binDir); err != nil {
		return false
	}
	goExe := filepath.Join(binDir, goExeName())
	if _, err := os.Stat(goExe); err != nil {
		return false
	}
	// Prefer pkg or src as additional validation
	if _, err := os.Stat(filepath.Join(goroot, "pkg")); err == nil {
		return true
	}
	if _, err := os.Stat(filepath.Join(goroot, "src")); err == nil {
		return true
	}
	return true
}
