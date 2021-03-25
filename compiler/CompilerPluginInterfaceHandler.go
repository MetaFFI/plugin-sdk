package compiler
/*
#include "compiler_plugin_interface.h"
 */
import "C"

var compilerPluginInterfaceHandler *CompilerPluginInterfaceHandler

type CompilerPluginInterface interface{
	CompileToGuest(idlDefinition *IDLDefinition, outputPath string, serializationCode string) error
	CompileFromHost(idlDefinition *IDLDefinition, outputPath string, serializationCode string) error
}
//--------------------------------------------------------------------
type CompilerPluginInterfaceHandler struct{
	wrapped CompilerPluginInterface
}
//--------------------------------------------------------------------
func CreateCompilerPluginInterfaceHandler(wrapped CompilerPluginInterface){
	compilerPluginInterfaceHandler = &CompilerPluginInterfaceHandler{wrapped: wrapped}
}
//--------------------------------------------------------------------
func (this *CompilerPluginInterfaceHandler) compile_to_guest(idl_def_json *C.char, idl_def_json_length C.uint,
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

	err = this.wrapped.CompileToGuest(def, outPath, serializationCode)

	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
}
//--------------------------------------------------------------------
func (this *CompilerPluginInterfaceHandler) compile_from_host(idl_def_json *C.char, idl_def_json_length C.uint,
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

	err = this.wrapped.CompileFromHost(def, outPath, serializationCode)

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

	if compilerPluginInterfaceHandler == nil{
		panic("compilerPluginInterfaceHandler is null!")
	}

	compilerPluginInterfaceHandler.compile_to_guest(idl_def_json, idl_def_json_length, output_path, output_path_length, serialization_code, serialization_code_length, out_err, out_err_len)
}
//--------------------------------------------------------------------
//export compile_from_host
func compile_from_host(idl_def_json *C.char, idl_def_json_length C.uint,
	output_path *C.char, output_path_length C.uint,
	serialization_code *C.char, serialization_code_length C.uint,
	out_err **C.char, out_err_len *C.uint){

	if compilerPluginInterfaceHandler == nil{
		panic("compilerPluginInterfaceHandler is null!")
	}

	compilerPluginInterfaceHandler.compile_from_host(idl_def_json, idl_def_json_length, output_path, output_path_length, serialization_code, serialization_code_length, out_err, out_err_len)
}
//--------------------------------------------------------------------