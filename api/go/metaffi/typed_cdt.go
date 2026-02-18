package metaffi

/*
#cgo !windows LDFLAGS: -L. -ldl
#cgo LDFLAGS: -Wl,--allow-multiple-definition

#include <include/cdt.h>
#include <include/metaffi_primitives.h>
#include <include/xllr_capi_loader.h>
#include <string.h>
#include <stdlib.h>

// td_ prefix = "typed direct" - all static to avoid symbol conflicts with CDT.go

static struct cdt* td_cdt_at(struct cdt* arr, int index) {
	return &arr[index];
}

// --- Scalar getters ---
static metaffi_int8    td_get_int8(struct cdt* c)    { return c->cdt_val.int8_val; }
static metaffi_int16   td_get_int16(struct cdt* c)   { return c->cdt_val.int16_val; }
static metaffi_int32   td_get_int32(struct cdt* c)   { return c->cdt_val.int32_val; }
static metaffi_int64   td_get_int64(struct cdt* c)   { return c->cdt_val.int64_val; }
static metaffi_uint8   td_get_uint8(struct cdt* c)   { return c->cdt_val.uint8_val; }
static metaffi_uint16  td_get_uint16(struct cdt* c)  { return c->cdt_val.uint16_val; }
static metaffi_uint32  td_get_uint32(struct cdt* c)  { return c->cdt_val.uint32_val; }
static metaffi_uint64  td_get_uint64(struct cdt* c)  { return c->cdt_val.uint64_val; }
static metaffi_float32 td_get_float32(struct cdt* c) { return c->cdt_val.float32_val; }
static metaffi_float64 td_get_float64(struct cdt* c) { return c->cdt_val.float64_val; }
static metaffi_bool    td_get_bool(struct cdt* c)    { return c->cdt_val.bool_val; }
static char*           td_get_string8(struct cdt* c) { return (char*)c->cdt_val.string8_val; }

// --- Scalar setters (set value, type tag, and free_required) ---
static void td_set_int8(struct cdt* c, metaffi_int8 v)       { c->type = metaffi_int8_type;    c->cdt_val.int8_val = v;    c->free_required = 0; }
static void td_set_int16(struct cdt* c, metaffi_int16 v)     { c->type = metaffi_int16_type;   c->cdt_val.int16_val = v;   c->free_required = 0; }
static void td_set_int32(struct cdt* c, metaffi_int32 v)     { c->type = metaffi_int32_type;   c->cdt_val.int32_val = v;   c->free_required = 0; }
static void td_set_int64(struct cdt* c, metaffi_int64 v)     { c->type = metaffi_int64_type;   c->cdt_val.int64_val = v;   c->free_required = 0; }
static void td_set_uint8(struct cdt* c, metaffi_uint8 v)     { c->type = metaffi_uint8_type;   c->cdt_val.uint8_val = v;   c->free_required = 0; }
static void td_set_uint16(struct cdt* c, metaffi_uint16 v)   { c->type = metaffi_uint16_type;  c->cdt_val.uint16_val = v;  c->free_required = 0; }
static void td_set_uint32(struct cdt* c, metaffi_uint32 v)   { c->type = metaffi_uint32_type;  c->cdt_val.uint32_val = v;  c->free_required = 0; }
static void td_set_uint64(struct cdt* c, metaffi_uint64 v)   { c->type = metaffi_uint64_type;  c->cdt_val.uint64_val = v;  c->free_required = 0; }
static void td_set_float32(struct cdt* c, metaffi_float32 v) { c->type = metaffi_float32_type; c->cdt_val.float32_val = v; c->free_required = 0; }
static void td_set_float64(struct cdt* c, metaffi_float64 v) { c->type = metaffi_float64_type; c->cdt_val.float64_val = v; c->free_required = 0; }
static void td_set_bool(struct cdt* c, metaffi_bool v)       { c->type = metaffi_bool_type;    c->cdt_val.bool_val = v;    c->free_required = 0; }

static void td_set_string8_val(struct cdt* c, char* v) {
	c->type = metaffi_string8_type;
	c->cdt_val.string8_val = (metaffi_string8)v;
	c->free_required = 1;
}

// --- Packed array helpers ---
static struct cdt_packed_array* td_get_packed(struct cdt* c) {
	return c->cdt_val.packed_array_val;
}

static void* td_packed_data(struct cdt_packed_array* p) {
	return p ? p->data : NULL;
}

static metaffi_size td_packed_len(struct cdt_packed_array* p) {
	return p ? p->length : 0;
}

static void td_set_packed(struct cdt* c, void* data, metaffi_size length, metaffi_type packed_type) {
	struct cdt_packed_array* p = (struct cdt_packed_array*)xllr_alloc_memory(sizeof(struct cdt_packed_array));
	p->data = data;
	p->length = length;
	c->type = packed_type;
	c->free_required = 1;
	c->cdt_val.packed_array_val = p;
}

*/
import "C"

