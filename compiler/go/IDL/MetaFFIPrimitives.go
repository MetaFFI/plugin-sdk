package IDL

/*
#cgo !windows LDFLAGS: -L. -ldl
#include <../../../runtime/metaffi_primitives.h>
*/
import "C"
import "fmt"
import "strings"

type MethodType string
type MetaFFIType string

type MetaFFITypeWithAlias struct {
	Type  IDL.MetaFFIType
	Alias string
}

const (
	FUNCTION    MethodType = "function"
	METHOD      MethodType = "method"
	CONSTRUCTOR MethodType = "constructor"
	DESTRUCTOR  MethodType = "destructor"
)

const (
	FLOAT64 MetaFFIType = "float64"
	FLOAT32 MetaFFIType = "float32"
	
	INT8  MetaFFIType = "int8"
	INT16 MetaFFIType = "int16"
	INT32 MetaFFIType = "int32"
	INT64 MetaFFIType = "int64"
	
	UINT8  MetaFFIType = "uint8"
	UINT16 MetaFFIType = "uint16"
	UINT32 MetaFFIType = "uint32"
	UINT64 MetaFFIType = "uint64"
	
	BOOL MetaFFIType = "bool"
	
	CHAR8  MetaFFIType = "char8"
	CHAR16 MetaFFIType = "char16"
	CHAR32 MetaFFIType = "char32"
	
	STRING8  MetaFFIType = "string8"
	STRING16 MetaFFIType = "string16"
	STRING32 MetaFFIType = "string32"
	
	HANDLE MetaFFIType = "handle"
	ARRAY  MetaFFIType = "array"
	
	ANY MetaFFIType = "any" // Parameter can receive any MetaFFI type
	
	SIZE MetaFFIType = "size"
	
	//--------------------------------------------------------------------
	
	FLOAT64_ARRAY MetaFFIType = "float64_array"
	FLOAT32_ARRAY MetaFFIType = "float32_array"
	
	INT8_ARRAY  MetaFFIType = "int8_array"
	INT16_ARRAY MetaFFIType = "int16_array"
	INT32_ARRAY MetaFFIType = "int32_array"
	INT64_ARRAY MetaFFIType = "int64_array"
	
	UINT8_ARRAY  MetaFFIType = "uint8_array"
	UINT16_ARRAY MetaFFIType = "uint16_array"
	UINT32_ARRAY MetaFFIType = "uint32_array"
	UINT64_ARRAY MetaFFIType = "uint64_array"
	
	BOOL_ARRAY MetaFFIType = "bool_array"
	
	CHAR8_ARRAY  MetaFFIType = "char8_array"
	CHAR16_ARRAY MetaFFIType = "char16_array"
	CHAR32_ARRAY MetaFFIType = "char32_array"
	
	STRING8_ARRAY  MetaFFIType = "string8_array"
	STRING16_ARRAY MetaFFIType = "string16_array"
	STRING32_ARRAY MetaFFIType = "string32_array"
	
	HANDLE_ARRAY MetaFFIType = "handle_array"
	ANY_ARRAY MetaFFIType = "any_array"
	
	SIZE_ARRAY MetaFFIType = "size_array"
)

var TypeStringToTypeEnum = map[MetaFFIType]uint64{
	FLOAT64:  uint64(C.metaffi_float64_type),
	FLOAT32:  uint64(C.metaffi_float32_type),
	INT8:     uint64(C.metaffi_int8_type),
	INT16:    uint64(C.metaffi_int16_type),
	INT32:    uint64(C.metaffi_int32_type),
	INT64:    uint64(C.metaffi_int64_type),
	UINT8:    uint64(C.metaffi_uint8_type),
	UINT16:   uint64(C.metaffi_uint16_type),
	UINT32:   uint64(C.metaffi_uint32_type),
	UINT64:   uint64(C.metaffi_uint64_type),
	BOOL:     uint64(C.metaffi_bool_type),
	STRING8:  uint64(C.metaffi_string8_type),
	STRING16: uint64(C.metaffi_string16_type),
	STRING32: uint64(C.metaffi_string32_type),
	CHAR8:    uint64(C.metaffi_char8_type),
	CHAR16:   uint64(C.metaffi_char16_type),
	CHAR32:   uint64(C.metaffi_char32_type),
	HANDLE:   uint64(C.metaffi_handle_type),
	
	ARRAY: uint64(C.metaffi_array_type),
	
	SIZE: uint64(C.metaffi_size_type),
	
	ANY: uint64(C.metaffi_any_type),
	
	FLOAT64_ARRAY:  uint64(C.metaffi_float64_array_type),
	FLOAT32_ARRAY:  uint64(C.metaffi_float32_array_type),
	INT8_ARRAY:     uint64(C.metaffi_int8_array_type),
	INT16_ARRAY:    uint64(C.metaffi_int16_array_type),
	INT32_ARRAY:    uint64(C.metaffi_int32_array_type),
	INT64_ARRAY:    uint64(C.metaffi_int64_array_type),
	UINT8_ARRAY:    uint64(C.metaffi_uint8_array_type),
	UINT16_ARRAY:   uint64(C.metaffi_uint16_array_type),
	UINT32_ARRAY:   uint64(C.metaffi_uint32_array_type),
	UINT64_ARRAY:   uint64(C.metaffi_uint64_array_type),
	BOOL_ARRAY:     uint64(C.metaffi_bool_array_type),
	STRING8_ARRAY:  uint64(C.metaffi_string8_array_type),
	STRING16_ARRAY: uint64(C.metaffi_string16_array_type),
	STRING32_ARRAY: uint64(C.metaffi_string32_array_type),

	ANY_ARRAY: uint64(C.metaffi_any_array_type),
	
	HANDLE_ARRAY: uint64(C.metaffi_handle_array_type),
	SIZE_ARRAY:   uint64(C.metaffi_size_array_type),
}

func IsMetaFFIType(metaffiType string) bool {
	_, found := TypeStringToTypeEnum[MetaFFIType(metaffiType)]
	return found
}

func ArgMetaFFIType(arg *ArgDefinition) uint64 {
	
	str := arg.Type
	if arg.Dimensions > 0 && !strings.HasSuffix(string(arg.Type), "_array") {
		str += "_array"
	}
	
	res, found := TypeStringToTypeEnum[MetaFFIType(str)]
	if !found {
		panic(fmt.Sprintf("%v is not a metaffi type. Dimensions: %v", str, arg.Dimensions))
	}
	
	return res
}
