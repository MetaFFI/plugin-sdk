package compiler

import "runtime"
import "github.com/MetaFFI/plugin-sdk/compiler/go/IDL"

//--------------------------------------------------------------------
func GetDynamicLibSuffix() string{

	switch runtime.GOOS{
		case "windows": return ".dll"
		case "darwin": return ".dylib"
		default: // We might need to make this more specific in the future
			return ".so"
	}
}
//--------------------------------------------------------------------
// To make sure IDL does not contain names of functions, methods, classes, arguments and return values
// that equals to keywords, for each name that matches a keyword, it calls to modifyKeyword function
// The value of keywords map is ignores. The map is used as a set.
func ModifyKeywords(definition *IDL.IDLDefinition, keywords map[string]bool, modifyKeyword func(string)string) {

	if definition == nil || definition.Modules == nil {
		return
	}

	for _, m := range definition.Modules {
		if m.Functions != nil {
			for _, f := range m.Functions {
				_, exists := keywords[f.Name]
				if exists {
					f.Name = modifyKeyword(f.Name)
				}

				for _, p := range f.Parameters {
					_, exists = keywords[p.Name]
					if exists {
						p.Name = modifyKeyword(p.Name)
					}
				}

				for _, p := range f.ReturnValues {
					_, exists = keywords[p.Name]
					if exists {
						p.Name = modifyKeyword(p.Name)
					}
				}
			}
		}

		if m.Classes != nil {
			for _, c := range m.Classes {

				for _, f := range c.Fields {
	                if f.Getter != nil {
	                    _, exists := keywords[f.Getter.Name]
	                    if exists {
	                        f.Getter.Name = modifyKeyword(f.Getter.Name)
	                    }

	                    for _, p := range f.Getter.Parameters {
	                        _, exists = keywords[p.Name]
	                        if exists {
	                            p.Name = modifyKeyword(p.Name)
	                        }
	                    }

	                    for _, p := range f.Getter.ReturnValues {
	                        _, exists = keywords[p.Name]
	                        if exists {
	                            p.Name = modifyKeyword(p.Name)
	                        }
	                    }
	                }
	                if f.Setter != nil {
	                    _, exists := keywords[f.Setter.Name]
	                    if exists {
	                        f.Setter.Name = modifyKeyword(f.Setter.Name)
	                    }

	                    for _, p := range f.Setter.Parameters {
	                        _, exists = keywords[p.Name]
	                        if exists {
	                            p.Name = modifyKeyword(p.Name)
	                        }
	                    }

	                    for _, p := range f.Setter.ReturnValues {
	                        _, exists = keywords[p.Name]
	                        if exists {
	                            p.Name = modifyKeyword(p.Name)
	                        }
	                    }
	                }
	            }

				if c.Constructors != nil {
					for _, cstr := range c.Constructors {
						_, exists := keywords[cstr.Name]
						if exists {
							cstr.Name = modifyKeyword(cstr.Name)
						}

						for _, p := range cstr.Parameters {
							_, exists = keywords[p.Name]
							if exists {
								p.Name = modifyKeyword(p.Name)
							}
						}

						for _, p := range cstr.ReturnValues {
							_, exists = keywords[p.Name]
							if exists {
								p.Name = modifyKeyword(p.Name)
							}
						}
					}
				}

				if c.Methods != nil {
					for _, meth := range c.Methods {
						_, exists := keywords[meth.Name]
						if exists {
							meth.Name = modifyKeyword(meth.Name)
						}

						for _, p := range meth.Parameters {
							_, exists = keywords[p.Name]
							if exists {
								p.Name = modifyKeyword(p.Name)
							}
						}

						for _, p := range meth.ReturnValues {
							_, exists = keywords[p.Name]
							if exists {
								p.Name = modifyKeyword(p.Name)
							}
						}
					}
				}

				if c.Releaser != nil {
					_, exists := keywords[c.Releaser.Name]
					if exists {
						c.Releaser.Name = modifyKeyword(c.Releaser.Name)
					}

					for _, p := range c.Releaser.Parameters {
						_, exists = keywords[p.Name]
						if exists {
							p.Name = modifyKeyword(p.Name)
						}
					}

					for _, p := range c.Releaser.ReturnValues {
						_, exists = keywords[p.Name]
						if exists {
							p.Name = modifyKeyword(p.Name)
						}
					}
				}
			}
		}
	}
}
//--------------------------------------------------------------------