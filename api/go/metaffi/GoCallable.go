package metaffi

/*
#include <include/xllr_capi_loader.h>
#include <include/xcall.h>
#include <stdlib.h>
#include <string.h>

// --- Context for Go callable dispatch ---
struct go_callable_context {
	unsigned long long func_handle;
	int8_t params_count;
	int8_t retval_count;
};

// Use xllr_alloc_memory for all allocations so that MetaFFI's cleanup
// (which uses xllr_free_memory) matches the allocator.

static struct go_callable_context* alloc_go_callable_context(unsigned long long func_handle, int8_t params_count, int8_t retval_count) {
	struct go_callable_context* ctx = (struct go_callable_context*)xllr_alloc_memory(sizeof(struct go_callable_context));
	ctx->func_handle = func_handle;
	ctx->params_count = params_count;
	ctx->retval_count = retval_count;
	return ctx;
}

static struct xcall* alloc_go_xcall(void* func_ptr, void* context) {
	struct xcall* x = (struct xcall*)xllr_alloc_memory(sizeof(struct xcall));
	x->pxcall_and_context[0] = func_ptr;
	x->pxcall_and_context[1] = context;
	return x;
}

static struct cdt_metaffi_callable* alloc_cdt_callable(
	struct xcall* val,
	metaffi_type* params_types, int8_t params_count,
	metaffi_type* retval_types, int8_t retval_count
) {
	struct cdt_metaffi_callable* c = (struct cdt_metaffi_callable*)xllr_alloc_memory(sizeof(struct cdt_metaffi_callable));
	c->val = val;
	c->parameters_types = params_types;
	c->params_types_length = params_count;
	c->retval_types = retval_types;
	c->retval_types_length = retval_count;
	return c;
}

static metaffi_type* alloc_metaffi_types(int8_t count) {
	if (count <= 0) return NULL;
	metaffi_type* arr = (metaffi_type*)xllr_alloc_memory(count * sizeof(metaffi_type));
	memset(arr, 0, count * sizeof(metaffi_type));
	return arr;
}

static void set_metaffi_type_at(metaffi_type* arr, int index, metaffi_type val) {
	arr[index] = val;
}

static struct cdts* get_cdts_at(struct cdts* arr, int index) {
	return &arr[index];
}

// Forward declarations of CGo-exported Go dispatchers
extern void GoCallable_ParamsRet(void* context, struct cdts* params_ret, char** out_err);
extern void GoCallable_NoParamsRet(void* context, struct cdts* return_values, char** out_err);
extern void GoCallable_ParamsNoRet(void* context, struct cdts* parameters, char** out_err);
extern void GoCallable_NoParamsNoRet(void* context, char** out_err);

static void* get_go_callable_dispatcher(int8_t has_params, int8_t has_retvals) {
	if (has_params && has_retvals) return (void*)GoCallable_ParamsRet;
	if (!has_params && has_retvals) return (void*)GoCallable_NoParamsRet;
	if (has_params && !has_retvals) return (void*)GoCallable_ParamsNoRet;
	return (void*)GoCallable_NoParamsNoRet;
}

*/
import "C"

import (
	"fmt"
	"reflect"
	"unsafe"
)

// -------- Go type → metaffi_type mapping for callable signatures --------

func goReflectTypeToMetaFFIType(t reflect.Type) C.metaffi_type {
	switch t.Kind() {
	case reflect.Float32:
		return C.metaffi_float32_type
	case reflect.Float64:
		return C.metaffi_float64_type
	case reflect.Int8:
		return C.metaffi_int8_type
	case reflect.Uint8:
		return C.metaffi_uint8_type
	case reflect.Int16:
		return C.metaffi_int16_type
	case reflect.Uint16:
		return C.metaffi_uint16_type
	case reflect.Int32:
		return C.metaffi_int32_type
	case reflect.Uint32:
		return C.metaffi_uint32_type
	case reflect.Int, reflect.Int64:
		return C.metaffi_int64_type
	case reflect.Uint, reflect.Uint64:
		return C.metaffi_uint64_type
	case reflect.Bool:
		return C.metaffi_bool_type
	case reflect.String:
		return C.metaffi_string8_type
	case reflect.Func:
		return C.metaffi_callable_type
	default:
		return C.metaffi_handle_type
	}
}

