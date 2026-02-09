package metaffi

/*
#cgo !windows LDFLAGS: -L. -ldl
#cgo LDFLAGS: -Wl,--allow-multiple-definition

#include <include/cdt.h>
#include <include/metaffi_primitives.h>
#include <include/xllr_capi_loader.h>
#include <stdlib.h>

metaffi_type get_cdt_type(struct cdt* c) {
    return c->type;
}

void set_cdt_type(struct cdt* c, metaffi_type t) {
    c->type = t;
}

metaffi_float32 get_cdt_float32_val(struct cdt* c) {
    return c->cdt_val.float32_val;
}

void set_cdt_float32_val(struct cdt* c, metaffi_float32 val) {
    c->cdt_val.float32_val = val;
}

metaffi_float64 get_cdt_float64_val(struct cdt* c) {
    return c->cdt_val.float64_val;
}

void set_cdt_float64_val(struct cdt* c, metaffi_float64 val) {
    c->cdt_val.float64_val = val;
}

metaffi_bool get_cdt_bool_val(struct cdt* c) {
    return c->cdt_val.bool_val;
}

void set_cdt_bool_val(struct cdt* c, metaffi_bool val) {
    c->cdt_val.bool_val = val;
}

void get_cdt_char8_val(struct cdt* c, uint8_t* out0, uint8_t* out1, uint8_t* out2, uint8_t* out3) {
	*out0 = c->cdt_val.char8_val.c[0];
	*out1 = 0;
	*out2 = 0;
	*out3 = 0;

	// get the number of bytes of the utf-8 character
	if (c->cdt_val.char8_val.c[0] & 0x80) {
		if ((c->cdt_val.char8_val.c[0] & 0xE0) == 0xC0) {
			*out1 = (uint8_t)c->cdt_val.char8_val.c[1];
		} else if ((c->cdt_val.char8_val.c[0] & 0xF0) == 0xE0) {
			*out1 = (uint8_t)c->cdt_val.char8_val.c[1];
			*out2 = (uint8_t)c->cdt_val.char8_val.c[2];
		} else if ((c->cdt_val.char8_val.c[0] & 0xF8) == 0xF0) {
			*out1 = (uint8_t)c->cdt_val.char8_val.c[1];
			*out2 = (uint8_t)c->cdt_val.char8_val.c[2];
			*out3 = (uint8_t)c->cdt_val.char8_val.c[3];
		}
	}
}

void set_cdt_char8_val(struct cdt* c, uint8_t val0, uint8_t val1, uint8_t val2, uint8_t val3_t) {
    c->cdt_val.char8_val.c[0] = val0;
	c->cdt_val.char8_val.c[1] = val1;
	c->cdt_val.char8_val.c[2] = val2;
	c->cdt_val.char8_val.c[3] = val3_t;
}

char* get_cdt_string8_val(struct cdt* c) {
	return (char*)c->cdt_val.string8_val;
}

void set_cdt_string8_val(struct cdt* c, char* val) {
	c->cdt_val.string8_val = val;
}

uint16_t get_cdt_char16_val(struct cdt* c) {
    return c->cdt_val.char16_val.c[0];
}

void set_cdt_char16_val(struct cdt* c, uint16_t val) {
    c->cdt_val.char16_val.c[0] = val;
	c->cdt_val.char16_val.c[1] = 0;
}

metaffi_string16 get_cdt_string16_val(struct cdt* c, int32_t* strlen16) {
	// find terminating null for utf-16, and return length
	*strlen16 = 0;
	while (c->cdt_val.string16_val[*strlen16] != 0) {
		*strlen16++;
	}

	return c->cdt_val.string16_val;
}

metaffi_string32 get_cdt_string32_val(struct cdt* c, int32_t* strlen32) {
	// find terminating null for utf-16, and return length
	*strlen32 = 0;
	while (c->cdt_val.string32_val[*strlen32] != 0) {
		*strlen32++;
	}

	return c->cdt_val.string32_val;
}

void set_cdt_string16_val(struct cdt* c, metaffi_string16 val) {
	c->cdt_val.string16_val = val;
}

void set_cdt_string32_val(struct cdt* c, metaffi_string32 val) {
	c->cdt_val.string32_val = val;
}

char32_t get_cdt_char32_val(struct cdt* c) {
	return c->cdt_val.char32_val.c;
}

void set_cdt_char32_val(struct cdt* c, uint32_t val) {
    c->cdt_val.char32_val.c = val;
}

struct cdt_metaffi_handle* get_cdt_handle_val(struct cdt* c) {
    return c->cdt_val.handle_val;
}

void set_cdt_handle_val(struct cdt* c, struct cdt_metaffi_handle* val) {
    c->cdt_val.handle_val = val;
}

struct cdt_metaffi_callable* get_cdt_callable_val(struct cdt* c) {
    return c->cdt_val.callable_val;
}

void set_cdt_callable_val(struct cdt* c, struct cdt_metaffi_callable* val) {
    c->cdt_val.callable_val = val;
}

metaffi_int8 get_cdt_int8_val(struct cdt* c) {
    return c->cdt_val.int8_val;
}

void set_cdt_int8_val(struct cdt* c, metaffi_int8 val) {
    c->cdt_val.int8_val = val;
}

metaffi_uint8 get_cdt_uint8_val(struct cdt* c) {
    return c->cdt_val.uint8_val;
}

void set_cdt_uint8_val(struct cdt* c, metaffi_uint8 val) {
    c->cdt_val.uint8_val = val;
}

metaffi_int16 get_cdt_int16_val(struct cdt* c) {
    return c->cdt_val.int16_val;
}

void set_cdt_int16_val(struct cdt* c, metaffi_int16 val) {
    c->cdt_val.int16_val = val;
}

metaffi_uint16 get_cdt_uint16_val(struct cdt* c) {
    return c->cdt_val.uint16_val;
}

void set_cdt_uint16_val(struct cdt* c, metaffi_uint16 val) {
    c->cdt_val.uint16_val = val;
}

metaffi_int32 get_cdt_int32_val(struct cdt* c) {
    return c->cdt_val.int32_val;
}

void set_cdt_int32_val(struct cdt* c, metaffi_int32 val) {
    c->cdt_val.int32_val = val;
}

metaffi_uint32 get_cdt_uint32_val(struct cdt* c) {
    return c->cdt_val.uint32_val;
}

void set_cdt_uint32_val(struct cdt* c, metaffi_uint32 val) {
    c->cdt_val.uint32_val = val;
}

metaffi_int64 get_cdt_int64_val(struct cdt* c) {
    return c->cdt_val.int64_val;
}

void set_cdt_int64_val(struct cdt* c, metaffi_int64 val) {
    c->cdt_val.int64_val = val;
}

metaffi_uint64 get_cdt_uint64_val(struct cdt* c) {
    return c->cdt_val.uint64_val;
}

void set_cdt_uint64_val(struct cdt* c, metaffi_uint64 val) {
    c->cdt_val.uint64_val = val;
}

struct cdt* get_cdt_at_index(struct cdts* pcdts, int val) {
	return &pcdts->arr[val];
}

struct cdts* get_cdt_array(struct cdt* c) {
	return c->cdt_val.array_val;
}

void set_cdt_array(struct cdt* c, struct cdts* cdtarray) {
	c->cdt_val.array_val = cdtarray;
}

void set_handle_releaser(struct cdt_metaffi_handle* handle, void* releaser) {
	handle->release = releaser;
}

*/
import "C"
import (
	"unicode/utf16"
	"unicode/utf8"
	"unsafe"
)

