package IDL

import "fmt"

type FunctionDefinition struct {
	Name string `json:"name"`
	Comment string `json:"comment"`
	Tags map[string]string `json:"tags"`
	FunctionPath map[string]string `json:"function_path"`
	Parameters []*ArgDefinition    `json:"parameters"`
	ReturnValues []*ArgDefinition  `json:"return_values"`
}
//--------------------------------------------------------------------
func NewFunctionDefinition(name string) *FunctionDefinition{
	return &FunctionDefinition{
		Name:         name,
		Comment:      "",
		Tags:         make(map[string]string),
		FunctionPath: make(map[string]string),
		Parameters:   make([]*ArgDefinition, 0),
		ReturnValues: make([]*ArgDefinition, 0),
	}
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) AddParameter(definition *ArgDefinition) *FunctionDefinition{
	this.Parameters = append(this.Parameters, definition)
	return this
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) AddReturnValues(definition *ArgDefinition) *FunctionDefinition{
	this.ReturnValues = append(this.ReturnValues, definition)
	return this
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) SetTag(tag string, val string){

	if this.Tags == nil{
		this.Tags = make(map[string]string)
	}

	this.Tags[tag] = val
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) AppendComment(comment string){
	if this.Comment != "" {
		this.Comment += "\n"
	}

	this.Comment += comment
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) FunctionPathAsString() string{

	res := ""

	for k, v := range this.FunctionPath{
		if res != ""{ res += "," }
		res += fmt.Sprintf("%v=%v", k, v)
	}

	return res
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) ParseWellKnownTags(pathToFunction map[string]string) error{

	var err error
	for tagName, tagVal := range this.Tags{
		switch tagName {
		case FUNCTION_PATH:
			pathToFunction, err = parseFunctionPath(tagVal, pathToFunction)
			if err != nil{ return err }
		}
	}

	// set function path to function definition
	this.FunctionPath = copyMap(pathToFunction)


	// parse well known tags in parameter
	for _, p := range this.Parameters{
		err := p.parseWellKnownTags()
		if err != nil{
			return err
		}
	}

	// parse well known tags in parameter
	for _, r := range this.ReturnValues{
		err := r.parseWellKnownTags()
		if err != nil{
			return err
		}
	}

	return nil
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) SetFunctionPath(key string, val string) {
	this.FunctionPath[key] = val
}
//--------------------------------------------------------------------
func (this *FunctionDefinition) IsMethod() bool{
	return false
}
//--------------------------------------------------------------------