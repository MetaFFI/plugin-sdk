package metaffi

/*
#cgo !windows LDFLAGS: -L. -ldl
#cgo LDFLAGS: -Wl,--allow-multiple-definition

#include <include/xllr_capi_loader.h>
#include <include/xllr_capi_loader.c>
#include <include/cdts_traverse_construct.h>
#include <stdint.h>


void GoMetaFFIHandleTocdt_metaffi_handle(struct cdt_metaffi_handle* p , void* handle, uint64_t runtime_id, void* release) {
	p->handle = (metaffi_handle)handle;
	p->runtime_id = runtime_id;
	p->release = (void (*)(struct cdt_metaffi_handle*))release;
}

*/
import "C"
import (
	"fmt"
	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
	"reflect"
	"unsafe"
)

func getElement(index []uint64, root interface{}) reflect.Value {

	if index == nil {
		return reflect.ValueOf(root)
	}

	// Traverse the root object
	v := reflect.ValueOf(root)
	for _, idx := range index {
		if v.Kind() == reflect.Slice || v.Kind() == reflect.Array {
			if idx < uint64(v.Len()) {
				v = v.Index(int(idx))
			} else {
				panic(fmt.Sprintf("Index out of range: %v. Length: %v", idx, v.Len()))
			}
		} else {
			panic(fmt.Sprintf("Unsupported type: %T", v.Interface()))
		}
	}

	return v
}

func createMultiDimSlice(length int, dimensions int, elemType reflect.Type) interface{} {
	if dimensions <= 0 {
		return nil
	}

	// Create a slice type for each dimension
	for i := 0; i < dimensions; i++ {
		elemType = reflect.SliceOf(elemType)
	}

	// Create a slice of the final type
	slice := reflect.MakeSlice(elemType, length, length)

	return slice.Interface()
}

func getGoTypeFromMetaFFIType(metaffiType C.metaffi_type, commonGoType reflect.Type) reflect.Type {

	switch metaffiType {
	case C.metaffi_float64_type:
		return reflect.TypeOf(float64(0))
	case C.metaffi_float32_type:
		return reflect.TypeOf(float32(0))
	case C.metaffi_int8_type:
		return reflect.TypeOf(int8(0))
	case C.metaffi_uint8_type:
		return reflect.TypeOf(uint8(0))
	case C.metaffi_int16_type:
		return reflect.TypeOf(int16(0))
	case C.metaffi_uint16_type:
		return reflect.TypeOf(uint16(0))
	case C.metaffi_int32_type:
		return reflect.TypeOf(int32(0))
	case C.metaffi_uint32_type:
		return reflect.TypeOf(uint32(0))
	case C.metaffi_int64_type:
		return reflect.TypeOf(int64(0))
	case C.metaffi_uint64_type:
		return reflect.TypeOf(uint64(0))
	case C.metaffi_bool_type:
		return reflect.TypeOf(false)
	case C.metaffi_char8_type:
		return reflect.TypeOf(rune(0))
	case C.metaffi_string8_type:
		return reflect.TypeOf(string(""))
	case C.metaffi_char16_type:
		return reflect.TypeOf(rune(0))
	case C.metaffi_string16_type:
		return reflect.TypeOf(string(""))
	case C.metaffi_char32_type:
		return reflect.TypeOf(rune(0))
	case C.metaffi_string32_type:
		return reflect.TypeOf("")
	case C.metaffi_any_type:
		fallthrough
	case C.metaffi_handle_type:
		if commonGoType == nil {
			panic("metaffi_handle_type requires a common Go type")
		}
		return commonGoType
	case C.metaffi_callable_type:
		return reflect.TypeOf(func() {})
	default:
		panic(fmt.Sprintf("Cannot find requested MetaFFI Type: %v", metaffiType))
	}
}

//--------------------------------------------------------------------

