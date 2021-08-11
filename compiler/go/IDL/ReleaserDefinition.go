package IDL

//--------------------------------------------------------------------
type ReleaseDefinition struct{
	FunctionDefinition
}
//--------------------------------------------------------------------
func NewReleaserDefinition(name string) *ReleaseDefinition{
	r := &ReleaseDefinition{
		FunctionDefinition: *NewFunctionDefinition(name),
	}

	r.AppendComment("Releases object")
	r.AddParameter(NewArgDefinition("this_instance", HANDLE))

	return r
}
//--------------------------------------------------------------------
func (this *ReleaseDefinition) IsMethod() bool{
	return false
}
//--------------------------------------------------------------------