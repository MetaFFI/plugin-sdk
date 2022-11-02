package IDL

type ModuleDefinition struct {
	Name              string                `json:"name"`
	Comment           string                `json:"comment"`
	Tags              map[string]string     `json:"tags"`
	Functions         []*FunctionDefinition `json:"functions"`
	Classes           []*ClassDefinition    `json:"classes"`
	Globals           []*GlobalDefinition   `json:"globals"`
	ExternalResources []string              `json:"external_resources"`
}

//--------------------------------------------------------------------
func NewModuleDefinition(name string) *ModuleDefinition {
	return &ModuleDefinition{
		Name:              name,
		Tags:              make(map[string]string),
		Functions:         make([]*FunctionDefinition, 0),
		Classes:           make([]*ClassDefinition, 0),
		Globals:           make([]*GlobalDefinition, 0),
		ExternalResources: make([]string, 0),
	}
}

//--------------------------------------------------------------------
func (this *ModuleDefinition) SetTag(tag string, val string) {
	
	if this.Tags == nil {
		this.Tags = make(map[string]string)
	}
	
	this.Tags[tag] = val
}

//--------------------------------------------------------------------
func (this *ModuleDefinition) AppendComment(comment string) {
	if this.Comment != "" {
		this.Comment += "\n"
	}
	
	this.Comment += comment
}

//--------------------------------------------------------------------
func (this *ModuleDefinition) AddFunctions(fs []*FunctionDefinition) {
	for _, f := range fs {
		this.AddFunction(f)
	}
}

//--------------------------------------------------------------------
func (this *ModuleDefinition) AddFunction(f *FunctionDefinition) {
	this.Functions = append(this.Functions, f)
}

//--------------------------------------------------------------------
func (this *ModuleDefinition) AddClasses(cs []*ClassDefinition) {
	for _, c := range cs {
		this.AddClass(c)
	}
}

//--------------------------------------------------------------------
func (this *ModuleDefinition) AddClass(c *ClassDefinition) {
	this.Classes = append(this.Classes, c)
}

//--------------------------------------------------------------------
func (this *ModuleDefinition) AddGlobals(gs []*GlobalDefinition) {
	for _, g := range gs {
		this.AddGlobal(g)
	}
}

//--------------------------------------------------------------------
func (this *ModuleDefinition) AddGlobal(g *GlobalDefinition) {
	this.Globals = append(this.Globals, g)
}

//--------------------------------------------------------------------
func (this *ModuleDefinition) AddExternalResource(r string) {
	this.ExternalResources = append(this.ExternalResources, r)
}

//--------------------------------------------------------------------
func (this *ModuleDefinition) SetFunctionPath(key string, val string) {
	
	for _, g := range this.Globals {
		if g.Getter != nil {
			g.Getter.SetFunctionPath(key, val)
		}
		if g.Setter != nil {
			g.Setter.SetFunctionPath(key, val)
		}
	}
	
	for _, f := range this.Functions {
		f.SetFunctionPath(key, val)
	}
	
	for _, c := range this.Classes {
		c.SetFunctionPath(key, val)
	}
	
}

//--------------------------------------------------------------------
// get set of all contained function paths
func (this *ModuleDefinition) GetFunctionPathSet(key string) []string {
	
	resmap := make(map[string]bool)
	
	for _, g := range this.Globals {
		if g.Getter != nil {
			resmap[g.Getter.GetFunctionPath(key)] = true
		}
		if g.Setter != nil {
			resmap[g.Setter.GetFunctionPath(key)] = true
		}
	}
	
	for _, f := range this.Functions {
		resmap[f.GetFunctionPath(key)] = true
	}
	
	for _, c := range this.Classes {
		
		for _, f := range c.Fields {
			if f.Getter != nil {
				resmap[f.Getter.GetFunctionPath(key)] = true
			}
			if f.Setter != nil {
				resmap[f.Setter.GetFunctionPath(key)] = true
			}
		}
		
		for _, cstr := range c.Constructors {
			resmap[cstr.GetFunctionPath(key)] = true
		}
		
		if c.Releaser != nil {
			resmap[c.Releaser.GetFunctionPath(key)] = true
		}
		
		for _, m := range c.Methods {
			resmap[m.GetFunctionPath(key)] = true
		}
		
	}
	
	res := make([]string, 0)
	for k, _ := range resmap {
		if k != "" {
			res = append(res, k)
		}
	}
	
	return res
}

//--------------------------------------------------------------------