type CDTS struct {
	c *C.struct_cdts
}

func NewCDTS(c *C.struct_cdts) *CDTS {
	return &CDTS{c: c}
}

func NewCDTSFromCDTS(c *C.struct_cdt, length uint64, fixedDimensions int64) *CDTS {

	// create new CDTS
	// place "c" as its array

	pcdts := (*C.struct_cdts)(C.xllr_alloc_memory(C.size_t(unsafe.Sizeof(C.struct_cdts{}))))
	pcdts.arr = c
	pcdts.length = C.metaffi_size(length)
	pcdts.fixed_dimensions = C.metaffi_int64(fixedDimensions)
	pcdts.allocated_on_cache = C.metaffi_bool(0)

	return &CDTS{c: (*C.struct_cdts)(pcdts)}
}

func NewCDTSFromSize(arrayLength uint64, fixedDimensions int64) *CDTS {
	return NewCDTSFromCDTS((*C.struct_cdt)(C.xllr_alloc_cdt_array(C.uint64_t(arrayLength))), arrayLength, fixedDimensions)
}

func (cdts *CDTS) GetCDT(index int) *CDT {
	return &CDT{c: C.get_cdt_at_index(cdts.c, C.int(index))}
}

func (cdts *CDTS) GetLength() C.metaffi_size {
	return cdts.c.length
}

