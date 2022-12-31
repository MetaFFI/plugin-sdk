package compiler

import "runtime"

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