package compiler

/*
#include "../idl_plugin_interface.h"
*/
import "C"
import (
	"encoding/json"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"path/filepath"
	"os"
)

var idlPluginInterfaceHandler *IDLPluginInterfaceHandler
var IDLPluginMain IDLPluginInterface

type IDLPluginInterface interface {
	ParseIDL(sourceCode string, filePath string) (*IDL.IDLDefinition, bool, error)
}

//--------------------------------------------------------------------
type IDLPluginInterfaceHandler struct {
	wrapped IDLPluginInterface
}

//--------------------------------------------------------------------
func CreateIDLPluginInterfaceHandler(wrapped IDLPluginInterface) {
	idlPluginInterfaceHandler = &IDLPluginInterfaceHandler{wrapped: wrapped}
}

//--------------------------------------------------------------------
func (this *IDLPluginInterfaceHandler) parse_idl(source_code *C.char, source_code_length C.uint,
	file_path *C.char, file_path_length C.uint,
	out_idl_def_json **C.char, out_idl_def_json_length *C.uint,
	out_err **C.char, out_err_len *C.uint) {
	
	sourceCode := C.GoStringN(source_code, C.int(source_code_length))
	filePath := C.GoStringN(file_path, C.int(file_path_length))

	if _, err := os.Stat(filePath); err == nil{ // it is a directory or a file - pass as full absolute path
		filePath, err = filepath.Abs(filePath)
		if err != nil {
            *out_err = C.CString(err.Error())
            *out_err_len = C.uint(len(err.Error()))
            return
        }
	}
	
	// if filePath is a code block, write the code to tmp
	def, _, err := this.wrapped.ParseIDL(sourceCode, filePath)
	
	if err != nil {
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
	
	strdef, err := json.Marshal(def)
	if err != nil {
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
	
	*out_idl_def_json = C.CString(string(strdef))
	*out_idl_def_json_length = C.uint(len(string(strdef)))
}

//--------------------------------------------------------------------
//export parse_idl
func parse_idl(source_code *C.char, source_code_length C.uint,
	file_path *C.char, file_path_length C.uint,
	out_idl_def_json **C.char, out_idl_def_json_length *C.uint,
	out_err **C.char, out_err_len *C.uint) {
	
	if idlPluginInterfaceHandler == nil {
		panic("idlPluginInterfaceHandler is null!")
	}
	
	idlPluginInterfaceHandler.parse_idl(source_code, source_code_length, file_path, file_path_length, out_idl_def_json, out_idl_def_json_length, out_err, out_err_len)
}

//--------------------------------------------------------------------