func getMetaFFITypeFromGoType(v reflect.Value) (detectedType C.metaffi_type, is1DArray bool) {
	if !v.IsValid() { // nil value → null type
		return C.metaffi_null_type, false
	}

	t := v.Type()
	arrayMask := C.metaffi_type(0)
	is1DArray = false

	if t.Kind() == reflect.Slice {
		is1DArray = true
		arrayMask = C.metaffi_array_type
		t = t.Elem()
	}

	for t.Kind() == reflect.Slice { // for multi-dimensional arrays
		is1DArray = false
		t = t.Elem()
	}

	switch t.Kind() {
	case reflect.Float64:
		return arrayMask | C.metaffi_float64_type, is1DArray
	case reflect.Float32:
		return arrayMask | C.metaffi_float32_type, is1DArray
	case reflect.Int8:
		return arrayMask | C.metaffi_int8_type, is1DArray
	case reflect.Uint8:
		return arrayMask | C.metaffi_uint8_type, is1DArray
	case reflect.Int16:
		return arrayMask | C.metaffi_int16_type, is1DArray
	case reflect.Uint16:
		return arrayMask | C.metaffi_uint16_type, is1DArray
	case reflect.Int32:
		return arrayMask | C.metaffi_int32_type, is1DArray
	case reflect.Uint32:
		return arrayMask | C.metaffi_uint32_type, is1DArray
	case reflect.Int64:
		return arrayMask | C.metaffi_int64_type, is1DArray
	case reflect.Uint64:
		return arrayMask | C.metaffi_uint64_type, is1DArray
	case reflect.Bool:
		return arrayMask | C.metaffi_bool_type, is1DArray
	case reflect.String:
		return arrayMask | C.metaffi_string8_type, is1DArray
	case reflect.Func:
		return arrayMask | C.metaffi_callable_type, is1DArray
	case reflect.Interface:
		if t.NumMethod() == 0 {

			// []interface{}
			if arrayMask == C.metaffi_array_type {

				// if one of the elements is a slice - 1D array is false
				for i := 0; i < v.Len(); i++ {
					curv := v.Index(i)
					if curv.Kind() == reflect.Slice {
						is1DArray = false
						detectedType, _ = getMetaFFITypeFromGoType(curv)
						break
					} else if curv.Elem().Kind() == reflect.Slice {
						is1DArray = false
						detectedType, _ = getMetaFFITypeFromGoType(curv.Elem())
						break
					} else {
						var isInner1DArray bool
						detectedType, isInner1DArray = getMetaFFITypeFromGoType(curv)
						if isInner1DArray && is1DArray {
							is1DArray = false
						}
					}
				}

			} else { // interface{}

				var isInner1DArray bool
				detectedType, isInner1DArray = getMetaFFITypeFromGoType(v.Elem())
				if isInner1DArray && is1DArray {
					is1DArray = false
				}
			}

			return arrayMask | detectedType, is1DArray

		} else {
			return arrayMask | C.metaffi_handle_type, is1DArray // interface of a struct - handle
		}
	default:
		return arrayMask | C.metaffi_handle_type, is1DArray
	}
}

func GetGoObject(h *C.struct_cdt_metaffi_handle) interface{} {

	if h == nil || uintptr(h.handle) == uintptr(0) {
		return nil
	}

	if h.runtime_id == GO_RUNTIME_ID {
		return GetObject(Handle(h.handle))
	} else {
		return MetaFFIHandle{
			Val:       Handle(h.handle),
			RuntimeID: uint64(h.runtime_id),
			CReleaser: unsafe.Pointer(h.release),
		}
	}
}

func GoObjectToMetaffiHandle(p *C.struct_cdt_metaffi_handle, val interface{}) {
	if h, ok := val.(MetaFFIHandle); ok {
		C.GoMetaFFIHandleTocdt_metaffi_handle(p, unsafe.Pointer(h.Val), C.uint64_t(h.RuntimeID), h.CReleaser)
	} else {

		// set Go object into cdt_metaffi_handle
		if val == nil {
			(*p).handle = C.metaffi_handle(uintptr(0))
			(*p).runtime_id = 0
			(*p).release = nil
		} else {
			C.GoMetaFFIHandleTocdt_metaffi_handle(p, unsafe.Pointer(SetObject(val)), GO_RUNTIME_ID, GetReleaserCFunction())
		}
	}
}

type ConstructContext struct {
	Input    interface{}
	TypeInfo IDL.MetaFFITypeInfo
	Cdt      CDT
}

