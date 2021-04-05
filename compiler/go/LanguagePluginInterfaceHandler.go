package compiler
/*
#include "../language_plugin_interface.h"
 */
import "C"
import (
	"encoding/json"
	"strings"
)

var languagePluginInterfaceHandler *LanguagePluginInterfaceHandler

type LanguagePluginInterface interface{
	CompileToGuest(idlDefinition *IDLDefinition, outputPath string, serializationCode map[string]string) error
	CompileFromHost(idlDefinition *IDLDefinition, outputPath string, serializationCode map[string]string, hostOptions map[string]string) error
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
															host_options *C.char, host_options_length C.int,
															out_err **C.char, out_err_len *C.uint){
	def, err := NewIDLDefinition(C.GoStringN(idl_def_json, C.int(idl_def_json_length)))
	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}

	outPath := C.GoStringN(output_path, C.int(output_path_length))
	serializationCode := C.GoStringN(serialization_code, C.int(serialization_code_length))
	hostOptions := C.GoStringN(host_options, C.int(host_options_length))

	// parse serialization code
	serializationCodeFiles := make(map[string]string)
	err = json.Unmarshal([]byte(serializationCode), &serializationCodeFiles)
	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}

	// parse hostOptions
	hostOptionsMap := make(map[string]string)
	if strings.TrimSpace(hostOptions) != ""{
		options := strings.Split(hostOptions, ",")
		for _, option := range options{
			keyval := strings.Split(option, "=")
			if len(keyval) != 2{
				msg := "Host options are invalid"
				*out_err = C.CString(msg)
				*out_err_len = C.uint(len(msg))
				return
			}

			hostOptionsMap[keyval[0]] = keyval[1]
		}
	}

	err = this.wrapped.CompileFromHost(def, outPath, serializationCodeFiles, hostOptionsMap)

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
	host_options *C.char, host_options_length C.int,
	out_err **C.char, out_err_len *C.uint){

	if languagePluginInterfaceHandler == nil{
		panic("languagePluginInterfaceHandler is null!")
	}

	languagePluginInterfaceHandler.compile_from_host(idl_def_json, idl_def_json_length, output_path, output_path_length, serialization_code, serialization_code_length, host_options, host_options_length, out_err, out_err_len)
}
//--------------------------------------------------------------------