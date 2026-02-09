package metaffi

/*
#cgo !windows LDFLAGS: -L. -ldl
#cgo LDFLAGS: -Wl,--allow-multiple-definition

#include <stdlib.h>
#include <stdio.h>
#include <include/xllr_capi_loader.h>
#include <include/xllr_capi_loader.c>
#include <include/xcall.h>

void call_plugin_xcall_no_params_no_ret(struct xcall* pxcall, char** err)
{
	void* pvoidxcall = pxcall->pxcall_and_context[0];
	void* pctxt = pxcall->pxcall_and_context[1];
	(((void(*)(void*,char**))pvoidxcall)(pctxt, err));
}

void call_plugin_xcall_no_params_ret(struct xcall* pxcall, struct cdts* cdts, char** err)
{
	void* pvoidxcall = pxcall->pxcall_and_context[0];
	void* pctxt = pxcall->pxcall_and_context[1];

	(((void(*)(void*,void*,char**))pvoidxcall)(pctxt, cdts, err));
}

void call_plugin_xcall_params_no_ret(struct xcall* pxcall, struct cdts* cdts, char** err)
{
	void* pvoidxcall = pxcall->pxcall_and_context[0];
	void* pctxt = pxcall->pxcall_and_context[1];

	(((void(*)(void*,void*,char**))pvoidxcall)(pctxt, cdts, err));
}

void call_plugin_xcall_params_ret(struct xcall* pxcall, struct cdts* cdts, char** err)
{
	void* pvoidxcall = pxcall->pxcall_and_context[0];
	void* pctxt = pxcall->pxcall_and_context[1];

	(((void(*)(void*,void*,char**))pvoidxcall)(pctxt, cdts, err));
}

struct cdts* cast_to_cdts(void* p)
{
	return (struct cdts*)p;
}

struct cdt* get_cdts_index_pcdt(struct cdts* p, int index)
{
	return p[index].arr;
}

*/
import "C"
import (
	"fmt"
	"unsafe"

	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

func init() {
	err := C.load_xllr()
	if err != nil {
		panic("Failed to load MetaFFI XLLR functions: " + C.GoString(err))
	}
}

func XLLRLoadEntity(runtimePlugin string, modulePath string, entityPath string, paramsTypes []uint64, retvalsTypes []uint64) (unsafe.Pointer, error) {

	var params []IDL.MetaFFITypeInfo
	if paramsTypes != nil {
		params = make([]IDL.MetaFFITypeInfo, 0)
		for _, p := range paramsTypes {
			params = append(params, IDL.MetaFFITypeInfo{Type: p})
		}
	}

	var retvals []IDL.MetaFFITypeInfo
	if retvalsTypes != nil {
		retvals = make([]IDL.MetaFFITypeInfo, 0)
		for _, r := range retvalsTypes {
			retvals = append(retvals, IDL.MetaFFITypeInfo{Type: r})
		}
	}

	return XLLRLoadEntityWithAliases(runtimePlugin, modulePath, entityPath, params, retvals)
}

func XLLRLoadEntityWithAliases(runtimePlugin string, modulePath string, entityPath string, paramsTypes []IDL.MetaFFITypeInfo, retvalsTypes []IDL.MetaFFITypeInfo) (unsafe.Pointer, error) {

	pruntimePlugin := C.CString(runtimePlugin)
	defer C.free(unsafe.Pointer(pruntimePlugin))

	pmodulePath := C.CString(modulePath)
	defer C.free(unsafe.Pointer(pmodulePath))

	pfuncpath := C.CString(entityPath)
	defer C.free(unsafe.Pointer(pfuncpath))

	var out_err *C.char

	var pparamTypes *C.struct_metaffi_type_info
	if paramsTypes != nil {
		pparamTypes = createMetaFFITypeInfoArray(paramsTypes)
		defer freeMetaFFITypeInfoArray(pparamTypes, len(paramsTypes))
	}

	pparamTypesLen := (C.int8_t)(len(paramsTypes))

	var ppretvalsTypes *C.struct_metaffi_type_info
	if retvalsTypes != nil {
		ppretvalsTypes = createMetaFFITypeInfoArray(retvalsTypes)
		defer freeMetaFFITypeInfoArray(ppretvalsTypes, len(retvalsTypes))
	}
	pretvalsTypesLen := (C.int8_t)(len(retvalsTypes))

	xcall := C.xllr_load_entity(pruntimePlugin,
		pmodulePath,
		pfuncpath,
		pparamTypes, pparamTypesLen,
		ppretvalsTypes, pretvalsTypesLen,
		&out_err)

	if out_err != nil {
		defer C.xllr_free_string(out_err)
		return nil, fmt.Errorf("Failed to xcall for \"%v\": %v", entityPath, C.GoString(out_err))
	}

	return unsafe.Pointer(xcall), nil
}

func XLLRFreeXCall(runtimePlugin string, xcall unsafe.Pointer) error {

	var out_err *C.char
	pruntimePlugin := C.CString(runtimePlugin)
	defer C.free(unsafe.Pointer(pruntimePlugin))
	C.xllr_free_xcall(pruntimePlugin, (*C.struct_xcall)(xcall), &out_err)

	if out_err != nil {
		defer C.xllr_free_string(out_err)
		return fmt.Errorf("Failed to free xcall: %v", C.GoString(out_err))
	}

	C.free(xcall)

	return nil
}

func XLLRXCallParamsRet(xcall unsafe.Pointer, pcdts unsafe.Pointer) error {

	var out_err *C.char

	C.call_plugin_xcall_params_ret((*C.struct_xcall)(xcall), C.cast_to_cdts(pcdts), &out_err)

	if out_err != nil {
		defer C.free(unsafe.Pointer(out_err))
		return fmt.Errorf("%v", C.GoString(out_err))
	}

	return nil
}

func XLLRXCallNoParamsRet(xcall unsafe.Pointer, cdts unsafe.Pointer) error {

	var out_err *C.char
	C.call_plugin_xcall_no_params_ret((*C.struct_xcall)(xcall), C.cast_to_cdts(cdts), &out_err)

	if out_err != nil {
		defer C.free(unsafe.Pointer(out_err))
		return fmt.Errorf("%v", C.GoString(out_err))
	}

	return nil
}

func XLLRXCallParamsNoRet(xcall unsafe.Pointer, cdts unsafe.Pointer) error {

	var out_err *C.char
	C.call_plugin_xcall_params_no_ret((*C.struct_xcall)(xcall), C.cast_to_cdts(cdts), &out_err)

	if out_err != nil {
		defer C.free(unsafe.Pointer(out_err))
		return fmt.Errorf("%v", C.GoString(out_err))
	}

	return nil
}

func XLLRXCallNoParamsNoRet(xcall unsafe.Pointer) error {

	var out_err *C.char
	C.call_plugin_xcall_no_params_no_ret((*C.struct_xcall)(xcall), &out_err)

	if out_err != nil {
		defer C.xllr_free_string(out_err)
		return fmt.Errorf("%v", C.GoString(out_err))
	}

	return nil
}

func XLLRAllocCDTSBuffer(paramsCount uint64, retsCount uint64) (pcdts unsafe.Pointer, parametersCDTS unsafe.Pointer, return_valuesCDTS unsafe.Pointer) {
	res := C.xllr_alloc_cdts_buffer(C.metaffi_size(paramsCount), C.metaffi_size(retsCount))
	pcdts = unsafe.Pointer(res)

	if res != nil {
		parametersCDTS = unsafe.Pointer(C.get_cdts_index_pcdt(res, 0))
		return_valuesCDTS = unsafe.Pointer(C.get_cdts_index_pcdt(res, 1))
	}

	return
}

func XLLRFreeCDTSBuffer(pcdts unsafe.Pointer) {
	C.xllr_free_cdts_buffer((*C.struct_cdts)(pcdts))
}

func XLLRLoadRuntimePlugin(runtimePlugin string) error {

	pruntime_plugin := C.CString(runtimePlugin)
	defer C.free(unsafe.Pointer(pruntime_plugin))

	// load foreign runtime
	var out_err *C.char

	C.xllr_load_runtime_plugin(pruntime_plugin, &out_err)

	if out_err != nil {
		defer C.xllr_free_string(out_err)
		return fmt.Errorf("Failed to load runtime %v: %v", runtimePlugin, C.GoString(out_err))
	}

	return nil
}

func XLLRFreeRuntimePlugin(runtimePlugin string) error {

	pruntime_plugin := C.CString(runtimePlugin)
	defer C.free(unsafe.Pointer(pruntime_plugin))

	var out_err *C.char
	C.xllr_free_runtime_plugin(pruntime_plugin, &out_err)

	if out_err != nil {
		defer C.xllr_free_string(out_err)
		return fmt.Errorf("Failed to free runtime %v: %v", runtimePlugin, C.GoString(out_err))
	}

	return nil
}
