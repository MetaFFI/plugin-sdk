package compiler

/*
#include <stdlib.h>
#include "idl_plugin_interface.h"

module_definition* get_next_module(module_definition* p)
{
	p++;
	return p;
}

function_definition* get_next_function(function_definition* p)
{
	p++;
	return p;
}

param_return_definition* get_next_param_return(param_return_definition* p)
{
	p++;
	return p;
}
 */
import "C"

type IDLDefinition struct {
	IDLFilename string
	IDLCode string
	IDLExtension string
	IDLFullPath string
	TargetLanguage string
	Modules []ModuleDefinition
}

type ModuleDefinition struct{
	Name string
	Functions []FunctionDefinition
}

type FunctionDefinition struct {
	Name string
	ForeignFunctionName string
	ParametersStructureName string
	ReturnValuesStructureName string
	Parameters []ParamReturnDefinition
	ReturnValues []ParamReturnDefinition
}

type ParamReturnDefinition struct{
	Name string
	Type string
	IsComplexType bool
	IsArray bool
}
//--------------------------------------------------------------------
func NewIDLDefinition(idl_def *C.idl_definition) *IDLDefinition{

	res := &IDLDefinition{
		IDLFilename: C.GoStringN(idl_def.idl_filename, C.int(idl_def.idl_filename_length)),
		IDLCode: C.GoStringN(idl_def.idl_code, C.int(idl_def.idl_code_length)),
		IDLExtension: C.GoStringN(idl_def.idl_extension, C.int(idl_def.idl_extension_length)),
		IDLFullPath: C.GoStringN(idl_def.idl_full_path, C.int(idl_def.idl_full_path_length)),
		TargetLanguage: C.GoStringN(idl_def.target_language, C.int(idl_def.target_language_length)),
		Modules: make([]ModuleDefinition, 0),
	}

	// iterate modules
	var i C.uint
	currentModulePtr := idl_def.modules
	for i=0 ; i<idl_def.modules_length ; i++{

		mod := ModuleDefinition{
			Name:      C.GoStringN(currentModulePtr.module_name, C.int(currentModulePtr.module_name_length)),
			Functions: make([]FunctionDefinition, 0),
		}

		// iterate functions
		var j C.uint
		currentFunctionPtr := currentModulePtr.functions
		for j=0 ; j<currentModulePtr.functions_length ; j++{

			function := FunctionDefinition{
				Name:                      C.GoStringN(currentFunctionPtr.function_name, C.int(currentFunctionPtr.function_name_length)),
				ForeignFunctionName:       C.GoStringN(currentFunctionPtr.foreign_function_name, C.int(currentFunctionPtr.foreign_function_name_length)),
				ParametersStructureName:   C.GoStringN(currentFunctionPtr.parameters_structure_name, C.int(currentFunctionPtr.parameters_structure_name_length)),
				ReturnValuesStructureName: C.GoStringN(currentFunctionPtr.return_values_structure_name, C.int(currentFunctionPtr.return_values_structure_name_length)),
				Parameters:                make([]ParamReturnDefinition, 0),
				ReturnValues:              make([]ParamReturnDefinition, 0),
			}

			// iterate Parameters/Return values
			var k C.uint
			currentParamPtr := currentFunctionPtr.parameters
			for k=0 ; k<currentFunctionPtr.parameters_length ; k++{
				param := ParamReturnDefinition{
					Name:          C.GoStringN(currentParamPtr.param_return_definition_name, C.int(currentParamPtr.param_return_definition_name_length)),
					Type:          C.GoStringN(currentParamPtr.param_return_definition_type, C.int(currentParamPtr.param_return_definition_type_length)),
					IsComplexType: currentParamPtr.is_complex_type != 0,
					IsArray:       currentParamPtr.is_array != 0,
				}

				function.Parameters = append(function.Parameters, param)
				currentParamPtr = C.get_next_param_return(currentParamPtr)
			}

			currentReturnValuePtr := currentFunctionPtr.return_values
			for k=0 ; k<currentFunctionPtr.return_values_length ; k++{
				retval := ParamReturnDefinition{
					Name:          C.GoStringN(currentReturnValuePtr.param_return_definition_name, C.int(currentReturnValuePtr.param_return_definition_name_length)),
					Type:          C.GoStringN(currentReturnValuePtr.param_return_definition_type, C.int(currentReturnValuePtr.param_return_definition_type_length)),
					IsComplexType: currentReturnValuePtr.is_complex_type != 0,
					IsArray:       currentReturnValuePtr.is_array != 0,
				}

				function.ReturnValues = append(function.ReturnValues, retval)
				currentParamPtr = C.get_next_param_return(currentReturnValuePtr)
			}

			mod.Functions = append(mod.Functions, function)
			currentFunctionPtr = C.get_next_function(currentFunctionPtr)
		}


		res.Modules = append(res.Modules, mod)
		currentModulePtr = C.get_next_module(currentModulePtr)
	}

	return res
}
//--------------------------------------------------------------------
func (this *IDLDefinition) ToCStruct() *C.idl_definition{

	var res *C.idl_definition
	res = (*C.idl_definition)(C.malloc(C.sizeof_struct_idl_definition))

	res.idl_filename = C.CString(this.IDLFilename)
	res.idl_filename_length = C.uint(len(this.IDLFilename))

	res.idl_code = C.CString(this.IDLCode)
	res.idl_code_length = C.uint(len(this.IDLCode))

	res.idl_extension = C.CString(this.IDLExtension)
	res.idl_extension_length = C.uint(len(this.IDLExtension))

	res.idl_full_path = C.CString(this.IDLFullPath)
	res.idl_full_path_length = C.uint(len(this.IDLFullPath))

	res.target_language = C.CString(this.TargetLanguage)
	res.target_language_length = C.uint(len(this.TargetLanguage))

	res.modules_length = C.uint(len(this.Modules))
	res.modules = (*C.module_definition)(C.malloc(C.sizeof_struct_module_definition*C.ulong(res.modules_length)))

	curModule := res.modules
	for i:=0 ; i<len(this.Modules) ; i++{

		curModule.module_name = C.CString(this.Modules[i].Name)
		curModule.module_name_length = C.uint(len(this.Modules[i].Name))

		curModule.functions_length = C.uint(len(this.Modules[i].Functions))
		curModule.functions = (*C.function_definition)(C.malloc(C.sizeof_struct_function_definition*C.ulong(curModule.functions_length)))

		curFunction := curModule.functions
		for j:=0 ; j<len(this.Modules[i].Functions) ; j++{

			curFunction.function_name = C.CString(this.Modules[i].Functions[j].Name)
			curFunction.function_name_length = C.uint(len(this.Modules[i].Functions[j].Name))

			curFunction.foreign_function_name = C.CString(this.Modules[i].Functions[j].ForeignFunctionName)
			curFunction.foreign_function_name_length = C.uint(len(this.Modules[i].Functions[j].ForeignFunctionName))

			curFunction.parameters_structure_name = C.CString(this.Modules[i].Functions[j].ParametersStructureName)
			curFunction.parameters_structure_name_length = C.uint(len(this.Modules[i].Functions[j].ParametersStructureName))

			curFunction.return_values_structure_name = C.CString(this.Modules[i].Functions[j].ReturnValuesStructureName)
			curFunction.return_values_structure_name_length = C.uint(len(this.Modules[i].Functions[j].ReturnValuesStructureName))

			// parameters

			curFunction.parameters_length = C.uint(len(this.Modules[i].Functions[j].Parameters))
			curFunction.parameters = (*C.param_return_definition)(C.malloc(C.sizeof_struct_param_return_definition*C.ulong(curFunction.parameters_length)))

			curParameter := curFunction.parameters
			for k:=0 ; k<len(this.Modules[i].Functions[j].Parameters) ; k++{

				curParameter.param_return_definition_name = C.CString(this.Modules[i].Functions[j].Parameters[k].Name)
				curParameter.param_return_definition_name_length = C.uint(len(this.Modules[i].Functions[j].Parameters[k].Name))

				curParameter.param_return_definition_type = C.CString(this.Modules[i].Functions[j].Parameters[k].Type)
				curParameter.param_return_definition_type_length = C.uint(len(this.Modules[i].Functions[j].Parameters[k].Type))

				if this.Modules[i].Functions[j].Parameters[k].IsComplexType{
					curParameter.is_complex_type = 1
				} else {
					curParameter.is_complex_type = 0
				}

				if this.Modules[i].Functions[j].Parameters[k].IsArray{
					curParameter.is_array = 1
				} else {
					curParameter.is_array = 0
				}

				curParameter = C.get_next_param_return(curParameter)
			}

			// Return Values
			curFunction.return_values_length = C.uint(len(this.Modules[i].Functions[j].ReturnValues))
			curFunction.return_values = (*C.param_return_definition)(C.malloc(C.sizeof_struct_param_return_definition*C.ulong(curFunction.return_values_length)))

			curReturnValue := curFunction.return_values
			for k:=0 ; k<len(this.Modules[i].Functions[j].ReturnValues) ; k++{

				curReturnValue.param_return_definition_name = C.CString(this.Modules[i].Functions[j].ReturnValues[k].Name)
				curReturnValue.param_return_definition_name_length = C.uint(len(this.Modules[i].Functions[j].ReturnValues[k].Name))

				curReturnValue.param_return_definition_type = C.CString(this.Modules[i].Functions[j].ReturnValues[k].Type)
				curReturnValue.param_return_definition_type_length = C.uint(len(this.Modules[i].Functions[j].ReturnValues[k].Type))

				if this.Modules[i].Functions[j].ReturnValues[k].IsComplexType{
					curReturnValue.is_complex_type = 1
				} else {
					curReturnValue.is_complex_type = 0
				}

				if this.Modules[i].Functions[j].ReturnValues[k].IsArray{
					curReturnValue.is_array = 1
				} else {
					curReturnValue.is_array = 0
				}

				curReturnValue = C.get_next_param_return(curReturnValue)
			}


			curFunction = C.get_next_function(curFunction)
		}

		curModule = C.get_next_module(curModule)
	}

	return res
}
//--------------------------------------------------------------------