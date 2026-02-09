package cdts_go_serializer

/*
#cgo CFLAGS: -I"${SRCDIR}/../../runtime"

#include <cdt.h>
#include <metaffi_primitives.h>
#include <xllr_capi_loader.h>
#include <stdlib.h>
#include <stdio.h>

// Initialize XLLR - call this before using any XLLR functions
static int xllr_initialized = 0;
static void ensure_xllr_loaded() {
    if (!xllr_initialized) {
        const char* err = load_xllr();
        if (err) {
            fprintf(stderr, "FATAL ERROR! Failed to load XLLR C-API. Error: %s\n", err);
            // In Go, we can't exit from C code easily, so we'll check in Go code
        } else {
            xllr_initialized = 1;
        }
    }
}

// C helper functions to access C struct fields
metaffi_type get_cdt_type(struct cdt* c) {
    return c->type;
}

void set_cdt_type(struct cdt* c, metaffi_type t) {
    c->type = t;
}

metaffi_bool get_cdt_free_required(struct cdt* c) {
    return c->free_required;
}

void set_cdt_free_required(struct cdt* c, metaffi_bool val) {
    c->free_required = val;
}

// Integer getters/setters
metaffi_int8 get_cdt_int8_val(struct cdt* c) {
    return c->cdt_val.int8_val;
}

void set_cdt_int8_val(struct cdt* c, metaffi_int8 val) {
    c->cdt_val.int8_val = val;
}

metaffi_int16 get_cdt_int16_val(struct cdt* c) {
    return c->cdt_val.int16_val;
}

void set_cdt_int16_val(struct cdt* c, metaffi_int16 val) {
    c->cdt_val.int16_val = val;
}

metaffi_int32 get_cdt_int32_val(struct cdt* c) {
    return c->cdt_val.int32_val;
}

void set_cdt_int32_val(struct cdt* c, metaffi_int32 val) {
    c->cdt_val.int32_val = val;
}

metaffi_int64 get_cdt_int64_val(struct cdt* c) {
    return c->cdt_val.int64_val;
}

void set_cdt_int64_val(struct cdt* c, metaffi_int64 val) {
    c->cdt_val.int64_val = val;
}

metaffi_uint8 get_cdt_uint8_val(struct cdt* c) {
    return c->cdt_val.uint8_val;
}

void set_cdt_uint8_val(struct cdt* c, metaffi_uint8 val) {
    c->cdt_val.uint8_val = val;
}

metaffi_uint16 get_cdt_uint16_val(struct cdt* c) {
    return c->cdt_val.uint16_val;
}

void set_cdt_uint16_val(struct cdt* c, metaffi_uint16 val) {
    c->cdt_val.uint16_val = val;
}

metaffi_uint32 get_cdt_uint32_val(struct cdt* c) {
    return c->cdt_val.uint32_val;
}

void set_cdt_uint32_val(struct cdt* c, metaffi_uint32 val) {
    c->cdt_val.uint32_val = val;
}

metaffi_uint64 get_cdt_uint64_val(struct cdt* c) {
    return c->cdt_val.uint64_val;
}

void set_cdt_uint64_val(struct cdt* c, metaffi_uint64 val) {
    c->cdt_val.uint64_val = val;
}

// Float getters/setters
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

// Bool getter/setter
metaffi_bool get_cdt_bool_val(struct cdt* c) {
    return c->cdt_val.bool_val;
}

void set_cdt_bool_val(struct cdt* c, metaffi_bool val) {
    c->cdt_val.bool_val = val;
}

// Char getters/setters
void get_cdt_char8_val(struct cdt* c, uint8_t* out0, uint8_t* out1, uint8_t* out2, uint8_t* out3) {
    out0[0] = c->cdt_val.char8_val.c[0];
    out1[0] = c->cdt_val.char8_val.c[1];
    out2[0] = c->cdt_val.char8_val.c[2];
    out3[0] = c->cdt_val.char8_val.c[3];
}

void set_cdt_char8_val(struct cdt* c, uint8_t val0, uint8_t val1, uint8_t val2, uint8_t val3) {
    c->cdt_val.char8_val.c[0] = val0;
    c->cdt_val.char8_val.c[1] = val1;
    c->cdt_val.char8_val.c[2] = val2;
    c->cdt_val.char8_val.c[3] = val3;
}

uint16_t get_cdt_char16_val(struct cdt* c) {
    return c->cdt_val.char16_val.c[0];
}

void set_cdt_char16_val(struct cdt* c, uint16_t val) {
    c->cdt_val.char16_val.c[0] = val;
    c->cdt_val.char16_val.c[1] = 0;
}

char32_t get_cdt_char32_val(struct cdt* c) {
    return c->cdt_val.char32_val.c;
}

void set_cdt_char32_val(struct cdt* c, char32_t val) {
    c->cdt_val.char32_val.c = val;
}

// String getters/setters
char* get_cdt_string8_val(struct cdt* c) {
    return (char*)c->cdt_val.string8_val;
}

void set_cdt_string8_val(struct cdt* c, char8_t* val) {
    c->cdt_val.string8_val = val;
}

char16_t* get_cdt_string16_val(struct cdt* c) {
    return c->cdt_val.string16_val;
}

void set_cdt_string16_val(struct cdt* c, char16_t* val) {
    c->cdt_val.string16_val = val;
}

char32_t* get_cdt_string32_val(struct cdt* c) {
    return c->cdt_val.string32_val;
}

void set_cdt_string32_val(struct cdt* c, char32_t* val) {
    c->cdt_val.string32_val = val;
}

// Array getter/setter
struct cdts* get_cdt_array(struct cdt* c) {
    return c->cdt_val.array_val;
}

void set_cdt_array(struct cdt* c, struct cdts* arr) {
    c->cdt_val.array_val = arr;
}

// Handle getter/setter
struct cdt_metaffi_handle* get_cdt_handle_val(struct cdt* c) {
    return c->cdt_val.handle_val;
}

void set_cdt_handle_val(struct cdt* c, struct cdt_metaffi_handle* val) {
    c->cdt_val.handle_val = val;
}

// Callable getter/setter
struct cdt_metaffi_callable* get_cdt_callable_val(struct cdt* c) {
    return c->cdt_val.callable_val;
}

void set_cdt_callable_val(struct cdt* c, struct cdt_metaffi_callable* val) {
    c->cdt_val.callable_val = val;
}

// CDTS accessors
struct cdt* get_cdt_at_index(struct cdts* pcdts, int index) {
    return &pcdts->arr[index];
}

metaffi_size get_cdts_length(struct cdts* pcdts) {
    return pcdts->length;
}

void set_cdts_length(struct cdts* pcdts, metaffi_size length) {
    pcdts->length = length;
}

metaffi_int64 get_cdts_fixed_dimensions(struct cdts* pcdts) {
    return pcdts->fixed_dimensions;
}

void set_cdts_fixed_dimensions(struct cdts* pcdts, metaffi_int64 dims) {
    pcdts->fixed_dimensions = dims;
}

metaffi_bool get_cdts_allocated_on_cache(struct cdts* pcdts) {
    return pcdts->allocated_on_cache;
}

void set_cdts_allocated_on_cache(struct cdts* pcdts, metaffi_bool val) {
    pcdts->allocated_on_cache = val;
}

struct cdt* get_cdts_arr(struct cdts* pcdts) {
    return pcdts->arr;
}

void set_cdts_arr(struct cdts* pcdts, struct cdt* arr) {
    pcdts->arr = arr;
}
*/
import "C"
import (
	"errors"
	"fmt"
	"unicode/utf16"
	"unicode/utf8"
	"unsafe"
)

