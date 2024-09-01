package IDL

import (
	"fmt"
	"sort"
)

// --------------------------------------------------------------------
type MethodDefinition struct {
	FunctionDefinition
	InstanceRequired bool `json:"instance_required"`
	parent           *ClassDefinition
}

// --------------------------------------------------------------------
func NewMethodDefinitionWithFunction(parent *ClassDefinition, function *FunctionDefinition, instanceRequired bool) *MethodDefinition {

	if parent == nil {
		panic("parent cannot be nil")
	}

	m := &MethodDefinition{
		FunctionDefinition: *function,
		InstanceRequired:   instanceRequired,
		parent:             parent,
	}

	// set first parameter as class instance
	if instanceRequired {
		m.Parameters = append([]*ArgDefinition{NewArgDefinition("this_instance", HANDLE)}, m.Parameters...)
	}

	return m
}

// --------------------------------------------------------------------
func NewMethodDefinition(parent *ClassDefinition, name string, instanceRequired bool) *MethodDefinition {

	if parent == nil {
		panic("parent cannot be nil")
	}

	m := &MethodDefinition{
		FunctionDefinition: *NewFunctionDefinition(name),
		parent:             parent,
		InstanceRequired:   instanceRequired,
	}

	// set first parameter as class instance
	if instanceRequired {
		m.AddParameter(NewArgDefinition("this_instance", HANDLE))
	}

	return m
}

// --------------------------------------------------------------------
func (this *MethodDefinition) Duplicate() *MethodDefinition {
	dupMethod := MethodDefinition{}
	dupMethod.FunctionDefinition = *this.FunctionDefinition.Duplicate()
	dupMethod.InstanceRequired = this.InstanceRequired
	dupMethod.parent = nil

	return &dupMethod
}

// --------------------------------------------------------------------
func (this *MethodDefinition) IsMethod() bool {
	return true
}

// --------------------------------------------------------------------
func (this *MethodDefinition) AddParameter(definition *ArgDefinition) *MethodDefinition {
	this.FunctionDefinition.AddParameter(definition)
	return this
}

// --------------------------------------------------------------------
func (this *MethodDefinition) AddReturnValues(definition *ArgDefinition) *MethodDefinition {
	this.FunctionDefinition.AddReturnValues(definition)
	return this
}

// --------------------------------------------------------------------
func (this *MethodDefinition) GetEntityIDName() string {
	if this.parent == nil {
		panic(fmt.Sprintf("parent class is not set for method %v", this.Name))
	}

	return this.parent.Name + "_" + this.GetNameWithOverloadIndex() + "ID"
}

// --------------------------------------------------------------------
func (this *MethodDefinition) EntityPathAsString(definition *IDLDefinition) string {

	res := ""

	keys := make(map[string]bool)
	for k := range this.EntityPath {
		keys[k] = true
	}
	for k := range this.GetParent().EntityPath {
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

		if !found { // if key not found, take from parent (notice, method has priority over parent)
			v, found = this.parent.EntityPath[k]

			if !found {
				switch k {
				case "metaffi_guest_lib":
					v = definition.MetaFFIGuestLib
				default:
					panic(fmt.Sprintf("Unexpected key %v", k))

				}
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
func (this *MethodDefinition) GetParent() *ClassDefinition {
	return this.parent
}

// --------------------------------------------------------------------
func (this *MethodDefinition) SetParent(parent *ClassDefinition) {
	this.parent = parent
}

//--------------------------------------------------------------------
