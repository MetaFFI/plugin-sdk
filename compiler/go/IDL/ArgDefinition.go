package IDL

import (
	"fmt"
	"strings"
)

type ArgDefinition struct{
	Name string `json:"name"`
	Type MetaFFIType `json:"type"`
	TypeAlias string `json:"type_alias"`
	Comment string `json:"comment"`
	Tags map[string]string `json:"tags"`
	Dimensions int `json:"dimensions"`
}
//--------------------------------------------------------------------
func NewArgDefinition(name string, ffiType MetaFFIType) *ArgDefinition {
	return NewArgDefinitionWithAlias(name, ffiType, "")
}
//--------------------------------------------------------------------
func NewArgDefinitionWithAlias(name string, ffiType MetaFFIType, alias string) *ArgDefinition {
	return &ArgDefinition{
		Name:       name,
		Type:       ffiType,
		TypeAlias:  alias,
		Comment:    "",
		Tags:       make(map[string]string),
		Dimensions: 0,
	}
}
//--------------------------------------------------------------------
func NewArgArrayDefinition(name string, ffiType MetaFFIType, dimensions int) *ArgDefinition {
	return NewArgArrayDefinitionWithAlias(name, ffiType, dimensions, "")
}
//--------------------------------------------------------------------
func NewArgArrayDefinitionWithAlias(name string, ffiType MetaFFIType, dimensions int, alias string) *ArgDefinition {
	f := NewArgDefinitionWithAlias(name, ffiType, alias)
	f.Dimensions = dimensions
	return f
}
//--------------------------------------------------------------------
func (this *ArgDefinition) SetAlias(alias string){
	this.TypeAlias = alias
}
//--------------------------------------------------------------------
func (this *ArgDefinition) GetTypeOrAlias() string{
	if this.TypeAlias != ""{
		return this.TypeAlias
	} else {
		return string(this.Type)
	}
}
//--------------------------------------------------------------------
func (this *ArgDefinition) IsTypeAlias() bool{
	return this.TypeAlias != ""
}
//--------------------------------------------------------------------
func (this *ArgDefinition) IsString() bool{
	return strings.Index(string(this.Type), "string") == 0
}
//--------------------------------------------------------------------
func (this *ArgDefinition) IsHandle() bool{
	return this.Type == HANDLE
}
//--------------------------------------------------------------------
func (this *ArgDefinition) IsAny() bool{
	return this.Type == ANY
}
//--------------------------------------------------------------------
func (this *ArgDefinition) IsBool() bool{
	return this.Type == BOOL
}
//--------------------------------------------------------------------
func (this *ArgDefinition) IsArray() bool{
	return this.Dimensions > 0
}
//--------------------------------------------------------------------
func (this *ArgDefinition) SetTag(tag string, val string){

	if this.Tags == nil{
		this.Tags = make(map[string]string)
	}

	this.Tags[tag] = val
}
//--------------------------------------------------------------------
func (this *ArgDefinition) AppendComment(comment string){
	if this.Comment != "" {
		this.Comment += "\n"
	}

	this.Comment += comment
}
//--------------------------------------------------------------------
func (this *ArgDefinition) parseWellKnownTags() error{

	for tagName, tagVal := range this.Tags{
		switch tagName {
		case FUNCTION_PATH:
			return fmt.Errorf("arg level cannot hold metaffi_function_path")

		case TYPE_ALIAS:
			this.TypeAlias = tagVal
		}
	}

	return nil
}
//--------------------------------------------------------------------
func (this *ArgDefinition) IsInteger() bool{
	return strings.Index(string(this.Type), "int") == 0
}
//--------------------------------------------------------------------