// CDTSGoSerializer wraps a C cdts struct and provides Go-friendly serialization API
type CDTSGoSerializer struct {
	cdts         *C.struct_cdts // Pointer to C cdts struct
	currentIndex C.metaffi_size // Current position in CDTS
}

// NewCDTSGoSerializer creates a new serializer from an existing C cdts pointer
// The cdts must be allocated externally (e.g., from MetaFFI call)
// Returns error if cdts is nil
func NewCDTSGoSerializer(cdts *C.struct_cdts) (*CDTSGoSerializer, error) {
	if cdts == nil {
		return nil, errors.New("cdts pointer cannot be nil")
	}
	return &CDTSGoSerializer{
		cdts:         cdts,
		currentIndex: 0,
	}, nil
}

// initXLLR ensures XLLR is loaded (called automatically)
// Returns error if XLLR failed to load
func initXLLR() error {
	C.ensure_xllr_loaded()
	// The C code will print errors to stderr, but we can't easily check
	// if it succeeded from Go. The first XLLR function call will trigger
	// auto-loading if needed, so we rely on that mechanism.
	// If load_xllr() fails, subsequent XLLR calls will fail with clear errors.
	return nil
}

// NewCDTSGoSerializerFromSize creates a new CDTS with specified size
// Returns error if memory allocation fails
func NewCDTSGoSerializerFromSize(length uint64) (*CDTSGoSerializer, error) {
	// Ensure XLLR is loaded before using it
	if err := initXLLR(); err != nil {
		return nil, fmt.Errorf("failed to initialize XLLR: %w", err)
	}

	arr := C.xllr_alloc_cdt_array(C.uint64_t(length))
	if arr == nil {
		return nil, errors.New("xllr_alloc_cdt_array failed: memory allocation error")
	}

	cdts := (*C.struct_cdts)(C.xllr_alloc_memory(C.size_t(unsafe.Sizeof(C.struct_cdts{}))))
	if cdts == nil {
		return nil, errors.New("xllr_alloc_memory failed: memory allocation error")
	}

	C.set_cdts_arr(cdts, arr)
	C.set_cdts_length(cdts, C.metaffi_size(length))
	C.set_cdts_fixed_dimensions(cdts, 1)
	C.set_cdts_allocated_on_cache(cdts, 0)

	return &CDTSGoSerializer{
		cdts:         cdts,
		currentIndex: 0,
	}, nil
}

// Helper method for bounds checking (used internally)
func (s *CDTSGoSerializer) checkBounds() error {
	if s.currentIndex >= C.metaffi_size(C.get_cdts_length(s.cdts)) {
		return fmt.Errorf("index %d out of bounds (size: %d)", s.currentIndex, C.get_cdts_length(s.cdts))
	}
	return nil
}

// Helper method to get CDT at current index (with validation)
func (s *CDTSGoSerializer) getCurrentCDT() (*C.struct_cdt, error) {
	if err := s.checkBounds(); err != nil {
		return nil, err
	}
	cdt := C.get_cdt_at_index(s.cdts, C.int(s.currentIndex))
	if cdt == nil {
		return nil, fmt.Errorf("failed to get CDT at index %d", s.currentIndex)
	}
	return cdt, nil
}

// ===== Serialization Methods (Go → CDTS) =====

