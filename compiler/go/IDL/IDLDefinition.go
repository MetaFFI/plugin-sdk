package IDL

import (
	"encoding/json"
	"fmt"
	"path/filepath"
	"strings"
)

//--------------------------------------------------------------------
// The expected syntax:
// metaffi_function_path: "key1=val1,key2=val2...."
func parseFunctionPath(pathToFunction string, pathMap map[string]string) (map[string]string, error){

	var res map[string]string
	if pathMap == nil{
		res = make(map[string]string)
	} else {
		res = pathMap
	}

	pairs := strings.Split(pathToFunction, ",")

	if len(pairs) == 0{
		return nil, nil
	}

	for _, pair := range pairs{
		elems := strings.Split(pair, "=")

		if len(elems) != 2{
			return nil, fmt.Errorf("Failed parsing metaffi_function_path tag. The pair \"%v\" is invalid.", pair)
		}

		elems[0] = strings.TrimSpace(elems[0])
		elems[1] = strings.TrimSpace(elems[1])

		if elems[0] == "" || elems[1] == ""{
			return nil, fmt.Errorf("Failed parsing metaffi_function_path tag. The pair \"%v\" is invalid.", pair)
		}

		res[elems[0]] = elems[1]
	}

	return res, nil
}
//--------------------------------------------------------------------
func copyMap(source map[string]string) (target map[string]string){
	target = make(map[string]string)

	if len(source) == 0{
		return
	}

	for k, v := range source{
		target[k] = v
	}

	return
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
type Taggable interface {
	SetTag(tag string, val string)
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
type Commentable interface {
	AppendComment(comment string)
}
//--------------------------------------------------------------------
type IDLDefinition struct {
	IDLFilename string `json:"idl_filename"`
	IDLExtension string `json:"idl_extension"`
	IDLFilenameWithExtension string `json:"idl_filename_with_extension"`
	IDLFullPath string `json:"idl_full_path"`
	MetaFFIGuestLib string `json:"metaffi_guest_lib"`
	TargetLanguage string `json:"target_language"`
	Modules []*ModuleDefinition `json:"modules"`
}
//--------------------------------------------------------------------
func NewIDLDefinition(idlFullPath string, targetLanguage string) *IDLDefinition{
	idl := &IDLDefinition{
		IDLFilename:              strings.ReplaceAll(filepath.Base(idlFullPath), filepath.Ext(idlFullPath), ""),
		IDLExtension:             filepath.Ext(idlFullPath),
		IDLFilenameWithExtension: filepath.Base(idlFullPath),
		IDLFullPath:              idlFullPath,
		TargetLanguage:           targetLanguage,
		Modules:                  nil,
	}

	idl.MetaFFIGuestLib = idl.IDLFilename + "_MetaFFIGuest"

	return idl
}
//--------------------------------------------------------------------
func NewIDLDefinitionFromJSON(idlDefinitionJson string) (*IDLDefinition, error){

	def := IDLDefinition{}
	err := json.Unmarshal([]byte(idlDefinitionJson), &def)
	if err != nil{
		return nil, fmt.Errorf("Failed to unmarshal IDL definition JSON: %v", err)
	}

	return &def, nil
}
//--------------------------------------------------------------------
func (this *IDLDefinition) ToJSON() (string, error){

	jsonStr, err := json.Marshal(this)
	if err != nil{
		return "", fmt.Errorf("Failed to marshal IDL definition to JSON: %v", err)
	}

	return string(jsonStr), nil
}
//--------------------------------------------------------------------
func (this *IDLDefinition) String() string{
	res, err := this.ToJSON()
	if err != nil{
		panic(err)
	}

	return res
}
//--------------------------------------------------------------------
func (this *IDLDefinition) FinalizeConstruction(){
	for _, m := range this.Modules{

		for _, g := range m.Globals{
			if g.Getter != nil{
				g.Getter.FunctionPath[METAFFI_GUEST_LIB] = this.MetaFFIGuestLib
				g.Getter.FunctionPath[ENTRYPOINT_FUNCTION] = "EntryPoint_"+g.Getter.Name
			}

			if g.Setter != nil{
				g.Setter.FunctionPath[METAFFI_GUEST_LIB] = this.MetaFFIGuestLib
				g.Setter.FunctionPath[ENTRYPOINT_FUNCTION] = "EntryPoint_"+g.Setter.Name
			}
		}

		for _, f := range m.Functions{
			f.FunctionPath[METAFFI_GUEST_LIB] = this.MetaFFIGuestLib
			f.FunctionPath[ENTRYPOINT_FUNCTION] = "EntryPoint_"+f.Name
		}

		for _, c := range m.Classes{

			for _, f := range c.Fields{
				if f.Getter != nil{
					f.Getter.FunctionPath[METAFFI_GUEST_LIB] = this.MetaFFIGuestLib
					f.Getter.FunctionPath[ENTRYPOINT_FUNCTION] = "EntryPoint_"+c.Name+"_get_"+f.Getter.Name
				}

				if f.Setter != nil{
					f.Setter.FunctionPath[METAFFI_GUEST_LIB] = this.MetaFFIGuestLib
					f.Setter.FunctionPath[ENTRYPOINT_FUNCTION] = "EntryPoint_"+c.Name+"_get_"+f.Setter.Name
				}
			}

			for _, cstr := range c.Constructors{
				cstr.FunctionPath[METAFFI_GUEST_LIB] = this.MetaFFIGuestLib
				cstr.FunctionPath[ENTRYPOINT_FUNCTION] = "EntryPoint_"+c.Name+"_"+cstr.Name
			}

			for _, method := range c.Methods{
				method.FunctionPath[METAFFI_GUEST_LIB] = this.MetaFFIGuestLib
				method.FunctionPath[ENTRYPOINT_FUNCTION] = "EntryPoint_"+c.Name+"_"+method.Name
				method.FunctionPath[ENTRYPOINT_CLASS] = c.Name
			}

			if c.Releaser != nil{
				c.Releaser.FunctionPath[METAFFI_GUEST_LIB] = this.MetaFFIGuestLib
				c.Releaser.FunctionPath[ENTRYPOINT_FUNCTION] = "EntryPoint_"+c.Name+"_"+c.Releaser.Name
			}
		}
	}
}
//--------------------------------------------------------------------
/*
The function iterates the definition and fills fields according to well-known tags.

Supported tags:
//metaffi_target_language
//metaffi_function_path
*/
func (this *IDLDefinition) ParseWellKnownTags() error{

	for _, m := range this.Modules{

		// make a copy for each module so items can be overridden
		modulePathToFunction := make(map[string]string)

		err := m.parseWellKnownTags(modulePathToFunction)
		if err != nil{
			return err
		}
	}

	return nil
}
//--------------------------------------------------------------------
func (this *IDLDefinition) AddModule(m *ModuleDefinition) {
	if this.Modules == nil{
		this.Modules = make([]*ModuleDefinition, 0)
	}

	this.Modules = append(this.Modules, m)
}
//--------------------------------------------------------------------