func ConstructCDT(item *CDT, currentIndex []uint64, ctxt *ConstructContext, knownType *IDL.MetaFFITypeInfo) error {
	var ti C.struct_metaffi_type_info
	if knownType == nil || knownType.Type == C.metaffi_any_type {
		ti = getTypeInfo(currentIndex, ctxt)
	} else {
		var cti C.struct_metaffi_type_info
		cti._type = C.metaffi_type(knownType.Type)

		if knownType.Alias != "" {
			cti.alias = nil                       // TODO: set alias
			cti.is_free_alias = C.metaffi_bool(0) // TODO: change to TRUE
			//		cMetaFFIType.is_free_alias = C.metaffi_bool(1)
		} else {
			cti.alias = nil
			cti.is_free_alias = C.metaffi_bool(0)
		}

		cti.fixed_dimensions = C.int64_t(knownType.Dimensions)

		ti = cti
	}

	if ti._type == C.metaffi_any_type {
		return fmt.Errorf("get_type_info must return a concrete type, not dynamic type like metaffi_any_type")
	}

	if ti._type != C.metaffi_any_type && ti.fixed_dimensions > 0 {
		ti._type |= C.metaffi_array_type
	}

	item.SetTypeVal(ti._type)

	var commonType C.metaffi_type
	if ti._type&C.metaffi_array_type != 0 && ti._type != C.metaffi_array_type {
		commonType = ti._type &^ C.metaffi_array_type
		item.SetTypeVal(C.metaffi_array_type)
	}

	switch item.GetTypeVal() {
	case C.metaffi_float64_type:
		goval := getElement(currentIndex, ctxt.Input)
		val := C.metaffi_float64(goval.Interface().(float64))
		item.SetFloat64Val(val)
		item.SetFreeRequired(false)

	case C.metaffi_float32_type:
		goval := getElement(currentIndex, ctxt.Input)
		val := C.metaffi_float32(goval.Interface().(float32))
		item.SetFloat32Val(val)
		item.SetFreeRequired(false)

	case C.metaffi_int8_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := C.metaffi_int8(goVal.Interface().(int8))
		item.SetInt8Val(val)
		item.SetFreeRequired(false)

	case C.metaffi_uint8_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := C.metaffi_uint8(goVal.Interface().(uint8))
		item.SetUInt8Val(val)
		item.SetFreeRequired(false)

	case C.metaffi_int16_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := C.metaffi_int16(goVal.Interface().(int16))
		item.SetInt16Val(val)
		item.SetFreeRequired(false)

	case C.metaffi_uint16_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := C.metaffi_uint16(goVal.Interface().(uint16))
		item.SetUInt16Val(val)
		item.SetFreeRequired(false)

	case C.metaffi_int32_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := C.metaffi_int32(goVal.Interface().(int32))
		item.SetInt32Val(val)
		item.SetFreeRequired(false)

	case C.metaffi_uint32_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := C.metaffi_uint32(goVal.Interface().(uint32))
		item.SetUInt32Val(val)
		item.SetFreeRequired(false)

	case C.metaffi_int64_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := C.metaffi_int64(goVal.Int())
		item.SetInt64Val(val)
		item.SetFreeRequired(false)

	case C.metaffi_uint64_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := C.metaffi_uint64(goVal.Int())
		item.SetUInt64Val(val)
		item.SetFreeRequired(false)

	case C.metaffi_bool_type:
		goVal := getElement(currentIndex, ctxt.Input)
		item.SetBool(goVal.Interface().(bool))
		item.SetFreeRequired(false)

	case C.metaffi_char8_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := goVal.Interface().(rune)
		item.SetChar8(val)
		item.SetFreeRequired(false)

	case C.metaffi_string8_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := goVal.Interface().(string)
		item.SetString8(val)
		item.SetFreeRequired(true)

	case C.metaffi_char16_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := goVal.Interface().(rune)
		item.SetChar16(val)
		item.SetFreeRequired(false)

	case C.metaffi_string16_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := goVal.Interface().(string)
		item.SetString16(val)
		item.SetFreeRequired(true)

	case C.metaffi_char32_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := goVal.Interface().(rune)
		item.SetChar32(val)
		item.SetFreeRequired(false)

	case C.metaffi_string32_type:
		goVal := getElement(currentIndex, ctxt.Input)
		val := goVal.Interface().(string)
		item.SetString32(val)
		item.SetFreeRequired(true)

	case C.metaffi_handle_type:
		goVal := getElement(currentIndex, ctxt.Input)

		if !goVal.IsValid() { // null handle
			cdtHandle := NewCDTMetaFFIHandle(nil, 0, nil)
			item.SetHandleStruct(cdtHandle)
		} else {
			val := goVal.Interface()
			if cdth, ok := val.(*CDTMetaFFIHandle); ok {
				item.SetHandleStruct(cdth)
			} else if h, ok := val.(MetaFFIHandle); ok {
				item.SetHandleStruct(NewCDTMetaFFIHandle(h.Val, h.RuntimeID, h.CReleaser))
			} else {
				newHandle := SetObject(val)
				cdtHandle := NewCDTMetaFFIHandle(newHandle, GO_RUNTIME_ID, GetReleaserCFunction())
				item.SetHandleStruct(cdtHandle)
			}
		}

	case C.metaffi_null_type:
		item.SetTypeVal(C.metaffi_null_type)
		item.SetFreeRequired(false)
	case C.metaffi_array_type:
		var isManuallyConstructArray, isFixedDimension, is1DArray C.metaffi_bool
		arrayLength := getArrayMetadata(currentIndex, &isFixedDimension, &is1DArray, &commonType, &isManuallyConstructArray, ctxt)

		if commonType != C.metaffi_any_type {
			item.SetTypeVal(C.metaffi_array_type | commonType)
		} else {
			item.SetTypeVal(C.metaffi_array_type)
		}

		item.SetFreeRequired(true)
		arr := NewCDTSFromSize(uint64(arrayLength), -1)

		item.SetArray(arr)
		arr.SetLength(arrayLength)

		var foundDims C.metaffi_int64
		if isFixedDimension != 0 {
			foundDims = C.INT_MIN
		} else {
			foundDims = C.MIXED_OR_UNKNOWN_DIMENSIONS
		}

		for i := 0; i < int(arrayLength); i++ {
			newIndex := append(currentIndex, uint64(i))
			newItem := arr.GetCDT(i)
			if err := ConstructCDT(newItem, newIndex, ctxt, nil); err != nil {
				return err
			}

			if is1DArray != 0 && newItem.GetTypeVal()&C.metaffi_array_type != 0 {
				return fmt.Errorf("Something is wrong - 1D array cannot contain another array")
			} else if is1DArray == 0 {
				if foundDims == C.MIXED_OR_UNKNOWN_DIMENSIONS {
					continue
				}

				if newItem.GetTypeVal()&C.metaffi_array_type == 0 {
					foundDims = C.MIXED_OR_UNKNOWN_DIMENSIONS
				}

				if foundDims == C.INT_MIN {
					foundDims = newItem.GetArray().GetFixedDimensions()
				} else if foundDims != newItem.GetArray().GetFixedDimensions() {
					foundDims = C.MIXED_OR_UNKNOWN_DIMENSIONS
				}
			}
		}

		if is1DArray != 0 {
			item.GetArray().SetFixedDimensions(1)
		} else if foundDims != C.MIXED_OR_UNKNOWN_DIMENSIONS {
			item.GetArray().SetFixedDimensions(foundDims + 1)
		} else {
			item.GetArray().SetFixedDimensions(C.MIXED_OR_UNKNOWN_DIMENSIONS)
		}
	case C.metaffi_callable_type:
		goVal := getElement(currentIndex, ctxt.Input)
		SetGoFuncCallableInCDT(item, goVal)
	default:
		return fmt.Errorf("Unknown type while constructing CDTS: %v", item.GetTypeVal())
	}
	return nil
}