func (cdts *CDTS) SetLength(val C.metaffi_size) {
	cdts.c.length = val
}

func (cdts *CDTS) GetFixedDimensions() C.metaffi_int64 {
	return cdts.c.fixed_dimensions
}
func (cdts *CDTS) SetFixedDimensions(val C.metaffi_int64) {
	cdts.c.fixed_dimensions = val
}

//------------------------------------------------------------

type CDT struct {
	c *C.struct_cdt
}

func (cdt *CDT) GetTypeVal() C.metaffi_type {
	return C.get_cdt_type(cdt.c)
}

func (cdt *CDT) SetTypeVal(t C.metaffi_type) {
	C.set_cdt_type(cdt.c, t)
}

func (cdt *CDT) GetFreeRequired() C.metaffi_bool {
	return cdt.c.free_required
}
func (cdt *CDT) SetFreeRequired(val bool) {
	if val {
		cdt.c.free_required = C.metaffi_bool(1)
	} else {
		cdt.c.free_required = C.metaffi_bool(0)
	}
}

func (cdt *CDT) GetFloat32Val() C.metaffi_float32 {
	return C.get_cdt_float32_val(cdt.c)
}

func (cdt *CDT) SetFloat32Val(val C.metaffi_float32) {
	C.set_cdt_float32_val(cdt.c, val)
}

func (cdt *CDT) GetFloat64Val() C.metaffi_float64 {
	return C.get_cdt_float64_val(cdt.c)
}

func (cdt *CDT) SetFloat64Val(val C.metaffi_float64) {
	C.set_cdt_float64_val(cdt.c, val)
}

func (cdt *CDT) GetBoolVal() C.metaffi_bool {
	return C.get_cdt_bool_val(cdt.c)
}

func (cdt *CDT) SetBoolVal(val C.metaffi_bool) {
	C.set_cdt_bool_val(cdt.c, val)
}

func (cdt *CDT) GetInt8Val() C.metaffi_int8 {
	return C.get_cdt_int8_val(cdt.c)
}

func (cdt *CDT) SetInt8Val(val C.metaffi_int8) {
	C.set_cdt_int8_val(cdt.c, val)
}

func (cdt *CDT) GetUInt8Val() C.metaffi_uint8 {
	return C.get_cdt_uint8_val(cdt.c)
}

func (cdt *CDT) SetUInt8Val(val C.metaffi_uint8) {
	C.set_cdt_uint8_val(cdt.c, val)
}

func (cdt *CDT) GetInt16Val() C.metaffi_int16 {
	return C.get_cdt_int16_val(cdt.c)
}

func (cdt *CDT) SetInt16Val(val C.metaffi_int16) {
	C.set_cdt_int16_val(cdt.c, val)
}

func (cdt *CDT) GetUInt16Val() C.metaffi_uint16 {
	return C.get_cdt_uint16_val(cdt.c)
}

func (cdt *CDT) SetUInt16Val(val C.metaffi_uint16) {
	C.set_cdt_uint16_val(cdt.c, val)
}