import "unsafe"

// ============================================================
// Scalar Get functions
// ============================================================

func DirectGetCDTInt8(cdtsArr unsafe.Pointer, index int) int8 {
	return int8(C.td_get_int8(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}
func DirectGetCDTInt16(cdtsArr unsafe.Pointer, index int) int16 {
	return int16(C.td_get_int16(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}
func DirectGetCDTInt32(cdtsArr unsafe.Pointer, index int) int32 {
	return int32(C.td_get_int32(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}
func DirectGetCDTInt64(cdtsArr unsafe.Pointer, index int) int64 {
	return int64(C.td_get_int64(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}
func DirectGetCDTUint8(cdtsArr unsafe.Pointer, index int) uint8 {
	return uint8(C.td_get_uint8(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}
func DirectGetCDTUint16(cdtsArr unsafe.Pointer, index int) uint16 {
	return uint16(C.td_get_uint16(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}
func DirectGetCDTUint32(cdtsArr unsafe.Pointer, index int) uint32 {
	return uint32(C.td_get_uint32(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}
func DirectGetCDTUint64(cdtsArr unsafe.Pointer, index int) uint64 {
	return uint64(C.td_get_uint64(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}
func DirectGetCDTFloat32(cdtsArr unsafe.Pointer, index int) float32 {
	return float32(C.td_get_float32(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}
func DirectGetCDTFloat64(cdtsArr unsafe.Pointer, index int) float64 {
	return float64(C.td_get_float64(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}
func DirectGetCDTBool(cdtsArr unsafe.Pointer, index int) bool {
	return C.td_get_bool(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))) != 0
}
func DirectGetCDTString8(cdtsArr unsafe.Pointer, index int) string {
	return C.GoString(C.td_get_string8(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))))
}

// ============================================================
// Scalar Set functions
// ============================================================

func DirectSetCDTInt8(cdtsArr unsafe.Pointer, index int, val int8) {
	C.td_set_int8(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), C.metaffi_int8(val))
}
func DirectSetCDTInt16(cdtsArr unsafe.Pointer, index int, val int16) {
	C.td_set_int16(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), C.metaffi_int16(val))
}
func DirectSetCDTInt32(cdtsArr unsafe.Pointer, index int, val int32) {
	C.td_set_int32(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), C.metaffi_int32(val))
}
func DirectSetCDTInt64(cdtsArr unsafe.Pointer, index int, val int64) {
	C.td_set_int64(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), C.metaffi_int64(val))
}
func DirectSetCDTUint8(cdtsArr unsafe.Pointer, index int, val uint8) {
	C.td_set_uint8(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), C.metaffi_uint8(val))
}
func DirectSetCDTUint16(cdtsArr unsafe.Pointer, index int, val uint16) {
	C.td_set_uint16(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), C.metaffi_uint16(val))
}
func DirectSetCDTUint32(cdtsArr unsafe.Pointer, index int, val uint32) {
	C.td_set_uint32(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), C.metaffi_uint32(val))
}
func DirectSetCDTUint64(cdtsArr unsafe.Pointer, index int, val uint64) {
	C.td_set_uint64(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), C.metaffi_uint64(val))
}
func DirectSetCDTFloat32(cdtsArr unsafe.Pointer, index int, val float32) {
	C.td_set_float32(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), C.metaffi_float32(val))
}
func DirectSetCDTFloat64(cdtsArr unsafe.Pointer, index int, val float64) {
	C.td_set_float64(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), C.metaffi_float64(val))
}
func DirectSetCDTBool(cdtsArr unsafe.Pointer, index int, val bool) {
	var cVal C.metaffi_bool
	if val {
		cVal = 1
	}
	C.td_set_bool(C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index)), cVal)
}
func DirectSetCDTString8(cdtsArr unsafe.Pointer, index int, val string) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	cVal := C.CString(val)
	pval := C.xllr_alloc_string(cVal, C.uint64_t(len(val)))
	C.free(unsafe.Pointer(cVal))
	C.td_set_string8_val(cdt, (*C.char)(pval))
}

// ============================================================
// Packed array Get functions (numeric types use memcpy)
// ============================================================

func DirectGetCDTInt8PackedSlice(cdtsArr unsafe.Pointer, index int) []int8 {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	result := make([]int8, length)
	C.memcpy(unsafe.Pointer(&result[0]), C.td_packed_data(packed), C.size_t(length)*C.size_t(unsafe.Sizeof(C.metaffi_int8(0))))
	return result
}

func DirectGetCDTInt16PackedSlice(cdtsArr unsafe.Pointer, index int) []int16 {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	result := make([]int16, length)
	C.memcpy(unsafe.Pointer(&result[0]), C.td_packed_data(packed), C.size_t(length)*C.size_t(unsafe.Sizeof(C.metaffi_int16(0))))
	return result
}

func DirectGetCDTInt32PackedSlice(cdtsArr unsafe.Pointer, index int) []int32 {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	result := make([]int32, length)
	C.memcpy(unsafe.Pointer(&result[0]), C.td_packed_data(packed), C.size_t(length)*C.size_t(unsafe.Sizeof(C.metaffi_int32(0))))
	return result
}

func DirectGetCDTInt64PackedSlice(cdtsArr unsafe.Pointer, index int) []int64 {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	result := make([]int64, length)
	C.memcpy(unsafe.Pointer(&result[0]), C.td_packed_data(packed), C.size_t(length)*C.size_t(unsafe.Sizeof(C.metaffi_int64(0))))
	return result
}

func DirectGetCDTUint8PackedSlice(cdtsArr unsafe.Pointer, index int) []uint8 {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	result := make([]uint8, length)
	C.memcpy(unsafe.Pointer(&result[0]), C.td_packed_data(packed), C.size_t(length))
	return result
}

func DirectGetCDTUint16PackedSlice(cdtsArr unsafe.Pointer, index int) []uint16 {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	result := make([]uint16, length)
	C.memcpy(unsafe.Pointer(&result[0]), C.td_packed_data(packed), C.size_t(length)*C.size_t(unsafe.Sizeof(C.metaffi_uint16(0))))
	return result
}

func DirectGetCDTUint32PackedSlice(cdtsArr unsafe.Pointer, index int) []uint32 {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	result := make([]uint32, length)
	C.memcpy(unsafe.Pointer(&result[0]), C.td_packed_data(packed), C.size_t(length)*C.size_t(unsafe.Sizeof(C.metaffi_uint32(0))))
	return result
}

func DirectGetCDTUint64PackedSlice(cdtsArr unsafe.Pointer, index int) []uint64 {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	result := make([]uint64, length)
	C.memcpy(unsafe.Pointer(&result[0]), C.td_packed_data(packed), C.size_t(length)*C.size_t(unsafe.Sizeof(C.metaffi_uint64(0))))
	return result
}

func DirectGetCDTFloat32PackedSlice(cdtsArr unsafe.Pointer, index int) []float32 {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	result := make([]float32, length)
	C.memcpy(unsafe.Pointer(&result[0]), C.td_packed_data(packed), C.size_t(length)*C.size_t(unsafe.Sizeof(C.metaffi_float32(0))))
	return result
}

func DirectGetCDTFloat64PackedSlice(cdtsArr unsafe.Pointer, index int) []float64 {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	result := make([]float64, length)
	C.memcpy(unsafe.Pointer(&result[0]), C.td_packed_data(packed), C.size_t(length)*C.size_t(unsafe.Sizeof(C.metaffi_float64(0))))
	return result
}

func DirectGetCDTBoolPackedSlice(cdtsArr unsafe.Pointer, index int) []bool {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	data := (*[1 << 30]C.metaffi_bool)(C.td_packed_data(packed))[:length:length]
	result := make([]bool, length)
	for i := 0; i < length; i++ {
		result[i] = data[i] != 0
	}
	return result
}

func DirectGetCDTString8PackedSlice(cdtsArr unsafe.Pointer, index int) []string {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	packed := C.td_get_packed(cdt)
	length := int(C.td_packed_len(packed))
	if length == 0 {
		return nil
	}
	data := (*[1 << 30]*C.char)(C.td_packed_data(packed))[:length:length]
	result := make([]string, length)
	for i := 0; i < length; i++ {
		result[i] = C.GoString(data[i])
	}
	return result
}

// ============================================================
// Packed array Set functions (numeric types use memcpy)
// ============================================================

func DirectSetCDTInt8PackedSlice(cdtsArr unsafe.Pointer, index int, data []int8) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_int8_packed_array_type)
		return
	}
	elemSize := C.size_t(unsafe.Sizeof(C.metaffi_int8(0)))
	buf := C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * elemSize))
	C.memcpy(buf, unsafe.Pointer(&data[0]), C.size_t(length)*elemSize)
	C.td_set_packed(cdt, buf, C.metaffi_size(length), C.metaffi_int8_packed_array_type)
}

