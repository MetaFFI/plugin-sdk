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

// --------------------------------------------------------------------
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

// --------------------------------------------------------------------
func (this *ModuleDefinition) IsContainsClass(clsName string) bool {
	for _, c := range this.Classes {
		if c.Name == clsName {
			return true
		}
	}

	return false
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) GetCallablesByName(name string, searchFunctions bool, searchMethods bool, searchConstructors bool) (functions []*FunctionDefinition, methods []*MethodDefinition, constructors []*ConstructorDefinition) {

	functions = make([]*FunctionDefinition, 0)
	methods = make([]*MethodDefinition, 0)
	constructors = make([]*ConstructorDefinition, 0)

	if searchFunctions {
		for _, f := range this.Functions {
			if f.Name == name {
				functions = append(functions, f)
			}
		}
	}

	if searchMethods || searchConstructors {

		for _, c := range this.Classes {

			if searchConstructors {
				for _, cstr := range c.Constructors {
					if cstr.Name == name {
						constructors = append(constructors, cstr)
					}
				}
			}

			if searchMethods {
				for _, m := range c.Methods {
					if m.Name == name {
						methods = append(methods, m)
					}
				}
			}
		}
	}

	return
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) GetCallablesWithOptionalParameters(searchFunctions bool, searchMethods bool, searchConstructors bool) (functions []*FunctionDefinition, methods []*MethodDefinition, constructors []*ConstructorDefinition) {
	functions = make([]*FunctionDefinition, 0)
	methods = make([]*MethodDefinition, 0)
	constructors = make([]*ConstructorDefinition, 0)

	isContainOptionalParameters := func(f *FunctionDefinition) bool {
		for _, p := range f.Parameters {
			if p.IsOptional {
				return true
			}
		}

		return false
	}

	if searchFunctions {
		for _, f := range this.Functions {
			if isContainOptionalParameters(f) { // get overloaded only once. index 1 states it is overloaded, but ignores all other overloads
				functions = append(functions, f)
			}
		}
	}

	if searchConstructors || searchMethods {
		for _, c := range this.Classes {
			if searchConstructors {
				for _, cstr := range c.Constructors {
					if isContainOptionalParameters(&cstr.FunctionDefinition) {
						constructors = append(constructors, cstr)
					}
				}
			}

			if searchMethods {
				for _, m := range c.Methods {
					if isContainOptionalParameters(&m.FunctionDefinition) {
						methods = append(methods, m)
					}
				}
			}
		}
	}

	return
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) GetOverloadedCallables(searchFunctions bool, searchMethods bool, searchConstructors bool) (functions [][]*FunctionDefinition, methods [][]*MethodDefinition, constructors [][]*ConstructorDefinition) {

	functions = make([][]*FunctionDefinition, 0)
	methods = make([][]*MethodDefinition, 0)
	constructors = make([][]*ConstructorDefinition, 0)

	if searchFunctions {
		for _, f := range this.Functions {
			if f.OverloadIndex == 1 { // get overloaded only once. index 1 states it is overloaded, but ignores all other overloads
				fs, _, _ := this.GetCallablesByName(f.Name, true, false, false)
				functions = append(functions, fs)
			}
		}
	}

	if searchConstructors || searchMethods {
		for _, c := range this.Classes {
			if searchConstructors {
				for _, cstr := range c.Constructors {
					if cstr.OverloadIndex == 1 {
						_, _, cstrs := this.GetCallablesByName(cstr.Name, false, false, true)
						constructors = append(constructors, cstrs)
					}
				}
			}

			if searchMethods {
				for _, m := range c.Methods {
					if m.OverloadIndex == 1 {
						_, meths, _ := this.GetCallablesByName(m.Name, false, true, false)
						methods = append(methods, meths)
					}
				}
			}
		}
	}

	return
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) SetTag(tag string, val string) {

	if this.Tags == nil {
		this.Tags = make(map[string]string)
	}

	this.Tags[tag] = val
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) AppendComment(comment string) {
	if this.Comment != "" {
		this.Comment += "\n"
	}

	this.Comment += comment
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) AddFunctions(fs []*FunctionDefinition) {
	for _, f := range fs {
		this.AddFunction(f)
	}
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) AddFunction(f *FunctionDefinition) {
	this.Functions = append(this.Functions, f)
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) AddClasses(cs []*ClassDefinition) {
	for _, c := range cs {
		this.AddClass(c)
	}
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) AddClass(c *ClassDefinition) {
	this.Classes = append(this.Classes, c)
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) AddGlobals(gs []*GlobalDefinition) {
	for _, g := range gs {
		this.AddGlobal(g)
	}
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) AddGlobal(g *GlobalDefinition) {
	this.Globals = append(this.Globals, g)
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) AddExternalResource(r string) {
	this.ExternalResources = append(this.ExternalResources, r)
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) AddExternalResourceIfNotExist(r string) {
	for _, ext := range this.ExternalResources {
		if ext == r {
			return
		}
	}

	this.ExternalResources = append(this.ExternalResources, r)
}

// --------------------------------------------------------------------
func (this *ModuleDefinition) SetEntityPath(key string, val string) {

	for _, g := range this.Globals {
		if g.Getter != nil {
			g.Getter.SetEntityPath(key, val)
		}
		if g.Setter != nil {
			g.Setter.SetEntityPath(key, val)
		}
	}

	for _, f := range this.Functions {
		f.SetEntityPath(key, val)
	}

	for _, c := range this.Classes {
		c.SetEntityPath(key, val)
	}

}

// --------------------------------------------------------------------
// get set of all contained function paths
func (this *ModuleDefinition) GetEntityPathSet(key string) []string {

	resmap := make(map[string]bool)

	for _, g := range this.Globals {
		if g.Getter != nil {
			resmap[g.Getter.GetEntityPath(key)] = true
		}
		if g.Setter != nil {
			resmap[g.Setter.GetEntityPath(key)] = true
		}
	}

	for _, f := range this.Functions {
		resmap[f.GetEntityPath(key)] = true
	}

	for _, c := range this.Classes {

		for _, f := range c.Fields {
			if f.Getter != nil {
				resmap[f.Getter.GetEntityPath(key)] = true
			}
			if f.Setter != nil {
				resmap[f.Setter.GetEntityPath(key)] = true
			}
		}

		for _, cstr := range c.Constructors {
			resmap[cstr.GetEntityPath(key)] = true
		}

		if c.Releaser != nil {
			resmap[c.Releaser.GetEntityPath(key)] = true
		}

		for _, m := range c.Methods {
			resmap[m.GetEntityPath(key)] = true
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
