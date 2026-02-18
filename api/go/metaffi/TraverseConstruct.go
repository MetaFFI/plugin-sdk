package metaffi

/*
#cgo !windows LDFLAGS: -L. -ldl
#cgo LDFLAGS: -Wl,--allow-multiple-definition

#include <include/xllr_capi_loader.h>
#include <include/xllr_capi_loader.c>
#include <include/cdts_traverse_construct.h>
#include <include/metaffi_primitives.h>
#include <stdint.h>
#include <stdlib.h>

int tc_is_packed_array(metaffi_type t) {
	return (t & metaffi_packed_type) && (t & metaffi_array_type);
}

metaffi_type tc_packed_element_type(metaffi_type t) {
	return t & ~(metaffi_array_type | metaffi_packed_type);
}

struct cdt_packed_array* tc_alloc_packed_array(void* data, metaffi_size length) {
	struct cdt_packed_array* p = (struct cdt_packed_array*)xllr_alloc_memory(sizeof(struct cdt_packed_array));
	p->data = data;
	p->length = length;
	return p;
}

void tc_set_cdt_packed_array(struct cdt* c, struct cdt_packed_array* packed, metaffi_type element_type) {
	c->type = element_type | metaffi_array_type | metaffi_packed_type;
	c->free_required = 1;
	c->cdt_val.packed_array_val = packed;
}

struct cdt_packed_array* tc_get_cdt_packed_array(struct cdt* c) {
	return c->cdt_val.packed_array_val;
}

void* tc_get_packed_array_data(struct cdt_packed_array* p) {
	return p ? p->data : 0;
}

metaffi_size tc_get_packed_array_length(struct cdt_packed_array* p) {
	return p ? p->length : 0;
}

void GoMetaFFIHandleTocdt_metaffi_handle(struct cdt_metaffi_handle* p , void* handle, uint64_t runtime_id, void* release) {
	p->handle = (metaffi_handle)handle;
	p->runtime_id = runtime_id;
	p->release = (void (*)(struct cdt_metaffi_handle*))release;
}

void copy_cdts_to_uint8_buffer(struct cdts* src, uint8_t* dst, metaffi_size len) {
	for (metaffi_size i = 0; i < len; i++) {
		dst[i] = (uint8_t)src->arr[i].cdt_val.uint8_val;
	}
}

void copy_cdts_to_int8_buffer(struct cdts* src, int8_t* dst, metaffi_size len) {
	for (metaffi_size i = 0; i < len; i++) {
		dst[i] = (int8_t)src->arr[i].cdt_val.int8_val;
	}
}

void fill_cdts_from_uint8_buffer(struct cdts* dst, const uint8_t* src, metaffi_size len) {
	for (metaffi_size i = 0; i < len; i++) {
		struct cdt* elem = &dst->arr[i];
		elem->free_required = 0;
		elem->type = metaffi_uint8_type;
		elem->cdt_val.uint8_val = (metaffi_uint8)src[i];
	}
}

void fill_cdts_from_int8_buffer(struct cdts* dst, const int8_t* src, metaffi_size len) {
	for (metaffi_size i = 0; i < len; i++) {
		struct cdt* elem = &dst->arr[i];
		elem->free_required = 0;
		elem->type = metaffi_int8_type;
		elem->cdt_val.int8_val = (metaffi_int8)src[i];
	}
}

#define DEFINE_COPY_CDTS_NUMERIC(NAME, CTYPE, METAFFI_TYPE, FIELD) \
void copy_cdts_to_##NAME##_buffer(struct cdts* src, CTYPE* dst, metaffi_size len) { \
	for (metaffi_size i = 0; i < len; i++) { \
		dst[i] = (CTYPE)src->arr[i].cdt_val.FIELD; \
	} \
}

#define DEFINE_FILL_CDTS_NUMERIC(NAME, CTYPE, METAFFI_TYPE, FIELD) \
void fill_cdts_from_##NAME##_buffer(struct cdts* dst, const CTYPE* src, metaffi_size len) { \
	for (metaffi_size i = 0; i < len; i++) { \
		struct cdt* elem = &dst->arr[i]; \
		elem->free_required = 0; \
		elem->type = METAFFI_TYPE; \
		elem->cdt_val.FIELD = src[i]; \
	} \
}

DEFINE_COPY_CDTS_NUMERIC(int16, int16_t, metaffi_int16_type, int16_val)
DEFINE_COPY_CDTS_NUMERIC(uint16, uint16_t, metaffi_uint16_type, uint16_val)
DEFINE_COPY_CDTS_NUMERIC(int32, int32_t, metaffi_int32_type, int32_val)
DEFINE_COPY_CDTS_NUMERIC(uint32, uint32_t, metaffi_uint32_type, uint32_val)
DEFINE_COPY_CDTS_NUMERIC(int64, int64_t, metaffi_int64_type, int64_val)
DEFINE_COPY_CDTS_NUMERIC(uint64, uint64_t, metaffi_uint64_type, uint64_val)
DEFINE_COPY_CDTS_NUMERIC(float32, float, metaffi_float32_type, float32_val)
DEFINE_COPY_CDTS_NUMERIC(float64, double, metaffi_float64_type, float64_val)
DEFINE_COPY_CDTS_NUMERIC(bool, metaffi_bool, metaffi_bool_type, bool_val)

DEFINE_FILL_CDTS_NUMERIC(int16, int16_t, metaffi_int16_type, int16_val)
DEFINE_FILL_CDTS_NUMERIC(uint16, uint16_t, metaffi_uint16_type, uint16_val)
DEFINE_FILL_CDTS_NUMERIC(int32, int32_t, metaffi_int32_type, int32_val)
DEFINE_FILL_CDTS_NUMERIC(uint32, uint32_t, metaffi_uint32_type, uint32_val)
DEFINE_FILL_CDTS_NUMERIC(int64, int64_t, metaffi_int64_type, int64_val)
DEFINE_FILL_CDTS_NUMERIC(uint64, uint64_t, metaffi_uint64_type, uint64_val)
DEFINE_FILL_CDTS_NUMERIC(float32, float, metaffi_float32_type, float32_val)
DEFINE_FILL_CDTS_NUMERIC(float64, double, metaffi_float64_type, float64_val)
DEFINE_FILL_CDTS_NUMERIC(bool, metaffi_bool, metaffi_bool_type, bool_val)

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
		return unwrapInterfaceValue(reflect.ValueOf(root))
	}

	// Traverse the root object
	v := unwrapInterfaceValue(reflect.ValueOf(root))
	for _, idx := range index {
		v = unwrapInterfaceValue(v)
		if v.Kind() == reflect.Slice || v.Kind() == reflect.Array {
			if idx < uint64(v.Len()) {
				v = unwrapInterfaceValue(v.Index(int(idx)))
			} else {
				panic(fmt.Sprintf("Index out of range: %v. Length: %v", idx, v.Len()))
			}
		} else {
			panic(fmt.Sprintf("Unsupported type: %T", v.Interface()))
		}
	}

	return unwrapInterfaceValue(v)
}

func isByteLikeCommonType(t C.metaffi_type) bool {
	return t == C.metaffi_uint8_type || t == C.metaffi_int8_type
}

func isFastPrimitiveCommonType(t C.metaffi_type) bool {
	switch t {
	case C.metaffi_int8_type, C.metaffi_uint8_type,
		C.metaffi_int16_type, C.metaffi_uint16_type,
		C.metaffi_int32_type, C.metaffi_uint32_type,
		C.metaffi_int64_type, C.metaffi_uint64_type,
		C.metaffi_float32_type, C.metaffi_float64_type,
		C.metaffi_bool_type:
		return true
	default:
		return false
	}
}

func unwrapInterfaceValue(v reflect.Value) reflect.Value {
	for v.IsValid() && v.Kind() == reflect.Interface && !v.IsNil() {
		v = v.Elem()
	}
	return v
}

func setArrayResultValue(target reflect.Value, value reflect.Value) error {
	if !target.CanSet() {
		return fmt.Errorf("target is not settable for fast array path")
	}
	if value.Type().AssignableTo(target.Type()) {
		target.Set(value)
		return nil
	}
	if value.Type().ConvertibleTo(target.Type()) {
		target.Set(value.Convert(target.Type()))
		return nil
	}
	if target.Kind() == reflect.Interface {
		target.Set(value)
		return nil
	}
	return fmt.Errorf("cannot assign fast array result %v to target type %v", value.Type(), target.Type())
}

func tryFastTraversePrimitiveArray(item *CDT, currentIndex []uint64, ctxt *TraverseContext, commonType C.metaffi_type) (bool, error) {
	if !isFastPrimitiveCommonType(commonType) {
		return false, nil
	}

	arr := item.GetArray()
	if arr == nil {
		return false, fmt.Errorf("Array value is null")
	}
	if arr.GetFixedDimensions() != 1 {
		return false, nil
	}

	length := int(arr.GetLength())
	if commonType == C.metaffi_uint8_type {
		result := make([]uint8, length)
		if length > 0 {
			C.copy_cdts_to_uint8_buffer(arr.c, (*C.uint8_t)(unsafe.Pointer(&result[0])), C.metaffi_size(length))
		}

		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}

		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	}

	switch commonType {
	case C.metaffi_int8_type:
		result := make([]int8, length)
		if length > 0 {
			C.copy_cdts_to_int8_buffer(arr.c, (*C.int8_t)(unsafe.Pointer(&result[0])), C.metaffi_size(length))
		}
		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}
		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	case C.metaffi_int16_type:
		result := make([]int16, length)
		if length > 0 {
			C.copy_cdts_to_int16_buffer(arr.c, (*C.int16_t)(unsafe.Pointer(&result[0])), C.metaffi_size(length))
		}
		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}
		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	case C.metaffi_uint16_type:
		result := make([]uint16, length)
		if length > 0 {
			C.copy_cdts_to_uint16_buffer(arr.c, (*C.uint16_t)(unsafe.Pointer(&result[0])), C.metaffi_size(length))
		}
		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}
		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	case C.metaffi_int32_type:
		result := make([]int32, length)
		if length > 0 {
			C.copy_cdts_to_int32_buffer(arr.c, (*C.int32_t)(unsafe.Pointer(&result[0])), C.metaffi_size(length))
		}
		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}
		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	case C.metaffi_uint32_type:
		result := make([]uint32, length)
		if length > 0 {
			C.copy_cdts_to_uint32_buffer(arr.c, (*C.uint32_t)(unsafe.Pointer(&result[0])), C.metaffi_size(length))
		}
		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}
		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	case C.metaffi_int64_type:
		result := make([]int64, length)
		if length > 0 {
			C.copy_cdts_to_int64_buffer(arr.c, (*C.int64_t)(unsafe.Pointer(&result[0])), C.metaffi_size(length))
		}
		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}
		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	case C.metaffi_uint64_type:
		result := make([]uint64, length)
		if length > 0 {
			C.copy_cdts_to_uint64_buffer(arr.c, (*C.uint64_t)(unsafe.Pointer(&result[0])), C.metaffi_size(length))
		}
		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}
		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	case C.metaffi_float32_type:
		result := make([]float32, length)
		if length > 0 {
			C.copy_cdts_to_float32_buffer(arr.c, (*C.float)(unsafe.Pointer(&result[0])), C.metaffi_size(length))
		}
		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}
		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	case C.metaffi_float64_type:
		result := make([]float64, length)
		if length > 0 {
			C.copy_cdts_to_float64_buffer(arr.c, (*C.double)(unsafe.Pointer(&result[0])), C.metaffi_size(length))
		}
		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}
		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	case C.metaffi_bool_type:
		cbuf := make([]C.metaffi_bool, length)
		if length > 0 {
			C.copy_cdts_to_bool_buffer(arr.c, (*C.metaffi_bool)(unsafe.Pointer(&cbuf[0])), C.metaffi_size(length))
		}
		result := make([]bool, length)
		for i, v := range cbuf {
			result[i] = v != 0
		}
		if currentIndex == nil || len(currentIndex) == 0 {
			ctxt.Result = result
			return true, nil
		}
		elem := getElement(currentIndex, ctxt.Result)
		return true, setArrayResultValue(elem, reflect.ValueOf(result))
	default:
		return false, nil
	}
}

func tryFastConstructPrimitiveArray(goVal reflect.Value, arr *CDTS, commonType C.metaffi_type) (bool, error) {
	if !isFastPrimitiveCommonType(commonType) {
		return false, nil
	}

	goVal = unwrapInterfaceValue(goVal)
	if !goVal.IsValid() || goVal.Kind() != reflect.Slice {
		return false, nil
	}

	length := goVal.Len()
	if int(arr.GetLength()) != length {
		return false, fmt.Errorf("array length mismatch in fast path: expected %d, got %d", int(arr.GetLength()), length)
	}
	if length == 0 {
		return true, nil
	}

	switch commonType {
	case C.metaffi_uint8_type:
		vals, ok := goVal.Interface().([]uint8)
		if !ok {
			return false, nil
		}
		C.fill_cdts_from_uint8_buffer(arr.c, (*C.uint8_t)(unsafe.Pointer(&vals[0])), C.metaffi_size(length))
		return true, nil
	case C.metaffi_int8_type:
		vals, ok := goVal.Interface().([]int8)
		if !ok {
			return false, nil
		}
		C.fill_cdts_from_int8_buffer(arr.c, (*C.int8_t)(unsafe.Pointer(&vals[0])), C.metaffi_size(length))
		return true, nil
	case C.metaffi_int16_type:
		vals, ok := goVal.Interface().([]int16)
		if !ok {
			return false, nil
		}
		C.fill_cdts_from_int16_buffer(arr.c, (*C.int16_t)(unsafe.Pointer(&vals[0])), C.metaffi_size(length))
		return true, nil
	case C.metaffi_uint16_type:
		vals, ok := goVal.Interface().([]uint16)
		if !ok {
			return false, nil
		}
		C.fill_cdts_from_uint16_buffer(arr.c, (*C.uint16_t)(unsafe.Pointer(&vals[0])), C.metaffi_size(length))
		return true, nil
	case C.metaffi_int32_type:
		vals, ok := goVal.Interface().([]int32)
		if !ok {
			return false, nil
		}
		C.fill_cdts_from_int32_buffer(arr.c, (*C.int32_t)(unsafe.Pointer(&vals[0])), C.metaffi_size(length))
		return true, nil
	case C.metaffi_uint32_type:
		vals, ok := goVal.Interface().([]uint32)
		if !ok {
			return false, nil
		}
		C.fill_cdts_from_uint32_buffer(arr.c, (*C.uint32_t)(unsafe.Pointer(&vals[0])), C.metaffi_size(length))
		return true, nil
	case C.metaffi_int64_type:
		vals, ok := goVal.Interface().([]int64)
		if !ok {
			return false, nil
		}
		C.fill_cdts_from_int64_buffer(arr.c, (*C.int64_t)(unsafe.Pointer(&vals[0])), C.metaffi_size(length))
		return true, nil
	case C.metaffi_uint64_type:
		vals, ok := goVal.Interface().([]uint64)
		if !ok {
			return false, nil
		}
		C.fill_cdts_from_uint64_buffer(arr.c, (*C.uint64_t)(unsafe.Pointer(&vals[0])), C.metaffi_size(length))
		return true, nil
	case C.metaffi_float32_type:
		vals, ok := goVal.Interface().([]float32)
		if !ok {
			return false, nil
		}
		C.fill_cdts_from_float32_buffer(arr.c, (*C.float)(unsafe.Pointer(&vals[0])), C.metaffi_size(length))
		return true, nil
	case C.metaffi_float64_type:
		vals, ok := goVal.Interface().([]float64)
		if !ok {
			return false, nil
		}
		C.fill_cdts_from_float64_buffer(arr.c, (*C.double)(unsafe.Pointer(&vals[0])), C.metaffi_size(length))
		return true, nil
	case C.metaffi_bool_type:
		vals, ok := goVal.Interface().([]bool)
		if !ok {
			return false, nil
		}
		cvals := make([]C.metaffi_bool, len(vals))
		for i, v := range vals {
			if v {
				cvals[i] = 1
			}
		}
		C.fill_cdts_from_bool_buffer(arr.c, (*C.metaffi_bool)(unsafe.Pointer(&cvals[0])), C.metaffi_size(length))
		return true, nil
	default:
		return false, nil
	}
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
				haveDetected := false
				for i := 0; i < v.Len(); i++ {
					curv := unwrapInterfaceValue(v.Index(i))
					if !curv.IsValid() {
						continue
					}

					if curv.Kind() == reflect.Slice {
						is1DArray = false
					}

					curDetectedType, isInner1DArray := getMetaFFITypeFromGoType(curv)
					if isInner1DArray && is1DArray {
						is1DArray = false
					}

					if !haveDetected {
						detectedType = curDetectedType
						haveDetected = true
						continue
					}

					// Mixed element types in []interface{} must be encoded as dynamic-any arrays.
					if detectedType != curDetectedType {
						detectedType = C.metaffi_any_type
						break
					}
				}

			} else { // interface{}

				var isInner1DArray bool
				detectedType, isInner1DArray = getMetaFFITypeFromGoType(unwrapInterfaceValue(v))
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

	// Handle packed array types: construct a contiguous typed buffer from a Go slice.
	// If the element type is unsupported (e.g. handle, callable), fall through to
	// regular per-element array construction by stripping the packed flag.
	if C.tc_is_packed_array(ti._type) != 0 {
		err := constructPackedArray(item, currentIndex, ctxt, ti._type)
		if err == nil {
			return nil
		}

		// Strip packed flag and fall through to regular array handling
		ti._type = ti._type &^ C.metaffi_packed_type
		item.SetTypeVal(ti._type)
	}

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
		// Go retains ownership of the handle; the CDT destructor must not release it.
		item.SetFreeRequired(false)

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

		if is1DArray != 0 {
			goVal := getElement(currentIndex, ctxt.Input)
			fastDone, fastErr := tryFastConstructPrimitiveArray(goVal, arr, commonType)
			if fastErr != nil {
				return fastErr
			}
			if fastDone {
				item.GetArray().SetFixedDimensions(1)
				return nil
			}
		}

		var foundDims C.metaffi_int64
		if isFixedDimension != 0 {
			foundDims = C.INT_MIN
		} else {
			foundDims = C.MIXED_OR_UNKNOWN_DIMENSIONS
		}

		var childKnownType *IDL.MetaFFITypeInfo
		if commonType != C.metaffi_any_type && ti.fixed_dimensions > 0 {
			dims := int(ti.fixed_dimensions) - 1
			if dims < 0 {
				dims = 0
			}
			known := IDL.MetaFFITypeInfo{Type: uint64(commonType), Dimensions: dims}
			childKnownType = &known
		}

		for i := 0; i < int(arrayLength); i++ {
			newIndex := append(currentIndex, uint64(i))
			newItem := arr.GetCDT(i)
			if err := ConstructCDT(newItem, newIndex, ctxt, childKnownType); err != nil {
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

	// Handle packed array types: extract contiguous typed buffer to Go slice.
	// If the element type is unsupported, fall through to regular per-element traversal.
	if C.tc_is_packed_array(item.GetTypeVal()) != 0 {
		err := traversePackedArray(item, currentIndex, ctxt)
		if err == nil {
			return nil
		}

		// Strip packed flag and fall through to regular array handling
		item.SetTypeVal(item.GetTypeVal() &^ C.metaffi_packed_type)
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
				if err := setArrayResultValue(elem, reflect.ValueOf(item.GetFloat64())); err != nil {
					return err
				}
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
			// Go takes ownership of the handle; prevent the CDT destructor
			// from releasing it (e.g. deleting the JNI global reference).
			item.SetFreeRequired(false)

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
			// Go takes ownership of the callable; prevent the CDT destructor
			// from freeing it (same as handle_type above).
			item.SetFreeRequired(false)

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
				elem.Set(reflect.Zero(elem.Type()))
			}
		}

	case C.metaffi_array_type:
		{
			fastDone, fastErr := tryFastTraversePrimitiveArray(item, currentIndex, ctxt, C.metaffi_type(commonType))
			if fastErr != nil {
				return fastErr
			}
			if fastDone {
				return nil
			}

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

// constructPackedArray converts a Go slice into a cdt_packed_array.
func constructPackedArray(item *CDT, currentIndex []uint64, ctxt *ConstructContext, fullType C.metaffi_type) error {
	elemType := C.tc_packed_element_type(fullType)
	goVal := getElement(currentIndex, ctxt.Input)

	if !goVal.IsValid() || (goVal.Kind() == reflect.Slice && goVal.IsNil()) {
		packed := C.tc_alloc_packed_array(nil, 0)
		C.tc_set_cdt_packed_array(item.c, packed, elemType)
		return nil
	}

	if goVal.Kind() != reflect.Slice {
		return fmt.Errorf("constructPackedArray: expected slice, got %v", goVal.Kind())
	}

	length := goVal.Len()
	if length == 0 {
		packed := C.tc_alloc_packed_array(nil, 0)
		C.tc_set_cdt_packed_array(item.c, packed, elemType)
		return nil
	}

	switch elemType {
	case C.metaffi_float32_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_float32(0))))
		dst := (*[1 << 30]C.metaffi_float32)(buf)[:length:length]
		for i := 0; i < length; i++ {
			dst[i] = C.metaffi_float32(goVal.Index(i).Float())
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_float64_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_float64(0))))
		dst := (*[1 << 30]C.metaffi_float64)(buf)[:length:length]
		for i := 0; i < length; i++ {
			dst[i] = C.metaffi_float64(goVal.Index(i).Float())
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_int8_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_int8(0))))
		dst := (*[1 << 30]C.metaffi_int8)(buf)[:length:length]
		for i := 0; i < length; i++ {
			dst[i] = C.metaffi_int8(goVal.Index(i).Int())
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_uint8_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_uint8(0))))
		dst := (*[1 << 30]C.metaffi_uint8)(buf)[:length:length]
		for i := 0; i < length; i++ {
			dst[i] = C.metaffi_uint8(goVal.Index(i).Uint())
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_int16_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_int16(0))))
		dst := (*[1 << 30]C.metaffi_int16)(buf)[:length:length]
		for i := 0; i < length; i++ {
			dst[i] = C.metaffi_int16(goVal.Index(i).Int())
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_uint16_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_uint16(0))))
		dst := (*[1 << 30]C.metaffi_uint16)(buf)[:length:length]
		for i := 0; i < length; i++ {
			dst[i] = C.metaffi_uint16(goVal.Index(i).Uint())
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_int32_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_int32(0))))
		dst := (*[1 << 30]C.metaffi_int32)(buf)[:length:length]
		for i := 0; i < length; i++ {
			dst[i] = C.metaffi_int32(goVal.Index(i).Int())
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_uint32_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_uint32(0))))
		dst := (*[1 << 30]C.metaffi_uint32)(buf)[:length:length]
		for i := 0; i < length; i++ {
			dst[i] = C.metaffi_uint32(goVal.Index(i).Uint())
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_int64_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_int64(0))))
		dst := (*[1 << 30]C.metaffi_int64)(buf)[:length:length]
		for i := 0; i < length; i++ {
			dst[i] = C.metaffi_int64(goVal.Index(i).Int())
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_uint64_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_uint64(0))))
		dst := (*[1 << 30]C.metaffi_uint64)(buf)[:length:length]
		for i := 0; i < length; i++ {
			dst[i] = C.metaffi_uint64(goVal.Index(i).Uint())
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_bool_type:
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(unsafe.Sizeof(C.metaffi_bool(0))))
		dst := (*[1 << 30]C.metaffi_bool)(buf)[:length:length]
		for i := 0; i < length; i++ {
			if goVal.Index(i).Bool() {
				dst[i] = 1
			} else {
				dst[i] = 0
			}
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	case C.metaffi_string8_type:
		ptrSize := unsafe.Sizeof((*C.char)(nil))
		buf := C.xllr_alloc_memory(C.uint64_t(length) * C.uint64_t(ptrSize))
		dst := (*[1 << 30]*C.char)(buf)[:length:length]
		for i := 0; i < length; i++ {
			s := goVal.Index(i).String()
			cstr := C.CString(s)
			allocated := C.xllr_alloc_string(cstr, C.uint64_t(len(s)))
			C.free(unsafe.Pointer(cstr))
			dst[i] = allocated
		}
		C.tc_set_cdt_packed_array(item.c, C.tc_alloc_packed_array(buf, C.metaffi_size(length)), elemType)

	default:
		return fmt.Errorf("constructPackedArray: unsupported element type %v", elemType)
	}

	return nil
}

// traversePackedArray converts a cdt_packed_array back to a Go slice.
func traversePackedArray(item *CDT, currentIndex []uint64, ctxt *TraverseContext) error {
	elemType := C.tc_packed_element_type(item.GetTypeVal())
	packed := C.tc_get_cdt_packed_array(item.c)
	length := int(C.tc_get_packed_array_length(packed))
	data := C.tc_get_packed_array_data(packed)

	var result interface{}

	if length == 0 || data == nil {
		switch elemType {
		case C.metaffi_float32_type:
			result = []float32{}
		case C.metaffi_float64_type:
			result = []float64{}
		case C.metaffi_int8_type:
			result = []int8{}
		case C.metaffi_uint8_type:
			result = []uint8{}
		case C.metaffi_int16_type:
			result = []int16{}
		case C.metaffi_uint16_type:
			result = []uint16{}
		case C.metaffi_int32_type:
			result = []int32{}
		case C.metaffi_uint32_type:
			result = []uint32{}
		case C.metaffi_int64_type:
			result = []int64{}
		case C.metaffi_uint64_type:
			result = []uint64{}
		case C.metaffi_bool_type:
			result = []bool{}
		case C.metaffi_string8_type:
			result = []string{}
		default:
			return fmt.Errorf("traversePackedArray: unsupported element type %v for empty array", elemType)
		}
	} else {
		switch elemType {
		case C.metaffi_float32_type:
			src := (*[1 << 30]C.metaffi_float32)(data)[:length:length]
			out := make([]float32, length)
			for i := 0; i < length; i++ {
				out[i] = float32(src[i])
			}
			result = out

		case C.metaffi_float64_type:
			src := (*[1 << 30]C.metaffi_float64)(data)[:length:length]
			out := make([]float64, length)
			for i := 0; i < length; i++ {
				out[i] = float64(src[i])
			}
			result = out

		case C.metaffi_int8_type:
			src := (*[1 << 30]C.metaffi_int8)(data)[:length:length]
			out := make([]int8, length)
			for i := 0; i < length; i++ {
				out[i] = int8(src[i])
			}
			result = out

		case C.metaffi_uint8_type:
			src := (*[1 << 30]C.metaffi_uint8)(data)[:length:length]
			out := make([]uint8, length)
			for i := 0; i < length; i++ {
				out[i] = uint8(src[i])
			}
			result = out

		case C.metaffi_int16_type:
			src := (*[1 << 30]C.metaffi_int16)(data)[:length:length]
			out := make([]int16, length)
			for i := 0; i < length; i++ {
				out[i] = int16(src[i])
			}
			result = out

		case C.metaffi_uint16_type:
			src := (*[1 << 30]C.metaffi_uint16)(data)[:length:length]
			out := make([]uint16, length)
			for i := 0; i < length; i++ {
				out[i] = uint16(src[i])
			}
			result = out

		case C.metaffi_int32_type:
			src := (*[1 << 30]C.metaffi_int32)(data)[:length:length]
			out := make([]int32, length)
			for i := 0; i < length; i++ {
				out[i] = int32(src[i])
			}
			result = out

		case C.metaffi_uint32_type:
			src := (*[1 << 30]C.metaffi_uint32)(data)[:length:length]
			out := make([]uint32, length)
			for i := 0; i < length; i++ {
				out[i] = uint32(src[i])
			}
			result = out

		case C.metaffi_int64_type:
			src := (*[1 << 30]C.metaffi_int64)(data)[:length:length]
			out := make([]int64, length)
			for i := 0; i < length; i++ {
				out[i] = int64(src[i])
			}
			result = out

		case C.metaffi_uint64_type:
			src := (*[1 << 30]C.metaffi_uint64)(data)[:length:length]
			out := make([]uint64, length)
			for i := 0; i < length; i++ {
				out[i] = uint64(src[i])
			}
			result = out

		case C.metaffi_bool_type:
			src := (*[1 << 30]C.metaffi_bool)(data)[:length:length]
			out := make([]bool, length)
			for i := 0; i < length; i++ {
				out[i] = src[i] != 0
			}
			result = out

		case C.metaffi_string8_type:
			src := (*[1 << 30]*C.char)(data)[:length:length]
			out := make([]string, length)
			for i := 0; i < length; i++ {
				if src[i] != nil {
					out[i] = C.GoString(src[i])
				}
			}
			result = out

		default:
			return fmt.Errorf("traversePackedArray: unsupported element type %v", elemType)
		}
	}

	if currentIndex == nil || len(currentIndex) == 0 {
		ctxt.Result = result
	} else {
		elem := getElement(currentIndex, ctxt.Result)
		elem.Set(reflect.ValueOf(result))
	}

	return nil
}
