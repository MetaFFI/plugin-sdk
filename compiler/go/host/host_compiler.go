package host

import (
	_ "embed"
	"fmt"
	"io/ioutil"
	"os"
	"strconv"
	"strings"
	"text/template"

	"github.com/MetaFFI/sdk/compiler/go/common"
	IDL "github.com/MetaFFI/sdk/idl_entities/go/IDL"
	"golang.org/x/text/cases"
	"golang.org/x/text/language"
)

var goKeywords = map[string]bool{
	"break": true, "default": true, "func": true, "interface": true, "select": true,
	"case": true, "defer": true, "go": true, "map": true, "struct": true,
	"chan": true, "else": true, "goto": true, "package": true, "switch": true,
	"const": true, "fallthrough": true, "if": true, "range": true, "type": true,
	"continue": true, "for": true, "import": true, "return": true, "var": true,
	"string": true, "int8": true, "int16": true, "int32": true, "int64": true,
	"uint8": true, "uint16": true, "uint32": true, "uint64": true,
	"float32": true, "float64": true, "bool": true, "fmt": true,
}

// --------------------------------------------------------------------
type HostCompiler struct {
	def            *IDL.IDLDefinition
	outputDir      string
	hostOptions    map[string]string
	outputFilename string
}

// --------------------------------------------------------------------
func NewHostCompiler() *HostCompiler {
	return &HostCompiler{}
}

// --------------------------------------------------------------------
func overloadCallablesWithOptionalParameters(def *IDL.IDLDefinition) {

	for _, mod := range def.Modules {
		functions, methods, constructors := mod.GetCallablesWithOptionalParameters(true, true, true)

		for _, f := range functions {
			firstIndexOfOptionalParameter := f.GetFirstIndexOfOptionalParameter()

			j := 0
			for i := firstIndexOfOptionalParameter; i < len(f.Parameters); i++ {
				j += 1
				dup := f.Duplicate()
				dup.Name += "_overload" + strconv.Itoa(j)
				dup.Parameters = dup.Parameters[:i]
				mod.Functions = append(mod.Functions, dup)
			}
		}

		for _, cstr := range constructors {
			firstIndexOfOptionalParameter := cstr.GetFirstIndexOfOptionalParameter()

			j := 0
			for i := firstIndexOfOptionalParameter; i < len(cstr.Parameters); i++ {
				j += 1
				dup := cstr.Duplicate()
				dup.Name += "_overload" + strconv.Itoa(j)
				dup.Parameters = dup.Parameters[:i]
				cstr.GetParent().AddConstructor(dup)
			}
		}

		for _, m := range methods {
			firstIndexOfOptionalParameter := m.GetFirstIndexOfOptionalParameter()

			j := 0
			for i := firstIndexOfOptionalParameter; i < len(m.Parameters); i++ {
				j += 1
				dup := m.Duplicate()
				dup.Name += "_overload" + strconv.Itoa(j)
				dup.Parameters = dup.Parameters[:i]
				m.GetParent().AddMethod(dup)
			}
		}
	}
}

// --------------------------------------------------------------------
func fixModulesMustNotContainDot(def *IDL.IDLDefinition) {
	for _, mod := range def.Modules {
		mod.Name = strings.ReplaceAll(mod.Name, ".", "_")
	}
}

// --------------------------------------------------------------------
func fixNameCollisionAfterConvertingToGoNameConvention(def *IDL.IDLDefinition) {
	for _, mod := range def.Modules {

		// There is no collision with globals due to MetaFFIGetter and MetaFFISetter suffixes
		// TODO: Maybe set suffix only on collisions?!

		for _, f := range mod.Functions {
			funcNewName := common.ToGoNameConv(f.Name)
			isRename := false
			for _, c := range mod.Classes {
				classNewName := common.ToGoNameConv(c.Name)
				if classNewName == funcNewName {
					isRename = true
					break
				}
			}

			if isRename {
				f.Name += "Func"
			}
		}
	}
}

