package IDL

import (
	"fmt"
	"sort"
)

//--------------------------------------------------------------------
type MethodDefinition struct {
	FunctionDefinition
	InstanceRequired bool `json:"instance_required"`
	parent           *ClassDefinition
}

//--------------------------------------------------------------------
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
	m.Parameters = append([]*ArgDefinition{NewArgDefinition("this_instance", HANDLE)}, m.Parameters...)
	
	return m
}

//--------------------------------------------------------------------
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
	m.AddParameter(NewArgDefinition("this_instance", HANDLE))
	
	return m
}

//--------------------------------------------------------------------
func (this *MethodDefinition) IsMethod() bool {
	return true
}

//--------------------------------------------------------------------
func (this *MethodDefinition) AddParameter(definition *ArgDefinition) *MethodDefinition {
	this.FunctionDefinition.AddParameter(definition)
	return this
}

//--------------------------------------------------------------------
func (this *MethodDefinition) AddReturnValues(definition *ArgDefinition) *MethodDefinition {
	this.FunctionDefinition.AddReturnValues(definition)
	return this
}

//--------------------------------------------------------------------
func (this *MethodDefinition) GetEntityIDName() string {
	if this.parent == nil {
		panic(fmt.Sprintf("parent class is not set for method %v", this.Name))
	}
	
	return this.parent.Name + "_" + this.Name + "ID"
}

//--------------------------------------------------------------------
func (this *MethodDefinition) FunctionPathAsString() string {
	
	res := ""
	
	sortedKeys := make([]string, 0, len(this.FunctionPath))
	for k := range this.FunctionPath {
		sortedKeys = append(sortedKeys, k)
	}
	
	sort.Strings(sortedKeys)
	
	for _, k := range sortedKeys {
		v := this.FunctionPath[k]
		if res != "" {
			res += ","
		}
		res += fmt.Sprintf("%v=%v", k, v)
	}

	return res
}

//--------------------------------------------------------------------
