package main


/*
#include "idl_plugin_intetrface.h"
 */
import "C"

type IDLPluginInterface interface{
	ParseIDL(idlName string, idl string) error
}
//--------------------------------------------------------------------
type IDLPluginInterfaceWrapper struct{
	wrapped IDLPluginInterface
}
//--------------------------------------------------------------------
func NewIDLPluginInterfaceWrapper(wrapped IDLPluginInterface) *IDLPluginInterfaceWrapper{
	return &IDLPluginInterfaceWrapper{wrapped: wrapped}
}
//--------------------------------------------------------------------
//export parse_idl
func (this *IDLPluginInterfaceWrapper) parse_idl(idl_name *C.char, idl_name_length C.uint,
												idl *C.char, idl_length C.uint,
												out_err **C.char, out_err_len *C.uint) {

	idlName := C.GoStringN(idl_name, C.int(idl_name_length))
	idlStr := C.GoStringN(idl, C.int(idl_length))

	err := this.wrapped.ParseIDL(idlName, idlStr)

	if err != nil{
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
}
//--------------------------------------------------------------------