package plugin

import (
	"errors"

	compiler "github.com/MetaFFI/sdk/compiler/go"
	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

var PluginMain *LanguagePluginMain

//--------------------------------------------------------------------
type LanguagePluginMain struct {
	hostCompiler  compiler.HostCompiler
	guestCompiler compiler.GuestCompiler
}

//--------------------------------------------------------------------
func NewLanguagePluginMain(hostCompiler compiler.HostCompiler, guestCompiler compiler.GuestCompiler) *LanguagePluginMain {
	this := &LanguagePluginMain{hostCompiler: hostCompiler, guestCompiler: guestCompiler}
	CreateLanguagePluginInterfaceHandler(this)
	return this
}

//--------------------------------------------------------------------
func (this *LanguagePluginMain) CompileToGuest(idlDefinition *IDL.IDLDefinition, outputPath string, outputFilename string, guestOptions map[string]string) error {
	
	if this.guestCompiler == nil {
		return errors.New("Guest is not supported by this plugin")
	}
	
	return this.guestCompiler.Compile(idlDefinition, outputPath, outputFilename, guestOptions)
}

//--------------------------------------------------------------------
func (this *LanguagePluginMain) CompileFromHost(idlDefinition *IDL.IDLDefinition, outputPath string, outputFilename string, hostOptions map[string]string) error {
	
	if this.hostCompiler == nil {
		return errors.New("Host is not supported by this plugin")
	}
	
	return this.hostCompiler.Compile(idlDefinition, outputPath, outputFilename, hostOptions)
}

//--------------------------------------------------------------------
