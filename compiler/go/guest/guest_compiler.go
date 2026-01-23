package guest

import (
	"fmt"
	"go/build"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"text/template"

	"github.com/MetaFFI/sdk/compiler/go/common"
	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

const VERSION = "main"

// --------------------------------------------------------------------
func getDynamicLibSuffix() string {
	switch runtime.GOOS {
	case "windows":
		return ".dll"
	case "darwin":
		return ".dylib"
	default: // We might need to make this more specific in the future
		return ".so"
	}
}

// --------------------------------------------------------------------
type GuestCompiler struct {
	def            *IDL.IDLDefinition
	outputDir      string
	outputFilename string
	blockName      string
	blockCode      string
}

// --------------------------------------------------------------------
func NewGuestCompiler() *GuestCompiler {
	return &GuestCompiler{}
}

// --------------------------------------------------------------------
func getBoolOption(options map[string]string, keys ...string) bool {
	if options == nil {
		return false
	}

	for _, key := range keys {
		if value, found := options[key]; found {
			switch strings.ToLower(strings.TrimSpace(value)) {
			case "1", "true", "yes", "y", "on":
				return true
			}
		}
	}

	return false
}

// --------------------------------------------------------------------
func (this *GuestCompiler) Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, guestOptions map[string]string) (err error) {

	if outputFilename == "" {
		outputFilename = definition.IDLSource
	}

	this.def = definition
	this.outputDir = outputDir
	this.outputFilename = outputFilename

	// generate code
	code, err := this.generateCode()
	if err != nil {
		return fmt.Errorf("Failed to generate guest code: %v", err)
	}

	if getBoolOption(guestOptions, "source_only", "generate_source_only") {
		if err := this.writeSourceFiles(code); err != nil {
			return fmt.Errorf("Failed to write guest source code: %v", err)
		}
		return nil
	}

	file, err := this.buildDynamicLibrary(code)
	if err != nil {
		return fmt.Errorf("Failed to generate guest code: %v", err)
	}

	// write to output
	genOutputFullFileName := fmt.Sprintf("%v%v%v_MetaFFIGuest%v", this.outputDir, string(os.PathSeparator), this.outputFilename, getDynamicLibSuffix())
	err = ioutil.WriteFile(genOutputFullFileName, file, 0700)
	if err != nil {
		return fmt.Errorf("Failed to write dynamic library to %v. Error: %v", this.outputDir+this.outputFilename, err)
	}

	return nil

}

// --------------------------------------------------------------------
func (this *GuestCompiler) parseHeader() (string, error) {
	tmp, err := template.New("headers").Parse(GuestHeaderTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestHeaderTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, this.def)

	return buf.String(), err
}

// --------------------------------------------------------------------
func GetImportsSet(def *IDL.IDLDefinition) (map[string]bool, error) {
	set := make(map[string]bool)

	for _, m := range def.Modules {

		handleEntityPath := func(entityPath map[string]string) error {

			if pack, found := entityPath["package"]; found {
				_, err := os.Stat(entityPath["module"]) // if module does not exist locally (e.g. github.com/...)

				if pack != `main` && err == nil {
					set[os.ExpandEnv(pack)] = true
				}
			}

			if mod, found := entityPath["module"]; found {

				if strings.Contains(strings.ToUpper(mod), "$PWD") {
					d, err := os.Getwd()
					if err != nil {
						return err
					}

					mod = strings.ReplaceAll(mod, "$PWD", d)
				}

				if strings.Contains(strings.ToUpper(mod), "%CD%") {
					d, err := os.Getwd()
					if err != nil {
						return err
					}

					mod = strings.ReplaceAll(mod, "$PWD", d)
				}

				mod = os.ExpandEnv(mod)
				if fi, _ := os.Stat(mod); fi == nil { // ignore if module is local item
					set[os.ExpandEnv(mod)] = true
				}
			}

			return nil
		}

		for _, f := range m.Functions {
			err := handleEntityPath(f.EntityPath)
			if err != nil {
				return nil, err
			}
		}

		for _, c := range m.Classes {
			for _, cstr := range c.Constructors {
				err := handleEntityPath(cstr.EntityPath)
				if err != nil {
					return nil, err
				}
			}

			for _, meth := range c.Methods {
				err := handleEntityPath(meth.EntityPath)
				if err != nil {
					return nil, err
				}
			}

			if c.Releaser != nil {
				err := handleEntityPath(c.Releaser.EntityPath)
				if err != nil {
					return nil, err
				}
			}
		}
	}

	return set, nil
}