// AddInt8 adds an int8 value to the serializer
func (s *CDTSGoSerializer) AddInt8(val int8) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_int8_type)
	C.set_cdt_int8_val(cdt, C.metaffi_int8(val))
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddInt16 adds an int16 value to the serializer
func (s *CDTSGoSerializer) AddInt16(val int16) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_int16_type)
	C.set_cdt_int16_val(cdt, C.metaffi_int16(val))
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddInt32 adds an int32 value to the serializer
func (s *CDTSGoSerializer) AddInt32(val int32) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_int32_type)
	C.set_cdt_int32_val(cdt, C.metaffi_int32(val))
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddInt64 adds an int64 value to the serializer
func (s *CDTSGoSerializer) AddInt64(val int64) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_int64_type)
	C.set_cdt_int64_val(cdt, C.metaffi_int64(val))
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddUint8 adds a uint8 value to the serializer
func (s *CDTSGoSerializer) AddUint8(val uint8) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_uint8_type)
	C.set_cdt_uint8_val(cdt, C.metaffi_uint8(val))
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddUint16 adds a uint16 value to the serializer
func (s *CDTSGoSerializer) AddUint16(val uint16) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_uint16_type)
	C.set_cdt_uint16_val(cdt, C.metaffi_uint16(val))
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddUint32 adds a uint32 value to the serializer
func (s *CDTSGoSerializer) AddUint32(val uint32) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_uint32_type)
	C.set_cdt_uint32_val(cdt, C.metaffi_uint32(val))
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddUint64 adds a uint64 value to the serializer
func (s *CDTSGoSerializer) AddUint64(val uint64) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_uint64_type)
	C.set_cdt_uint64_val(cdt, C.metaffi_uint64(val))
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// validateIntRange validates that an int value fits in the target type
func validateIntRange(val int, targetType C.metaffi_type) error {
	switch targetType {
	case C.metaffi_int8_type:
		if val < -128 || val > 127 {
			return fmt.Errorf("value %d out of range for int8 [-128, 127]", val)
		}
	case C.metaffi_uint8_type:
		if val < 0 || val > 255 {
			return fmt.Errorf("value %d out of range for uint8 [0, 255]", val)
		}
	case C.metaffi_int16_type:
		if val < -32768 || val > 32767 {
			return fmt.Errorf("value %d out of range for int16 [-32768, 32767]", val)
		}
	case C.metaffi_uint16_type:
		if val < 0 || val > 65535 {
			return fmt.Errorf("value %d out of range for uint16 [0, 65535]", val)
		}
	case C.metaffi_int32_type:
		if val < -2147483648 || val > 2147483647 {
			return fmt.Errorf("value %d out of range for int32 [-2147483648, 2147483647]", val)
		}
	case C.metaffi_uint32_type:
		if val < 0 || uint64(val) > 4294967295 {
			return fmt.Errorf("value %d out of range for uint32 [0, 4294967295]", val)
		}
	case C.metaffi_int64_type, C.metaffi_uint64_type:
		// All int values fit in int64/uint64
		return nil
	default:
		return fmt.Errorf("invalid int target type: %d", targetType)
	}
	return nil
}

// AddInt adds a platform-dependent int value with explicit target type
func (s *CDTSGoSerializer) AddInt(val int, targetType C.metaffi_type) (*CDTSGoSerializer, error) {
	if err := validateIntRange(val, targetType); err != nil {
		return nil, err
	}

	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, targetType)
	switch targetType {
	case C.metaffi_int8_type:
		C.set_cdt_int8_val(cdt, C.metaffi_int8(val))
	case C.metaffi_uint8_type:
		C.set_cdt_uint8_val(cdt, C.metaffi_uint8(val))
	case C.metaffi_int16_type:
		C.set_cdt_int16_val(cdt, C.metaffi_int16(val))
	case C.metaffi_uint16_type:
		C.set_cdt_uint16_val(cdt, C.metaffi_uint16(val))
	case C.metaffi_int32_type:
		C.set_cdt_int32_val(cdt, C.metaffi_int32(val))
	case C.metaffi_uint32_type:
		C.set_cdt_uint32_val(cdt, C.metaffi_uint32(val))
	case C.metaffi_int64_type:
		C.set_cdt_int64_val(cdt, C.metaffi_int64(val))
	case C.metaffi_uint64_type:
		C.set_cdt_uint64_val(cdt, C.metaffi_uint64(val))
	default:
		return nil, fmt.Errorf("invalid int target type: %d", targetType)
	}
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddFloat32 adds a float32 value to the serializer
func (s *CDTSGoSerializer) AddFloat32(val float32) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_float32_type)
	C.set_cdt_float32_val(cdt, C.metaffi_float32(val))
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddFloat64 adds a float64 value to the serializer
func (s *CDTSGoSerializer) AddFloat64(val float64) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_float64_type)
	C.set_cdt_float64_val(cdt, C.metaffi_float64(val))
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddBool adds a bool value to the serializer
func (s *CDTSGoSerializer) AddBool(val bool) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_bool_type)
	var cVal C.metaffi_bool
	if val {
		cVal = 1
	} else {
		cVal = 0
	}
	C.set_cdt_bool_val(cdt, cVal)
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// validateRuneRange validates that a rune fits in the target char type
func validateRuneRange(val rune, targetType C.metaffi_type) error {
	switch targetType {
	case C.metaffi_char8_type:
		if val < 0 || val > 255 {
			return fmt.Errorf("rune %d out of range for char8 [0, 255]", val)
		}
	case C.metaffi_char16_type:
		if val < 0 || val > 65535 {
			return fmt.Errorf("rune %d out of range for char16 [0, 65535]", val)
		}
	case C.metaffi_char32_type:
		// All rune values fit in char32
		return nil
	default:
		return fmt.Errorf("invalid rune target type: %d", targetType)
	}
	return nil
}

