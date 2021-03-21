package compiler

/*
#include "idl_plugin_interface.h"
 */
import "C"

var idlPluginInterfaceHandler *IDLPluginInterfaceHandler

type IDLPluginInterface interface{
	ParseIDL(idlName string, idl string) (*IDLDefinition, error)
}
//--------------------------------------------------------------------
type IDLPluginInterfaceHandler struct{
	wrapped IDLPluginInterface
}
//--------------------------------------------------------------------
func CreateIDLPluginInterfaceHandler(wrapped IDLPluginInterface){
	idlPluginInterfaceHandler = &IDLPluginInterfaceHandler{wrapped: wrapped}
}
//--------------------------------------------------------------------
func (this *IDLPluginInterfaceHandler) parse_idl(idl_name *C.char, idl_name_length C.uint,
												idl *C.char, idl_length C.uint,
												out_err **C.char, out_err_len *C.uint) *C.idl_definition{

	idlName := C.GoStringN(idl_name, C.int(idl_name_length))
	idlStr := C.GoStringN(idl, C.int(idl_length))

	def, err := this.wrapped.ParseIDL(idlName, idlStr)

	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return nil
	}

	return def
}
//--------------------------------------------------------------------
//export parse_idl
func parse_idl(idl_name *C.char, idl_name_length C.uint,
	idl *C.char, idl_length C.uint,
	out_err **C.char, out_err_len *C.uint) *C.idl_definition{

	if idlPluginInterfaceHandler == nil{
		panic("idlPluginInterfaceHandler is null!")
	}

	return idlPluginInterfaceHandler.parse_idl(idl_name, idl_name_length, idl, idl_length, out_err, out_err_len)
}
//--------------------------------------------------------------------