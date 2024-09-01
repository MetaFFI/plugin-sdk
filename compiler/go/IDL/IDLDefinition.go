package IDL

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

// --------------------------------------------------------------------
// The expected syntax:
// metaffi_entity_path: "key1=val1,key2=val2...."
func parseEntityPath(pathToFunction string, pathMap map[string]string) (map[string]string, error) {

	var res map[string]string
	if pathMap == nil {
		res = make(map[string]string)
	} else {
		res = pathMap
	}

	pairs := strings.Split(pathToFunction, ",")

	if len(pairs) == 0 {
		return nil, nil
	}

	for _, pair := range pairs {
		elems := strings.Split(pair, "=")

		if len(elems) != 2 {
			return nil, fmt.Errorf("Failed parsing metaffi_entity_path tag. The pair \"%v\" is invalid.", pair)
		}

		elems[0] = strings.TrimSpace(elems[0])
		elems[1] = strings.TrimSpace(elems[1])

		if elems[0] == "" || elems[1] == "" {
			return nil, fmt.Errorf("Failed parsing metaffi_entity_path tag. The pair \"%v\" is invalid.", pair)
		}

		res[elems[0]] = elems[1]
	}

	return res, nil
}

// --------------------------------------------------------------------
func copyMap(source map[string]string) (target map[string]string) {
	target = make(map[string]string)

	if len(source) == 0 {
		return
	}

	for k, v := range source {
		target[k] = v
	}

	return
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
// --------------------------------------------------------------------
type Taggable interface {
	SetTag(tag string, val string)
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
// --------------------------------------------------------------------
type Commentable interface {
	AppendComment(comment string)
}

// --------------------------------------------------------------------
type EntityIDer interface {
	GetEntityIDName() string
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
// --------------------------------------------------------------------
type EntityPather interface {
	SetEntityPath(key string, val string)
	GetEntityPath(key string) string
}

// --------------------------------------------------------------------
type IDLDefinition struct {
	IDLSource                string              `json:"idl_source"`
	IDLExtension             string              `json:"idl_extension"`
	IDLFilenameWithExtension string              `json:"idl_filename_with_extension"`
	IDLFullPath              string              `json:"idl_full_path"`
	MetaFFIGuestLib          string              `json:"metaffi_guest_lib"`
	TargetLanguage           string              `json:"target_language"`
	Modules                  []*ModuleDefinition `json:"modules"`
}

// --------------------------------------------------------------------
func NewIDLDefinition(idlFullPath string, targetLanguage string) *IDLDefinition {
	idl := &IDLDefinition{
		IDLSource:                strings.ReplaceAll(filepath.Base(idlFullPath), filepath.Ext(idlFullPath), ""),
		IDLExtension:             filepath.Ext(idlFullPath),
		IDLFilenameWithExtension: filepath.Base(idlFullPath),
		IDLFullPath:              idlFullPath,
		TargetLanguage:           targetLanguage,
		Modules:                  nil,
	}

	idl.MetaFFIGuestLib = idl.IDLSource + "_MetaFFIGuest"

	return idl
}

// --------------------------------------------------------------------
func NewIDLDefinitionFromJSON(idlDefinitionJson string) (*IDLDefinition, error) {

	def := IDLDefinition{}
	err := json.Unmarshal([]byte(idlDefinitionJson), &def)
	if err != nil {
		return nil, fmt.Errorf("Failed to unmarshal IDL definition JSON: %v", err)
	}

	def.FinalizeConstruction()

	return &def, nil
}

// --------------------------------------------------------------------
func (this *IDLDefinition) ToJSON() (string, error) {

	jsonStr, err := json.Marshal(this)
	if err != nil {
		return "", fmt.Errorf("Failed to marshal IDL definition to JSON: %v", err)
	}

	return string(jsonStr), nil
}

// --------------------------------------------------------------------
func (this *IDLDefinition) String() string {
	res, err := this.ToJSON()
	if err != nil {
		panic(err)
	}

	return res
}

// --------------------------------------------------------------------
func (this *IDLDefinition) FinalizeConstruction() {

	for _, m := range this.Modules {

		for i, ex := range m.ExternalResources {
			m.ExternalResources[i] = os.ExpandEnv(ex)
		}

		for _, c := range m.Classes {

			for _, f := range c.Fields {

				f.SetParent(c)

				if f.Getter != nil {
					f.Getter.parent = c
				}

				if f.Setter != nil {
					f.Setter.parent = c
				}
			}

			for _, cstr := range c.Constructors {
				cstr.parent = c
			}

			for _, method := range c.Methods {
				method.parent = c
			}

			if c.Releaser != nil {
				c.Releaser.parent = c
			}
		}
	}
}

// --------------------------------------------------------------------
func (this *IDLDefinition) AddModule(m *ModuleDefinition) {
	if this.Modules == nil {
		this.Modules = make([]*ModuleDefinition, 0)
	}

	this.Modules = append(this.Modules, m)
}

// --------------------------------------------------------------------
// in case a name is a keyword, it must be replaced
// use this method to replace keywords
func (this *IDLDefinition) ReplaceKeywords(mapping map[string]string) {

	for _, m := range this.Modules {
		for _, g := range m.Globals {
			if _, found := mapping[g.Name]; found {
				g.Name = mapping[g.Name]
			}

			if g.Getter != nil {

				if _, found := mapping[g.Getter.Name]; found {
					g.Getter.Name = mapping[g.Getter.Name]
				}

				for _, p := range g.Getter.Parameters {
					if _, found := mapping[p.Name]; found {
						p.Name = mapping[p.Name]
					}
				}

				for _, r := range g.Getter.ReturnValues {
					if _, found := mapping[r.Name]; found {
						r.Name = mapping[r.Name]
					}
				}
			}

			if g.Setter != nil {

				if _, found := mapping[g.Setter.Name]; found {
					g.Name = mapping[g.Setter.Name]
				}

				for _, p := range g.Setter.Parameters {
					if _, found := mapping[p.Name]; found {
						p.Name = mapping[p.Name]
					}
				}

				for _, r := range g.Setter.ReturnValues {
					if _, found := mapping[r.Name]; found {
						r.Name = mapping[r.Name]
					}
				}
			}
		}

		for _, f := range m.Functions {
			if _, found := mapping[f.Name]; found {
				f.Name = mapping[f.Name]
			}

			for _, p := range f.Parameters {
				if _, found := mapping[p.Name]; found {
					p.Name = mapping[p.Name]
				}
			}

			for _, r := range f.ReturnValues {
				if _, found := mapping[r.Name]; found {
					r.Name = mapping[r.Name]
				}
			}
		}

		for _, c := range m.Classes {

			if _, found := mapping[c.Name]; found {
				c.Name = mapping[c.Name]
			}

			for _, f := range c.Fields {
				if _, found := mapping[f.Name]; found {
					f.Name = mapping[f.Name]
				}

				if f.Getter != nil {

					if _, found := mapping[f.Getter.Name]; found {
						f.Getter.Name = mapping[f.Getter.Name]
					}

					for _, p := range f.Getter.Parameters {
						if _, found := mapping[p.Name]; found {
							p.Name = mapping[p.Name]
						}
					}

					for _, r := range f.Getter.ReturnValues {
						if _, found := mapping[r.Name]; found {
							r.Name = mapping[r.Name]
						}
					}
				}

				if f.Setter != nil {

					if _, found := mapping[f.Setter.Name]; found {
						f.Setter.Name = mapping[f.Setter.Name]
					}

					for _, p := range f.Setter.Parameters {
						if _, found := mapping[p.Name]; found {
							p.Name = mapping[p.Name]
						}
					}

					for _, r := range f.Setter.ReturnValues {
						if _, found := mapping[r.Name]; found {
							r.Name = mapping[r.Name]
						}
					}
				}
			}

			for _, m := range c.Methods {

				if _, found := mapping[m.Name]; found {
					m.Name = mapping[m.Name]
				}

				for _, p := range m.Parameters {
					if _, found := mapping[p.Name]; found {
						p.Name = mapping[p.Name]
					}
				}

				for _, r := range m.ReturnValues {
					if _, found := mapping[r.Name]; found {
						r.Name = mapping[r.Name]
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------