func DirectSetCDTInt16PackedSlice(cdtsArr unsafe.Pointer, index int, data []int16) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_int16_packed_array_type)
		return
	}
	elemSize := C.size_t(unsafe.Sizeof(C.metaffi_int16(0)))
	buf := C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * elemSize))
	C.memcpy(buf, unsafe.Pointer(&data[0]), C.size_t(length)*elemSize)
	C.td_set_packed(cdt, buf, C.metaffi_size(length), C.metaffi_int16_packed_array_type)
}

func DirectSetCDTInt32PackedSlice(cdtsArr unsafe.Pointer, index int, data []int32) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_int32_packed_array_type)
		return
	}
	elemSize := C.size_t(unsafe.Sizeof(C.metaffi_int32(0)))
	buf := C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * elemSize))
	C.memcpy(buf, unsafe.Pointer(&data[0]), C.size_t(length)*elemSize)
	C.td_set_packed(cdt, buf, C.metaffi_size(length), C.metaffi_int32_packed_array_type)
}

func DirectSetCDTInt64PackedSlice(cdtsArr unsafe.Pointer, index int, data []int64) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_int64_packed_array_type)
		return
	}
	elemSize := C.size_t(unsafe.Sizeof(C.metaffi_int64(0)))
	buf := C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * elemSize))
	C.memcpy(buf, unsafe.Pointer(&data[0]), C.size_t(length)*elemSize)
	C.td_set_packed(cdt, buf, C.metaffi_size(length), C.metaffi_int64_packed_array_type)
}

