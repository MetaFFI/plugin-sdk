package IDL

//--------------------------------------------------------------------
type ReleaseDefinition struct {
	MethodDefinition
}

//--------------------------------------------------------------------
func NewReleaserDefinition(parent *ClassDefinition, name string) *ReleaseDefinition {
	r := &ReleaseDefinition{
		MethodDefinition: *NewMethodDefinition(parent, name, true),
	}
	
	r.AppendComment("Releases object")
	//r.AddParameter(NewArgDefinition("this_instance", HANDLE))
	
	return r
}

//--------------------------------------------------------------------
func NewReleaserDefinitionFromFunctionDefinition(parent *ClassDefinition, f *FunctionDefinition) *ReleaseDefinition {
	return &ReleaseDefinition{MethodDefinition: *NewMethodDefinitionWithFunction(parent, f, true)}
}

//--------------------------------------------------------------------
func (this *ReleaseDefinition) IsMethod() bool {
	return false
}

//--------------------------------------------------------------------