// --------------------------------------------------------------------

func (this *GuestCompiler) parseImports() (string, error) {

	// get all imports from the def file
	imports := struct {
		Imports []string
		Modules []*IDL.ModuleDefinition
	}{
		Imports: make([]string, 0),
		Modules: this.def.Modules,
	}

	set, err := GetImportsSet(this.def)
	if err != nil {
		return "", err
	}

	for k, _ := range set {
		imports.Imports = append(imports.Imports, k)
	}

	tmp, err := template.New("imports").Funcs(common.TemplateFuncMap).Parse(GuestImportsTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestImportsTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, imports)
	importsCode := buf.String()

	return importsCode, err
}

// --------------------------------------------------------------------
func (this *GuestCompiler) parseGuestHelperFunctions() (string, error) {

	tmpEntryPoint, err := template.New("helper").Funcs(common.TemplateFuncMap).Parse(GuestHelperFunctionsTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestFunctionXLLRTemplate: %v", err)
	}

	bufEntryPoint := strings.Builder{}
	err = tmpEntryPoint.Execute(&bufEntryPoint, this.def)

	return bufEntryPoint.String(), err
}

// --------------------------------------------------------------------
func (this *GuestCompiler) parseForeignFunctions() (string, error) {

	tmpEntryPoint, err := template.New("foreignfuncs").Funcs(common.TemplateFuncMap).Parse(GuestFunctionXLLRTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestFunctionXLLRTemplate: %v", err)
	}

	bufEntryPoint := strings.Builder{}
	err = tmpEntryPoint.Execute(&bufEntryPoint, this.def)

	return bufEntryPoint.String(), err
}

// --------------------------------------------------------------------
func (this *GuestCompiler) parseCImportsCGoFile() (string, error) {

	tmp, err := template.New("guest").Funcs(common.TemplateFuncMap).Parse(GuestCImportCGoFileTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestCImportCGoFileTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, nil)

	return buf.String(), err
}

// --------------------------------------------------------------------
func (this *GuestCompiler) parseCImports() (string, error) {

	tmp, err := template.New("guest").Funcs(common.TemplateFuncMap).Parse(GuestCImportTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestCImportTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, nil)

	return buf.String(), err
}