func (cdt *CDT) GetInt32Val() C.metaffi_int32 {
	return C.get_cdt_int32_val(cdt.c)
}

func (cdt *CDT) SetInt32Val(val C.metaffi_int32) {
	C.set_cdt_int32_val(cdt.c, val)
}

func (cdt *CDT) GetUInt32Val() C.metaffi_uint32 {
	return C.get_cdt_uint32_val(cdt.c)
}

func (cdt *CDT) SetUInt32Val(val C.metaffi_uint32) {
	C.set_cdt_uint32_val(cdt.c, val)
}

func (cdt *CDT) GetInt64Val() C.metaffi_int64 {
	return C.get_cdt_int64_val(cdt.c)
}

func (cdt *CDT) SetInt64Val(val C.metaffi_int64) {
	C.set_cdt_int64_val(cdt.c, val)
}

func (cdt *CDT) GetUInt64Val() C.metaffi_uint64 {
	return C.get_cdt_uint64_val(cdt.c)
}

func (cdt *CDT) SetUInt64Val(val C.metaffi_uint64) {
	C.set_cdt_uint64_val(cdt.c, val)
}

func (cdt *CDT) GetFloat32() float32 {
	return float32(C.get_cdt_float32_val(cdt.c))
}

func (cdt *CDT) SetFloat32(val float32) {
	C.set_cdt_float32_val(cdt.c, C.metaffi_float32(val))
}

func (cdt *CDT) GetFloat64() float64 {
	return float64(C.get_cdt_float64_val(cdt.c))
}

func (cdt *CDT) SetFloat64(val float64) {
	C.set_cdt_float64_val(cdt.c, C.metaffi_float64(val))
}

func (cdt *CDT) GetInt8() int8 {
	return int8(C.get_cdt_int8_val(cdt.c))
}

func (cdt *CDT) SetInt8(val int8) {
	C.set_cdt_int8_val(cdt.c, C.metaffi_int8(val))
}

func (cdt *CDT) GetUInt8() uint8 {
	return uint8(C.get_cdt_uint8_val(cdt.c))
}

func (cdt *CDT) SetUInt8(val uint8) {
	C.set_cdt_uint8_val(cdt.c, C.metaffi_uint8(val))
}

func (cdt *CDT) GetInt16() int16 {
	return int16(C.get_cdt_int16_val(cdt.c))
}

func (cdt *CDT) SetInt16(val int16) {
	C.set_cdt_int16_val(cdt.c, C.metaffi_int16(val))
}

func (cdt *CDT) GetUInt16() uint16 {
	return uint16(C.get_cdt_uint16_val(cdt.c))
}

func (cdt *CDT) SetUInt16(val uint16) {
	C.set_cdt_uint16_val(cdt.c, C.metaffi_uint16(val))
}

func (cdt *CDT) GetInt32() int32 {
	return int32(C.get_cdt_int32_val(cdt.c))
}

func (cdt *CDT) SetInt32(val int32) {
	C.set_cdt_int32_val(cdt.c, C.metaffi_int32(val))
}

func (cdt *CDT) GetUInt32() uint32 {
	return uint32(C.get_cdt_uint32_val(cdt.c))
}

func (cdt *CDT) SetUInt32(val uint32) {
	C.set_cdt_uint32_val(cdt.c, C.metaffi_uint32(val))
}

func (cdt *CDT) GetInt64() int64 {
	return int64(C.get_cdt_int64_val(cdt.c))
}

func (cdt *CDT) SetInt64(val int64) {
	C.set_cdt_int64_val(cdt.c, C.metaffi_int64(val))
}

func (cdt *CDT) GetUInt64() uint64 {
	return uint64(C.get_cdt_uint64_val(cdt.c))
}

func (cdt *CDT) SetUInt64(val uint64) {
	C.set_cdt_uint64_val(cdt.c, C.metaffi_uint64(val))
}

func (cdt *CDT) GetBool() bool {
	return C.get_cdt_bool_val(cdt.c) != 0
}