// AddRune adds a rune value with explicit target type (char8/char16/char32)
func (s *CDTSGoSerializer) AddRune(val rune, targetType C.metaffi_type) (*CDTSGoSerializer, error) {
	if err := validateRuneRange(val, targetType); err != nil {
		return nil, err
	}

	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, targetType)
	switch targetType {
	case C.metaffi_char8_type:
		// Encode rune as UTF-8
		buf := make([]byte, 4)
		n := utf8.EncodeRune(buf, val)
		var c0, c1, c2, c3 C.uint8_t
		c0 = C.uint8_t(buf[0])
		if n > 1 {
			c1 = C.uint8_t(buf[1])
		}
		if n > 2 {
			c2 = C.uint8_t(buf[2])
		}
		if n > 3 {
			c3 = C.uint8_t(buf[3])
		}
		C.set_cdt_char8_val(cdt, c0, c1, c2, c3)
	case C.metaffi_char16_type:
		// Encode rune as UTF-16
		utf16Buf := utf16.Encode([]rune{val})
		if len(utf16Buf) == 0 {
			return nil, errors.New("failed to encode rune as UTF-16")
		}
		C.set_cdt_char16_val(cdt, C.uint16_t(utf16Buf[0]))
	case C.metaffi_char32_type:
		C.set_cdt_char32_val(cdt, C.char32_t(val))
	default:
		return nil, fmt.Errorf("invalid rune target type: %d", targetType)
	}
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddString adds a Go string as UTF-8 (string8)
func (s *CDTSGoSerializer) AddString(val string) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	cStr := C.CString(val)
	defer C.free(unsafe.Pointer(cStr))

	allocated := C.xllr_alloc_string8((*C.char8_t)(unsafe.Pointer(cStr)), C.uint64_t(len(val)))
	if allocated == nil {
		return nil, errors.New("xllr_alloc_string8 failed: memory allocation error")
	}

	C.set_cdt_string8_val(cdt, allocated)
	C.set_cdt_type(cdt, C.metaffi_string8_type)
	C.set_cdt_free_required(cdt, 1)
	s.currentIndex++
	return s, nil
}

// AddString16 adds a Go string as UTF-16 (string16)
func (s *CDTSGoSerializer) AddString16(val string) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	// Convert Go string to UTF-16
	utf16Buf := utf16.Encode([]rune(val))
	if len(utf16Buf) == 0 && len(val) > 0 {
		return nil, errors.New("failed to encode string as UTF-16")
	}

	// Allocate UTF-16 string
	allocated := C.xllr_alloc_string16((*C.char16_t)(unsafe.Pointer(&utf16Buf[0])), C.uint64_t(len(utf16Buf)))
	if allocated == nil {
		return nil, errors.New("xllr_alloc_string16 failed: memory allocation error")
	}

	C.set_cdt_string16_val(cdt, allocated)
	C.set_cdt_type(cdt, C.metaffi_string16_type)
	C.set_cdt_free_required(cdt, 1)
	s.currentIndex++
	return s, nil
}

// AddString32 adds a Go string as UTF-32 (string32)
func (s *CDTSGoSerializer) AddString32(val string) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	// Convert Go string to UTF-32 (runes)
	runes := []rune(val)
	if len(runes) == 0 && len(val) > 0 {
		return nil, errors.New("failed to convert string to runes")
	}

	// Allocate UTF-32 string
	allocated := C.xllr_alloc_string32((*C.char32_t)(unsafe.Pointer(&runes[0])), C.uint64_t(len(runes)))
	if allocated == nil {
		return nil, errors.New("xllr_alloc_string32 failed: memory allocation error")
	}

	C.set_cdt_string32_val(cdt, allocated)
	C.set_cdt_type(cdt, C.metaffi_string32_type)
	C.set_cdt_free_required(cdt, 1)
	s.currentIndex++
	return s, nil
}

// AddNull adds a null value to the serializer
func (s *CDTSGoSerializer) AddNull() (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	C.set_cdt_type(cdt, C.metaffi_null_type)
	C.set_cdt_free_required(cdt, 0)
	s.currentIndex++
	return s, nil
}

// AddHandle adds a handle value to the serializer
func (s *CDTSGoSerializer) AddHandle(handle uintptr, runtimeID uint64) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	// Allocate handle struct
	handlePtr := (*C.struct_cdt_metaffi_handle)(C.xllr_alloc_memory(C.size_t(unsafe.Sizeof(C.struct_cdt_metaffi_handle{}))))
	if handlePtr == nil {
		return nil, errors.New("xllr_alloc_memory failed: memory allocation error")
	}

	handlePtr.handle = C.metaffi_handle(unsafe.Pointer(handle))
	handlePtr.runtime_id = C.uint64_t(runtimeID)
	handlePtr.release = nil

	C.set_cdt_handle_val(cdt, handlePtr)
	C.set_cdt_type(cdt, C.metaffi_handle_type)
	C.set_cdt_free_required(cdt, 1)
	s.currentIndex++
	return s, nil
}

