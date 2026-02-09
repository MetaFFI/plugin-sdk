package metaffi

/*
#include <include/cdt.h>

static struct cdts* get_cdts_at_export(struct cdts* arr, int index) {
	return &arr[index];
}
*/
import "C"

import "unsafe"

// CGo-exported dispatchers for Go callable invocation.
// These are the C entry points stored in xcall structs; they forward
// to dispatchGoCallable (defined in GoCallable.go) which uses reflection
// to invoke the actual Go func.

//export GoCallable_ParamsRet
func GoCallable_ParamsRet(context unsafe.Pointer, params_ret *C.struct_cdts, out_err **C.char) {
	paramsCDTS := &CDTS{c: C.get_cdts_at_export(params_ret, 0)}
	retvalsCDTS := &CDTS{c: C.get_cdts_at_export(params_ret, 1)}
	dispatchGoCallable(context, paramsCDTS, retvalsCDTS, out_err)
}

//export GoCallable_NoParamsRet
func GoCallable_NoParamsRet(context unsafe.Pointer, return_values *C.struct_cdts, out_err **C.char) {
	retvalsCDTS := &CDTS{c: return_values}
	dispatchGoCallable(context, nil, retvalsCDTS, out_err)
}

//export GoCallable_ParamsNoRet
func GoCallable_ParamsNoRet(context unsafe.Pointer, parameters *C.struct_cdts, out_err **C.char) {
	paramsCDTS := &CDTS{c: parameters}
	dispatchGoCallable(context, paramsCDTS, nil, out_err)
}

//export GoCallable_NoParamsNoRet
func GoCallable_NoParamsNoRet(context unsafe.Pointer, out_err **C.char) {
	dispatchGoCallable(context, nil, nil, out_err)
}
