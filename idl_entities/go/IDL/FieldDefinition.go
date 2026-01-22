package IDL

import "fmt"

//--------------------------------------------------------------------
type FieldDefinition struct {
	ArgDefinition
	Getter *MethodDefinition `json:"getter"`
	Setter *MethodDefinition `json:"setter"`
	parent *ClassDefinition
}

//--------------------------------------------------------------------
func NewFieldDefinition(cls *ClassDefinition, name string, ffiType MetaFFIType, getter string, setter string, instanceRequired bool) *FieldDefinition {
	return NewFieldDefinitionWithAlias(cls, name, ffiType, "", getter, setter, instanceRequired)
}

//--------------------------------------------------------------------
func NewFieldDefinitionWithAlias(cls *ClassDefinition, name string, ffiType MetaFFIType, alias string, getter string, setter string, instanceRequired bool) *FieldDefinition {
	g := &FieldDefinition{
		ArgDefinition: *NewArgDefinitionWithAlias(name, ffiType, alias),
		parent:        cls,
	}
	
	if getter != "" {
		g.Getter = NewMethodDefinition(cls, getter, instanceRequired).AddReturnValues(&g.ArgDefinition)
	}
	
	if setter != "" {
		g.Setter = NewMethodDefinition(cls, setter, instanceRequired).AddParameter(&g.ArgDefinition)
	}
	
	return g
}

//--------------------------------------------------------------------
func NewFieldArrayDefinitionWithAlias(cls *ClassDefinition, name string, ffiType MetaFFIType, dimensions int, alias string, getter string, setter string, instanceRequired bool) *FieldDefinition {
	g := &FieldDefinition{
		ArgDefinition: *NewArgArrayDefinitionWithAlias(name, ffiType, dimensions, alias),
		parent:        cls,
	}
	
	if getter != "" {
		g.Getter = NewMethodDefinition(cls, getter, instanceRequired).AddReturnValues(&g.ArgDefinition)
	}
	
	if setter != "" {
		g.Setter = NewMethodDefinition(cls, setter, instanceRequired).AddParameter(&g.ArgDefinition)
	}
	
	return g
}

//--------------------------------------------------------------------
func (this *FieldDefinition) GetEntityIDName() string {
	if this.parent == nil {
		panic(fmt.Sprintf("parent class is not set for field %v", this.Name))
	}
	
	return this.parent.Name + "_" + this.Name + "ID"
}
//--------------------------------------------------------------------\
func (this *FieldDefinition) SetParent(cls *ClassDefinition){
	this.parent = cls
}
//--------------------------------------------------------------------