// AddCallable adds a callable value to the serializer
func (s *CDTSGoSerializer) AddCallable(val uintptr, paramTypes []C.metaffi_type, retvalTypes []C.metaffi_type) (*CDTSGoSerializer, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	// Allocate callable struct
	callablePtr := (*C.struct_cdt_metaffi_callable)(C.xllr_alloc_memory(C.size_t(unsafe.Sizeof(C.struct_cdt_metaffi_callable{}))))
	if callablePtr == nil {
		return nil, errors.New("xllr_alloc_memory failed: memory allocation error")
	}

	callablePtr.val = C.metaffi_callable(unsafe.Pointer(val))
	callablePtr.params_types_length = C.metaffi_int8(len(paramTypes))
	callablePtr.retval_types_length = C.metaffi_int8(len(retvalTypes))

	// Allocate parameter types array
	if len(paramTypes) > 0 {
		paramTypesPtr := (*C.metaffi_type)(C.xllr_alloc_memory(C.size_t(len(paramTypes) * int(unsafe.Sizeof(C.metaffi_type(0))))))
		if paramTypesPtr == nil {
			return nil, errors.New("xllr_alloc_memory failed: memory allocation error for parameter types")
		}
		paramTypesSlice := (*[1 << 30]C.metaffi_type)(unsafe.Pointer(paramTypesPtr))[:len(paramTypes):len(paramTypes)]
		copy(paramTypesSlice, paramTypes)
		callablePtr.parameters_types = paramTypesPtr
	} else {
		callablePtr.parameters_types = nil
	}

	// Allocate return value types array
	if len(retvalTypes) > 0 {
		retvalTypesPtr := (*C.metaffi_type)(C.xllr_alloc_memory(C.size_t(len(retvalTypes) * int(unsafe.Sizeof(C.metaffi_type(0))))))
		if retvalTypesPtr == nil {
			return nil, errors.New("xllr_alloc_memory failed: memory allocation error for return value types")
		}
		retvalTypesSlice := (*[1 << 30]C.metaffi_type)(unsafe.Pointer(retvalTypesPtr))[:len(retvalTypes):len(retvalTypes)]
		copy(retvalTypesSlice, retvalTypes)
		callablePtr.retval_types = retvalTypesPtr
	} else {
		callablePtr.retval_types = nil
	}

	C.set_cdt_callable_val(cdt, callablePtr)
	C.set_cdt_type(cdt, C.metaffi_callable_type)
	C.set_cdt_free_required(cdt, 1)
	s.currentIndex++
	return s, nil
}

// addSlice is a helper for adding slices/arrays
func (s *CDTSGoSerializer) addSlice(length uint64, dimensions int64, elementType C.metaffi_type, fillFunc func(*C.struct_cdt, int) error) error {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return err
	}

	// Allocate CDT array
	arr := C.xllr_alloc_cdt_array(C.uint64_t(length))
	if arr == nil {
		return errors.New("xllr_alloc_cdt_array failed: memory allocation error")
	}

	// Allocate CDTS struct
	cdts := (*C.struct_cdts)(C.xllr_alloc_memory(C.size_t(unsafe.Sizeof(C.struct_cdts{}))))
	if cdts == nil {
		return errors.New("xllr_alloc_memory failed: memory allocation error")
	}

	C.set_cdts_arr(cdts, arr)
	C.set_cdts_length(cdts, C.metaffi_size(length))
	C.set_cdts_fixed_dimensions(cdts, C.metaffi_int64(dimensions))
	C.set_cdts_allocated_on_cache(cdts, 0)

	// Fill array elements
	for i := uint64(0); i < length; i++ {
		elem := C.get_cdt_at_index(cdts, C.int(i))
		if elem == nil {
			return fmt.Errorf("failed to get CDT element at index %d", i)
		}
		if err := fillFunc(elem, int(i)); err != nil {
			return fmt.Errorf("failed to fill element at index %d: %w", i, err)
		}
	}

	C.set_cdt_array(cdt, cdts)
	arrayType := C.metaffi_type(C.metaffi_array_type | elementType)
	C.set_cdt_type(cdt, arrayType)
	C.set_cdt_free_required(cdt, 1)
	s.currentIndex++
	return nil
}

