package common

import "C"
import (
	"fmt"
	"os"
	"strings"

	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

var TemplateFuncMap = map[string]interface{}{
	"AsPublic":                        asPublic,
	"ToGoNameConv":                    ToGoNameConv,
	"CastIfNeeded":                    castIfNeeded,
	"ParamActual":                     paramActual,
	"GetEnvVar":                       getEnvVar,
	"Sizeof":                          Sizeof,
	"CalculateArgsLength":             calculateArgsLength,
	"CalculateArgLength":              calculateArgLength,
	"Add":                             add,
	"IsInteger":                       isInteger,
	"IsParametersOrReturnValues":      isParametersOrReturnValues,
	"ConvertToCType":                  convertToCType,
	"ConvertToGoType":                 convertToGoType,
	"GetNumericTypes":                 getNumericTypes,
	"GetMetaFFIType":                  getMetaFFIType,
	"GetMetaFFIArrayType":             getMetaFFIArrayType,
	"GetMetaFFIStringTypes":           getMetaFFIStringTypes,
	"MakeMetaFFIType":                 makeMetaFFIType,
	"MethodNameNotExists":             methodNameNotExists,
	"GenerateCodeAllocateCDTS":        generateCodeAllocateCDTS,
	"GenerateCodeXCall":               generateCodeXCall,
	"GenerateCodeEntryPointSignature": generateCodeEntrypointSignature,
	"GenerateCodeEntryPointEmptyStructSignature": generateCodeEntryPointEmptyStructSignature,
	"GetCDTReturnValueIndex":                     getCDTReturnValueIndex,
	"GetCDTParametersIndex":                      getCDTParametersIndex,
	"GenerateMethodReceiverCode":                 generateMethodReceiverCode,
	"GenerateMethodName":                         generateMethodName,
	"GenerateMethodParams":                       generateMethodParams,
	"GetTypeOrAlias":                             getTypeOrAlias,
	"HandleNoneGoObject":                         handleNoneGoObject,
	"IsGoRuntimePackNeeded":                      isGoRuntimePackNeeded,
	"ConvertEmptyInterfaceFromCDTSToCorrectType": convertEmptyInterfaceFromCDTSToCorrectType,
	"CallParameters":                             callParameters,
	"GetMetaFFITypeInfos":                        GetMetaFFITypeInfos,
	"Repeat":                                     strings.Repeat,
	"Iterate":                                    Iterate,
	"GetMetaFFINumericType":                      GetMetaFFINumericType,
	"AssertAndConvert":                           assertAndConvert,
	"GetTypeForCDTToGo":                          getTypeForCDTToGo,
}

func getTypeForCDTToGo(arg *IDL.ArgDefinition, mod *IDL.ModuleDefinition) string {

	// if arg is an array of handles - "reflect.TypeOf(name of the type without [])"
	if arg.Type == IDL.HANDLE_ARRAY {
		return fmt.Sprintf("reflect.TypeFor[%v]()", strings.ReplaceAll(convertToGoType(arg, mod), "[]", ""))
	}

	// otherwise return "nil"
	return "nil"
}

func GetMetaFFINumericType(typeName IDL.MetaFFIType) uint64 {
	t := IDL.TypeStringToTypeEnum[IDL.MetaFFIType(strings.ToLower(string(typeName)))]
	if t == 0 {
		panic("Failed to find numeric MetaFFI type for " + strings.ToLower(string(typeName)))
	}
	return t
}

func Iterate(count *uint) []uint {
	var i uint
	var items []uint
	for i = 0; i < (*count); i++ {
		items = append(items, i)
	}
	return items
}

func GetMetaFFITypeInfos(funcDef *IDL.FunctionDefinition) string {
	paramsTypes := funcDef.GetParametersMetaFFITypeInfo()
	retvalTypes := funcDef.GetReturnValuesMetaFFITypeInfo()

	strParams := "nil"
	strRetVals := "nil"

	if paramsTypes != nil && len(paramsTypes) > 0 {
		strParams = "[]IDL.MetaFFITypeInfo{"
		for _, p := range paramsTypes {
			strParams += `IDL.MetaFFITypeInfo{StringType: IDL.` + IDL.TypeStringToEnumName[p.StringType] + `, Alias: "` + p.Alias + `", Type: ` + fmt.Sprintf("%v", GetMetaFFINumericType(p.StringType)) + `, Dimensions: ` + fmt.Sprintf("%v", p.Dimensions) + ` },`
		}
		strParams += "}"
	}

	if retvalTypes != nil && len(retvalTypes) > 0 {
		strRetVals = "[]IDL.MetaFFITypeInfo{"
		for _, p := range retvalTypes {
			strRetVals += `IDL.MetaFFITypeInfo{StringType: IDL.` + IDL.TypeStringToEnumName[p.StringType] + `, Alias: "` + p.Alias + `", Type: ` + fmt.Sprintf("%v", GetMetaFFINumericType(p.StringType)) + `, Dimensions: ` + fmt.Sprintf("%v", p.Dimensions) + ` },`
		}
		strRetVals += "}"
	}

	return strParams + "," + strRetVals
}

// --------------------------------------------------------------------
func callParameters(funcDef *IDL.FunctionDefinition, startIndex int) string {
	// {{range $index, $elem := $f.Parameters}}{{if $index}},{{end}}{{if $elem.IsTypeAlias}}({{$elem.GetTypeOrAlias}})({{$elem.Name}}){{else}}{{$elem.Name}}{{end}}{{end}}
	params := make([]string, 0)
	for i, p := range funcDef.Parameters {
		if i < startIndex {
			continue
		}

		if i+1 == len(funcDef.Parameters) { // if last parameter - check if function is variadic
			if _, exists := funcDef.Tags["variadic_parameter"]; exists {
				params = append(params, p.Name+"...")
				continue
			}
		}

		params = append(params, fmt.Sprintf("%v", p.Name))
	}

	return strings.Join(params, ",")
}

// --------------------------------------------------------------------
func isGoRuntimePackNeeded(idl *IDL.IDLDefinition) bool {

	for _, m := range idl.Modules {
		for _, g := range m.Globals {
			if g.IsHandle() {
				return true
			}
		}

		for _, f := range m.Functions {
			for _, p := range f.Parameters {
				if p.IsHandle() {
					return true
				}
			}

			for _, r := range f.ReturnValues {
				if r.IsHandle() {
					return true
				}
			}
		}

		if len(m.Classes) > 0 {
			return true
		}
	}

	return false
}

// --------------------------------------------------------------------
func handleNoneGoObject(arg *IDL.ArgDefinition, module *IDL.ModuleDefinition) string {
	if arg.IsTypeAlias() && module.IsContainsClass(arg.TypeAlias) {
		return asPublic(arg.TypeAlias) + "{ h: obj }" // construct a "wrapper class" for handle
	} else {
		return "obj" // just return handle
	}
}

// --------------------------------------------------------------------
func getTypeOrAlias(arg *IDL.ArgDefinition, module *IDL.ModuleDefinition) string {
	if arg.IsTypeAlias() && module.IsContainsClass(arg.TypeAlias) {
		return asPublic(arg.TypeAlias)
	} else {
		return asPublic(string(IDL.HANDLE))
	}
}

// --------------------------------------------------------------------
func generateMethodParams(meth *IDL.MethodDefinition, mod *IDL.ModuleDefinition) string {
	//{{range $index, $elem := $f.Parameters}}{{if gt $index 0}}{{if gt $index 1}},{{end}} {{$elem.Name}} {{ConvertToGoType $elem}}{{end}}{{end}}

	res := make([]string, 0)

	for i, p := range meth.Parameters {
		if i == 0 {
			if !meth.InstanceRequired {
				res = append(res, fmt.Sprintf("%v %v", p.Name, convertToGoType(p, mod)))
			}
			continue
		}

		res = append(res, fmt.Sprintf("%v %v", p.Name, convertToGoType(p, mod)))
	}

	return strings.Join(res, ",")
}

// --------------------------------------------------------------------
func generateMethodName(meth *IDL.MethodDefinition) string {
	if meth.InstanceRequired {
		return ToGoNameConv(meth.GetNameWithOverloadIndex())
	} else {
		return fmt.Sprintf("%v_%v", ToGoNameConv(meth.GetParent().Name), ToGoNameConv(meth.GetNameWithOverloadIndex()))
	}
}

// --------------------------------------------------------------------
func generateMethodReceiverCode(meth *IDL.MethodDefinition) string {
	if meth.InstanceRequired {
		return fmt.Sprintf("(this *%v)", asPublic(meth.GetParent().Name))
	} else {
		return "" // No receiver
	}
}

// --------------------------------------------------------------------
func getCDTReturnValueIndex(params []*IDL.ArgDefinition, retvals []*IDL.ArgDefinition) int {
	return 1 // return values are always at index 1
}

// --------------------------------------------------------------------
func getCDTParametersIndex(params []*IDL.ArgDefinition) int {
	if len(params) > 0 {
		return 0
	} else {
		panic("Both parameters and return values are 0 - parameters should not be used")
	}
}

// --------------------------------------------------------------------
func generateCodeEntrypointSignature(className string, funcName string, params []*IDL.ArgDefinition, retvals []*IDL.ArgDefinition) string {

	name := ""
	if className != "" {
		name += className + "_"
	}

	name += funcName

	if len(params) > 0 || len(retvals) > 0 {
		return fmt.Sprintf("%v(_ *C.void, xcall_params *C.struct_cdts, out_err **C.char)", name)
	} else {
		return fmt.Sprintf("%v(_ *C.void, out_err **C.char)", name)
	}
}

// --------------------------------------------------------------------
func generateCodeEntryPointEmptyStructSignature(className string) string {

	rettype := IDL.NewArgArrayDefinition("instance", IDL.HANDLE, 0)
	return generateCodeEntrypointSignature(className, "EmptyStruct_MetaFFI", nil, []*IDL.ArgDefinition{rettype})
}

// --------------------------------------------------------------------
func generateCodeXCall(className string, funcName string, params []*IDL.ArgDefinition, retvals []*IDL.ArgDefinition) string {
	/*
		err = XLLRXCallParamsRet(metaffi_positional_args_Gett_id, xcall_params)  // call function pointer metaffi_positional_args_Gett_id via XLLR

		// check errors
		if err != nil{
			err = fmt.Errorf("Function failed metaffi_positional_args.Gett. Error: %v", err)
			return
		}
	*/
	var name string
	if className != "" {
		name += className + "_"
	}

	name += funcName

	code := fmt.Sprintf("\terr = XLLR%v(%v_id", xcall(params, retvals), name)

	if len(params) > 0 || len(retvals) > 0 {
		code += ", xcall_params"
	}

	code += fmt.Sprintf(")  // call function pointer %v_id via XLLR\n", name)

	code += "\t\n"
	code += "\t// check errors\n"
	code += "\tif err != nil{\n"
	code += "\t\terr = fmt.Errorf(\"Failed calling function" + className + "." + funcName + ". Error: %v\", err)\n"
	code += "\t\treturn\n"
	code += "\t}\n"

	return code
}

// --------------------------------------------------------------------
func xcall(params []*IDL.ArgDefinition, retvals []*IDL.ArgDefinition) string {

	// name of xcall
	if len(params) > 0 && len(retvals) > 0 {
		return "XCallParamsRet"
	} else if len(params) > 0 {
		return "XCallParamsNoRet"
	} else if len(retvals) > 0 {
		return "XCallNoParamsRet"
	} else {
		return "XCallNoParamsNoRet"
	}
}

// --------------------------------------------------------------------
func generateCodeAllocateCDTS(params []*IDL.ArgDefinition, retval []*IDL.ArgDefinition) string {
	/*
		xcall_params, parametersCDTS, return_valuesCDTS := XLLRAllocCDTSBuffer(1, 1)
	*/

	if len(params) == 0 && len(retval) == 0 {
		return ""
	}

	code := "xcall_params, "
	if len(params) > 0 {
		code += "parametersCDTS, "
	} else {
		code += "_, "
	}

	if len(retval) > 0 {
		code += "return_valuesCDTS "
	} else {
		code += "_ "
	}

	code += fmt.Sprintf(":= XLLRAllocCDTSBuffer(%v, %v)\n", len(params), len(retval))

	return code
}

// --------------------------------------------------------------------
func methodNameNotExists(c *IDL.ClassDefinition, fieldName string, prefix string) bool {
	for _, m := range c.Methods {
		if m.Name == prefix+fieldName {
			return false
		}
	}

	return true
}

// --------------------------------------------------------------------
func isPrimitiveType(typeName string) bool {
	switch typeName {
	case "bool", "byte", "complex64", "complex128", "error", "float32", "float64", "int", "int8", "int16", "int32", "int64", "rune", "string", "uint", "uint8", "uint16", "uint32", "uint64", "uintptr":
		return true
	default:
		return false
	}
}

func convertToGoType(def *IDL.ArgDefinition, mod *IDL.ModuleDefinition) string {

	var res string

	t := IDL.MetaFFIType(strings.ReplaceAll(string(def.Type), "_array", ""))

	switch t {
	case IDL.STRING8:
		fallthrough
	case IDL.STRING16:
		fallthrough
	case IDL.STRING32:
		res = "string"
	case IDL.ANY:
		res = "interface{}"
	case IDL.HANDLE:
		if def.IsTypeAlias() && mod.IsContainsClass(def.TypeAlias) {
			res = asPublic(def.TypeAlias)
		} else if def.IsTypeAlias() && isPrimitiveType(def.TypeAlias) {
			res = def.TypeAlias
		} else {
			res = "interface{}"
		}
	default:
		res = string(t)
	}

	if def.Dimensions > 0 {
		for i := 0; i < def.Dimensions; i++ {
			res = "[]" + res
		}
	}

	return res
}

// --------------------------------------------------------------------
func convertToCType(metaffiType IDL.MetaFFIType) string {
	switch metaffiType {
	case "float32":
		return "float"
	case "float64":
		return "double"
	default:
		return string("C." + metaffiType)
	}
}

// --------------------------------------------------------------------
func isParametersOrReturnValues(f *IDL.FunctionDefinition) bool {
	return len(f.Parameters) > 0 || len(f.ReturnValues) > 0
}

// --------------------------------------------------------------------
func isInteger(t string) bool {
	return strings.Index(t, "int") == 0
}

// --------------------------------------------------------------------
func add(x int, y int) int {
	return x + y
}

// --------------------------------------------------------------------
func calculateArgLength(f *IDL.ArgDefinition) int {

	if f.IsString() {
		if f.IsArray() {
			return 3 // pointer to string array, pointer to sizes array, length of array
		} else {
			return 2 // pointer to string, size of string
		}
	} else {
		if f.IsArray() {
			return 2 // pointer to type array, length of array
		} else {
			return 1 // value
		}
	}
}

// --------------------------------------------------------------------
func calculateArgsLength(fields []*IDL.ArgDefinition) int {

	length := 0

	for _, f := range fields {
		length += calculateArgLength(f)
	}

	return length
}

// --------------------------------------------------------------------
func Sizeof(field *IDL.ArgDefinition) string {
	return fmt.Sprintf("C.sizeof_metaffi_%v", field.Type)
}

// --------------------------------------------------------------------
func getEnvVar(env string, is_path bool) string {
	res := os.Getenv(env)
	if is_path {
		res = strings.ReplaceAll(res, "\\", "/")
	}
	return res
}

// --------------------------------------------------------------------
func paramActual(field *IDL.ArgDefinition, direction string, namePrefix string) string {

	var prefix string
	if namePrefix != "" {
		prefix = namePrefix + "_"
	} else {
		prefix = direction + "_"
	}

	switch field.Type {
	case IDL.STRING8:
		fallthrough
	case IDL.STRING16:
		fallthrough
	case IDL.STRING32:
		if field.IsArray() {
			if direction == "out" {
				return fmt.Sprintf("&" + prefix + field.Name + ",&" + prefix + field.Name + "_sizes" + ",&" + prefix + field.Name + "_len")
			} else {
				return fmt.Sprintf(prefix + field.Name + "," + prefix + field.Name + "_sizes" + "," + prefix + field.Name + "_len")
			}

		} else {

			if direction == "out" {
				return fmt.Sprintf("&" + prefix + field.Name + ",&" + prefix + field.Name + "_len")
			} else {
				return fmt.Sprintf(prefix + field.Name + "," + prefix + field.Name + "_len")
			}
		}

	default:
		if field.IsArray() {
			if direction == "out" {
				return fmt.Sprintf("&" + prefix + field.Name + ",&" + prefix + field.Name + "_len")
			} else {
				return fmt.Sprintf(prefix + field.Name + "," + prefix + field.Name + "_len")
			}

		} else {
			if direction == "out" {
				return fmt.Sprintf("&" + prefix + field.Name)
			} else {
				return fmt.Sprintf(prefix + field.Name)
			}
		}
	}
}

// --------------------------------------------------------------------
func asPublic(elem string) string {

	return ToGoNameConv(elem)
	//if len(elem) == 0 {
	//	return ""
	//} else if len(elem) == 1 {
	//	return strings.ToUpper(elem)
	//} else {
	//	return strings.ToUpper(elem[0:1]) + elem[1:]
	//}
}

// --------------------------------------------------------------------
func countUnderscores(s string) (int, int) {
	startCount := 0
	endCount := 0
	for i := 0; i < len(s); i++ {
		if s[i] == '_' {
			startCount++
		} else {
			break
		}
	}
	for i := len(s) - 1; i >= 0; i-- {
		if s[i] == '_' {
			endCount++
		} else {
			break
		}
	}
	return startCount, endCount
}

// --------------------------------------------------------------------
func ToGoNameConv(elem string) string {

	underscoreAtStart, underscoreAtEnd := countUnderscores(elem)

	elem = strings.Replace(elem, "_", " ", -1)
	elem = strings.Title(elem)
	elem = strings.Replace(elem, " ", "_", -1)

	if underscoreAtEnd > 0 {
		elem += strings.Repeat("_", underscoreAtEnd)
	}

	if underscoreAtStart > 0 { // This is because Go doesn't support _ at the beginning of the element.
		elem = "U_" + elem
	}

	return elem
}

// --------------------------------------------------------------------
func castIfNeeded(elem string) string {
	if strings.Contains(elem, "int") {
		return "int(" + elem + ")"
	}
	return elem
}

// --------------------------------------------------------------------
func getNumericTypes() (numericTypes []string) {
	return []string{"float64", "float32", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64"}
}

// --------------------------------------------------------------------
func makeMetaFFIType(t string) string {
	return "metaffi_" + strings.ToLower(t)
}

// --------------------------------------------------------------------
func getMetaFFIStringTypes() (numericTypes []string) {
	return []string{"string8"}
}

// --------------------------------------------------------------------
func getMetaFFIType(numericType string) (numericTypes uint64) {
	return IDL.TypeStringToTypeEnum[IDL.MetaFFIType(numericType)]
}

// --------------------------------------------------------------------
func getMetaFFIArrayType(numericType string) (numericTypes uint64) {
	return IDL.TypeStringToTypeEnum[IDL.MetaFFIType(numericType+"_array")]
}

//--------------------------------------------------------------------

func assertAndConvert(varName string, def *IDL.ArgDefinition, mod *IDL.ModuleDefinition) string {

	if def.Type == IDL.HANDLE || def.Type == IDL.HANDLE_ARRAY {
		return varName
	}

	if def.IsTypeAlias() { // assert
		return fmt.Sprintf("%v(%v.(%v))", convertToGoType(def, mod), varName, def.TypeAlias)
	} else {
		return varName
	}
}

func convertEmptyInterfaceFromCDTSToCorrectType(elem *IDL.ArgDefinition, mod *IDL.ModuleDefinition, outputVarExists bool) string {

	if elem.IsAny() {
		return fmt.Sprintf("%v := %vAsInterface", elem.Name, elem.Name)
	}

	if elem.Dimensions > 0 {
		/*
			{{if gt $f.Dimensions 0}}
			{{$f.Name}} := {{$f.Name}}AsInterface.({{Repeat [] $f.Dimensions}}{{ConvertToGoType $f.ArgDefinition $m}})
		*/
		code := fmt.Sprintf("%v := %vAsInterface.(%v)\n", elem.Name, elem.Name, convertToGoType(elem, mod))

		return code
	} else {
		// if casting is needed
		castingCode := ""
		if !elem.IsTypeAlias() {
			castingCode = convertToGoType(elem, mod)
		} else if elem.IsTypeAlias() && !elem.IsHandle() {
			castingCode = elem.TypeAlias
		}

		// if the type is a HANDLE - assertion needs to be the Alias
		// if the type is NOT a handle - assertion needs to be to "covertToGoType"
		assertion := ""
		if elem.IsHandle() && elem.IsTypeAlias() {
			assertion = elem.TypeAlias
		} else {
			assertion = convertToGoType(elem, mod)
		}

		return fmt.Sprintf("%v := %v(%vAsInterface.(%v))", elem.Name, castingCode, elem.Name, assertion)

	}
}
