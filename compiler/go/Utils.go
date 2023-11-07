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

	replaceIfNeeded := func(input string)string{
		_, exists := keywords[input]
        if exists {
            return modifyKeyword(input)
        }
        return input
	}

	for _, m := range definition.Modules {

	    // also globals
	    for _, f := range m.Globals {

	        f.Name = replaceIfNeeded(f.Name)

            if f.Getter != nil {
                f.Getter.Name = replaceIfNeeded(f.Getter.Name)

                for _, p := range f.Getter.Parameters {
                    p.Name = replaceIfNeeded(p.Name)
                    p.TypeAlias = replaceIfNeeded(p.TypeAlias)
                }

                for _, p := range f.Getter.ReturnValues {
                    p.Name = replaceIfNeeded(p.Name)
                    p.TypeAlias = replaceIfNeeded(p.TypeAlias)
                }
            }
            if f.Setter != nil {
                f.Setter.Name = replaceIfNeeded(f.Setter.Name)

                for _, p := range f.Setter.Parameters {
                    p.Name = replaceIfNeeded(p.Name)
                    p.TypeAlias = replaceIfNeeded(p.TypeAlias)
                }

                for _, p := range f.Setter.ReturnValues {
                    p.Name = replaceIfNeeded(p.Name)
                    p.TypeAlias = replaceIfNeeded(p.TypeAlias)
                }
            }
        }

		if m.Functions != nil {
			for _, f := range m.Functions {
				f.Name = replaceIfNeeded(f.Name)

				for _, p := range f.Parameters {
					p.Name = replaceIfNeeded(p.Name)
					p.TypeAlias = replaceIfNeeded(p.TypeAlias)
				}

				for _, p := range f.ReturnValues {
					p.Name = replaceIfNeeded(p.Name)
					p.TypeAlias = replaceIfNeeded(p.TypeAlias)
				}
			}
		}

		if m.Classes != nil {
			for _, c := range m.Classes {

				c.Name = replaceIfNeeded(c.Name)

				for _, f := range c.Fields {

	                if f.Getter != nil {
	                    f.Getter.Name = replaceIfNeeded(f.Getter.Name)

	                    for _, p := range f.Getter.Parameters {
	                        p.Name = replaceIfNeeded(p.Name)
                            p.TypeAlias = replaceIfNeeded(p.TypeAlias)
	                    }

	                    for _, p := range f.Getter.ReturnValues {
	                        p.Name = replaceIfNeeded(p.Name)
                            p.TypeAlias = replaceIfNeeded(p.TypeAlias)
	                    }
	                }
	                if f.Setter != nil {
	                    f.Setter.Name = replaceIfNeeded(f.Setter.Name)

	                    for _, p := range f.Setter.Parameters {
	                        p.Name = replaceIfNeeded(p.Name)
                            p.TypeAlias = replaceIfNeeded(p.TypeAlias)
	                    }

	                    for _, p := range f.Setter.ReturnValues {
	                        p.Name = replaceIfNeeded(p.Name)
                            p.TypeAlias = replaceIfNeeded(p.TypeAlias)
	                    }
	                }
	            }

				if c.Constructors != nil {
					for _, cstr := range c.Constructors {

						cstr.Name = replaceIfNeeded(cstr.Name)

						for _, p := range cstr.Parameters {
							p.Name = replaceIfNeeded(p.Name)
                            p.TypeAlias = replaceIfNeeded(p.TypeAlias)
						}

						for _, p := range cstr.ReturnValues {
							p.Name = replaceIfNeeded(p.Name)
                            p.TypeAlias = replaceIfNeeded(p.TypeAlias)
						}
					}
				}

				if c.Methods != nil {
					for _, meth := range c.Methods {

						meth.Name = replaceIfNeeded(meth.Name)

						for _, p := range meth.Parameters {
							p.Name = replaceIfNeeded(p.Name)
                            p.TypeAlias = replaceIfNeeded(p.TypeAlias)
						}

						for _, p := range meth.ReturnValues {
							p.Name = replaceIfNeeded(p.Name)
                            p.TypeAlias = replaceIfNeeded(p.TypeAlias)
						}
					}
				}

				if c.Releaser != nil {

					c.Releaser.Name = replaceIfNeeded(c.Releaser.Name)

					for _, p := range c.Releaser.Parameters {
						p.Name = replaceIfNeeded(p.Name)
                        p.TypeAlias = replaceIfNeeded(p.TypeAlias)
					}

					for _, p := range c.Releaser.ReturnValues {
						p.Name = replaceIfNeeded(p.Name)
                        p.TypeAlias = replaceIfNeeded(p.TypeAlias)
					}
				}
			}
		}
	}
}
//--------------------------------------------------------------------