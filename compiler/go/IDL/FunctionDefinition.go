package IDL

import (
	"fmt"
	"sort"
)

type FunctionDefinition struct {
	Name          string            `json:"name"`
	Comment       string            `json:"comment"`
	Tags          map[string]string `json:"tags"`
	FunctionPath  map[string]string `json:"function_path"`
	Parameters    []*ArgDefinition  `json:"parameters"`
	ReturnValues  []*ArgDefinition  `json:"return_values"`
	OverloadIndex int32             `json:"overload_index"`
}

// --------------------------------------------------------------------
func NewFunctionDefinition(name string) *FunctionDefinition {
	return &FunctionDefinition{
		Name:         name,
		Comment:      "",
		Tags:         make(map[string]string),
		FunctionPath: make(map[string]string),
		Parameters:   make([]*ArgDefinition, 0),
		ReturnValues: make([]*ArgDefinition, 0),
	}
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) AddParameter(definition *ArgDefinition) *FunctionDefinition {
	this.Parameters = append(this.Parameters, definition)
	return this
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) AddReturnValues(definition *ArgDefinition) *FunctionDefinition {
	this.ReturnValues = append(this.ReturnValues, definition)
	return this
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) SetTag(tag string, val string) {

	if this.Tags == nil {
		this.Tags = make(map[string]string)
	}

	this.Tags[tag] = val
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) AppendComment(comment string) {
	if this.Comment != "" {
		this.Comment += "\n"
	}

	this.Comment += comment
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) FunctionPathAsString(definition *IDLDefinition) string {

	res := ""

	keys := make(map[string]bool)
	for k := range this.FunctionPath {
		keys[k] = true
	}
	keys["metaffi_guest_lib"] = true

	sortedKeys := make([]string, 0, len(keys))
	for k := range keys {
		sortedKeys = append(sortedKeys, k)
	}
	sort.Strings(sortedKeys)

	for _, k := range sortedKeys {
		v, found := this.FunctionPath[k]

		if !found {
			switch k {
			case "metaffi_guest_lib":
				v = definition.MetaFFIGuestLib
			default:
				panic(fmt.Sprintf("Unexpected key %v", k))

			}
		}

		if res != "" {
			res += ","
		}
		res += fmt.Sprintf("%v=%v", k, v)
	}

	return res
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) SetFunctionPath(key string, val string) {
	this.FunctionPath[key] = val
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) GetFunctionPath(key string) string {
	val, found := this.FunctionPath[key]
	if !found {
		return ""
	} else {
		return val
	}
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) IsMethod() bool {
	return false
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) GetEntityIDName() string {
	return this.Name + "ID"
}

//--------------------------------------------------------------------