// AddInt8Slice adds a slice of int8 values
func (s *CDTSGoSerializer) AddInt8Slice(vals []int8) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_int8_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_int8_type)
		C.set_cdt_int8_val(elem, C.metaffi_int8(vals[i]))
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// AddInt16Slice adds a slice of int16 values
func (s *CDTSGoSerializer) AddInt16Slice(vals []int16) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_int16_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_int16_type)
		C.set_cdt_int16_val(elem, C.metaffi_int16(vals[i]))
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// AddInt32Slice adds a slice of int32 values
func (s *CDTSGoSerializer) AddInt32Slice(vals []int32) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_int32_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_int32_type)
		C.set_cdt_int32_val(elem, C.metaffi_int32(vals[i]))
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// AddInt64Slice adds a slice of int64 values
func (s *CDTSGoSerializer) AddInt64Slice(vals []int64) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_int64_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_int64_type)
		C.set_cdt_int64_val(elem, C.metaffi_int64(vals[i]))
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// AddUint8Slice adds a slice of uint8 values
func (s *CDTSGoSerializer) AddUint8Slice(vals []uint8) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_uint8_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_uint8_type)
		C.set_cdt_uint8_val(elem, C.metaffi_uint8(vals[i]))
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// AddUint16Slice adds a slice of uint16 values
func (s *CDTSGoSerializer) AddUint16Slice(vals []uint16) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_uint16_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_uint16_type)
		C.set_cdt_uint16_val(elem, C.metaffi_uint16(vals[i]))
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// AddUint32Slice adds a slice of uint32 values
func (s *CDTSGoSerializer) AddUint32Slice(vals []uint32) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_uint32_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_uint32_type)
		C.set_cdt_uint32_val(elem, C.metaffi_uint32(vals[i]))
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// AddUint64Slice adds a slice of uint64 values
func (s *CDTSGoSerializer) AddUint64Slice(vals []uint64) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_uint64_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_uint64_type)
		C.set_cdt_uint64_val(elem, C.metaffi_uint64(vals[i]))
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// AddFloat32Slice adds a slice of float32 values
func (s *CDTSGoSerializer) AddFloat32Slice(vals []float32) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_float32_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_float32_type)
		C.set_cdt_float32_val(elem, C.metaffi_float32(vals[i]))
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// AddFloat64Slice adds a slice of float64 values
func (s *CDTSGoSerializer) AddFloat64Slice(vals []float64) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_float64_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_float64_type)
		C.set_cdt_float64_val(elem, C.metaffi_float64(vals[i]))
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// AddBoolSlice adds a slice of bool values
func (s *CDTSGoSerializer) AddBoolSlice(vals []bool) (*CDTSGoSerializer, error) {
	err := s.addSlice(uint64(len(vals)), 1, C.metaffi_bool_type, func(elem *C.struct_cdt, i int) error {
		C.set_cdt_type(elem, C.metaffi_bool_type)
		var cVal C.metaffi_bool
		if vals[i] {
			cVal = 1
		} else {
			cVal = 0
		}
		C.set_cdt_bool_val(elem, cVal)
		C.set_cdt_free_required(elem, 0)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return s, nil
}

// ===== Deserialization Methods (CDTS → Go) =====

// ExtractInt8 extracts an int8 value from the serializer
func (s *CDTSGoSerializer) ExtractInt8() (int8, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_int8_type {
		return 0, fmt.Errorf("type mismatch at index %d: expected int8, got %d", s.currentIndex, actualType)
	}

	val := int8(C.get_cdt_int8_val(cdt))
	s.currentIndex++
	return val, nil
}

// ExtractInt16 extracts an int16 value from the serializer
func (s *CDTSGoSerializer) ExtractInt16() (int16, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_int16_type {
		return 0, fmt.Errorf("type mismatch at index %d: expected int16, got %d", s.currentIndex, actualType)
	}

	val := int16(C.get_cdt_int16_val(cdt))
	s.currentIndex++
	return val, nil
}

// ExtractInt32 extracts an int32 value from the serializer
func (s *CDTSGoSerializer) ExtractInt32() (int32, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_int32_type {
		return 0, fmt.Errorf("type mismatch at index %d: expected int32, got %d", s.currentIndex, actualType)
	}

	val := int32(C.get_cdt_int32_val(cdt))
	s.currentIndex++
	return val, nil
}

// ExtractInt64 extracts an int64 value from the serializer
func (s *CDTSGoSerializer) ExtractInt64() (int64, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_int64_type {
		return 0, fmt.Errorf("type mismatch at index %d: expected int64, got %d", s.currentIndex, actualType)
	}

	val := int64(C.get_cdt_int64_val(cdt))
	s.currentIndex++
	return val, nil
}

// ExtractUint8 extracts a uint8 value from the serializer
func (s *CDTSGoSerializer) ExtractUint8() (uint8, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_uint8_type {
		return 0, fmt.Errorf("type mismatch at index %d: expected uint8, got %d", s.currentIndex, actualType)
	}

	val := uint8(C.get_cdt_uint8_val(cdt))
	s.currentIndex++
	return val, nil
}

// ExtractUint16 extracts a uint16 value from the serializer
func (s *CDTSGoSerializer) ExtractUint16() (uint16, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_uint16_type {
		return 0, fmt.Errorf("type mismatch at index %d: expected uint16, got %d", s.currentIndex, actualType)
	}

	val := uint16(C.get_cdt_uint16_val(cdt))
	s.currentIndex++
	return val, nil
}

// ExtractUint32 extracts a uint32 value from the serializer
func (s *CDTSGoSerializer) ExtractUint32() (uint32, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_uint32_type {
		return 0, fmt.Errorf("type mismatch at index %d: expected uint32, got %d", s.currentIndex, actualType)
	}

	val := uint32(C.get_cdt_uint32_val(cdt))
	s.currentIndex++
	return val, nil
}

// ExtractUint64 extracts a uint64 value from the serializer
func (s *CDTSGoSerializer) ExtractUint64() (uint64, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_uint64_type {
		return 0, fmt.Errorf("type mismatch at index %d: expected uint64, got %d", s.currentIndex, actualType)
	}

	val := uint64(C.get_cdt_uint64_val(cdt))
	s.currentIndex++
	return val, nil
}

// ExtractInt extracts a platform-dependent int value with explicit target type
func (s *CDTSGoSerializer) ExtractInt(targetType C.metaffi_type) (int, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != targetType {
		return 0, fmt.Errorf("type mismatch at index %d: expected %d, got %d", s.currentIndex, targetType, actualType)
	}

	var val int
	switch targetType {
	case C.metaffi_int8_type:
		val = int(C.get_cdt_int8_val(cdt))
	case C.metaffi_uint8_type:
		val = int(C.get_cdt_uint8_val(cdt))
	case C.metaffi_int16_type:
		val = int(C.get_cdt_int16_val(cdt))
	case C.metaffi_uint16_type:
		val = int(C.get_cdt_uint16_val(cdt))
	case C.metaffi_int32_type:
		val = int(C.get_cdt_int32_val(cdt))
	case C.metaffi_uint32_type:
		val = int(C.get_cdt_uint32_val(cdt))
	case C.metaffi_int64_type:
		val = int(C.get_cdt_int64_val(cdt))
	case C.metaffi_uint64_type:
		val = int(C.get_cdt_uint64_val(cdt))
	default:
		return 0, fmt.Errorf("invalid int target type: %d", targetType)
	}

	s.currentIndex++
	return val, nil
}

// ExtractFloat32 extracts a float32 value from the serializer
func (s *CDTSGoSerializer) ExtractFloat32() (float32, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_float32_type {
		return 0, fmt.Errorf("type mismatch at index %d: expected float32, got %d", s.currentIndex, actualType)
	}

	val := float32(C.get_cdt_float32_val(cdt))
	s.currentIndex++
	return val, nil
}

// ExtractFloat64 extracts a float64 value from the serializer
func (s *CDTSGoSerializer) ExtractFloat64() (float64, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_float64_type {
		return 0, fmt.Errorf("type mismatch at index %d: expected float64, got %d", s.currentIndex, actualType)
	}

	val := float64(C.get_cdt_float64_val(cdt))
	s.currentIndex++
	return val, nil
}

// ExtractBool extracts a bool value from the serializer
func (s *CDTSGoSerializer) ExtractBool() (bool, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return false, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_bool_type {
		return false, fmt.Errorf("type mismatch at index %d: expected bool, got %d", s.currentIndex, actualType)
	}

	val := C.get_cdt_bool_val(cdt) != 0
	s.currentIndex++
	return val, nil
}

// ExtractRune extracts a rune value from the serializer
func (s *CDTSGoSerializer) ExtractRune() (rune, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return 0, err
	}

	actualType := C.get_cdt_type(cdt)
	var val rune
	switch actualType {
	case C.metaffi_char8_type:
		var c0, c1, c2, c3 C.uint8_t
		C.get_cdt_char8_val(cdt, &c0, &c1, &c2, &c3)
		buf := []byte{byte(c0), byte(c1), byte(c2), byte(c3)}
		r, _ := utf8.DecodeRune(buf)
		val = r
	case C.metaffi_char16_type:
		c16 := C.get_cdt_char16_val(cdt)
		utf16Buf := []uint16{uint16(c16)}
		runes := utf16.Decode(utf16Buf)
		if len(runes) == 0 {
			return 0, errors.New("failed to decode UTF-16 to rune")
		}
		val = runes[0]
	case C.metaffi_char32_type:
		c32 := C.get_cdt_char32_val(cdt)
		val = rune(c32)
	default:
		return 0, fmt.Errorf("type mismatch at index %d: expected char8/char16/char32, got %d", s.currentIndex, actualType)
	}

	s.currentIndex++
	return val, nil
}

// ExtractString extracts a UTF-8 string from the serializer
func (s *CDTSGoSerializer) ExtractString() (string, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return "", err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_string8_type {
		return "", fmt.Errorf("type mismatch at index %d: expected string8, got %d", s.currentIndex, actualType)
	}

	cStr := C.get_cdt_string8_val(cdt)
	if cStr == nil {
		return "", fmt.Errorf("string8 value is nil at index %d", s.currentIndex)
	}

	val := C.GoString(cStr)
	s.currentIndex++
	return val, nil
}

// ExtractString16 extracts a UTF-16 string from the serializer
func (s *CDTSGoSerializer) ExtractString16() (string, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return "", err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_string16_type {
		return "", fmt.Errorf("type mismatch at index %d: expected string16, got %d", s.currentIndex, actualType)
	}

	cStr16 := C.get_cdt_string16_val(cdt)
	if cStr16 == nil {
		return "", fmt.Errorf("string16 value is nil at index %d", s.currentIndex)
	}

	// Find length (null-terminated) - use pointer arithmetic
	length := 0
	cStr16Ptr := (*[1 << 30]C.char16_t)(unsafe.Pointer(cStr16))
	for length < (1<<30) && cStr16Ptr[length] != 0 {
		length++
	}

	// Convert UTF-16 to Go string
	utf16Buf := (*[1 << 30]uint16)(unsafe.Pointer(cStr16))[:length:length]
	runes := utf16.Decode(utf16Buf)
	val := string(runes)

	s.currentIndex++
	return val, nil
}

// ExtractString32 extracts a UTF-32 string from the serializer
func (s *CDTSGoSerializer) ExtractString32() (string, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return "", err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_string32_type {
		return "", fmt.Errorf("type mismatch at index %d: expected string32, got %d", s.currentIndex, actualType)
	}

	cStr32 := C.get_cdt_string32_val(cdt)
	if cStr32 == nil {
		return "", fmt.Errorf("string32 value is nil at index %d", s.currentIndex)
	}

	// Find length (null-terminated) - use pointer arithmetic
	length := 0
	cStr32Ptr := (*[1 << 30]C.char32_t)(unsafe.Pointer(cStr32))
	for length < (1<<30) && cStr32Ptr[length] != 0 {
		length++
	}

	// Convert UTF-32 (runes) to Go string
	runes := (*[1 << 30]rune)(unsafe.Pointer(cStr32))[:length:length]
	val := string(runes)

	s.currentIndex++
	return val, nil
}

// HandleInfo represents extracted handle information
type HandleInfo struct {
	Handle    uintptr
	RuntimeID uint64
}

// ExtractHandle extracts a handle value from the serializer
func (s *CDTSGoSerializer) ExtractHandle() (HandleInfo, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return HandleInfo{}, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_handle_type {
		return HandleInfo{}, fmt.Errorf("type mismatch at index %d: expected handle, got %d", s.currentIndex, actualType)
	}

	handlePtr := C.get_cdt_handle_val(cdt)
	if handlePtr == nil {
		return HandleInfo{}, fmt.Errorf("handle value is nil at index %d", s.currentIndex)
	}

	info := HandleInfo{
		Handle:    uintptr(unsafe.Pointer(handlePtr.handle)),
		RuntimeID: uint64(handlePtr.runtime_id),
	}

	s.currentIndex++
	return info, nil
}

// CallableInfo represents extracted callable information
type CallableInfo struct {
	Callable    uintptr
	ParamTypes  []C.metaffi_type
	RetvalTypes []C.metaffi_type
}

// ExtractCallable extracts a callable value from the serializer
func (s *CDTSGoSerializer) ExtractCallable() (CallableInfo, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return CallableInfo{}, err
	}

	actualType := C.get_cdt_type(cdt)
	if actualType != C.metaffi_callable_type {
		return CallableInfo{}, fmt.Errorf("type mismatch at index %d: expected callable, got %d", s.currentIndex, actualType)
	}

	callablePtr := C.get_cdt_callable_val(cdt)
	if callablePtr == nil {
		return CallableInfo{}, fmt.Errorf("callable value is nil at index %d", s.currentIndex)
	}

	info := CallableInfo{
		Callable: uintptr(unsafe.Pointer(callablePtr.val)),
	}

	// Copy parameter types
	if callablePtr.params_types_length > 0 && callablePtr.parameters_types != nil {
		paramCount := int(callablePtr.params_types_length)
		paramTypesSlice := (*[1 << 30]C.metaffi_type)(unsafe.Pointer(callablePtr.parameters_types))[:paramCount:paramCount]
		info.ParamTypes = make([]C.metaffi_type, paramCount)
		copy(info.ParamTypes, paramTypesSlice)
	} else {
		info.ParamTypes = []C.metaffi_type{}
	}

	// Copy return value types
	if callablePtr.retval_types_length > 0 && callablePtr.retval_types != nil {
		retvalCount := int(callablePtr.retval_types_length)
		retvalTypesSlice := (*[1 << 30]C.metaffi_type)(unsafe.Pointer(callablePtr.retval_types))[:retvalCount:retvalCount]
		info.RetvalTypes = make([]C.metaffi_type, retvalCount)
		copy(info.RetvalTypes, retvalTypesSlice)
	} else {
		info.RetvalTypes = []C.metaffi_type{}
	}

	s.currentIndex++
	return info, nil
}