// --------------------------------------------------------------------
func (this *HostCompiler) Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, hostOptions map[string]string) (err error) {

	// make sure definition does not use "go syntax-keywords" as names. If so, change the names a bit...
	common.ModifyKeywords(definition, goKeywords, func(keyword string) string { return keyword + "__" })

	// support optional parameters in guests by overloading the functions/methods, each time
	// adding another optional parameter to the parameter list.
	// As Go does not support overloads, simply append an index to the end of the function/method name
	overloadCallablesWithOptionalParameters(definition)

	// convert "." is module names to "_" as modules in Go must not contain "."
	fixModulesMustNotContainDot(definition)

	fixNameCollisionAfterConvertingToGoNameConvention(definition)

	if outputFilename == "" {
		outputFilename = definition.IDLSource
	}

	outputFilename = strings.ReplaceAll(outputFilename, ".", "_") // filename must not contains "."

	this.def = definition
	this.outputDir = outputDir
	this.hostOptions = hostOptions
	this.outputFilename = outputFilename
	caser := cases.Title(language.Und, cases.NoLower)
	this.def.ReplaceKeywords(map[string]string{
		"type":  caser.String("type"),
		"class": caser.String("class"),
		"func":  caser.String("func"),
		"var":   caser.String("var"),
		"const": caser.String("const"),
	})

	// generate code
	code, _, err := this.generateCode()
	if err != nil {
		return fmt.Errorf("Failed to generate host code: %v", err)
	}

	// TODO: handle multiple modules

	_ = os.Mkdir(this.outputDir+string(os.PathSeparator)+strings.ToLower(this.def.Modules[0].Name), 0777)

	// write to output
	genOutputFilename := this.outputDir + string(os.PathSeparator) + strings.ToLower(this.def.Modules[0].Name) + string(os.PathSeparator) + this.outputFilename + "_MetaFFIHost.go"
	err = ioutil.WriteFile(genOutputFilename, []byte(code), 0600)
	if err != nil {
		return fmt.Errorf("Failed to write host code to %v. Error: %v", this.outputDir+this.outputFilename, err)
	}

	return nil
}

// --------------------------------------------------------------------
func (this *HostCompiler) parseHeader() (string, error) {
	tmp, err := template.New("HostHeaderTemplate").Parse(HostHeaderTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse HostHeaderTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, this.def)

	return buf.String(), err
}

// --------------------------------------------------------------------
func (this *HostCompiler) parseForeignStubs() (string, error) {

	tmp, err := template.New("Go HostFunctionStubsTemplate").Funcs(common.TemplateFuncMap).Parse(HostFunctionStubsTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse Go HostFunctionStubsTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, this.def)

	return buf.String(), err
}

// --------------------------------------------------------------------
func (this *HostCompiler) parsePackage() (code string, packageName string, err error) {
	tmp, err := template.New("HostPackageTemplate").Funcs(common.TemplateFuncMap).Parse(HostPackageTemplate)
	if err != nil {
		return "", "", fmt.Errorf("Failed to parse Go HostPackageTemplate: %v", err)
	}

	PackageName := struct {
		Package string
	}{
		Package: this.def.Modules[0].Name, // TODO: support multiple modules
	}

	if pckName, found := this.hostOptions["package"]; found {
		PackageName.Package = pckName
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, &PackageName)

	return buf.String(), PackageName.Package, err
}

// --------------------------------------------------------------------
func (this *HostCompiler) generateCode() (code string, packageName string, err error) {

	header, err := this.parseHeader()
	if err != nil {
		return "", "", err
	}

	packageDeclaration, packageName, err := this.parsePackage()
	if err != nil {
		return "", "", err
	}

	functionStubs, err := this.parseForeignStubs()
	if err != nil {
		return "", "", err
	}

	res := header + packageDeclaration + functionStubs

	return res, packageName, err
}

//--------------------------------------------------------------------