// --------------------------------------------------------------------
func removeFromAliasesDotImportedPackagesAndPathToPackages(def *IDL.IDLDefinition) {
	// if the type include "/" (for instance io/fs), remove the "/"
	// and everything before it
	removeBeforeLastSlash := func(s string) string {
		lastSlashIndex := strings.LastIndex(s, "/")
		if lastSlashIndex != -1 {
			pointersCount := strings.Count(s, "*")
			s = s[lastSlashIndex+1:]
			s = strings.Repeat("*", pointersCount) + s
		}

		return s
	}

	getPackageName := func(s string) string {
		s = strings.ReplaceAll(s, "*", "")
		firstDotIndex := strings.Index(s, ".")
		if firstDotIndex != -1 {
			return s[:firstDotIndex]
		}
		return s
	}

	removePackageName := func(s string) string {
		packageName := getPackageName(s) + "."
		return strings.ReplaceAll(s, packageName, "")
	}

	removePackageNameIfInDotImports := func(s string, dotImportedPackage map[string]bool) string {
		if strings.Index(s, ".") != -1 {
			if _, exists := dotImportedPackage[getPackageName(s)]; exists {
				s = removePackageName(s)
			}
		}
		return s
	}

	// if path to package has been imported (using ".") by the definition, remove
	// the package name as well.
	dotImportedFullPackageNames, err := GetImportsSet(def)
	if err != nil {
		panic(err)
	}
	dotImportedPackage := make(map[string]bool)
	for k, _ := range dotImportedFullPackageNames {
		dotImportedPackage[removeBeforeLastSlash(k)] = true
	}

	handleArgDefinition := func(p *IDL.ArgDefinition) {
		if p.IsTypeAlias() {
			//beforeAlias := p.TypeAlias
			p.TypeAlias = removeBeforeLastSlash(p.TypeAlias)
			p.TypeAlias = removePackageNameIfInDotImports(p.TypeAlias, dotImportedPackage)

			//fmt.Printf("+++ %v ==> %v\n", beforeAlias, p.TypeAlias)
		}
	}

	handleFunctionDefinition := func(f *IDL.FunctionDefinition) {
		if f == nil {
			return
		}

		for _, p := range f.Parameters {
			handleArgDefinition(p)
		}
		for _, p := range f.ReturnValues {
			handleArgDefinition(p)
		}
	}

	for _, m := range def.Modules {
		for _, g := range m.Globals {
			handleFunctionDefinition(g.Getter)
			handleFunctionDefinition(g.Setter)
		}

		for _, f := range m.Functions {
			handleFunctionDefinition(f)
		}

		for _, c := range m.Classes {
			for _, cstr := range c.Constructors {
				handleFunctionDefinition(&cstr.FunctionDefinition)
			}
			for _, field := range c.Fields {
				handleFunctionDefinition(&field.Getter.FunctionDefinition)
				handleFunctionDefinition(&field.Setter.FunctionDefinition)
			}
			for _, method := range c.Methods {
				handleFunctionDefinition(&method.FunctionDefinition)
			}
			handleFunctionDefinition(&c.Releaser.FunctionDefinition)
		}
	}
}

// --------------------------------------------------------------------
func (this *GuestCompiler) generateCode() (string, error) {

	header, err := this.parseHeader()
	if err != nil {
		return "", err
	}

	imports, err := this.parseImports()
	if err != nil {
		return "", err
	}

	// remove from aliases the dot imported packages
	removeFromAliasesDotImportedPackagesAndPathToPackages(this.def)

	cimports, err := this.parseCImports()
	if err != nil {
		return "", err
	}

	guestHelpers, err := this.parseGuestHelperFunctions()
	if err != nil {
		return "", err
	}

	functionStubs, err := this.parseForeignFunctions()
	if err != nil {
		return "", err
	}

	res := header + imports + cimports + functionStubs + guestHelpers + GuestMainFunction

	return res, nil
}

// --------------------------------------------------------------------
func (this *GuestCompiler) writeSourceFiles(code string) error {
	if err := os.MkdirAll(this.outputDir, 0777); err != nil {
		return fmt.Errorf("failed to create output directory: %v", err)
	}

	baseName := fmt.Sprintf("%v_MetaFFIGuest", this.outputFilename)
	sourcePath := filepath.Join(this.outputDir, baseName+".go")
	if err := ioutil.WriteFile(sourcePath, []byte(code), 0644); err != nil {
		return fmt.Errorf("failed to write guest source code: %v", err)
	}

	cgoCode, err := this.parseCImportsCGoFile()
	if err != nil {
		return fmt.Errorf("failed to generate CGo guest go code: %v", err)
	}

	cgoPath := filepath.Join(this.outputDir, baseName+"_cgo.go")
	if err := ioutil.WriteFile(cgoPath, []byte(cgoCode), 0644); err != nil {
		return fmt.Errorf("failed to write guest CGo code: %v", err)
	}

	return nil
}