// ExtractInt32Slice extracts a slice of int32 from the serializer (array type)
func (s *CDTSGoSerializer) ExtractInt32Slice() ([]int32, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	actualType := C.get_cdt_type(cdt)
	expectedType := C.metaffi_type(C.metaffi_array_type | C.metaffi_int32_type)
	if actualType != expectedType {
		return nil, fmt.Errorf("type mismatch at index %d: expected int32 array, got %d", s.currentIndex, actualType)
	}

	innerCdts := C.get_cdt_array(cdt)
	if innerCdts == nil {
		return nil, fmt.Errorf("array value is nil at index %d", s.currentIndex)
	}

	length := uint64(C.get_cdts_length(innerCdts))
	result := make([]int32, length)
	for i := uint64(0); i < length; i++ {
		elem := C.get_cdt_at_index(innerCdts, C.int(i))
		if elem == nil {
			return nil, fmt.Errorf("failed to get array element at index %d", i)
		}
		result[i] = int32(C.get_cdt_int32_val(elem))
	}

	s.currentIndex++
	return result, nil
}

// ExtractFloat64Slice extracts a slice of float64 from the serializer (array type)
func (s *CDTSGoSerializer) ExtractFloat64Slice() ([]float64, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return nil, err
	}

	actualType := C.get_cdt_type(cdt)
	expectedType := C.metaffi_type(C.metaffi_array_type | C.metaffi_float64_type)
	if actualType != expectedType {
		return nil, fmt.Errorf("type mismatch at index %d: expected float64 array, got %d", s.currentIndex, actualType)
	}

	innerCdts := C.get_cdt_array(cdt)
	if innerCdts == nil {
		return nil, fmt.Errorf("array value is nil at index %d", s.currentIndex)
	}

	length := uint64(C.get_cdts_length(innerCdts))
	result := make([]float64, length)
	for i := uint64(0); i < length; i++ {
		elem := C.get_cdt_at_index(innerCdts, C.int(i))
		if elem == nil {
			return nil, fmt.Errorf("failed to get array element at index %d", i)
		}
		result[i] = float64(C.get_cdt_float64_val(elem))
	}

	s.currentIndex++
	return result, nil
}

