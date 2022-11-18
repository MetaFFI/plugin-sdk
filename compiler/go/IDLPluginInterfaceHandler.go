package compiler

/*
#include "../idl_plugin_interface.h"
*/
import "C"
import (
	"encoding/json"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
)

var idlPluginInterfaceHandler *IDLPluginInterfaceHandler
var IDLPluginMain IDLPluginInterface

type IDLPluginInterface interface {
	ParseIDL(sourceCode string, filePath string, isEmbeddedCode bool) (*IDL.IDLDefinition, bool, error)
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
func (this *IDLPluginInterfaceHandler) parse_idl(idl_file_path *C.char, idl_file_path_length C.uint,
	idl *C.char, idl_length C.uint,
	out_idl_def_json **C.char, out_idl_def_json_length *C.uint,
	out_err **C.char, out_err_len *C.uint) {
	
	idlName := C.GoStringN(idl_file_path, C.int(idl_file_path_length))
	idlStr := C.GoStringN(idl, C.int(idl_length))
	
	idlName, err := filepath.Abs(idlName)
	if err != nil {
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
	
	// if filePath is a code block, write the code to tmp
	isEmbeddedCode := false
	isDeleteGeneratedFile := true
	if strings.Contains(idlName, "#") {
		isEmbeddedCode = true
		
		// extract file path to write source block
		sourceBlockFile := filepath.Dir(idlName) + string(os.PathSeparator)
		sourceBlockFile += idlName[strings.LastIndex(idlName, "#")+1:]
		idlName = sourceBlockFile
		
		err := ioutil.WriteFile(idlName, []byte(idlStr), 0700)
		if err != nil {
			*out_err = C.CString(err.Error())
			*out_err_len = C.uint(len(err.Error()))
			return
		}
		defer func() {
			if isDeleteGeneratedFile {
				_ = os.Remove(idlName)
			}
		}()
	}
	
	def, isDeleteGeneratedFile, err := this.wrapped.ParseIDL(idlStr, idlName, isEmbeddedCode)
	
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
func parse_idl(idl_file_path *C.char, idl_file_path_length C.uint,
	idl *C.char, idl_length C.uint,
	out_idl_def_json **C.char, out_idl_def_json_length *C.uint,
	out_err **C.char, out_err_len *C.uint) {
	
	if idlPluginInterfaceHandler == nil {
		panic("idlPluginInterfaceHandler is null!")
	}
	
	idlPluginInterfaceHandler.parse_idl(idl_file_path, idl_file_path_length, idl, idl_length, out_idl_def_json, out_idl_def_json_length, out_err, out_err_len)
}

//--------------------------------------------------------------------