// --------------------------------------------------------------------
func (this *GuestCompiler) buildDynamicLibrary(code string) ([]byte, error) {

	dir, err := os.MkdirTemp("", "metaffi_go_compiler*")
	if err != nil {
		return nil, fmt.Errorf("Failed to create temp dir to build code: %v", err)
	}
	defer func() {
		if err == nil {
			//_ = os.RemoveAll(dir)
		}
	}()

	dir = dir + string(os.PathSeparator)

	err = ioutil.WriteFile(dir+"metaffi_guest.go", []byte(code), 0700)
	if err != nil {
		return nil, fmt.Errorf("Failed to write guest go code: %v", err)
	}

	// TODO: This should move to "generate code" that need to return a map of files
	cgoCode, err := this.parseCImportsCGoFile()
	if err != nil {
		return nil, fmt.Errorf("Failed to generate CGo guest go code: %v", err)
	}

	err = ioutil.WriteFile(dir+"metaffi_guest_cgo.go", []byte(cgoCode), 0700)
	if err != nil {
		return nil, fmt.Errorf("Failed to write guest go code: %v", err)
	}

	fmt.Println("Building Go foreign functions")

	// add go.mod
	_, err = this.goModInit(dir, "main")
	if err != nil {
		return nil, err
	}

	err = this.applyDevReplaces(dir)
	if err != nil {
		return nil, err
	}

	addedLocalModules := make(map[string]bool)

	handleEntityPathPackage := func(m *IDL.ModuleDefinition, entityPath map[string]string) error {
		for k, v := range entityPath {
			if k == "module" {

				if strings.Contains(strings.ToUpper(v), "$PWD") {
					d, err := os.Getwd()
					if err != nil {
						return err
					}

					v = strings.ReplaceAll(v, "$PWD", d)
				}

				if strings.Contains(strings.ToUpper(v), "%CD%") {
					d, err := os.Getwd()
					if err != nil {
						return err
					}

					v = strings.ReplaceAll(v, "$PWD", d)
				}

				v = os.ExpandEnv(v)
				if fi, _ := os.Stat(v); fi != nil && fi.IsDir() { // if module is local dir
					if _, alreadyAdded := addedLocalModules[v]; !alreadyAdded {
						// if embedded code, write the source code into a Package folder and skip "-replace"
						if this.blockCode != "" {
							packageDir := dir + os.ExpandEnv(entityPath["package"]) + string(os.PathSeparator)
							err = os.Mkdir(packageDir, 0777)
							if err != nil {
								return fmt.Errorf("Failed creating directory for embedded code: %v.\nError:\n%v", packageDir, err)
							}

							err = ioutil.WriteFile(packageDir+entityPath["package"]+".go", []byte(this.blockCode), 0700)
							if err != nil {
								return fmt.Errorf("Failed to embedded block go code: %v", err)
							}

							_, err := this.goModInit(packageDir, entityPath["package"])
							if err != nil {
								return err
							}

							_, err = this.goGet(packageDir)
							if err != nil {
								return err
							}

							err = this.goReplace(dir, os.ExpandEnv(entityPath["package"]), "./"+entityPath["package"])
							if err != nil {
								return err
							}

						} else {
							// point module to
							err = this.goReplace(dir, os.ExpandEnv(entityPath["package"]), v)
							if err != nil {
								return err
							}
						}

						addedLocalModules[v] = true
					}
				}
			}
		}

		return nil
	}

	// add "replace"s if there are local imports
	for _, m := range this.def.Modules {
		for _, f := range m.Functions {
			err = handleEntityPathPackage(m, f.EntityPath)
			if err != nil {
				return nil, err
			}
		}

		for _, c := range m.Classes {
			for _, cstr := range c.Constructors {
				err = handleEntityPathPackage(m, cstr.EntityPath)
				if err != nil {
					return nil, err
				}
			}

			if c.Releaser != nil {
				err = handleEntityPathPackage(m, c.Releaser.EntityPath)
				if err != nil {
					return nil, err
				}
			}

			for _, meth := range c.Methods {
				err = handleEntityPathPackage(m, meth.EntityPath)
				if err != nil {
					return nil, err
				}
			}
		}
	}

	_, err = ioutil.ReadFile(dir + "go.mod")
	if err != nil {
		println("Failed to find go.mod in " + dir + "go.mod")
	}

	// build dynamic library
	_, err = this.goGet(dir)
	if err != nil {
		return nil, err
	}

	_, err = this.goClean(dir)
	if err != nil {
		return nil, err
	}

	_, err = this.goBuild(dir)
	if err != nil {
		return nil, err
	}

	// copy to output dir
	fileData, err := ioutil.ReadFile(dir + this.outputFilename + getDynamicLibSuffix())
	if err != nil {
		return nil, fmt.Errorf("Failed to read created dynamic library. Error: %v", err)
	}

	return fileData, nil
}