// ===== Utility Methods =====

// Reset resets the current index to 0
func (s *CDTSGoSerializer) Reset() {
	s.currentIndex = 0
}

// GetIndex returns the current index
func (s *CDTSGoSerializer) GetIndex() uint64 {
	return uint64(s.currentIndex)
}

// SetIndex sets the current index (can fail if index out of bounds)
func (s *CDTSGoSerializer) SetIndex(index uint64) error {
	if index >= uint64(C.get_cdts_length(s.cdts)) {
		return fmt.Errorf("index %d out of bounds (size: %d)", index, C.get_cdts_length(s.cdts))
	}
	s.currentIndex = C.metaffi_size(index)
	return nil
}

// Size returns the size of the CDTS
func (s *CDTSGoSerializer) Size() uint64 {
	return uint64(C.get_cdts_length(s.cdts))
}

// HasMore returns true if there are more elements to extract
func (s *CDTSGoSerializer) HasMore() bool {
	return s.currentIndex < C.metaffi_size(C.get_cdts_length(s.cdts))
}

// PeekType returns the type of the current element without advancing the index
func (s *CDTSGoSerializer) PeekType() (C.metaffi_type, error) {
	if err := s.checkBounds(); err != nil {
		return 0, err
	}
	cdt := C.get_cdt_at_index(s.cdts, C.int(s.currentIndex))
	if cdt == nil {
		return 0, fmt.Errorf("failed to get CDT at index %d", s.currentIndex)
	}
	return C.get_cdt_type(cdt), nil
}

// IsNull checks if the current element is null
func (s *CDTSGoSerializer) IsNull() (bool, error) {
	cdt, err := s.getCurrentCDT()
	if err != nil {
		return false, err
	}
	return C.get_cdt_type(cdt) == C.metaffi_null_type, nil
}
