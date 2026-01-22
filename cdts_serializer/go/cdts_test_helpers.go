package cdts_go_serializer

/*
#cgo CFLAGS: -I"${SRCDIR}/../../runtime"

#include <cdt.h>
#include <metaffi_primitives.h>
#include <xllr_capi_loader.h>
#include <stdlib.h>
#include <string.h>

// Helper functions for test setup
void set_cdt_int8_val_direct(struct cdt* c, metaffi_int8 val) {
    c->type = metaffi_int8_type;
    c->cdt_val.int8_val = val;
    c->free_required = 0;
}

void set_cdt_int16_val_direct(struct cdt* c, metaffi_int16 val) {
    c->type = metaffi_int16_type;
    c->cdt_val.int16_val = val;
    c->free_required = 0;
}

void set_cdt_int32_val_direct(struct cdt* c, metaffi_int32 val) {
    c->type = metaffi_int32_type;
    c->cdt_val.int32_val = val;
    c->free_required = 0;
}

void set_cdt_int64_val_direct(struct cdt* c, metaffi_int64 val) {
    c->type = metaffi_int64_type;
    c->cdt_val.int64_val = val;
    c->free_required = 0;
}

void set_cdt_uint8_val_direct(struct cdt* c, metaffi_uint8 val) {
    c->type = metaffi_uint8_type;
    c->cdt_val.uint8_val = val;
    c->free_required = 0;
}

void set_cdt_uint16_val_direct(struct cdt* c, metaffi_uint16 val) {
    c->type = metaffi_uint16_type;
    c->cdt_val.uint16_val = val;
    c->free_required = 0;
}

void set_cdt_uint32_val_direct(struct cdt* c, metaffi_uint32 val) {
    c->type = metaffi_uint32_type;
    c->cdt_val.uint32_val = val;
    c->free_required = 0;
}

void set_cdt_uint64_val_direct(struct cdt* c, metaffi_uint64 val) {
    c->type = metaffi_uint64_type;
    c->cdt_val.uint64_val = val;
    c->free_required = 0;
}

void set_cdt_float32_val_direct(struct cdt* c, metaffi_float32 val) {
    c->type = metaffi_float32_type;
    c->cdt_val.float32_val = val;
    c->free_required = 0;
}

void set_cdt_float64_val_direct(struct cdt* c, metaffi_float64 val) {
    c->type = metaffi_float64_type;
    c->cdt_val.float64_val = val;
    c->free_required = 0;
}

void set_cdt_bool_val_direct(struct cdt* c, metaffi_bool val) {
    c->type = metaffi_bool_type;
    c->cdt_val.bool_val = val;
    c->free_required = 0;
}

void set_cdt_string8_val_direct(struct cdt* c, const char* val) {
    c->type = metaffi_string8_type;
    char8_t* allocated = xllr_alloc_string8((const char8_t*)val, strlen(val));
    c->cdt_val.string8_val = allocated;
    c->free_required = 1;
}

void set_cdt_string16_val_direct(struct cdt* c, const char16_t* val, size_t len) {
    c->type = metaffi_string16_type;
    if (val != NULL && len > 0) {
        char16_t* allocated = xllr_alloc_string16(val, len);
        c->cdt_val.string16_val = allocated;
        c->free_required = 1;
    } else {
        c->cdt_val.string16_val = NULL;
        c->free_required = 0;
    }
}

void set_cdt_string32_val_direct(struct cdt* c, const char32_t* val, size_t len) {
    c->type = metaffi_string32_type;
    if (val != NULL && len > 0) {
        char32_t* allocated = xllr_alloc_string32(val, len);
        c->cdt_val.string32_val = allocated;
        c->free_required = 1;
    } else {
        c->cdt_val.string32_val = NULL;
        c->free_required = 0;
    }
}

void set_cdt_null_direct(struct cdt* c) {
    c->type = metaffi_null_type;
    c->free_required = 0;
}

struct cdt* get_cdt_at_index_helper(struct cdts* pcdts, int index) {
    return &pcdts->arr[index];
}

void set_cdt_handle_val_direct(struct cdt* c, void* handle, uint64_t runtime_id) {
    c->type = metaffi_handle_type;
    struct cdt_metaffi_handle* handle_ptr = (struct cdt_metaffi_handle*)xllr_alloc_memory(sizeof(struct cdt_metaffi_handle));
    handle_ptr->handle = (metaffi_handle)handle;
    handle_ptr->runtime_id = runtime_id;
    handle_ptr->release = NULL;
    c->cdt_val.handle_val = handle_ptr;
    c->free_required = 1;
}

void set_cdt_callable_val_direct(struct cdt* c, void* callable, metaffi_type* param_types, int8_t param_count, metaffi_type* retval_types, int8_t retval_count) {
    c->type = metaffi_callable_type;
    struct cdt_metaffi_callable* callable_ptr = (struct cdt_metaffi_callable*)xllr_alloc_memory(sizeof(struct cdt_metaffi_callable));
    callable_ptr->val = (metaffi_callable)callable;
    callable_ptr->params_types_length = param_count;
    callable_ptr->retval_types_length = retval_count;

    if (param_count > 0 && param_types != NULL) {
        callable_ptr->parameters_types = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * param_count);
        for (int i = 0; i < param_count; i++) {
            callable_ptr->parameters_types[i] = param_types[i];
        }
    } else {
        callable_ptr->parameters_types = NULL;
    }

    if (retval_count > 0 && retval_types != NULL) {
        callable_ptr->retval_types = (metaffi_type*)xllr_alloc_memory(sizeof(metaffi_type) * retval_count);
        for (int i = 0; i < retval_count; i++) {
            callable_ptr->retval_types[i] = retval_types[i];
        }
    } else {
        callable_ptr->retval_types = NULL;
    }

    c->cdt_val.callable_val = callable_ptr;
    c->free_required = 1;
}
*/
import "C"
import (
	"unicode/utf16"
	"unsafe"
)

