package idl_compiler

import (
	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

// FunctionInfo represents an extracted function
type FunctionInfo struct {
	Name         string
	Parameters   []*ParameterInfo
	ReturnValues []*ParameterInfo
	Comment      string
	IsVariadic   bool
	ReceiverType string // For methods: the receiver type name
	ReceiverPtr  bool   // For methods: true if receiver is pointer
}

// ParameterInfo represents a function parameter or return value
type ParameterInfo struct {
	Name       string
	Type       IDL.MetaFFIType
	TypeAlias  string
	Dimensions int
	Comment    string
}

// StructInfo represents an extracted struct
type StructInfo struct {
	Name        string
	Fields      []*FieldInfo
	Methods     []*FunctionInfo
	Comment     string
	Constructor *FunctionInfo // NewXxx function if found
}

// FieldInfo represents a struct field
type FieldInfo struct {
	Name       string
	Type       IDL.MetaFFIType
	TypeAlias  string
	Dimensions int
	Comment    string
	Tag        string // Struct tag string
	IsEmbedded bool   // True for embedded fields
}

// InterfaceInfo represents an extracted interface
type InterfaceInfo struct {
	Name    string
	Methods []*FunctionInfo
	Comment string
}

// GlobalInfo represents a global variable or constant
type GlobalInfo struct {
	Name       string
	Type       IDL.MetaFFIType
	TypeAlias  string
	Dimensions int
	Comment    string
	IsConst    bool // True for constants
}
