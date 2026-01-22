package compiler

import (
	"github.com/MetaFFI/sdk/compiler/go/guest"
	"github.com/MetaFFI/sdk/compiler/go/host"
	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

// GuestCompiler generates guest entrypoints for the target runtime.
type GuestCompiler interface {
	Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, guestOptions map[string]string) error
}

// HostCompiler generates host stubs for the target runtime.
type HostCompiler interface {
	Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, hostOptions map[string]string) error
}

// NewGuestCompiler returns the Go guest compiler implementation.
func NewGuestCompiler() GuestCompiler {
	return guest.NewGuestCompiler()
}

// NewHostCompiler returns the Go host compiler implementation.
func NewHostCompiler() HostCompiler {
	return host.NewHostCompiler()
}