// Test helper functions that use CGo

// SetInt8Direct sets an int8 value directly in a CDT (for test setup)
func SetInt8Direct(cdt *C.struct_cdt, val C.metaffi_int8) {
	C.set_cdt_int8_val_direct(cdt, val)
}

// SetInt16Direct sets an int16 value directly in a CDT (for test setup)
func SetInt16Direct(cdt *C.struct_cdt, val C.metaffi_int16) {
	C.set_cdt_int16_val_direct(cdt, val)
}

// SetInt32Direct sets an int32 value directly in a CDT (for test setup)
func SetInt32Direct(cdt *C.struct_cdt, val C.metaffi_int32) {
	C.set_cdt_int32_val_direct(cdt, val)
}

// SetInt64Direct sets an int64 value directly in a CDT (for test setup)
func SetInt64Direct(cdt *C.struct_cdt, val C.metaffi_int64) {
	C.set_cdt_int64_val_direct(cdt, val)
}

// SetUint8Direct sets a uint8 value directly in a CDT (for test setup)
func SetUint8Direct(cdt *C.struct_cdt, val C.metaffi_uint8) {
	C.set_cdt_uint8_val_direct(cdt, val)
}

// SetUint16Direct sets a uint16 value directly in a CDT (for test setup)
func SetUint16Direct(cdt *C.struct_cdt, val C.metaffi_uint16) {
	C.set_cdt_uint16_val_direct(cdt, val)
}

// SetUint32Direct sets a uint32 value directly in a CDT (for test setup)
func SetUint32Direct(cdt *C.struct_cdt, val C.metaffi_uint32) {
	C.set_cdt_uint32_val_direct(cdt, val)
}

// SetUint64Direct sets a uint64 value directly in a CDT (for test setup)
func SetUint64Direct(cdt *C.struct_cdt, val C.metaffi_uint64) {
	C.set_cdt_uint64_val_direct(cdt, val)
}

// SetFloat32Direct sets a float32 value directly in a CDT (for test setup)
func SetFloat32Direct(cdt *C.struct_cdt, val C.metaffi_float32) {
	C.set_cdt_float32_val_direct(cdt, val)
}

// SetFloat64Direct sets a float64 value directly in a CDT (for test setup)
func SetFloat64Direct(cdt *C.struct_cdt, val C.metaffi_float64) {
	C.set_cdt_float64_val_direct(cdt, val)
}

// SetBoolDirect sets a bool value directly in a CDT (for test setup)
func SetBoolDirect(cdt *C.struct_cdt, val bool) {
	var cVal C.metaffi_bool
	if val {
		cVal = 1
	} else {
		cVal = 0
	}
	C.set_cdt_bool_val_direct(cdt, cVal)
}

// SetString8Direct sets a string8 value directly in a CDT (for test setup)
func SetString8Direct(cdt *C.struct_cdt, val string) {
	cStr := C.CString(val)
	defer C.free(unsafe.Pointer(cStr))
	C.set_cdt_string8_val_direct(cdt, cStr)
}

// SetString16Direct sets a string16 value directly in a CDT (for test setup)
func SetString16Direct(cdt *C.struct_cdt, val string) {
	// Convert Go string to UTF-16
	utf16Buf := utf16.Encode([]rune(val))
	if len(utf16Buf) > 0 {
		C.set_cdt_string16_val_direct(cdt, (*C.char16_t)(unsafe.Pointer(&utf16Buf[0])), C.size_t(len(utf16Buf)))
	} else {
		C.set_cdt_string16_val_direct(cdt, nil, 0)
	}
}

// SetString32Direct sets a string32 value directly in a CDT (for test setup)
func SetString32Direct(cdt *C.struct_cdt, val string) {
	// Convert Go string to UTF-32 (runes)
	runes := []rune(val)
	if len(runes) > 0 {
		C.set_cdt_string32_val_direct(cdt, (*C.char32_t)(unsafe.Pointer(&runes[0])), C.size_t(len(runes)))
	} else {
		C.set_cdt_string32_val_direct(cdt, nil, 0)
	}
}