// -------- Construct callable from Go func (Go func → CDT callable) --------

// SetGoFuncCallableInCDT stores a Go func value as a callable in a CDT slot.
// It stores the func in the handle table, creates an xcall pointing to the
// appropriate dispatcher, and packs everything into a cdt_metaffi_callable.
func SetGoFuncCallableInCDT(item *CDT, funcVal reflect.Value) {
	if funcVal.Kind() != reflect.Func {
		panic(fmt.Sprintf("SetGoFuncCallableInCDT: expected func, got %v", funcVal.Kind()))
	}

	funcType := funcVal.Type()
	paramsCount := C.int8_t(funcType.NumIn())
	retvalCount := C.int8_t(funcType.NumOut())

	// Store the func in the Go handle table (prevents GC collection)
	handle := SetObject(funcVal.Interface())

	// Allocate C context
	ctx := C.alloc_go_callable_context(
		C.ulonglong(uintptr(unsafe.Pointer(handle))),
		paramsCount,
		retvalCount,
	)

	// Get the appropriate dispatcher function pointer
	hasParams := C.int8_t(0)
	hasRetvals := C.int8_t(0)
	if paramsCount > 0 {
		hasParams = 1
	}
	if retvalCount > 0 {
		hasRetvals = 1
	}
	dispatcher := C.get_go_callable_dispatcher(hasParams, hasRetvals)

	// Allocate xcall (dispatcher + context)
	pxcall := C.alloc_go_xcall(dispatcher, unsafe.Pointer(ctx))

	// Build metaffi_type arrays for the callable's signature metadata
	cParamsTypes := C.alloc_metaffi_types(paramsCount)
	for i := 0; i < int(paramsCount); i++ {
		C.set_metaffi_type_at(cParamsTypes, C.int(i), goReflectTypeToMetaFFIType(funcType.In(i)))
	}

	cRetvalTypes := C.alloc_metaffi_types(retvalCount)
	for i := 0; i < int(retvalCount); i++ {
		C.set_metaffi_type_at(cRetvalTypes, C.int(i), goReflectTypeToMetaFFIType(funcType.Out(i)))
	}

	// Allocate and fill cdt_metaffi_callable
	callable := C.alloc_cdt_callable(pxcall, cParamsTypes, paramsCount, cRetvalTypes, retvalCount)

	item.SetCallableVal(&MetaFFICallable{Val: callable})
	item.SetFreeRequired(true)
}

// -------- Core dispatch logic (shared by all 4 dispatcher variants) --------

// dispatchGoCallable retrieves the Go func from the handle table and invokes it,
// converting between CDT and Go values via reflection.
func dispatchGoCallable(context unsafe.Pointer, paramsCDTS *CDTS, retvalsCDTS *CDTS, out_err **C.char) {
	defer func() {
		if r := recover(); r != nil {
			errMsg := fmt.Sprintf("Go callable dispatch panic: %v", r)
			cstr := C.CString(errMsg)
			*out_err = cstr
		}
	}()

	ctx := (*C.struct_go_callable_context)(context)
	handle := Handle(C.metaffi_handle(uintptr(ctx.func_handle)))
	obj := GetObject(handle)
	if obj == nil {
		*out_err = C.CString(fmt.Sprintf("Go callable: handle %v not found in object table", ctx.func_handle))
		return
	}

	funcVal := reflect.ValueOf(obj)
	if funcVal.Kind() != reflect.Func {
		*out_err = C.CString(fmt.Sprintf("Go callable: object is not a func, got %v", funcVal.Kind()))
		return
	}

	funcType := funcVal.Type()

	// Build argument list from params CDTs
	numIn := funcType.NumIn()
	args := make([]reflect.Value, numIn)
	for i := 0; i < numIn; i++ {
		if paramsCDTS == nil {
			*out_err = C.CString("Go callable: expected params but params CDTS is nil")
			return
		}
		cdt := paramsCDTS.GetCDT(i)
		args[i] = cdtToReflectValue(cdt, funcType.In(i))
	}

	// Call the Go function
	results := funcVal.Call(args)

	// Write results to retvals CDTs
	numOut := funcType.NumOut()
	for i := 0; i < numOut; i++ {
		if retvalsCDTS == nil {
			*out_err = C.CString("Go callable: expected retvals but retvals CDTS is nil")
			return
		}
		cdt := retvalsCDTS.GetCDT(i)
		reflectValueToCDT(results[i], cdt)
	}
}

