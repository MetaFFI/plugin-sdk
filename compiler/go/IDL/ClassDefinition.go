package IDL

//--------------------------------------------------------------------
type ClassDefinition struct {
	Name         string                   `json:"name"`
	Comment      string                   `json:"comment"`
	Tags         map[string]string        `json:"tags"`
	EntityPath   map[string]string        `json:"entity_path"`
	Constructors []*ConstructorDefinition `json:"constructors"`
	Releaser     *ReleaseDefinition       `json:"release"`
	Methods      []*MethodDefinition      `json:"methods"`
	Fields       []*FieldDefinition       `json:"fields"`
}

//--------------------------------------------------------------------
func NewClassDefinition(name string) *ClassDefinition {
	c := &ClassDefinition{
		Name:         name,
		Comment:      "",
		Tags:         make(map[string]string),
		Constructors: make([]*ConstructorDefinition, 0),
		Releaser:     nil,
		Methods:      make([]*MethodDefinition, 0),
		Fields:       make([]*FieldDefinition, 0),
	}

	c.Releaser = NewReleaserDefinition(c, "Release"+c.Name)

	return c
}

//--------------------------------------------------------------------
func (this *ClassDefinition) SetTag(tag string, val string) {

	if this.Tags == nil {
		this.Tags = make(map[string]string)
	}

	this.Tags[tag] = val
}

//--------------------------------------------------------------------
func (this *ClassDefinition) AppendComment(comment string) {
	if this.Comment != "" {
		this.Comment += "\n"
	}

	this.Comment += comment
}

//--------------------------------------------------------------------
func (this *ClassDefinition) AddConstructor(definition *ConstructorDefinition) {
	definition.SetParent(this)
	this.Constructors = append(this.Constructors, definition)
}

//--------------------------------------------------------------------
func (this *ClassDefinition) AddMethod(def *MethodDefinition) {
	def.SetParent(this)
	this.Methods = append(this.Methods, def)
}

//--------------------------------------------------------------------
func (this *ClassDefinition) SetEntityPath(key string, val string) {

	if this.EntityPath == nil {
		this.EntityPath = make(map[string]string)
	}

	this.EntityPath[key] = val

	for _, f := range this.Fields {
		if f.Getter != nil {
			f.Getter.SetEntityPath(key, val)
		}
		if f.Setter != nil {
			f.Setter.SetEntityPath(key, val)
		}
	}

	for _, cstr := range this.Constructors {
		cstr.SetEntityPath(key, val)
	}

	if this.Releaser != nil {
		this.Releaser.SetEntityPath(key, val)
	}

	for _, m := range this.Methods {
		m.SetEntityPath(key, val)
	}
}

//--------------------------------------------------------------------
func (this *ClassDefinition) AddField(f *FieldDefinition) {
	this.Fields = append(this.Fields, f)
}

//--------------------------------------------------------------------
