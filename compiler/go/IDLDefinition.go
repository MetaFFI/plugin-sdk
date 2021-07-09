package compiler

import (
	"encoding/json"
	"fmt"
	"strings"
)

//--------------------------------------------------------------------
// The expected syntax:
// openffi_function_path: "key1=val1,key2=val2...."
func parsePathToFunction(pathToFunction string, pathMap map[string]string) (map[string]string, error){

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
			return nil, fmt.Errorf("Failed parsing openffi_function_path tag. The pair \"%v\" is invalid.", pair)
		}

		elems[0] = strings.TrimSpace(elems[0])
		elems[1] = strings.TrimSpace(elems[1])

		if elems[0] == "" || elems[1] == ""{
			return nil, fmt.Errorf("Failed parsing openffi_function_path tag. The pair \"%v\" is invalid.", pair)
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
//--------------------------------------------------------------------
//--------------------------------------------------------------------
type IDLDefinition struct {
	IDLFilename string `json:"idl_filename"`
	IDLExtension string `json:"idl_extension"`
	IDLFilenameWithExtension string `json:"idl_filename_with_extension"`
	IDLFullPath string `json:"idl_full_path"`
	Modules []*ModuleDefinition `json:"modules"`
}
//--------------------------------------------------------------------
func NewIDLDefinition(idlDefinitionJson string) (*IDLDefinition, error){

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
func (this *IDLDefinition) AddWellKnownFunctionPath(){
	for _, m := range this.Modules{
		for _, f := range m.Functions{
			f.PathToForeignFunction[OPENFFI_GUEST_LIB] = m.Name+"_OpenFFIGuest"
			f.PathToForeignFunction[ENTRYPOINT_FUNCTION] = "EntryPoint_"+f.PathToForeignFunction[FOREIGN_FUNCTION_NAME]
			f.PathToForeignFunction[ENTRYPOINT_CLASS] = m.Name
		}
	}
}
//--------------------------------------------------------------------
/*
The function iterates the definition and fills fields according to well-known tags.

Supported tags:
//openffi_target_language
//openffi_function_path
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
/*
Validates:
- Target Language is set
- Every function has a path
 */
func (this *IDLDefinition) Validate() error{

	// validate function path is set for every function
	for _, m := range this.Modules{

		// validate Target language exists
		if m.TargetLanguage == ""{
			return fmt.Errorf("TargetLanguage is missing for module: %v", m.Name)
		}

		for _, f := range m.Functions{
			if len(f.PathToForeignFunction) == 0{
				return fmt.Errorf("Path to foreign function is not set to function %v", f.Name)
			}
		}
	}

	return nil
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
type ModuleDefinition struct{
	Name string `json:"name"`
	TargetLanguage string `json:"target_language"`
	Comment string `json:"comment"`
	TypeAliases map[string]OpenFFIType `json:"type_aliases"` // aliases can be used as parameters in functions
	Tags map[string]string `json:"tags"`
	Functions []*FunctionDefinition `json:"functions"`
}
func (this *ModuleDefinition) SetTag(tag string, val string){

	if this.Tags == nil{
		this.Tags = make(map[string]string)
	}

	this.Tags[tag] = val
}
func (this *ModuleDefinition) AppendComment(comment string){
	this.Comment += (comment + "\n")
}
//--------------------------------------------------------------------
func (this *ModuleDefinition) parseWellKnownTags(pathToFunction map[string]string) error{

	var err error
	for tagName, tagVal := range this.Tags{
		switch tagName {
			case FUNCTION_PATH:
				pathToFunction, err = parsePathToFunction(tagVal, pathToFunction)
				if err != nil{ return err }

			case TARGET_LANGUAGE:
				this.TargetLanguage = strings.TrimSpace(tagVal)
		}
	}

	for _, f := range this.Functions{

		functionPathToFunction := copyMap(pathToFunction)

		err := f.parseWellKnownTags(functionPathToFunction)
		if err != nil{
			return err
		}
	}

	return nil
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
type FunctionDefinition struct {
	Name                  string             `json:"name"`
	Comment               string             `json:"comment"`
	Operator              Operator           `json:"operator"` // in case function is an operator overloading
	Type                  FunctionType       `json:"function_type"`
	Tags                  map[string]string  `json:"tags"`
	PathToForeignFunction map[string]string  `json:"path_to_foreign_function"`
	Parameters            []*FieldDefinition `json:"parameters"`
	ReturnValues          []*FieldDefinition `json:"return_values"`
}
func (this *FunctionDefinition) SetTag(tag string, val string){

	if this.Tags == nil{
		this.Tags = make(map[string]string)
	}

	this.Tags[tag] = val
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) AppendComment(comment string){
	this.Comment += (comment + "\n")
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) PathToForeignFunctionAsString() string{

	res := ""

	for k, v := range this.PathToForeignFunction{

		if res != ""{ res += "," }
		res += fmt.Sprintf("%v=%v", k, v)
	}

	return res
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) parseWellKnownTags(pathToFunction map[string]string) error{

	var err error
	for tagName, tagVal := range this.Tags{
		switch tagName {
			case FUNCTION_PATH:
				pathToFunction, err = parsePathToFunction(tagVal, pathToFunction)
				if err != nil{ return err }
		}
	}

	// set function path to function definition
	this.PathToForeignFunction = copyMap(pathToFunction)

	for _, p := range this.Parameters{
		err := p.parseWellKnownTags()
		if err != nil{
			return err
		}
	}

	for _, r := range this.ReturnValues{
		err := r.parseWellKnownTags()
		if err != nil{
			return err
		}
	}

	return nil
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
type FieldDefinition struct{
	Name string `json:"name"`
	Type OpenFFIType `json:"type"`
	Comment string `json:"comment"`
	Tags map[string]string `json:"tags"`
	Dimensions int `json:"dimensions"`
}
func (this *FieldDefinition) IsString() bool{
	return strings.Index(string(this.Type), "string") == 0
}

func (this *FieldDefinition) IsBool() bool{
	return this.Type == "bool"
}

func (this *FieldDefinition) IsArray() bool{
	return this.Dimensions > 0
}

func (this *FieldDefinition) SetTag(tag string, val string){

	if this.Tags == nil{
		this.Tags = make(map[string]string)
	}

	this.Tags[tag] = val
}
func (this *FieldDefinition) AppendComment(comment string){
	this.Comment += (comment + "\n")
}
//--------------------------------------------------------------------
func (this *FieldDefinition) parseWellKnownTags() error{

	for tagName, _ := range this.Tags{
		switch tagName {
			case FUNCTION_PATH:
				return fmt.Errorf("field level cannot hold openffi_function_path")
		}
	}

	return nil
}
//--------------------------------------------------------------------
func (this *FieldDefinition) IsInteger() bool{
	return strings.Index(string(this.Type), "int") == 0
}
//--------------------------------------------------------------------