// -------- CDT ↔ Go reflection conversion helpers --------

// cdtToReflectValue reads a CDT slot and converts it to a reflect.Value of the given target type.
func cdtToReflectValue(cdt *CDT, targetType reflect.Type) reflect.Value {
	switch cdt.GetTypeVal() {
	case C.metaffi_float32_type:
		return reflect.ValueOf(float32(cdt.GetFloat32Val())).Convert(targetType)
	case C.metaffi_float64_type:
		return reflect.ValueOf(float64(cdt.GetFloat64Val())).Convert(targetType)
	case C.metaffi_int8_type:
		return reflect.ValueOf(int8(cdt.GetInt8Val())).Convert(targetType)
	case C.metaffi_uint8_type:
		return reflect.ValueOf(uint8(cdt.GetUInt8Val())).Convert(targetType)
	case C.metaffi_int16_type:
		return reflect.ValueOf(int16(cdt.GetInt16Val())).Convert(targetType)
	case C.metaffi_uint16_type:
		return reflect.ValueOf(uint16(cdt.GetUInt16Val())).Convert(targetType)
	case C.metaffi_int32_type:
		return reflect.ValueOf(int32(cdt.GetInt32Val())).Convert(targetType)
	case C.metaffi_uint32_type:
		return reflect.ValueOf(uint32(cdt.GetUInt32Val())).Convert(targetType)
	case C.metaffi_int64_type:
		return reflect.ValueOf(int64(cdt.GetInt64Val())).Convert(targetType)
	case C.metaffi_uint64_type:
		return reflect.ValueOf(uint64(cdt.GetUInt64Val())).Convert(targetType)
	case C.metaffi_bool_type:
		return reflect.ValueOf(cdt.GetBool()).Convert(targetType)
	case C.metaffi_string8_type:
		return reflect.ValueOf(cdt.GetString8()).Convert(targetType)
	case C.metaffi_handle_type:
		h := cdt.GetHandleStruct()
		obj := GetGoObject(h)
		if obj == nil {
			return reflect.Zero(targetType)
		}
		return reflect.ValueOf(obj).Convert(targetType)
	case C.metaffi_callable_type:
		// Foreign callable → wrap as invocable Go func matching the target type
		callableStruct := cdt.GetCallableVal()
		return wrapForeignCallableAsGoFunc(callableStruct, targetType)
	case C.metaffi_null_type:
		return reflect.Zero(targetType)
	default:
		panic(fmt.Sprintf("cdtToReflectValue: unsupported CDT type %v", cdt.GetTypeVal()))
	}
}

// reflectValueToCDT writes a Go reflect.Value to a CDT slot.
func reflectValueToCDT(val reflect.Value, cdt *CDT) {
	if !val.IsValid() {
		cdt.SetTypeVal(C.metaffi_null_type)
		return
	}

	detectedType, _ := getMetaFFITypeFromGoType(val)
	cdt.SetTypeVal(detectedType)

	switch detectedType {
	case C.metaffi_float32_type:
		cdt.SetFloat32Val(C.metaffi_float32(val.Float()))
	case C.metaffi_float64_type:
		cdt.SetFloat64Val(C.metaffi_float64(val.Float()))
	case C.metaffi_int8_type:
		cdt.SetInt8Val(C.metaffi_int8(val.Int()))
	case C.metaffi_uint8_type:
		cdt.SetUInt8Val(C.metaffi_uint8(val.Uint()))
	case C.metaffi_int16_type:
		cdt.SetInt16Val(C.metaffi_int16(val.Int()))
	case C.metaffi_uint16_type:
		cdt.SetUInt16Val(C.metaffi_uint16(val.Uint()))
	case C.metaffi_int32_type:
		cdt.SetInt32Val(C.metaffi_int32(val.Int()))
	case C.metaffi_uint32_type:
		cdt.SetUInt32Val(C.metaffi_uint32(val.Uint()))
	case C.metaffi_int64_type:
		cdt.SetInt64Val(C.metaffi_int64(val.Int()))
	case C.metaffi_uint64_type:
		cdt.SetUInt64Val(C.metaffi_uint64(val.Uint()))
	case C.metaffi_bool_type:
		cdt.SetBool(val.Bool())
	case C.metaffi_string8_type:
		cdt.SetString8(val.String())
		cdt.SetFreeRequired(true)
	case C.metaffi_callable_type:
		SetGoFuncCallableInCDT(cdt, val)
	case C.metaffi_handle_type:
		h := SetObject(val.Interface())
		cdtHandle := NewCDTMetaFFIHandle(h, GO_RUNTIME_ID, GetReleaserCFunction())
		cdt.SetHandleStruct(cdtHandle)
	case C.metaffi_null_type:
		// Already set above
	default:
		panic(fmt.Sprintf("reflectValueToCDT: unsupported type %v", detectedType))
	}
}

