package IDL

import (
	"strings"
)

type ArgDefinition struct {
	Name       string            `json:"name"`
	Type       MetaFFIType       `json:"type"`
	TypeAlias  string            `json:"type_alias"`
	Comment    string            `json:"comment"`
	Tags       map[string]string `json:"tags"`
	Dimensions int               `json:"dimensions"`
	IsOptional bool              `json:"is_optional"`
}

// --------------------------------------------------------------------
func NewArgDefinition(name string, ffiType MetaFFIType) *ArgDefinition {
	return NewArgDefinitionWithAlias(name, ffiType, "")
}

// --------------------------------------------------------------------
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

// --------------------------------------------------------------------
func NewArgArrayDefinition(name string, ffiType MetaFFIType, dimensions int) *ArgDefinition {
	return NewArgArrayDefinitionWithAlias(name, ffiType, dimensions, "")
}

// --------------------------------------------------------------------
func NewArgArrayDefinitionWithAlias(name string, ffiType MetaFFIType, dimensions int, alias string) *ArgDefinition {
	f := NewArgDefinitionWithAlias(name, ffiType, alias)
	f.Dimensions = dimensions
	return f
}

// --------------------------------------------------------------------
func (this *ArgDefinition) Duplicate() *ArgDefinition {
	dupArg := ArgDefinition{
		Name:       this.Name,
		Type:       this.Type,
		TypeAlias:  this.TypeAlias,
		Comment:    this.Comment,
		Tags:       make(map[string]string),
		Dimensions: this.Dimensions,
		IsOptional: this.IsOptional,
	}

	for k, v := range this.Tags {
		dupArg.Tags[k] = v
	}

	return &dupArg
}

// --------------------------------------------------------------------
func (this *ArgDefinition) SetAlias(alias string) {
	this.TypeAlias = alias
}

// --------------------------------------------------------------------
func (this *ArgDefinition) GetTypeOrAlias() string {
	if this.TypeAlias != "" {
		return this.TypeAlias
	} else {
		return string(this.Type)
	}
}

// --------------------------------------------------------------------
func (this *ArgDefinition) IsTypeAlias() bool {
	return this.TypeAlias != ""
}

// --------------------------------------------------------------------
func (this *ArgDefinition) IsHandleTypeAlias() bool {
	return this.IsHandle() && this.IsTypeAlias()
}

// --------------------------------------------------------------------
func (this *ArgDefinition) IsString() bool {
	return strings.Index(string(this.Type), "string") == 0
}

// --------------------------------------------------------------------
func (this *ArgDefinition) IsHandle() bool {
	return this.Type == HANDLE || this.Type == HANDLE_ARRAY
}

// --------------------------------------------------------------------
func (this *ArgDefinition) IsAny() bool {
	return this.Type == ANY
}

// --------------------------------------------------------------------
func (this *ArgDefinition) IsBool() bool {
	return this.Type == BOOL
}

// --------------------------------------------------------------------
func (this *ArgDefinition) IsArray() bool {
	return this.Dimensions > 0
}

// --------------------------------------------------------------------
func (this *ArgDefinition) SetTag(tag string, val string) {

	if this.Tags == nil {
		this.Tags = make(map[string]string)
	}

	this.Tags[tag] = val
}

// --------------------------------------------------------------------
func (this *ArgDefinition) AppendComment(comment string) {
	if this.Comment != "" {
		this.Comment += "\n"
	}

	this.Comment += comment
}

// --------------------------------------------------------------------
func (this *ArgDefinition) IsInteger() bool {
	return strings.Index(string(this.Type), "int") == 0
}

// --------------------------------------------------------------------
func (this *ArgDefinition) GetMetaFFITypeAlias() MetaFFITypeInfo {
	res := MetaFFITypeInfo{
		StringType: this.Type,
		Alias:      this.TypeAlias,
		Type:       TypeStringToTypeEnum[this.Type],
	}

	return res
}

//--------------------------------------------------------------------
