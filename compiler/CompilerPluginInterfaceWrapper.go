package main


/*
#include "compiler_plugin_intetrface.h"
 */
import "C"

type CompilerPluginInterface interface{
	CompileToGuest(idlDefinition *IDLDefinition, outputPath string, serializationCode string) error
	CompileFromHost(idlDefinition *IDLDefinition, outputPath string, serializationCode string) error
}
//--------------------------------------------------------------------
type CompilerPluginInterfaceWrapper struct{
	wrapped CompilerPluginInterface
}
//--------------------------------------------------------------------
func NewCompilerPluginInterfaceWrapper(wrapped CompilerPluginInterface) *CompilerPluginInterfaceWrapper{
	return &CompilerPluginInterfaceWrapper{wrapped: wrapped}
}
//--------------------------------------------------------------------
//export compile_to_guest
func (this *CompilerPluginInterfaceWrapper) compile_to_guest(idl_def *C.idl_definition,
															output_path *C.char, output_path_length C.uint,
															serialization_code *C.char, serialization_code_length C.uint,
															out_err **C.char, out_err_len *C.uint) {
	def := NewIDLDefinition(idl_def)
	outPath := C.GoStringN(output_path, C.int(output_path_length))
	serializationCode := C.GoStringN(serialization_code, C.int(serialization_code_length))

	err := this.wrapped.CompileToGuest(def, outPath, serializationCode)

	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
}
//--------------------------------------------------------------------
//export compile_from_host
func (this *CompilerPluginInterfaceWrapper) compile_from_host(idl_def *C.idl_definition,
															output_path *C.char, output_path_length C.uint,
															serialization_code *C.char, serialization_code_length C.uint,
															out_err **C.char, out_err_len *C.uint){
	def := NewIDLDefinition(idl_def)
	outPath := C.GoStringN(output_path, C.int(output_path_length))
	serializationCode := C.GoStringN(serialization_code, C.int(serialization_code_length))

	err := this.wrapped.CompileFromHost(def, outPath, serializationCode)

	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
}
//--------------------------------------------------------------------