// -------- Foreign callable → Go func wrapper --------

// wrapForeignCallableAsGoFunc wraps a foreign xcall (from cdt_metaffi_callable) as a
// Go function that matches the specified target type, using reflect.MakeFunc.
func wrapForeignCallableAsGoFunc(callable *MetaFFICallable, targetType reflect.Type) reflect.Value {
	if callable == nil || callable.Val == nil {
		return reflect.Zero(targetType)
	}

	numIn := targetType.NumIn()
	numOut := targetType.NumOut()

	// Capture the xcall pointer (metaffi_callable = struct xcall*)
	xcallPtr := callable.Val.val

	goFunc := reflect.MakeFunc(targetType, func(args []reflect.Value) []reflect.Value {
		// Determine CDT layout
		hasParams := numIn > 0
		hasRetvals := numOut > 0

		var cdtsArr *C.struct_cdts
		var paramsCDTS, retvalsCDTS *CDTS

		if hasParams && hasRetvals {
			// cdts[2]: params at [0], retvals at [1]
			cdtsArr = (*C.struct_cdts)(C.xllr_alloc_memory(C.size_t(unsafe.Sizeof(C.struct_cdts{})) * 2))
			paramsCDTS = &CDTS{c: C.get_cdts_at(cdtsArr, 0)}
			retvalsCDTS = &CDTS{c: C.get_cdts_at(cdtsArr, 1)}
		} else if hasParams {
			// cdts[1]: params at [0]
			cdtsArr = (*C.struct_cdts)(C.xllr_alloc_memory(C.size_t(unsafe.Sizeof(C.struct_cdts{}))))
			paramsCDTS = &CDTS{c: cdtsArr}
		} else if hasRetvals {
			// cdts[1]: retvals at [0]
			cdtsArr = (*C.struct_cdts)(C.xllr_alloc_memory(C.size_t(unsafe.Sizeof(C.struct_cdts{}))))
			retvalsCDTS = &CDTS{c: cdtsArr}
		}

		// Fill params CDTs
		if paramsCDTS != nil {
			paramsCDTS.c.arr = (*C.struct_cdt)(C.xllr_alloc_cdt_array(C.uint64_t(numIn)))
			paramsCDTS.c.length = C.metaffi_size(numIn)
			for i, arg := range args {
				cdt := paramsCDTS.GetCDT(i)
				reflectValueToCDT(arg, cdt)
			}
		}

		// Prepare retvals CDTs
		if retvalsCDTS != nil {
			retvalsCDTS.c.arr = (*C.struct_cdt)(C.xllr_alloc_cdt_array(C.uint64_t(numOut)))
			retvalsCDTS.c.length = C.metaffi_size(numOut)
		}

		// Invoke the foreign xcall
		var out_err *C.char
		pxcall := (*C.struct_xcall)(unsafe.Pointer(xcallPtr))
		if hasParams && hasRetvals {
			C.xllr_xcall_params_ret(pxcall, cdtsArr, &out_err)
		} else if hasParams {
			C.xllr_xcall_params_no_ret(pxcall, cdtsArr, &out_err)
		} else if hasRetvals {
			C.xllr_xcall_no_params_ret(pxcall, cdtsArr, &out_err)
		} else {
			C.xllr_xcall_no_params_no_ret(pxcall, &out_err)
		}

		if out_err != nil {
			panic(C.GoString(out_err))
		}

		// Read results from retvals CDTs
		results := make([]reflect.Value, numOut)
		for i := 0; i < numOut; i++ {
			cdt := retvalsCDTS.GetCDT(i)
			results[i] = cdtToReflectValue(cdt, targetType.Out(i))
		}

		return results
	})

	return goFunc
}
