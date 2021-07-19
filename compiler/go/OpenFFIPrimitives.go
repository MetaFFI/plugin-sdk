package compiler

/*
#cgo !windows LDFLAGS: -L. -ldl
#include <../../runtime/openffi_primitives.h>
*/
import "C"

type FunctionType string
type Operator string
type OpenFFIType string

const(
	FUNCTION FunctionType = "function"
	METHOD FunctionType = "method"
	CONSTRUCTOR FunctionType = "constructor"
	DESTRUCTOR FunctionType = "destructor"
)

const(
	ASSIGN Operator = "="
	DOT Operator = "."

	ADD Operator = "BI+"
	SUB Operator = "BI-"
	MULT Operator = "BI*"
	DIV Operator = "BI/"
	MOD Operator = "BI%"
	ADD_ASSIGN Operator = "+="
	SUB_ASSIGN Operator = "-="
	MULT_ASSIGN Operator = "*="
	DIV_ASSIGN Operator = "/="
	MOD_ASSIGN Operator = "%="

	INC Operator = "++"
	DEC Operator = "--"

	AFFIRMATION Operator = "UN+"
	NEGATION Operator = "UN-"

	LEFT_SHIFT Operator = "<<"
	RIGHT_SHIFT Operator = ">>"
	LEFT_SHIFT_ASSIGN Operator = "<<="
	RIGHT_SHIFT_ASSIGN Operator = ">>="

	BITWISE_OR Operator = "|"
	BITWISE_AND Operator = "&"
	BITWISE_XOR Operator = "^"
	BITWISE_COMPLIMENT Operator = "~"
	BITWISE_OR_ASSIGN Operator = "|="
	BITWISE_AND_ASSIGN Operator = "&="
	BITWISE_XOR_ASSIGN Operator = "^="

	EQUAL Operator = "=="
	NOT_EQUAL Operator = "!="
	LESS Operator = "<"
	LESS_OR_EQUAL Operator = "<="
	GREATER Operator = ">"
	GREATER_OR_EQUAL Operator = ">="
	AND Operator = "&&"
	OR Operator = "||"
	NOT Operator = "!"

	ARRAY Operator = "[]"
)

const(
	FLOAT64 OpenFFIType = "float64"
	FLOAT32 OpenFFIType = "float32"

	INT8 OpenFFIType = "int8"
	INT16 OpenFFIType = "int16"
	INT32 OpenFFIType = "int32"
	INT64 OpenFFIType = "int64"

	UINT8 OpenFFIType = "uint8"
	UINT16 OpenFFIType = "uint16"
	UINT32 OpenFFIType = "uint32"
	UINT64 OpenFFIType = "uint64"

	BOOL OpenFFIType = "bool"

	CHAR8 OpenFFIType = "char8"
	CHAR16 OpenFFIType = "char16"
	CHAR32 OpenFFIType = "char32"

	STRING8 OpenFFIType = "string8"
	STRING16 OpenFFIType = "string16"
	STRING32 OpenFFIType = "string32"

	HANDLE OpenFFIType = "handle"

	ANY OpenFFIType = "any" // Parameter can receive any OpenFFI type

	SIZE OpenFFIType = "size"

	//--------------------------------------------------------------------
	
	FLOAT64_ARRAY OpenFFIType = "float64_array"
	FLOAT32_ARRAY OpenFFIType = "float32_array"

	INT8_ARRAY OpenFFIType = "int8_array"
	INT16_ARRAY OpenFFIType = "int16_array"
	INT32_ARRAY OpenFFIType = "int32_array"
	INT64_ARRAY OpenFFIType = "int64_array"

	UINT8_ARRAY OpenFFIType = "uint8_array"
	UINT16_ARRAY OpenFFIType = "uint16_array"
	UINT32_ARRAY OpenFFIType = "uint32_array"
	UINT64_ARRAY OpenFFIType = "uint64_array"

	BOOL_ARRAY OpenFFIType = "bool_array"

	CHAR8_ARRAY OpenFFIType = "char8_array"
	CHAR16_ARRAY OpenFFIType = "char16_array"
	CHAR32_ARRAY OpenFFIType = "char32_array"

	STRING8_ARRAY OpenFFIType = "string8_array"
	STRING16_ARRAY OpenFFIType = "string16_array"
	STRING32_ARRAY OpenFFIType = "string32_array"

	HANDLE_ARRAY OpenFFIType = "handle_array"

	SIZE_ARRAY OpenFFIType = "size_array"
)

var TypeStringToTypeEnum = map[OpenFFIType]uint64{
	FLOAT64: uint64(C.openffi_float64_type),
	FLOAT32: uint64(C.openffi_float32_type),
	INT8: uint64(C.openffi_int8_type),
	INT16: uint64(C.openffi_int16_type),
	INT32: uint64(C.openffi_int32_type),
	INT64: uint64(C.openffi_int64_type),
	UINT8: uint64(C.openffi_uint8_type),
	UINT16: uint64(C.openffi_uint16_type),
	UINT32: uint64(C.openffi_uint32_type),
	UINT64: uint64(C.openffi_uint64_type),
	BOOL: uint64(C.openffi_bool_type),
	STRING8: uint64(C.openffi_string8_type),
	STRING16: uint64(C.openffi_string16_type),
	STRING32: uint64(C.openffi_string32_type),
	CHAR8: uint64(C.openffi_char8_type),
	CHAR16: uint64(C.openffi_char16_type),
	CHAR32: uint64(C.openffi_char32_type),
	HANDLE: uint64(C.openffi_handle_type),

	"array": uint64(C.openffi_array_type),

	SIZE: uint64(C.openffi_size_type),

	ANY: uint64(C.openffi_any_type),

	FLOAT64_ARRAY: uint64(C.openffi_float64_array_type),
	FLOAT32_ARRAY: uint64(C.openffi_float32_array_type),
	INT8_ARRAY: uint64(C.openffi_int8_array_type),
	INT16_ARRAY: uint64(C.openffi_int16_array_type),
	INT32_ARRAY: uint64(C.openffi_int32_array_type),
	INT64_ARRAY: uint64(C.openffi_int64_array_type),
	UINT8_ARRAY: uint64(C.openffi_uint8_array_type),
	UINT16_ARRAY: uint64(C.openffi_uint16_array_type),
	UINT32_ARRAY: uint64(C.openffi_uint32_array_type),
	UINT64_ARRAY: uint64(C.openffi_uint64_array_type),
	BOOL_ARRAY: uint64(C.openffi_bool_array_type),
	STRING8_ARRAY: uint64(C.openffi_string8_array_type),
	STRING16_ARRAY: uint64(C.openffi_string16_array_type),
	STRING32_ARRAY: uint64(C.openffi_string32_array_type),

	HANDLE_ARRAY: uint64(C.openffi_handle_array_type),
	SIZE_ARRAY: uint64(C.openffi_size_array_type),
}

func IsOpenFFIType(openffiType string) bool{
	_, found := TypeStringToTypeEnum[OpenFFIType(openffiType)]
	return found
}