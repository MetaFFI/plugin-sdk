package compiler

/*
#include "../language_plugin_interface.h"
*/
import "C"
import (
	"fmt"
	. "github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"os"
	"path/filepath"
	"strings"
	"time"
)

var languagePluginInterfaceHandler *LanguagePluginInterfaceHandler

type LanguagePluginInterface interface {
	CompileToGuest(idlDefinition *IDLDefinition, outputPath string, outputFilename string, guestOptions map[string]string) error
	CompileFromHost(idlDefinition *IDLDefinition, outputPath string, outputFilename string, hostOptions map[string]string) error
}

//--------------------------------------------------------------------
type LanguagePluginInterfaceHandler struct {
	wrapped LanguagePluginInterface
}

//--------------------------------------------------------------------
func CreateLanguagePluginInterfaceHandler(wrapped LanguagePluginInterface) {
	languagePluginInterfaceHandler = &LanguagePluginInterfaceHandler{wrapped: wrapped}
}

//--------------------------------------------------------------------
func (this *LanguagePluginInterfaceHandler) compile_to_guest(idl_def_json *C.char, idl_def_json_length C.uint,
                                                            output_path *C.char, output_path_length C.uint,
                                                            guest_options *C.char, guest_options_length C.uint,
                                                            out_err **C.char, out_err_len *C.uint) {
	def, err := NewIDLDefinitionFromJSON(C.GoStringN(idl_def_json, C.int(idl_def_json_length)))
	if err != nil {
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
	
	outPath := C.GoStringN(output_path, C.int(output_path_length))
	outFilename := ""
	
	// if outPath doesn't exist OR is a file, split outPath and filename
	outPathStat, err := os.Stat(outPath)
	if (err != nil && os.IsNotExist(err)) || (err == nil && !outPathStat.IsDir()) {
		splitted := filepath.SplitList(outPath)
		
		if len(splitted) < 2 {
			*out_err = C.CString(fmt.Sprintf("Invalid output path: %v", outPath))
			*out_err_len = C.uint(len(fmt.Sprintf("Invalid output path: %v", outPath)))
			return
		}
		
		outPath = filepath.Join(splitted[:len(splitted)-2]...)
		outFilename = splitted[len(splitted)-1]
	}
	
	guestOptions := C.GoStringN(guest_options, C.int(guest_options_length))

    // parse hostOptions
    guestOptionsMap := make(map[string]string)
    if strings.TrimSpace(guestOptions) != "" {
        options := strings.Split(guestOptions, ",")
        for _, option := range options {
            keyval := strings.Split(option, "=")
            if len(keyval) != 2 {
                msg := "Guest options are invalid"
                *out_err = C.CString(msg)
                *out_err_len = C.uint(len(msg))
                return
            }

            guestOptionsMap[keyval[0]] = keyval[1]
        }
    }
	
	if outFilename == "" {
		outFilename = def.IDLSource
	}

	
	err = this.wrapped.CompileToGuest(def, outPath, outFilename, guestOptionsMap)
	if err != nil {
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
}

//--------------------------------------------------------------------
func (this *LanguagePluginInterfaceHandler) compile_from_host(idl_def_json *C.char, idl_def_json_length C.uint,
	output_path *C.char, output_path_length C.uint,
	host_options *C.char, host_options_length C.int,
	out_err **C.char, out_err_len *C.uint) {
	def, err := NewIDLDefinitionFromJSON(C.GoStringN(idl_def_json, C.int(idl_def_json_length)))
	if err != nil {
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
	
	outPath := C.GoStringN(output_path, C.int(output_path_length))
	outFilename := ""
	
	// if outPath doesn't exist OR is a file, split outPath and filename
	outPathStat, err := os.Stat(outPath)
	if (err != nil && os.IsNotExist(err)) || (err == nil && !outPathStat.IsDir()) {
		splitted := filepath.SplitList(outPath)
		
		if len(splitted) < 2 {
			*out_err = C.CString(fmt.Sprintf("Invalid output path: %v", outPath))
			*out_err_len = C.uint(len(fmt.Sprintf("Invalid output path: %v", outPath)))
			return
		}
		
		outPath = filepath.Join(splitted[:len(splitted)-2]...)
		outFilename = splitted[len(splitted)-1]
	}
	
	hostOptions := C.GoStringN(host_options, C.int(host_options_length))
	
	// to_py_tuple hostOptions
	hostOptionsMap := make(map[string]string)
	if strings.TrimSpace(hostOptions) != "" {
		options := strings.Split(hostOptions, ",")
		for _, option := range options {
			keyval := strings.Split(option, "=")
			if len(keyval) != 2 {
				msg := "Host options are invalid"
				*out_err = C.CString(msg)
				*out_err_len = C.uint(len(msg))
				return
			}
			
			hostOptionsMap[keyval[0]] = keyval[1]
		}
	}
	
	if outFilename == "" {
		outFilename = def.IDLSource
	}
	
	err = this.wrapped.CompileFromHost(def, outPath, outFilename, hostOptionsMap)
	
	if err != nil {
		*out_err = C.CString(err.Error())
		*out_err_len = C.uint(len(err.Error()))
		return
	}
}

//--------------------------------------------------------------------
//export compile_to_guest
func compile_to_guest(idl_def_json *C.char, idl_def_json_length C.uint,
	output_path *C.char, output_path_length C.uint,
	guest_options *C.char, guest_options_length C.uint,
	out_err **C.char, out_err_len *C.uint) {
	
	if languagePluginInterfaceHandler == nil {
		*out_err = C.CString("Go Compiler plugin not initialised!")
		*out_err_len = C.uint(len("Go Compiler plugin not initialised!"))
		return
	}
	
	defer func() {
		if err := recover(); err != nil {
			msg := fmt.Sprintf("%v", err)
			*out_err = C.CString(msg)
			*out_err_len = C.uint(len(msg))
		}
	}()
	
	languagePluginInterfaceHandler.compile_to_guest(idl_def_json, idl_def_json_length, output_path, output_path_length, guest_options, guest_options_length, out_err, out_err_len)
}

//--------------------------------------------------------------------
//export compile_from_host
func compile_from_host(idl_def_json *C.char, idl_def_json_length C.uint,
	output_path *C.char, output_path_length C.uint,
	host_options *C.char, host_options_length C.int,
	out_err **C.char, out_err_len *C.uint) {
	
	time.Sleep(10 * time.Second)
	
	if languagePluginInterfaceHandler == nil {
		*out_err = C.CString("Go Compiler plugin not initialised!")
		*out_err_len = C.uint(len("Go Compiler plugin not initialised!"))
		return
	}
	
	defer func() {
		if err := recover(); err != nil {
			msg := fmt.Sprintf("%v", err)
			*out_err = C.CString(msg)
			*out_err_len = C.uint(len(msg))
		}
	}()
	
	languagePluginInterfaceHandler.compile_from_host(idl_def_json, idl_def_json_length, output_path, output_path_length, host_options, host_options_length, out_err, out_err_len)
	
}

//--------------------------------------------------------------------
