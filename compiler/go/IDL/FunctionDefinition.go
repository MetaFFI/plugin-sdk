package IDL

import (
	"fmt"
	"sort"
	"strconv"
)

type FunctionDefinition struct {
	Name          string            `json:"name"`
	Comment       string            `json:"comment"`
	Tags          map[string]string `json:"tags"`
	EntityPath    map[string]string `json:"entity_path"`
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
		EntityPath:   make(map[string]string),
		Parameters:   make([]*ArgDefinition, 0),
		ReturnValues: make([]*ArgDefinition, 0),
	}
}

// --------------------------------------------------------------------
func DefaultParamComparer(left *ArgDefinition, right *ArgDefinition) bool {
	return left.Type == right.Type && left.Dimensions == right.Dimensions
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) GetFirstIndexOfOptionalParameter() int {

	for i, p := range this.Parameters {
		if p.IsOptional {
			return i
		}
	}
	return -1
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) Duplicate() *FunctionDefinition {
	dupFunc := FunctionDefinition{
		Name:          this.Name,
		Comment:       this.Comment,
		OverloadIndex: this.OverloadIndex,
		Tags:          make(map[string]string),
		EntityPath:    make(map[string]string),
		Parameters:    make([]*ArgDefinition, 0),
		ReturnValues:  make([]*ArgDefinition, 0),
	}

	for k, v := range this.Tags {
		dupFunc.Tags[k] = v
	}

	for k, v := range this.EntityPath {
		dupFunc.EntityPath[k] = v
	}

	for _, p := range this.Parameters {
		dupArg := p.Duplicate()
		dupFunc.Parameters = append(dupFunc.Parameters, dupArg)
	}

	for _, p := range this.ReturnValues {
		dupArg := p.Duplicate()
		dupFunc.ReturnValues = append(dupFunc.ReturnValues, dupArg)
	}

	return &dupFunc
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) EqualsSignature(f *FunctionDefinition, paramComparer func(left *ArgDefinition, right *ArgDefinition) bool) bool {

	if this.Name != f.Name {
		return false
	}

	if len(this.Parameters) != len(f.Parameters) || len(this.ReturnValues) != len(f.ReturnValues) {
		return false
	}

	for i, p := range this.Parameters {
		if !paramComparer(p, f.Parameters[i]) {
			return false
		}
	}

	for i, p := range this.ReturnValues {
		if paramComparer(p, f.ReturnValues[i]) {
			return false
		}
	}

	return true
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) GetNameWithOverloadIndex() string {
	return this.Name + this.GetOverloadIndexIfExists()
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) GetOverloadIndexIfExists() string {
	if this.OverloadIndex > 0 {
		return "_overload" + strconv.Itoa(int(this.OverloadIndex))
	} else {
		return ""
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
func (this *FunctionDefinition) EntityPathAsString(definition *IDLDefinition) string {

	res := ""

	keys := make(map[string]bool)
	for k := range this.EntityPath {
		keys[k] = true
	}
	keys["metaffi_guest_lib"] = true

	sortedKeys := make([]string, 0, len(keys))
	for k := range keys {
		sortedKeys = append(sortedKeys, k)
	}
	sort.Strings(sortedKeys)

	for _, k := range sortedKeys {
		v, found := this.EntityPath[k]

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
func (this *FunctionDefinition) SetEntityPath(key string, val string) {
	this.EntityPath[key] = val
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) GetEntityPath(key string) string {
	val, found := this.EntityPath[key]
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
	return this.GetNameWithOverloadIndex() + "ID"
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) GetParametersMetaFFITypeInfo() []MetaFFITypeInfo {
	if this.Parameters == nil {
		return nil
	}

	res := make([]MetaFFITypeInfo, 0, len(this.Parameters))
	for _, a := range this.Parameters {
		res = append(res, a.GetMetaFFITypeAlias())
	}
	return res
}

// --------------------------------------------------------------------
func (this *FunctionDefinition) GetReturnValuesMetaFFITypeInfo() []MetaFFITypeInfo {
	if this.ReturnValues == nil {
		return nil
	}

	res := make([]MetaFFITypeInfo, 0, len(this.ReturnValues))
	for _, a := range this.ReturnValues {
		res = append(res, a.GetMetaFFITypeAlias())
	}
	return res
}
