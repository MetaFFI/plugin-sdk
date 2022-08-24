package compiler

import (
	"errors"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
)

var PluginMain *LanguagePluginMain

type GuestCompiler interface {
	Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, blockName string, blockCode string) (err error)
}

type HostCompiler interface {
	Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, hostOptions map[string]string) (err error)
}

//--------------------------------------------------------------------
type LanguagePluginMain struct {
	hostCompiler  HostCompiler
	guestCompiler GuestCompiler
}

//--------------------------------------------------------------------
func NewLanguagePluginMain(hostCompiler HostCompiler, guestCompiler GuestCompiler) *LanguagePluginMain {
	this := &LanguagePluginMain{hostCompiler: hostCompiler, guestCompiler: guestCompiler}
	CreateLanguagePluginInterfaceHandler(this)
	return this
}

//--------------------------------------------------------------------
func (this *LanguagePluginMain) CompileToGuest(idlDefinition *IDL.IDLDefinition, outputPath string, outputFilename string, blockName string, blockCode string) error {
	
	if this.guestCompiler == nil {
		return errors.New("Guest is not supported by this plugin")
	}
	
	return this.guestCompiler.Compile(idlDefinition, outputPath, outputFilename, blockName, blockCode)
}

//--------------------------------------------------------------------
func (this *LanguagePluginMain) CompileFromHost(idlDefinition *IDL.IDLDefinition, outputPath string, outputFilename string, hostOptions map[string]string) error {
	
	if this.hostCompiler == nil {
		return errors.New("Host is not supported by this plugin")
	}
	
	return this.hostCompiler.Compile(idlDefinition, outputPath, outputFilename, hostOptions)
}

//--------------------------------------------------------------------
