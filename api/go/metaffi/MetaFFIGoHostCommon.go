package metaffi

/*
#cgo !windows LDFLAGS: -L. -ldl
#cgo LDFLAGS: -Wl,--allow-multiple-definition

#include <stdlib.h>
#include <stdint.h>
#include <include/cdt.h>
#include <include/xllr_capi_loader.h>
#include <include/xllr_capi_loader.c>

struct cdt_metaffi_handle get_null_handle()
{
	struct cdt_metaffi_handle res;
	res.handle = NULL;
	res.runtime_id = 0;
	res.release = NULL;
	return res;
}

metaffi_size get_int_item(metaffi_size* array, int index)
{
	return array[index];
}

void set_int_item(metaffi_size* array, int index, metaffi_size value)
{
	array[index] = value;
}

void* convert_union_to_ptr(void* p)
{
	return p;
}

struct cdts* cast_to_cdts(void* p)
{
	return (struct cdts*)p;
}

struct cdt* get_cdt_index(struct cdt* p, int index)
{
	return &p[index];
}

struct cdt* cast_to_cdt(void* p)
{
	return (struct cdt*)p;
}

void set_cdt_string8(struct cdt* p, metaffi_string8 val)
{
	p->cdt_val.string8_val = val;
}

void set_cdt_type(struct cdt* p, metaffi_type t)
{
	p->type = t;
}

metaffi_type get_cdt_type(struct cdt* p)
{
	return p->type;
}

void* get_index(void* p, int index)
{
	return p + index;
}

int8_t get_int8_item(metaffi_int8* array, int index)
{
	return array[index];
}

metaffi_size get_metaffi_size_item(metaffi_size* array, int index)
{
	return array[index];
}

void set_metaffi_size_item(metaffi_size* array, int index, metaffi_size value)
{
	array[index] = value;
}

*/
import "C"
import (
	"reflect"
	"unsafe"

	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

func GetCacheSize() int {
	return int(C.get_cache_size())
}

func createMetaFFITypeInfoArray(paramsTypes []IDL.MetaFFITypeInfo) *C.struct_metaffi_type_info {
	size := len(paramsTypes)
	metaffiArray := C.malloc(C.size_t(size) * C.size_t(unsafe.Sizeof(C.struct_metaffi_type_info{})))

	for i, v := range paramsTypes {
		metaffi := (*C.struct_metaffi_type_info)(unsafe.Pointer(uintptr(metaffiArray) + uintptr(i)*unsafe.Sizeof(C.struct_metaffi_type_info{})))
		metaffi._type = C.metaffi_type(v.Type)

		if v.Alias != "" {
			metaffi.alias = C.CString(v.Alias)
		} else {
			metaffi.alias = nil
		}

		metaffi.fixed_dimensions = C.int64_t(v.Dimensions)
	}

	return (*C.struct_metaffi_type_info)(metaffiArray)
}

func freeMetaFFITypeInfoArray(metaffiArray *C.struct_metaffi_type_info, size int) {
	for i := 0; i < size; i++ {
		metaffi := (*C.struct_metaffi_type_info)(unsafe.Pointer(uintptr(unsafe.Pointer(metaffiArray)) + uintptr(i)*unsafe.Sizeof(C.struct_metaffi_type_info{})))
		if metaffi.alias != nil {
			C.free(unsafe.Pointer(metaffi.alias))
		}
	}
	C.free(unsafe.Pointer(metaffiArray))
}

//--------------------------------------------------------------------

func FromCDTToGo(pvcdt unsafe.Pointer, i int, objectType reflect.Type) interface{} {
	pcdt := C.cast_to_cdt(pvcdt)
	pcdt = C.get_cdt_index(pcdt, C.int(i))
	ctxt := &TraverseContext{ObjectType: objectType}
	cdtWrap := &CDT{c: pcdt}
	if err := TraverseCDT(cdtWrap, nil, ctxt); err != nil {
		panic(err)
	}
	return ctxt.Result
}

func GetMetaFFITypeInfo(input interface{}) (IDL.MetaFFITypeInfo, reflect.Type) {

	if input == nil {
		return IDL.MetaFFITypeInfo{StringType: IDL.NULL,
			Alias:      "",
			Type:       IDL.METAFFI_TYPE_NULL,
			Dimensions: 0}, nil
	}

	t := reflect.TypeOf(input)
	var metaFFIType IDL.MetaFFIType
	var alias string
	var dimensions int

	// Check if it's a slice
	if t.Kind() == reflect.Slice {
		dimensions = 1
		t = t.Elem()
		for t.Kind() == reflect.Slice {
			dimensions++
			t = t.Elem()
		}
	}

	// Check if it's a primitive type
	switch t.Kind() {
	case reflect.Float64:
		metaFFIType = IDL.FLOAT64
	case reflect.Float32:
		metaFFIType = IDL.FLOAT32
	case reflect.Int8:
		metaFFIType = IDL.INT8
	case reflect.Int16:
		metaFFIType = IDL.INT16
	case reflect.Int32:
		metaFFIType = IDL.INT32
	case reflect.Int64:
		metaFFIType = IDL.INT64
	case reflect.Uint8:
		metaFFIType = IDL.UINT8
	case reflect.Uint16:
		metaFFIType = IDL.UINT16
	case reflect.Uint32:
		metaFFIType = IDL.UINT32
	case reflect.Uint64:
		metaFFIType = IDL.UINT64
	case reflect.Bool:
		metaFFIType = IDL.BOOL
	case reflect.String:
		metaFFIType = IDL.STRING8
	default:
		metaFFIType = IDL.HANDLE
	}

	// If it's a slice, append "_array" to the metaFFIType
	if dimensions > 0 {
		metaFFIType = IDL.MetaFFIType(string(metaFFIType) + "_array")
	}

	// Check if it's a named type
	if t.Name() != "" && t.Name() != string(metaFFIType) {
		alias = t.Name()
	}

	return IDL.MetaFFITypeInfo{
		StringType: metaFFIType,
		Alias:      alias,
		Type:       IDL.TypeStringToTypeEnum[metaFFIType],
		Dimensions: dimensions,
	}, t
}

func FromGoToCDT(input interface{}, pvcdt unsafe.Pointer, t IDL.MetaFFITypeInfo, i int) {
	pcdt := C.cast_to_cdt(pvcdt)
	pcdt = C.get_cdt_index(pcdt, C.int(i))
	ctxt := &ConstructContext{Input: input, TypeInfo: t, Cdt: CDT{c: pcdt}}
	gocdt := &CDT{c: pcdt}
	index := make([]uint64, 0)
	if err := ConstructCDT(gocdt, index, ctxt, &t); err != nil {
		panic(err)
	}
}