func DirectSetCDTUint8PackedSlice(cdtsArr unsafe.Pointer, index int, data []uint8) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_uint8_packed_array_type)
		return
	}
	buf := C.xllr_alloc_memory(C.uint64_t(length))
	C.memcpy(buf, unsafe.Pointer(&data[0]), C.size_t(length))
	C.td_set_packed(cdt, buf, C.metaffi_size(length), C.metaffi_uint8_packed_array_type)
}

func DirectSetCDTUint16PackedSlice(cdtsArr unsafe.Pointer, index int, data []uint16) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_uint16_packed_array_type)
		return
	}
	elemSize := C.size_t(unsafe.Sizeof(C.metaffi_uint16(0)))
	buf := C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * elemSize))
	C.memcpy(buf, unsafe.Pointer(&data[0]), C.size_t(length)*elemSize)
	C.td_set_packed(cdt, buf, C.metaffi_size(length), C.metaffi_uint16_packed_array_type)
}

func DirectSetCDTUint32PackedSlice(cdtsArr unsafe.Pointer, index int, data []uint32) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_uint32_packed_array_type)
		return
	}
	elemSize := C.size_t(unsafe.Sizeof(C.metaffi_uint32(0)))
	buf := C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * elemSize))
	C.memcpy(buf, unsafe.Pointer(&data[0]), C.size_t(length)*elemSize)
	C.td_set_packed(cdt, buf, C.metaffi_size(length), C.metaffi_uint32_packed_array_type)
}

