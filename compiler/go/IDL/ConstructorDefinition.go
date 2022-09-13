package IDL

//--------------------------------------------------------------------
type ConstructorDefinition struct {
	FunctionDefinition
}

//--------------------------------------------------------------------
func NewConstructorDefinition(name string) *ConstructorDefinition {
	cstr := &ConstructorDefinition{
		FunctionDefinition: *NewFunctionDefinition(name),
	}
	
	// set return value to instance of the class
	// NOTICE: this forces that the first return value is the new instance,
	// in case something different is required, this return value need to be removed
	cstr.AddReturnValues(NewArgDefinition("new_instance", HANDLE))
	
	return cstr
}

//--------------------------------------------------------------------
func NewConstructorDefinitionFromFunctionDefinition(f *FunctionDefinition) *ConstructorDefinition {
	return &ConstructorDefinition{FunctionDefinition: *f}
}

//--------------------------------------------------------------------
func (this *ConstructorDefinition) IsMethod() bool {
	return false
}

//--------------------------------------------------------------------
