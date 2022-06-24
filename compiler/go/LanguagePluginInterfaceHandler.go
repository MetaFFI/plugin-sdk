package compiler
/*
#include "../language_plugin_interface.h"
 */
import "C"
import (
	. "github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"strings"
	"fmt"
	"time"
)

var languagePluginInterfaceHandler *LanguagePluginInterfaceHandler

type LanguagePluginInterface interface{
	CompileToGuest(idlDefinition *IDLDefinition, outputPath string, blockName string, blockCode string) error
	CompileFromHost(idlDefinition *IDLDefinition, outputPath string, hostOptions map[string]string) error
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
															block_name *C.char, block_name_length C.uint,
															block_code *C.char, block_code_length C.uint,
															out_err **C.char, out_err_len *C.uint) {
	def, err := NewIDLDefinitionFromJSON(C.GoStringN(idl_def_json, C.int(idl_def_json_length)))
	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}

	outPath := C.GoStringN(output_path, C.int(output_path_length))
	blockName := C.GoStringN(block_name, C.int(block_name_length))
	blockCode := C.GoStringN(block_code, C.int(block_code_length))

	err = this.wrapped.CompileToGuest(def, outPath, blockName, blockCode)

	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
}
//--------------------------------------------------------------------
func (this *LanguagePluginInterfaceHandler) compile_from_host(idl_def_json *C.char, idl_def_json_length C.uint,
															output_path *C.char, output_path_length C.uint,
															host_options *C.char, host_options_length C.int,
															out_err **C.char, out_err_len *C.uint){
	def, err := NewIDLDefinitionFromJSON(C.GoStringN(idl_def_json, C.int(idl_def_json_length)))
	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}

	outPath := C.GoStringN(output_path, C.int(output_path_length))
	hostOptions := C.GoStringN(host_options, C.int(host_options_length))

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

	err = this.wrapped.CompileFromHost(def, outPath, hostOptionsMap)

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
	block_name *C.char, block_name_length C.uint,
	block_code *C.char, block_code_length C.uint,
	out_err **C.char, out_err_len *C.uint) {
	
	if languagePluginInterfaceHandler == nil{
		*out_err = C.CString("Go Compiler plugin not initialised!")
		*out_err_len = C.uint(len("Go Compiler plugin not initialised!"))
		return
	}
	
	defer func() {
		if err := recover(); err != nil {
			msg := fmt.Sprintf("%v", err)
			*out_err = C.CString(msg)
			*out_err_len = C.uint(len(msg))
		}
	}()

	languagePluginInterfaceHandler.compile_to_guest(idl_def_json, idl_def_json_length, output_path, output_path_length, block_name, block_name_length, block_code, block_code_length, out_err, out_err_len)
}
//--------------------------------------------------------------------
//export compile_from_host
func compile_from_host(idl_def_json *C.char, idl_def_json_length C.uint,
	output_path *C.char, output_path_length C.uint,
	host_options *C.char, host_options_length C.int,
	out_err **C.char, out_err_len *C.uint){

	time.Sleep(10*time.Second)

	if languagePluginInterfaceHandler == nil{
		*out_err = C.CString("Go Compiler plugin not initialised!")
		*out_err_len = C.uint(len("Go Compiler plugin not initialised!"))
		return
	}

	defer func() {
		if err := recover(); err != nil {
			msg := fmt.Sprintf("%v", err)
			*out_err = C.CString(msg)
			*out_err_len = C.uint(len(msg))
		}
	}()
	
	languagePluginInterfaceHandler.compile_from_host(idl_def_json, idl_def_json_length, output_path, output_path_length, host_options, host_options_length, out_err, out_err_len)

}
//--------------------------------------------------------------------