type TraverseContext struct {
	ObjectType  reflect.Type
	ObjectValue reflect.Value
	Result      interface{}
}

func TraverseCDTS(arr *CDTS, startingIndex []uint64, ctxt *TraverseContext) error {
	type queueItem struct {
		currentIndex []uint64
		pcdt         *CDT
	}

	queue := make([]queueItem, 0)

	if startingIndex == nil {
		startingIndex = make([]uint64, 0)
	}

	for i := uint64(0); i < uint64(arr.GetLength()); i++ {
		index := append([]uint64(nil), startingIndex...)
		index = append(index, i)

		item := queueItem{
			currentIndex: index,
			pcdt:         arr.GetCDT(int(i)),
		}

		queue = append(queue, item)
	}

	for len(queue) > 0 {
		current := queue[0]
		queue = queue[1:]

		err := TraverseCDT(current.pcdt, current.currentIndex, ctxt)
		if err != nil {
			return err
		}
	}

	return nil
}

func TraverseCDT(item *CDT, currentIndex []uint64, ctxt *TraverseContext) error {

	if item.GetTypeVal() == C.metaffi_any_type {
		return fmt.Errorf("traversed CDT must have a concrete type, not dynamic type like metaffi_any_type")
	}

	commonType := C.metaffi_type(C.metaffi_any_type) // common type for all elements in the array
	typeToUse := item.GetTypeVal()
	if (typeToUse&C.metaffi_array_type) != 0 && (typeToUse != C.metaffi_array_type) {
		removeArrayFlag := ^C.metaffi_array_type
		commonType = C.metaffi_type(C.metaffi_type(typeToUse) & C.metaffi_type(removeArrayFlag))
		typeToUse = C.metaffi_array_type
	}

	switch typeToUse {
	case C.metaffi_float64_type:
		{
			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = item.GetFloat64()
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.SetFloat(item.GetFloat64())
			}
		}

	case C.metaffi_float32_type:
		{
			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = item.GetFloat32()
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(item.GetFloat32()))
			}
		}

	case C.metaffi_int8_type:
		{
			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = item.GetInt8()
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(item.GetInt8()))
			}
		}

	case C.metaffi_uint8_type:
		{
			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = item.GetUInt8()
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(item.GetUInt8()))
			}
		}

	case C.metaffi_int16_type:
		{
			val := item.GetInt16()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_uint16_type:
		{
			val := item.GetUInt16()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_int32_type:
		{
			val := item.GetInt32()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_uint32_type:
		{
			val := item.GetUInt32()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_int64_type:
		{
			val := item.GetInt64()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_uint64_type:
		{
			val := item.GetUInt64()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_bool_type:
		{
			val := item.GetBool()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_char8_type:
		{
			val := item.GetChar8()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_string8_type:
		{
			val := item.GetString8()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_char16_type:
		{
			val := item.GetChar16()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_string16_type:
		{
			val := item.GetString16()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_char32_type:
		{
			val := item.GetChar32()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_string32_type:
		{
			val := item.GetString32()

			if currentIndex == nil || len(currentIndex) == 0 {
				ctxt.Result = val
			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(val))
			}
		}

	case C.metaffi_handle_type:
		{
			val := item.GetHandleStruct()

			if currentIndex == nil || len(currentIndex) == 0 {
				// If not Go, return CDTMetaFFIHandle
				ctxt.Result = GetGoObject(val)

			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(GetGoObject(val)))
			}
		}

	case C.metaffi_callable_type:
		{
			callableStruct := item.GetCallableVal()

			if currentIndex == nil || len(currentIndex) == 0 {
				// Return a Go func wrapper around the foreign callable
				if callableStruct == nil || callableStruct.Val == nil {
					ctxt.Result = nil
				} else {
					// If we have a target type (from ObjectType), use it; otherwise
					// return the raw MetaFFICallable for the caller to interpret.
					if ctxt.ObjectType != nil && ctxt.ObjectType.Kind() == reflect.Func {
						goFunc := wrapForeignCallableAsGoFunc(callableStruct, ctxt.ObjectType)
						ctxt.Result = goFunc.Interface()
					} else {
						ctxt.Result = callableStruct
					}
				}
			} else {
				// Within an array – store MetaFFICallable as interface{}
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(callableStruct))
			}
		}

	case C.metaffi_null_type:
		{
			if currentIndex == nil || len(currentIndex) == 0 {
				// If not Go, return CDTMetaFFIHandle
				ctxt.Result = nil

			} else { // within an array
				elem := getElement(currentIndex, ctxt.Result)
				elem.Set(reflect.ValueOf(nil))
			}
		}

	case C.metaffi_array_type:
		{
			arr := item.GetArray()

			if arr == nil {
				return fmt.Errorf("Array value is null")
			}

			isContinueTraverse := onArray(currentIndex, arr.c, arr.GetFixedDimensions(), C.metaffi_type(commonType), ctxt)

			if isContinueTraverse {
				err := TraverseCDTS(arr, currentIndex, ctxt)
				if err != nil {
					return err
				}
			}
		}

	default:
		{
			return fmt.Errorf("Unknown type while traversing CDTS: %v", item.GetTypeVal())
		}
	}

	return nil
}