func (cdt *CDT) SetBool(val bool) {
	var cVal C.metaffi_bool
	if val {
		cVal = 1
	} else {
		cVal = 0
	}
	C.set_cdt_bool_val(cdt.c, cVal)
}

func (cdt *CDT) GetChar8() rune {
	metaffi_char8_0 := C.uint8_t(0)
	metaffi_char8_1 := C.uint8_t(0)
	metaffi_char8_2 := C.uint8_t(0)
	metaffi_char8_3 := C.uint8_t(0)
	C.get_cdt_char8_val(cdt.c, &metaffi_char8_0, &metaffi_char8_1, &metaffi_char8_2, &metaffi_char8_3)

	goSlice := []uint8{
		uint8(metaffi_char8_0),
		uint8(metaffi_char8_1),
		uint8(metaffi_char8_2),
		uint8(metaffi_char8_3),
	}

	decoded := string(goSlice)
	firstRune, _ := utf8.DecodeRuneInString(decoded)

	return firstRune
}

func (cdt *CDT) SetChar8(r rune) {
	utf8Bytes := make([]byte, 4)
	utf8.EncodeRune(utf8Bytes, r)

	C.set_cdt_char8_val(cdt.c, C.uint8_t(utf8Bytes[0]), C.uint8_t(utf8Bytes[1]), C.uint8_t(utf8Bytes[2]), C.uint8_t(utf8Bytes[3]))
}

func (cdt *CDT) GetChar16() rune {
	val := C.get_cdt_char16_val(cdt.c)

	goSlice := []uint16{
		uint16(val),
	}
	decoded := utf16.Decode(goSlice)

	return decoded[0]
}

func (cdt *CDT) SetChar16(r rune) {
	// Convert the rune to a UTF-16 byte array
	utf16Bytes := utf16.Encode([]rune{r})

	C.set_cdt_char16_val(cdt.c, C.uint16_t(utf16Bytes[0]))
}

func (cdt *CDT) GetChar32() rune {
	goRune := rune(C.get_cdt_char32_val(cdt.c))
	return goRune
}

func (cdt *CDT) SetChar32(r rune) {
	// Return a new C.struct_metaffi_char32 with the converted rune
	C.set_cdt_char32_val(cdt.c, C.char32_t(r))
}

func (cdt *CDT) GetString8() string {
	return C.GoString(C.get_cdt_string8_val(cdt.c))
}

func (cdt *CDT) SetString8(val string) {
	cVal := C.CString(val)
	pval := C.xllr_alloc_string(cVal, C.uint64_t(len(val)))
	defer C.free(unsafe.Pointer(cVal))
	C.set_cdt_string8_val(cdt.c, pval)
}

func (cdt *CDT) GetString16() string {

	var strlen16 C.int32_t
	cdtstr16 := C.get_cdt_string16_val(cdt.c, &strlen16)

	// Convert C array to Go slice
	cSlice := (*[1 << 30]C.char16_t)(unsafe.Pointer(cdtstr16))[:strlen16:strlen16]

	// Convert C array to Go slice
	goSlice := make([]uint16, int(strlen16))
	for i, c := range cSlice {
		goSlice[i] = uint16(c)
	}

	// Decode UTF-16 to Go string
	goString := string(utf16.Decode(goSlice))

	return goString
}

func (cdt *CDT) SetString16(val string) {
	// convert val to UTF-16 bytes
	utf16Bytes := utf16.Encode([]rune(val))

	// convert UTF-16 bytes to C array
	// use malloc to allocate memory for C array as it goes out of Go's scope
	// add null terminator
	cVal := C.xllr_alloc_memory(C.size_t(len(utf16Bytes)+1) * C.size_t(2))
	cdtstr16 := (*[1 << 30]C.char16_t)(cVal)
	for i, c := range utf16Bytes {
		cdtstr16[i] = C.char16_t(c)
	}
	cdtstr16[len(utf16Bytes)] = 0

	C.set_cdt_string16_val(cdt.c, (*C.char16_t)(cVal))
}

