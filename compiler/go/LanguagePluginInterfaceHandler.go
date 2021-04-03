package compiler
/*
#include "../language_plugin_interface.h"
 */
import "C"
import "encoding/json"

var languagePluginInterfaceHandler *LanguagePluginInterfaceHandler

type LanguagePluginInterface interface{
	CompileToGuest(idlDefinition *IDLDefinition, outputPath string, serializationCode map[string]string) error
	CompileFromHost(idlDefinition *IDLDefinition, outputPath string, serializationCode map[string]string) error
}
//--------------------------------------------------------------------
type LanguagePluginInterfaceHandler struct{
	wrapped LanguagePluginInterface
}
//--------------------------------------------------------------------
func CreateLanguagePluginInterfaceHandler(wrapped LanguagePluginInterface){
	languagePluginInterfaceHandler = &LanguagePluginInterfaceHandler{wrapped: wrapped}
}
//--------------------------------------------------------------------
func (this *LanguagePluginInterfaceHandler) compile_to_guest(idl_def_json *C.char, idl_def_json_length C.uint,
															output_path *C.char, output_path_length C.uint,
															serialization_code *C.char, serialization_code_length C.uint,
															out_err **C.char, out_err_len *C.uint) {
	def, err := NewIDLDefinition(C.GoStringN(idl_def_json, C.int(idl_def_json_length)))
	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}

	outPath := C.GoStringN(output_path, C.int(output_path_length))
	serializationCode := C.GoStringN(serialization_code, C.int(serialization_code_length))

	serializationCodeFiles := make(map[string]string)
	err = json.Unmarshal([]byte(serializationCode), &serializationCodeFiles)
	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}

	err = this.wrapped.CompileToGuest(def, outPath, serializationCodeFiles)

	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
}
//--------------------------------------------------------------------
func (this *LanguagePluginInterfaceHandler) compile_from_host(idl_def_json *C.char, idl_def_json_length C.uint,
															output_path *C.char, output_path_length C.uint,
															serialization_code *C.char, serialization_code_length C.uint,
															out_err **C.char, out_err_len *C.uint){
	def, err := NewIDLDefinition(C.GoStringN(idl_def_json, C.int(idl_def_json_length)))
	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}

	outPath := C.GoStringN(output_path, C.int(output_path_length))
	serializationCode := C.GoStringN(serialization_code, C.int(serialization_code_length))

	serializationCodeFiles := make(map[string]string)
	err = json.Unmarshal([]byte(serializationCode), &serializationCodeFiles)
	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}

	err = this.wrapped.CompileFromHost(def, outPath, serializationCodeFiles)

	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
}
//--------------------------------------------------------------------
//export compile_to_guest
func compile_to_guest(idl_def_json *C.char, idl_def_json_length C.uint,
	output_path *C.char, output_path_length C.uint,
	serialization_code *C.char, serialization_code_length C.uint,
	out_err **C.char, out_err_len *C.uint) {

	if languagePluginInterfaceHandler == nil{
		panic("languagePluginInterfaceHandler is null!")
	}

	languagePluginInterfaceHandler.compile_to_guest(idl_def_json, idl_def_json_length, output_path, output_path_length, serialization_code, serialization_code_length, out_err, out_err_len)
}
//--------------------------------------------------------------------
//export compile_from_host
func compile_from_host(idl_def_json *C.char, idl_def_json_length C.uint,
	output_path *C.char, output_path_length C.uint,
	serialization_code *C.char, serialization_code_length C.uint,
	out_err **C.char, out_err_len *C.uint){

	if languagePluginInterfaceHandler == nil{
		panic("languagePluginInterfaceHandler is null!")
	}

	languagePluginInterfaceHandler.compile_from_host(idl_def_json, idl_def_json_length, output_path, output_path_length, serialization_code, serialization_code_length, out_err, out_err_len)
}
//--------------------------------------------------------------------