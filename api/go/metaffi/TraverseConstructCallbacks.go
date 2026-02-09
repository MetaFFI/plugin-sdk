package metaffi

/*
#cgo !windows LDFLAGS: -L. -ldl
#cgo LDFLAGS: -Wl,--allow-multiple-definition

#include <string.h>
#include <include/cdts_traverse_construct.h>
#include <include/xllr_capi_loader.h>
#include <stdlib.h>

void set_metaffi_type_info_type(struct metaffi_type_info* info, uint64_t type) {
    info->type = type;
}

metaffi_type get_metaffi_type(struct metaffi_type_info* info) {
    return info->type;
}

uint32_t* cast_char32_t_to_uint32_t(char32_t* input) {
    return (uint32_t*)input;
}

uint16_t* cast_char16_t_to_uint16_t(char16_t* input) {
    return (uint16_t*)input;
}

uint8_t* cast_char8_t_to_uint8_t(char8_t* input) {
    return (uint8_t*)input;
}

struct metaffi_type_info* cast_to_metaffi_type_info(void* input) {
    return (struct metaffi_type_info*)input;
}

metaffi_string8 cast_to_metaffi_string8(char* input) {
    return (metaffi_string8)input;
}

struct cdt_metaffi_handle get_null_handle(){
	struct cdt_metaffi_handle res;
	res.handle = NULL;
	res.runtime_id = 0;
	res.release = NULL;
	return res;
}

char* copy_string(char* s, int n) {
	char* cstr = (char*)malloc(n*sizeof(char) + 1);
	memcpy(cstr, s, n);
	cstr[n] = 0;
	return cstr;
}

*/
import "C"
import (
	"fmt"
	"reflect"
	"unsafe"

	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

func init() {
	err := C.load_xllr()
	if err != nil {
		panic("Failed to load MetaFFI XLLR functions: " + C.GoString(err))
	}
}

func onArray(index []uint64, val *C.struct_cdts, fixedDimensions C.metaffi_int64, commonType C.metaffi_type, tctxt *TraverseContext) bool {

	if commonType&C.metaffi_type(C.metaffi_array_type) != 0 {
		commonType = commonType & ^C.metaffi_type(C.metaffi_array_type)
	}

	// if metaffi_any_type, get the common metaffi type
	var tempForAnyTempDynamicChecking C.metaffi_type = 0
	if commonType&C.metaffi_type(C.metaffi_any_type) != 0 {
		cdts := CDTS{c: val}
		for i := C.metaffi_size(0); i < cdts.GetLength(); i++ {
			elem := cdts.GetCDT(int(i))
			if tempForAnyTempDynamicChecking == 0 {
				tempForAnyTempDynamicChecking = elem.GetTypeVal()
			} else if tempForAnyTempDynamicChecking != elem.GetTypeVal() { // no common type - use metaffi_any_type
				commonType = C.metaffi_any_type
				break
			}
		}
	}

	var commonGoType reflect.Type = nil
	if commonType&C.metaffi_type(C.metaffi_handle_type) != 0 { // if metaffi_handle, get the Go common type

		cdts := CDTS{c: val}
		for i := C.metaffi_size(0); i < cdts.GetLength(); i++ {

			if cdts.GetCDT(int(i)).GetTypeVal()&C.metaffi_array_type == 0 {
				elem := GetGoObject(cdts.GetCDT(int(i)).GetHandleStruct())

				if elem == nil {
					panic(fmt.Sprintf("Go Object returned nil - Handle: %v %v", cdts.GetCDT(int(i)).GetHandleVal(), cdts.GetCDT(int(i)).GetHandleRuntime()))
				}

				curType := reflect.ValueOf(elem).Type()
				if commonGoType == nil {
					commonGoType = curType
				} else if commonGoType != curType { // no common type - use interface{}
					commonGoType = reflect.TypeFor[interface{}]()
					break
				}
			} else {
				commonGoType = reflect.TypeFor[interface{}]()
			}
		}
	} else if commonType == C.metaffi_any_type {
		commonGoType = reflect.TypeFor[interface{}]()
	}

	if index == nil || len(index) == 0 { // check roots

		// if handle, get the common type
		tctxt.Result = createMultiDimSlice(int(val.length), int(fixedDimensions), getGoTypeFromMetaFFIType(commonType, commonGoType))
	} else { // within an array
		elem := getElement(index, tctxt.Result)
		elem.Set(reflect.ValueOf(createMultiDimSlice(int(val.length), int(fixedDimensions), getGoTypeFromMetaFFIType(commonType, commonGoType))))
	}

	return true
}

func getTypeInfo(index []uint64, cctxt *ConstructContext) C.struct_metaffi_type_info {

	if index == nil { // root
		var mt C.struct_metaffi_type_info
		mt.is_free_alias = C.metaffi_bool(0)
		C.set_metaffi_type_info_type(&mt, C.uint64_t(cctxt.TypeInfo.Type))

		if C.get_metaffi_type(&mt) == C.metaffi_any_type {
			mffitype, _ := getMetaFFITypeFromGoType(reflect.ValueOf(cctxt.Input))
			C.set_metaffi_type_info_type(&mt, C.uint64_t(mffitype))
		}
		return mt
	} else {
		val := getElement(index, cctxt.Input)

		detectedType, _ := getMetaFFITypeFromGoType(val)
		ti := IDL.MetaFFITypeInfo{Type: uint64(detectedType)}

		idlTypeInfo := ti.AsCMetaFFITypeInfo()
		cTypeInfo := C.cast_to_metaffi_type_info(unsafe.Pointer(&idlTypeInfo))

		res := *cTypeInfo
		return res
	}
}

func getArrayMetadata(index []uint64, isFixedDimension *C.metaffi_bool, is1DArray *C.metaffi_bool, commonType *C.metaffi_type, isManuallyConstructArray *C.metaffi_bool, cctxt *ConstructContext) C.metaffi_size {
	// Initialize isFixedDimension to true
	*isFixedDimension = C.metaffi_bool(1)
	// Initialize is1DArray to true
	*is1DArray = C.metaffi_bool(1)
	// Initialize isManuallyConstructArray to false
	*isManuallyConstructArray = C.metaffi_bool(0)

	// get array
	v := getElement(index, cctxt.Input)

	arrayOfType, is1darray := getMetaFFITypeFromGoType(v)
	if is1darray {
		*is1DArray = C.metaffi_bool(1)
	} else {
		*is1DArray = C.metaffi_bool(0)
	}

	*commonType = arrayOfType & ^C.metaffi_type(C.metaffi_array_type)

	// Return the length of the slice
	return C.metaffi_size(v.Len())
}
