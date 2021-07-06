package compiler

/*
#cgo !windows LDFLAGS: -L. -ldl
#include <../../runtime/openffi_primitives.h>
*/
import "C"

const(
	FLOAT64 = "float64"
	FLOAT32 = "float32"

	INT8 = "int8"
	INT16 = "int16"
	INT32 = "int32"
	INT64 = "int64"

	UINT8 = "uint8"
	UINT16 = "uint16"
	UINT32 = "uint32"
	UINT64 = "uint64"

	BOOL = "bool"

	STRING = "string" // undefined encoding
	STRING8 = "string-utf8"
	STRING16 = "string-utf16"
	STRING32 = "string-utf32"

	SIZE = "size"
)

var TypeToOpenFFIType = map[string]uint64{
	"openffi_float64_type": uint64(C.openffi_float64_type),
	"openffi_float32_type": uint64(C.openffi_float32_type),
	"openffi_int8_type": uint64(C.openffi_int8_type),
	"openffi_int16_type": uint64(C.openffi_int16_type),
	"openffi_int32_type": uint64(C.openffi_int32_type),
	"openffi_int64_type": uint64(C.openffi_int64_type),
	"openffi_uint8_type": uint64(C.openffi_uint8_type),
	"openffi_uint16_type": uint64(C.openffi_uint16_type),
	"openffi_uint32_type": uint64(C.openffi_uint32_type),
	"openffi_uint64_type": uint64(C.openffi_uint64_type),
	"openffi_bool_type": uint64(C.openffi_bool_type),
	"openffi_string_type": uint64(C.openffi_string_type),
	"openffi_string8_type": uint64(C.openffi_string8_type),
	"openffi_string16_type": uint64(C.openffi_string16_type),
	"openffi_string32_type": uint64(C.openffi_string32_type),
	"openffi_handle_type": uint64(C.openffi_handle_type),

	"openffi_array_type": uint64(C.openffi_array_type),

	"openffi_size_type": uint64(C.openffi_size_type),

	"openffi_pointer_type": uint64(C.openffi_pointer_type),

	"openffi_float64_ptr_type": uint64(C.openffi_float64_ptr_type),
	"openffi_float32_ptr_type": uint64(C.openffi_float32_ptr_type),
	"openffi_int8_ptr_type": uint64(C.openffi_int8_ptr_type),
	"openffi_int16_ptr_type": uint64(C.openffi_int16_ptr_type),
	"openffi_int32_ptr_type": uint64(C.openffi_int32_ptr_type),
	"openffi_int64_ptr_type": uint64(C.openffi_int64_ptr_type),
	"openffi_uint8_ptr_type": uint64(C.openffi_uint8_ptr_type),
	"openffi_uint16_ptr_type": uint64(C.openffi_uint16_ptr_type),
	"openffi_uint32_ptr_type": uint64(C.openffi_uint32_ptr_type),
	"openffi_uint64_ptr_type": uint64(C.openffi_uint64_ptr_type),
	"openffi_bool_ptr_type": uint64(C.openffi_bool_ptr_type),
	"openffi_string_ptr_type": uint64(C.openffi_string_ptr_type),
	"openffi_string8_ptr_type": uint64(C.openffi_string8_ptr_type),
	"openffi_string16_ptr_type": uint64(C.openffi_string16_ptr_type),
	"openffi_string32_ptr_type": uint64(C.openffi_string32_ptr_type),

	"openffi_float64_array_type": uint64(C.openffi_float64_array_type),
	"openffi_float32_array_type": uint64(C.openffi_float32_array_type),
	"openffi_int8_array_type": uint64(C.openffi_int8_array_type),
	"openffi_int16_array_type": uint64(C.openffi_int16_array_type),
	"openffi_int32_array_type": uint64(C.openffi_int32_array_type),
	"openffi_int64_array_type": uint64(C.openffi_int64_array_type),
	"openffi_uint8_array_type": uint64(C.openffi_uint8_array_type),
	"openffi_uint16_array_type": uint64(C.openffi_uint16_array_type),
	"openffi_uint32_array_type": uint64(C.openffi_uint32_array_type),
	"openffi_uint64_array_type": uint64(C.openffi_uint64_array_type),
	"openffi_bool_array_type": uint64(C.openffi_bool_array_type),
	"openffi_string_array_type": uint64(C.openffi_string_array_type),
	"openffi_string8_array_type": uint64(C.openffi_string8_array_type),
	"openffi_string16_array_type": uint64(C.openffi_string16_array_type),
	"openffi_string32_array_type": uint64(C.openffi_string32_array_type),

	"openffi_handle_array_type": uint64(C.openffi_handle_array_type),
	"openffi_size_array_type": uint64(C.openffi_size_array_type),
}