// --------------------------------------------------------------------
func (this *GuestCompiler) goModInit(dir string, packageName string) (string, error) {
	modInitCmd := exec.Command("go", "mod", "init", packageName)
	modInitCmd.Dir = dir

	var symbol string
	if runtime.GOOS == "windows" {
		symbol = ">"
	} else {
		symbol = "$"
	}

	if strings.HasSuffix(dir, "/") {
		dir = dir[:len(dir)-1]
	}

	fmt.Printf("%v%v %v\n", dir, symbol, strings.Join(modInitCmd.Args, " "))
	output, err := modInitCmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("Failed building Go foreign function with error: %v.\nOutput:\n%v", err, string(output))
	}

	return string(output), err
}

// --------------------------------------------------------------------
func (this *GuestCompiler) goGet(dir string) (string, error) {

	goGetCommand := func(module string) (string, error) {

		var getCmd *exec.Cmd
		if module == "" {
			getCmd = exec.Command("go", "get", "-v")
		} else {
			getCmd = exec.Command("go", "get", "-v", module)
		}

		getCmd.Dir = dir

		var symbol string
		if runtime.GOOS == "windows" {
			symbol = ">"
		} else {
			symbol = "$"
		}

		fmt.Printf("%v%v %v\n", getCmd.Dir[:len(getCmd.Dir)-1], symbol, strings.Join(getCmd.Args, " "))
		output, err := getCmd.CombinedOutput()
		if err != nil {
			println(string(output))
			return "", fmt.Errorf("Failed building Go foreign function in \"%v\" with error: %v.\nOutput:\n%v", dir, err, string(output))
		}

		return string(output), err
	}

	_, err := goGetCommand("")
	if err != nil {
		return "", err
	}

	idlEntitiesModule := "github.com/MetaFFI/sdk/idl_entities/go"
	goRuntimeModule := "github.com/MetaFFI/lang-plugin-go/go-runtime"
	if os.Getenv("METAFFI_SOURCE_ROOT") == "" {
		idlEntitiesModule += "@" + VERSION
		goRuntimeModule += "@" + VERSION
	}

	_, err = goGetCommand(idlEntitiesModule)
	if err != nil {
		return "", err
	}

	_, err = goGetCommand(goRuntimeModule)
	if err != nil {
		return "", err
	}

	return "", nil
}

// --------------------------------------------------------------------
func (this *GuestCompiler) applyDevReplaces(dir string) error {
	metaffiRoot := strings.TrimSpace(os.Getenv("METAFFI_SOURCE_ROOT"))
	if metaffiRoot == "" {
		return nil
	}

	idlEntitiesPath := filepath.Join(metaffiRoot, "sdk", "idl_entities", "go")
	if _, err := os.Stat(idlEntitiesPath); err != nil {
		return fmt.Errorf("METAFFI_SOURCE_ROOT is set, but idl_entities path is invalid: %v", err)
	}

	goRuntimePath := filepath.Join(metaffiRoot, "lang-plugin-go", "go-runtime")
	if _, err := os.Stat(goRuntimePath); err != nil {
		return fmt.Errorf("METAFFI_SOURCE_ROOT is set, but go-runtime path is invalid: %v", err)
	}

	err := this.goReplace(dir, "github.com/MetaFFI/sdk/idl_entities/go", idlEntitiesPath)
	if err != nil {
		return err
	}

	return this.goReplace(dir, "github.com/MetaFFI/lang-plugin-go/go-runtime", goRuntimePath)
}

