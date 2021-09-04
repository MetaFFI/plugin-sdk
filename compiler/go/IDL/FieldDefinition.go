package IDL

//--------------------------------------------------------------------
type FieldDefinition struct {
	ArgDefinition
	Getter *MethodDefinition `json:"getter"`
	Setter *MethodDefinition `json:"setter"`
}
//--------------------------------------------------------------------
func NewFieldDefinition(cls *ClassDefinition, name string, ffiType MetaFFIType, getter string, setter string, instanceRequired bool) *FieldDefinition {
	return NewFieldDefinitionWithAlias(cls, name, ffiType, "", getter, setter, instanceRequired)
}
//--------------------------------------------------------------------
func NewFieldDefinitionWithAlias(cls *ClassDefinition, name string, ffiType MetaFFIType, alias string, getter string, setter string, instanceRequired bool) *FieldDefinition {
	g := &FieldDefinition{
		ArgDefinition: *NewArgDefinitionWithAlias(name, ffiType, alias),
	}

	if getter != ""{
		g.Getter = NewMethodDefinition(cls, getter, instanceRequired).AddReturnValues(&g.ArgDefinition)
	}

	if setter != ""{
		g.Setter = NewMethodDefinition(cls, setter, instanceRequired).AddParameter(&g.ArgDefinition)
	}

	return g
}
//--------------------------------------------------------------------
func NewFieldArrayDefinitionWithAlias(cls *ClassDefinition, name string, ffiType MetaFFIType, dimensions int, alias string, getter string, setter string, instanceRequired bool) *FieldDefinition {
	g := &FieldDefinition{
		ArgDefinition: *NewArgArrayDefinitionWithAlias(name, ffiType, dimensions, alias),
	}

	if getter != ""{
		g.Getter = NewMethodDefinition(cls, getter, instanceRequired).AddReturnValues(&g.ArgDefinition)
	}

	if setter != ""{
		g.Setter = NewMethodDefinition(cls, setter, instanceRequired).AddParameter(&g.ArgDefinition)
	}

	return g
}
//--------------------------------------------------------------------