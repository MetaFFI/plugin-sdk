package compiler


/*
#include "../serializer_plugin_interface.h"
 */
import "C"

var serializerPluginInterfaceHandler *SerializerPluginInterfaceHandler

type SerializerPluginInterface interface{
	CompileSerialization(idl string, language string) (serializationCodeJSON string, err error)
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
func (this *SerializerPluginInterfaceHandler) compile_serialization(idl *C.char, idl_length C.uint,
																	language *C.char, language_length C.uint,
																	out_serialization_code_json **C.char, out_serialization_code_json_length *C.uint,
																	out_err **C.char, out_err_len *C.uint) {

	idlStr := C.GoStringN(idl, C.int(idl_length))
	languageStr := C.GoStringN(language, C.int(language_length))

	serializationCodeJSON, err := this.wrapped.CompileSerialization(idlStr, languageStr)

	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}

	*out_serialization_code_json = C.CString(serializationCodeJSON)
	*out_serialization_code_json_length = C.uint(len(serializationCodeJSON))
}
//--------------------------------------------------------------------
//export compile_serialization
func compile_serialization(idl *C.char, idl_length C.uint,
						language *C.char, language_length C.uint,
						out_serialization_code_json **C.char, out_serialization_code_json_length *C.uint,
						out_err **C.char, out_err_len *C.uint) {

	if serializerPluginInterfaceHandler == nil{
		panic("serializerPluginInterfaceHandler is null!")
	}

	serializerPluginInterfaceHandler.compile_serialization(idl, idl_length, language, language_length, out_serialization_code_json, out_serialization_code_json_length, out_err, out_err_len)
}
//--------------------------------------------------------------------