func DirectSetCDTUint64PackedSlice(cdtsArr unsafe.Pointer, index int, data []uint64) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_uint64_packed_array_type)
		return
	}
	elemSize := C.size_t(unsafe.Sizeof(C.metaffi_uint64(0)))
	buf := C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * elemSize))
	C.memcpy(buf, unsafe.Pointer(&data[0]), C.size_t(length)*elemSize)
	C.td_set_packed(cdt, buf, C.metaffi_size(length), C.metaffi_uint64_packed_array_type)
}

func DirectSetCDTFloat32PackedSlice(cdtsArr unsafe.Pointer, index int, data []float32) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_float32_packed_array_type)
		return
	}
	elemSize := C.size_t(unsafe.Sizeof(C.metaffi_float32(0)))
	buf := C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * elemSize))
	C.memcpy(buf, unsafe.Pointer(&data[0]), C.size_t(length)*elemSize)
	C.td_set_packed(cdt, buf, C.metaffi_size(length), C.metaffi_float32_packed_array_type)
}

func DirectSetCDTFloat64PackedSlice(cdtsArr unsafe.Pointer, index int, data []float64) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_float64_packed_array_type)
		return
	}
	elemSize := C.size_t(unsafe.Sizeof(C.metaffi_float64(0)))
	buf := C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * elemSize))
	C.memcpy(buf, unsafe.Pointer(&data[0]), C.size_t(length)*elemSize)
	C.td_set_packed(cdt, buf, C.metaffi_size(length), C.metaffi_float64_packed_array_type)
}

func DirectSetCDTBoolPackedSlice(cdtsArr unsafe.Pointer, index int, data []bool) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_bool_packed_array_type)
		return
	}
	elemSize := C.size_t(unsafe.Sizeof(C.metaffi_bool(0)))
	buf := (*[1 << 30]C.metaffi_bool)(C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * elemSize)))[:length:length]
	for i, v := range data {
		if v {
			buf[i] = 1
		} else {
			buf[i] = 0
		}
	}
	C.td_set_packed(cdt, unsafe.Pointer(&buf[0]), C.metaffi_size(length), C.metaffi_bool_packed_array_type)
}

func DirectSetCDTString8PackedSlice(cdtsArr unsafe.Pointer, index int, data []string) {
	cdt := C.td_cdt_at((*C.struct_cdt)(cdtsArr), C.int(index))
	length := len(data)
	if length == 0 {
		C.td_set_packed(cdt, nil, 0, C.metaffi_string8_packed_array_type)
		return
	}
	ptrSize := C.size_t(unsafe.Sizeof((*C.char)(nil)))
	ptrs := (*[1 << 30]*C.char)(C.xllr_alloc_memory(C.uint64_t(C.size_t(length) * ptrSize)))[:length:length]
	for i, v := range data {
		cStr := C.CString(v)
		allocated := C.xllr_alloc_string(cStr, C.uint64_t(len(v)))
		C.free(unsafe.Pointer(cStr))
		ptrs[i] = (*C.char)(allocated)
	}
	C.td_set_packed(cdt, unsafe.Pointer(&ptrs[0]), C.metaffi_size(length), C.metaffi_string8_packed_array_type)
}
