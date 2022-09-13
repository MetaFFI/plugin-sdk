package IDL

//--------------------------------------------------------------------
type GlobalDefinition struct {
	ArgDefinition
	Getter *FunctionDefinition `json:"getter"`
	Setter *FunctionDefinition `json:"setter"`
}

//--------------------------------------------------------------------
func NewGlobalDefinition(name string, ffiType MetaFFIType, getter string, setter string) *GlobalDefinition {
	return NewGlobalDefinitionWithAlias(name, ffiType, "", getter, setter)
}

//--------------------------------------------------------------------
func NewGlobalDefinitionWithAlias(name string, ffiType MetaFFIType, alias string, getter string, setter string) *GlobalDefinition {
	g := &GlobalDefinition{
		ArgDefinition: *NewArgDefinitionWithAlias(name, ffiType, alias),
	}
	
	if getter != "" {
		g.Getter = NewFunctionDefinition(getter).AddReturnValues(&g.ArgDefinition)
	}
	
	if setter != "" {
		g.Setter = NewFunctionDefinition(setter).AddParameter(&g.ArgDefinition)
	}
	
	return g
}

//--------------------------------------------------------------------
func (this *GlobalDefinition) GetEntityIDName() string {
	return this.Name + "ID"
}

//--------------------------------------------------------------------
