package IDL


//--------------------------------------------------------------------
type MethodDefinition struct{
	FunctionDefinition
	InstanceRequired bool `json:"instance_required"`
	parent *ClassDefinition
}
//--------------------------------------------------------------------
func NewMethodDefinition(parent *ClassDefinition, name string, instanceRequired bool) *MethodDefinition{
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
func (this *MethodDefinition) IsMethod() bool{
	return true
}
//--------------------------------------------------------------------
func (this *MethodDefinition) AddParameter(definition *ArgDefinition) *MethodDefinition{
	this.FunctionDefinition.AddParameter(definition)
	return this
}
//--------------------------------------------------------------------
func (this *MethodDefinition) AddReturnValues(definition *ArgDefinition) *MethodDefinition{
	this.FunctionDefinition.AddReturnValues(definition)
	return this
}
//--------------------------------------------------------------------