// --------------------------------------------------------------------
func (this *GuestCompiler) goClean(dir string) (string, error) {
	cleanCmd := exec.Command("go", "clean", "-cache")
	cleanCmd.Dir = dir

	var symbol string
	if runtime.GOOS == "windows" {
		symbol = ">"
	} else {
		symbol = "$"
	}

	if strings.HasSuffix(dir, "/") {
		dir = dir[:len(dir)-1]
	}

	fmt.Printf("%v%v %v\n", dir, symbol, strings.Join(cleanCmd.Args, " "))
	output, err := cleanCmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("Failed building Go foreign function in \"%v\" with error: %v.\nOutput:\n%v", dir, err, string(output))
	}

	return string(output), err
}

// --------------------------------------------------------------------
func (this *GuestCompiler) goBuild(dir string) (string, error) {
	buildCmd := exec.Command("go", "build", "-v", "-tags=guest", "-buildmode=c-shared", "-gcflags=-shared", "-o", dir+this.outputFilename+getDynamicLibSuffix())
	buildCmd.Dir = dir

	var symbol string
	if runtime.GOOS == "windows" {
		symbol = ">"
	} else {
		symbol = "$"
	}

	if strings.HasSuffix(dir, "/") {
		dir = dir[:len(dir)-1]
	}

	fmt.Printf("%v%v %v\n", dir, symbol, strings.Join(buildCmd.Args, " "))
	output, err := buildCmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("Failed building Go foreign function in \"%v\" with error: %v.\nOutput:\n%v", dir, err, string(output))
	}

	return string(output), err
}

// --------------------------------------------------------------------
func replaceUpper(s string) string {
	result := ""
	for _, c := range s {
		if c >= 'A' && c <= 'Z' {
			result += "!" + strings.ToLower(string(c))
		} else {
			result += string(c)
		}
	}
	return result
}

func (this *GuestCompiler) goReplace(dir string, packageName string, packagePath string) error {

	// first check if the package is inside GOROOT
	goroot := os.Getenv("GOPATH")
	if goroot == "" {
		goroot = build.Default.GOPATH
	}

	goroot = strings.ToLower(strings.ReplaceAll(goroot, "\\", "/"))
	packagePathToCheck := strings.ToLower(strings.ReplaceAll(replaceUpper(packagePath), "\\", "/"))

	if strings.HasPrefix(packagePathToCheck, goroot) {
		// package in GOROOT - no need to "replace"
		return nil
	}

	// if not in GOROOT - we need to place it in go.mod

	replaceCommand := fmt.Sprintf("\nreplace %v => %v\n", packageName, packagePath)

	fmt.Printf("Writing to %v the command: %v\n", dir+"/go.mod", replaceCommand)

	f, err := os.OpenFile(dir+"/go.mod", os.O_APPEND|os.O_WRONLY, 0644)
	if err != nil {
		return err
	}

	_, err = f.Write([]byte(replaceCommand))
	if err != nil {
		return err
	}

	err = f.Close()

	// 	getCmd := exec.Command("go", "mod", "edit", "-replace", fmt.Sprintf("%v=%v", packageName, packagePath))
	// 	getCmd.Dir = dir
	// 	fmt.Printf("%v\n", strings.Join(getCmd.Args, " "))
	// 	output, err := getCmd.CombinedOutput()
	// 	if err != nil {
	// 		return "", fmt.Errorf("Failed building Go foreign function with error: %v.\nOutput:\n%v", err, string(output))
	// 	}

	return err
}

//--------------------------------------------------------------------
