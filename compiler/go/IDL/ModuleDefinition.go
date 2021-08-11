package IDL

type ModuleDefinition struct{
	Name string `json:"name"`
	Comment string `json:"comment"`
	Tags map[string]string `json:"tags"`
	Functions []*FunctionDefinition `json:"functions"`
	Classes           []*ClassDefinition `json:"classes"`
	Globals           []*GlobalDefinition `json:"globals"`
	ExternalResources []string           `json:"external_resources"`
}
//--------------------------------------------------------------------
func NewModuleDefinition(name string, targetLanguage string) *ModuleDefinition{
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
func (this *ModuleDefinition) SetTag(tag string, val string){

	if this.Tags == nil{
		this.Tags = make(map[string]string)
	}

	this.Tags[tag] = val
}
//--------------------------------------------------------------------
func (this *ModuleDefinition) AppendComment(comment string){
	if this.Comment != "" {
		this.Comment += "\n"
	}

	this.Comment += comment
}
//--------------------------------------------------------------------
func (this *ModuleDefinition) parseWellKnownTags(pathToFunction map[string]string) error{

	var err error
	for tagName, tagVal := range this.Tags{
		switch tagName {
		case FUNCTION_PATH:
			pathToFunction, err = parseFunctionPath(tagVal, pathToFunction)
			if err != nil{ return err }
		}
	}

	for _, f := range this.Functions{

		functionPathToFunction := copyMap(pathToFunction)

		err := f.ParseWellKnownTags(functionPathToFunction)
		if err != nil{
			return err
		}
	}

	return nil
}
//--------------------------------------------------------------------
func (this *ModuleDefinition) AddFunction(f *FunctionDefinition) {
	this.Functions = append(this.Functions, f)
}
//--------------------------------------------------------------------
func (this *ModuleDefinition) AddClass(c *ClassDefinition) {
	this.Classes = append(this.Classes, c)
}
//--------------------------------------------------------------------
func (this *ModuleDefinition) AddGlobal(c *GlobalDefinition) {
	this.Globals = append(this.Globals, c)
}
//--------------------------------------------------------------------
func (this *ModuleDefinition) AddExternalResource(r string) {
	this.ExternalResources = append(this.ExternalResources, r)
}
//--------------------------------------------------------------------
func (this *ModuleDefinition) SetFunctionPath(key string, val string) {

	for _, f := range this.Functions{
		f.SetFunctionPath(key, val)
	}

	for _, c := range this.Classes{
		c.SetFunctionPath(key, val)
	}

}
//--------------------------------------------------------------------