func (cdt *CDT) GetString32() string {

	var strlen32 C.int32_t
	cdtstr32 := C.get_cdt_string32_val(cdt.c, &strlen32)

	cSlice := (*[1 << 30]C.char32_t)(unsafe.Pointer(cdtstr32))[:strlen32:strlen32]

	// Convert C array to Go slice
	goSlice := make([]rune, strlen32)
	for i, c := range cSlice {
		goSlice[i] = rune(c)
	}

	// Convert rune slice to string
	goString := string(goSlice)

	return goString
}

func (cdt *CDT) SetString32(val string) {
	// convert val to UTF-32 bytes
	utf32Bytes := []rune(val)

	// convert UTF-32 bytes to C array
	// use malloc to allocate memory for C array as it goes out of Go's scope
	// add null terminator
	cVal := C.xllr_alloc_memory(C.size_t(len(utf32Bytes)+1) * C.size_t(4))
	cdtstr32 := (*[1 << 30]C.char32_t)(cVal)
	for i, c := range utf32Bytes {
		cdtstr32[i] = C.char32_t(c)
	}
	cdtstr32[len(utf32Bytes)] = 0

	C.set_cdt_string32_val(cdt.c, (*C.char32_t)(cVal))
}

func (cdt *CDT) GetHandleStruct() *C.struct_cdt_metaffi_handle {
	return C.get_cdt_handle_val(cdt.c)
}

func (cdt *CDT) GetHandleRuntime() uint64 {
	return uint64(C.get_cdt_handle_val(cdt.c).runtime_id)
}

func (cdt *CDT) GetHandleVal() uintptr {
	return uintptr(C.get_cdt_handle_val(cdt.c).handle)
}

func (cdt *CDT) SetHandleStruct(val *CDTMetaFFIHandle) {
	if val == nil {
		C.set_cdt_handle_val(cdt.c, nil)
	} else {
		C.set_cdt_handle_val(cdt.c, val.Val)
	}
}

func (cdt *CDT) GetCallableVal() *MetaFFICallable {
	val := C.get_cdt_callable_val(cdt.c)
	return &MetaFFICallable{Val: val}
}

func (cdt *CDT) SetCallableVal(val *MetaFFICallable) {
	C.set_cdt_callable_val(cdt.c, val.Val)
}

func (cdt *CDT) GetArray() *CDTS {

	parr := C.get_cdt_array(cdt.c)
	if unsafe.Pointer(parr) == nil {
		return nil
	}

	return &CDTS{c: parr}
}

func (cdt *CDT) SetArray(val *CDTS) {
	C.set_cdt_array(cdt.c, val.c)
}

//------------------------------------------------------------

type MetaFFIChar8 struct {
	Val *C.struct_metaffi_char8
}

//------------------------------------------------------------

type MetaFFIChar16 struct {
	Val *C.struct_metaffi_char16
}

//------------------------------------------------------------

type MetaFFIChar32 struct {
	Val *C.struct_metaffi_char32
}

//------------------------------------------------------------

type CDTMetaFFIHandle struct {
	Val *C.struct_cdt_metaffi_handle
}

func NewCDTMetaFFIHandle(handle Handle, runtimeID uint64, releaserFunc unsafe.Pointer) *CDTMetaFFIHandle {
	cstruct := (*C.struct_cdt_metaffi_handle)(C.xllr_alloc_memory(C.size_t(unsafe.Sizeof(C.struct_cdt_metaffi_handle{}))))
	cstruct.handle = C.metaffi_handle(handle)
	cstruct.runtime_id = C.metaffi_uint64(runtimeID)
	C.set_handle_releaser(cstruct, releaserFunc)
	return &CDTMetaFFIHandle{Val: cstruct}
}

func (handle *CDTMetaFFIHandle) GetHandle() C.metaffi_handle {
	if handle.Val == nil {
		return nil
	}

	return handle.Val.handle
}

//------------------------------------------------------------

type MetaFFICallable struct {
	Val *C.struct_cdt_metaffi_callable
}

//--------------------------------------------------------------------
