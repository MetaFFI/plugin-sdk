The following readme explains how to use the provided SDK to implement a new language support for MetaFFI.

Using the SDK is not mandatory, as the sole requirement is to implement the interfaces `idl_plugin_interface.h`, `language_plugin_interface.h` and `runtime_plugin_interface.h` in anyway the developer see fit.

# Legend
* H - Mandatory for host
* G - Mandatory for guest
* OH - Optional for host
* OG - Optional for guest

# Compiler
The compilers plugin compiles MetaFFI IDL to either host language code or guest language code to provide access to the MetaFFI system from the host language, or from MetaFFI system to the guest language. A new language support does not have to implement both guest and host.

The plugin main (`PluginMain.go`) exports a C function called `void init_plugin(void)`, which MetaFFI compiler uses to initialize the compiler plugin. A typical `PluginMain.go` would be:

```go
package main

import (
	"github.com/MetaFFI/sdk/compiler/go/plugin"
)

import "C"

//export init_plugin
func init_plugin() {
	plugin.PluginMain = plugin.NewLanguagePluginMain(NewHostCompiler(), NewGuestCompiler())
}

//--------------------------------------------------------------------
func main() {}

//--------------------------------------------------------------------
```

The line `plugin.PluginMain = plugin.NewLanguagePluginMain` creates an object for MetaFFI providing the objects to use for compilation in MetaFFI. The compilers must implement the following interfaces:
```google go
type GuestCompiler interface {
	Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, guestOptions map[string]string) (err error)
}

type HostCompiler interface {
	Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, hostOptions map[string]string) (err error)
}
```
## Guest Compiler (G)
Let's examine `GuestCompiler.Compile`:
* `definition` - MetaFFI definition of the foreign entities
* `outputDir` - Directory to write compiler output
* `outputFilename` - Filename to write output. If empty, uses `definition.IDLSource`. If contains a coed block (using # delimiter), contains the code block name
* `guestOptions` - Plugin specific options passed from MetaFFI CLI to the plugin

The function `NewGuestCompiler()` returns an instance of the object implementing the interface.

## Host Compiler (H)
Let's examine `HostCompiler.Compile`:
* `definition` - MetaFFI definition of the foreign entities
* `outputDir` - Directory to write compiler output
* `outputFilename` - Filename to write output. If empty, uses `definition.IDLSource`. If contains a coed block (using # delimiter), contains the code block name 

The function `NewHostCompiler()` returns an instance of the object implementing the interface.

The steps a Host compiler `Compile` method usually are:
* Generate code - generate the stubs from the host language to call MetaFFI system (to XLLR API)
* (if applicable) Generate executable code for the host language. For example, for Java, the code can be compiled to a JAR file which is referenced by the Java to call the foreign entities
* Write generated code (or executable code) to output file in output path

# Runtime (G)


# IDL (G)

# Code Embedding (OG)

# CDTS
## MetaFFI Primitives (H,G)
## Object Support (OG, OH)