// SetNullDirect sets a null value directly in a CDT (for test setup)
func SetNullDirect(cdt *C.struct_cdt) {
	C.set_cdt_null_direct(cdt)
}

// GetCDTAtIndex is a helper to get CDT at index (for tests)
// Uses the C helper function defined in this file's CGo block
func GetCDTAtIndex(cdts *C.struct_cdts, index int) *C.struct_cdt {
	return C.get_cdt_at_index_helper(cdts, C.int(index))
}

// Type constants for tests (exposed from C)
func MetaffiInt8Type() C.metaffi_type     { return C.metaffi_int8_type }
func MetaffiInt16Type() C.metaffi_type    { return C.metaffi_int16_type }
func MetaffiInt32Type() C.metaffi_type    { return C.metaffi_int32_type }
func MetaffiInt64Type() C.metaffi_type    { return C.metaffi_int64_type }
func MetaffiUint8Type() C.metaffi_type    { return C.metaffi_uint8_type }
func MetaffiUint16Type() C.metaffi_type   { return C.metaffi_uint16_type }
func MetaffiUint32Type() C.metaffi_type   { return C.metaffi_uint32_type }
func MetaffiUint64Type() C.metaffi_type   { return C.metaffi_uint64_type }
func MetaffiFloat32Type() C.metaffi_type  { return C.metaffi_float32_type }
func MetaffiFloat64Type() C.metaffi_type  { return C.metaffi_float64_type }
func MetaffiBoolType() C.metaffi_type     { return C.metaffi_bool_type }
func MetaffiChar8Type() C.metaffi_type    { return C.metaffi_char8_type }
func MetaffiChar16Type() C.metaffi_type   { return C.metaffi_char16_type }
func MetaffiChar32Type() C.metaffi_type   { return C.metaffi_char32_type }
func MetaffiString8Type() C.metaffi_type  { return C.metaffi_string8_type }
func MetaffiString16Type() C.metaffi_type { return C.metaffi_string16_type }
func MetaffiString32Type() C.metaffi_type { return C.metaffi_string32_type }
func MetaffiNullType() C.metaffi_type     { return C.metaffi_null_type }
func MetaffiArrayType() C.metaffi_type    { return C.metaffi_array_type }

// CombineArrayType combines array type with element type
func CombineArrayType(elementType C.metaffi_type) C.metaffi_type {
	return C.metaffi_type(C.metaffi_array_type | elementType)
}

// GetExpectedTypeForIndex returns the expected type for a given index in type preservation test
func GetExpectedTypeForIndex(index int) C.metaffi_type {
	switch index {
	case 0:
		return MetaffiInt8Type()
	case 1:
		return MetaffiInt16Type()
	case 2:
		return MetaffiInt32Type()
	case 3:
		return MetaffiInt64Type()
	case 4:
		return MetaffiUint8Type()
	case 5:
		return MetaffiUint16Type()
	case 6:
		return MetaffiUint32Type()
	case 7:
		return MetaffiUint64Type()
	default:
		return MetaffiInt8Type() // fallback
	}
}

// SetHandleDirect sets a handle value directly in a CDT (for test setup)
func SetHandleDirect(cdt *C.struct_cdt, handle uintptr, runtimeID uint64) {
	// metaffi_handle is void*, so we can pass the pointer directly
	C.set_cdt_handle_val_direct(cdt, unsafe.Pointer(handle), C.uint64_t(runtimeID))
}

// SetCallableDirect sets a callable value directly in a CDT (for test setup)
func SetCallableDirect(cdt *C.struct_cdt, callable uintptr, paramTypes []C.metaffi_type, retvalTypes []C.metaffi_type) {
	var paramTypesPtr *C.metaffi_type
	var retvalTypesPtr *C.metaffi_type

	if len(paramTypes) > 0 {
		paramTypesPtr = (*C.metaffi_type)(unsafe.Pointer(&paramTypes[0]))
	}
	if len(retvalTypes) > 0 {
		retvalTypesPtr = (*C.metaffi_type)(unsafe.Pointer(&retvalTypes[0]))
	}

	// metaffi_callable is void*, so we can pass the pointer directly
	C.set_cdt_callable_val_direct(cdt, unsafe.Pointer(callable),
		paramTypesPtr, C.int8_t(len(paramTypes)),
		retvalTypesPtr, C.int8_t(len(retvalTypes)))
}

func MetaffiHandleType() C.metaffi_type   { return C.metaffi_handle_type }
func MetaffiCallableType() C.metaffi_type { return C.metaffi_callable_type }

// CreateParamTypesSlice creates a slice of metaffi_type from helper function calls
// This avoids direct C type references in test files
func CreateParamTypesSlice(types ...C.metaffi_type) []C.metaffi_type {
	return types
}

// CreateRetvalTypesSlice creates a slice of metaffi_type from helper function calls
// This avoids direct C type references in test files
func CreateRetvalTypesSlice(types ...C.metaffi_type) []C.metaffi_type {
	return types
}
