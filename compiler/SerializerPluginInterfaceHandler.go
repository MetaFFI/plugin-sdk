package main


/*
#include "serializer_plugin_interface.h"
 */
import "C"

var serializerPluginInterfaceHandler *SerializerPluginInterfaceHandler

type SerializerPluginInterface interface{
	CompileSerialization(idlName string, idl string) (serializationCode string, err error)
}
//--------------------------------------------------------------------
type SerializerPluginInterfaceHandler struct{
	wrapped SerializerPluginInterface
}
//--------------------------------------------------------------------
func CreateSerializerPluginInterfaceHandler(wrapped SerializerPluginInterface){
	serializerPluginInterfaceHandler = &SerializerPluginInterfaceHandler{wrapped: wrapped}
}
//--------------------------------------------------------------------
func (this *SerializerPluginInterfaceHandler) compile_serialization(idl_name *C.char, idl_name_length C.uint,
												idl *C.char, idl_length C.uint,
												out_serialization_code **C.char, out_serialization_code_length *C.uint,
												out_err **C.char, out_err_len *C.uint) {

	idlName := C.GoStringN(idl_name, C.int(idl_name_length))
	idlStr := C.GoStringN(idl, C.int(idl_length))

	serializationCode, err := this.wrapped.CompileSerialization(idlName, idlStr)

	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}

	*out_serialization_code = C.CString(serializationCode)
	*out_serialization_code_length = C.uint(len(serializationCode))
}
//--------------------------------------------------------------------
//export compile_serialization
func compile_serialization(idl_name *C.char, idl_name_length C.uint,
						idl *C.char, idl_length C.uint,
						out_serialization_code **C.char, out_serialization_code_length *C.uint,
						out_err **C.char, out_err_len *C.uint) {

	if serializerPluginInterfaceHandler == nil{
		panic("serializerPluginInterfaceHandler is null!")
	}

	serializerPluginInterfaceHandler.compile_serialization(idl_name, idl_name_length, idl, idl_length, out_serialization_code, out_serialization_code_length, out_err, out_err_len)
}
//--------------------------------------------------------------------