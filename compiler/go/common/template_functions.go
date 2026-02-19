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
	"SafeTypeForAssertion":                       safeTypeForAssertion,
	"ConvertGlobalSetterExpression":              convertGlobalSetterExpression,
	"AutoPackedTypeStr":                          AutoPackedTypeStr,
	"AutoPackedNumericType":                      AutoPackedNumericType,
	"IsDirectlySerializable":                     isDirectlySerializable,
	"IsDirectlySerializableScalarOnly":           isDirectlySerializableScalarOnly,
	"DirectExtractExpr":                          directExtractExpr,
	"DirectSetStmt":                              directSetStmt,
}

// stripArraySuffix strips "_packed_array" or "_array" suffix from a MetaFFI type string,
// returning the base type (e.g. "int64_packed_array" -> "int64", "int64_array" -> "int64").
func stripArraySuffix(t IDL.MetaFFIType) IDL.MetaFFIType {
	s := string(t)
	if strings.HasSuffix(s, "_packed_array") {
		return IDL.MetaFFIType(strings.TrimSuffix(s, "_packed_array"))
	}
	return IDL.MetaFFIType(strings.ReplaceAll(s, "_array", ""))
}

// isGoKeywordType returns true if the type string is a Go keyword that cannot stand alone as a type
// (e.g. "func", "chan", "map", "struct"). These require additional syntax (parameter lists, element types, etc.)
// and must be replaced with "interface{}" when used in type assertions or conversions.
func isGoKeywordType(t string) bool {
	switch t {
	case "func", "chan", "map", "struct", "handle", "callable":
		return true
	}
	return false
}

// safeTypeForAssertion returns a Go type string safe for use in .(type) or (type)(x); never returns empty.
func safeTypeForAssertion(def *IDL.ArgDefinition, mod *IDL.ModuleDefinition) string {
	s := def.GetTypeOrAlias()
	if s != "" && !isGoKeywordType(s) {
		if def.Dimensions > 0 {
			s = strings.Repeat("[]", def.Dimensions) + s
		}
		return s
	}
	s = convertToGoType(def, mod)
	if s == "" || isGoKeywordType(s) {
		return "interface{}"
	}
	return s
}

func getTypeForCDTToGo(arg *IDL.ArgDefinition, mod *IDL.ModuleDefinition) string {

	// if arg is an array of handles - "reflect.TypeOf(name of the type without [])"
	if arg.Type == IDL.HANDLE_ARRAY {
		return fmt.Sprintf("reflect.TypeFor[%v]()", strings.ReplaceAll(convertToGoType(arg, mod), "[]", ""))
	}

	// For callable types, pass the function type to TraverseCDT so it can create
	// a properly-typed Go func via reflect.MakeFunc.
	if arg.Type == IDL.CALLABLE && arg.IsTypeAlias() {
		return fmt.Sprintf("reflect.TypeOf((%v)(nil))", arg.TypeAlias)
	}

	// otherwise return "nil"
	return "nil"
}

func GetMetaFFINumericType(typeName IDL.MetaFFIType) uint64 {
	if typeName == "" {
		return 0
	}
	t := IDL.TypeStringToTypeEnum[IDL.MetaFFIType(strings.ToLower(string(typeName)))]
	if t == 0 {
		return 0
	}
	return t
}

// autoPackedType returns the packed array type string for a 1D primitive array,
// or the original type string if packing is not applicable.
// Packed arrays are only used for 1-dimensional arrays of primitive types that
// have a defined packed variant (numerics, bool, string8, handle, callable).
func autoPackedType(arg *IDL.ArgDefinition) IDL.MetaFFIType {
	typeStr := arg.Type
	dims := arg.Dimensions

	// If dimensions not explicitly set but type ends with _array, treat as 1D
	if dims == 0 && strings.HasSuffix(string(typeStr), "_array") {
		dims = 1
	}

	// Only pack 1D arrays
	if dims != 1 {
		return typeStr
	}

	// Already a packed type - return as-is
	if strings.HasSuffix(string(typeStr), "_packed_array") {
		return typeStr
	}

	// Convert regular array to packed if a packed variant exists
	if strings.HasSuffix(string(typeStr), "_array") {
		baseType := strings.TrimSuffix(string(typeStr), "_array")
		packedType := IDL.MetaFFIType(baseType + "_packed_array")
		if _, found := IDL.TypeStringToTypeEnum[packedType]; found {
			return packedType
		}
	}

	// Handle IDL format where type is base type (e.g. "uint8") with dimensions=1
	// (as opposed to "uint8_array" with dimensions=0). Promote to packed if applicable.
	if !strings.Contains(string(typeStr), "_array") {
		packedType := IDL.MetaFFIType(string(typeStr) + "_packed_array")
		if _, found := IDL.TypeStringToTypeEnum[packedType]; found {
			return packedType
		}
	}

	return typeStr
}

// AutoPackedTypeStr is a template function that returns the (possibly packed)
// type string for an ArgDefinition. Used in guest template code generation.
func AutoPackedTypeStr(arg *IDL.ArgDefinition) string {
	return string(autoPackedType(arg))
}

// AutoPackedNumericType is a template function that returns the numeric MetaFFI
// type enum value for an ArgDefinition, using packed type when applicable.
func AutoPackedNumericType(arg *IDL.ArgDefinition) uint64 {
	return GetMetaFFINumericType(autoPackedType(arg))
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
	strParams := "nil"
	strRetVals := "nil"

	if funcDef.Parameters != nil && len(funcDef.Parameters) > 0 {
		strParams = "[]IDL.MetaFFITypeInfo{"
		for _, a := range funcDef.Parameters {
			packedT := autoPackedType(a)
			strParams += `IDL.MetaFFITypeInfo{StringType: IDL.` + IDL.TypeStringToEnumName[packedT] + `, Alias: "` + a.TypeAlias + `", Type: ` + fmt.Sprintf("%v", GetMetaFFINumericType(packedT)) + `, Dimensions: ` + fmt.Sprintf("%v", a.Dimensions) + ` },`
		}
		strParams += "}"
	}

	if funcDef.ReturnValues != nil && len(funcDef.ReturnValues) > 0 {
		strRetVals = "[]IDL.MetaFFITypeInfo{"
		for _, a := range funcDef.ReturnValues {
			packedT := autoPackedType(a)
			strRetVals += `IDL.MetaFFITypeInfo{StringType: IDL.` + IDL.TypeStringToEnumName[packedT] + `, Alias: "` + a.TypeAlias + `", Type: ` + fmt.Sprintf("%v", GetMetaFFINumericType(packedT)) + `, Dimensions: ` + fmt.Sprintf("%v", a.Dimensions) + ` },`
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
	// Caller always passes cdts[2]: [0]=params, [1]=retvals. get_cdt_element(xcall_params, i) returns pdata[i].arr.
	if len(retvals) > 0 {
		return 1 // retvals always at buffer index 1
	}
	panic("getCDTReturnValueIndex called with no retvals")
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

	// Strip array suffixes to get the base type.
	// Must strip "_packed_array" before "_array" since "_packed_array" ends with "_array".
	baseStr := string(def.Type)
	if strings.HasSuffix(baseStr, "_packed_array") {
		baseStr = strings.TrimSuffix(baseStr, "_packed_array")
	} else {
		baseStr = strings.ReplaceAll(baseStr, "_array", "")
	}
	t := IDL.MetaFFIType(baseStr)

	switch t {
	case IDL.STRING8:
		fallthrough
	case IDL.STRING16:
		fallthrough
	case IDL.STRING32:
		res = "string"
	case IDL.ANY:
		res = "interface{}"
	case IDL.NULL:
		res = "interface{}"
	case IDL.HANDLE:
		if def.IsTypeAlias() && mod.IsContainsClass(def.TypeAlias) {
			res = asPublic(def.TypeAlias)
		} else if def.IsTypeAlias() && isPrimitiveType(def.TypeAlias) {
			res = def.TypeAlias
		} else {
			res = "interface{}"
		}
	case IDL.CALLABLE:
		// Callable types: use the full type alias (e.g. "func(int64, int64) int64")
		// if available; otherwise fall back to interface{}.
		if def.IsTypeAlias() {
			res = def.TypeAlias
		} else {
			res = "interface{}"
		}
	default:
		res = string(t)
		if isGoKeywordType(res) {
			res = "interface{}"
		}
	}

	if def.Dimensions > 0 {
		for i := 0; i < def.Dimensions; i++ {
			res = "[]" + res
		}
	}

	if res == "" {
		res = "interface{}"
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

	if def.Type == IDL.HANDLE || def.Type == IDL.HANDLE_ARRAY || def.Type == IDL.CALLABLE {
		return varName
	}

	if def.IsTypeAlias() { // assert
		return fmt.Sprintf("%v(%v.(%v))", convertToGoType(def, mod), varName, def.TypeAlias)
	} else {
		return varName
	}
}

// cdtStorageGoType returns the Go type that FromCDTToGo stores a value as,
// based on the MetaFFI type (e.g., int64, string, float64, interface{}).
func cdtStorageGoType(metaffiType IDL.MetaFFIType) string {
	t := stripArraySuffix(metaffiType)
	switch t {
	case IDL.STRING8, IDL.STRING16, IDL.STRING32:
		return "string"
	case IDL.BOOL:
		return "bool"
	case IDL.FLOAT32:
		return "float32"
	case IDL.FLOAT64:
		return "float64"
	case IDL.HANDLE, IDL.ANY:
		return "interface{}"
	default:
		return string(t) // int8, int16, int32, int64, uint8, etc.
	}
}

func convertEmptyInterfaceFromCDTSToCorrectType(elem *IDL.ArgDefinition, mod *IDL.ModuleDefinition, outputVarExists bool) string {

	if elem.IsAny() {
		return fmt.Sprintf("%v := %vAsInterface", elem.Name, elem.Name)
	}

	if elem.Dimensions > 0 {
		// For arrays, prefer TypeAlias which has the correct Go type with [] prefixes
		// (e.g., [][][]int instead of [][][]int64, []*SomeClass instead of []interface{})
		typ := elem.TypeAlias
		if typ == "" {
			typ = convertToGoType(elem, mod)
		}
		if typ == "" {
			typ = "interface{}"
		}
		return fmt.Sprintf("%v := %vAsInterface.(%v)\n", elem.Name, elem.Name, typ)
	}

	// Non-array scalar types
	baseType := stripArraySuffix(elem.Type)

	if baseType == IDL.HANDLE {
		// HANDLE: CDT stores the original Go value as interface{}.
		// Assert directly to the target Go type (TypeAlias).
		targetType := elem.TypeAlias
		if targetType == "" || isGoKeywordType(targetType) {
			targetType = "interface{}"
		}
		return fmt.Sprintf("%v := %vAsInterface.(%v)", elem.Name, elem.Name, targetType)
	}

	if baseType == IDL.CALLABLE {
		// CALLABLE: TraverseCDT returns a Go func of the correct type.
		// Assert directly to the function type alias (e.g., func(string) int).
		targetType := elem.TypeAlias
		if targetType == "" {
			targetType = "interface{}"
		}
		return fmt.Sprintf("%v := %vAsInterface.(%v)", elem.Name, elem.Name, targetType)
	}

	// Primitive types: CDT stores as the MetaFFI Go type (e.g., int64).
	// The Go function may expect a different type (e.g., int from TypeAlias).
	cdtType := cdtStorageGoType(baseType)
	targetType := safeTypeForAssertion(elem, mod)

	if cdtType == targetType {
		// Same type, just assert
		return fmt.Sprintf("%v := %vAsInterface.(%v)", elem.Name, elem.Name, cdtType)
	}

	// Different types: assert to CDT type, then convert to target
	// e.g., value := int(valueAsInterface.(int64))
	return fmt.Sprintf("%v := %v(%vAsInterface.(%v))", elem.Name, targetType, elem.Name, cdtType)
}

// ============================================================
// Typed CDT template helpers (Direct* serialization)
// ============================================================

// isDirectlySerializable returns true if the arg can be handled by Direct*
// CDT functions (scalars and 1D packed arrays of primitive types).
func isDirectlySerializable(elem *IDL.ArgDefinition) bool {
	effectiveType := autoPackedType(elem)
	typeStr := string(effectiveType)

	// Packed array → supported if base type has Direct* function
	// AND the Go element type matches the CDT element type.
	if strings.HasSuffix(typeStr, "_packed_array") {
		base := strings.TrimSuffix(typeStr, "_packed_array")
		if typeToGoFuncCase(base) == "" {
			return false
		}
		// Verify TypeAlias compatibility to avoid []int vs []int64 mismatches
		if elem.TypeAlias != "" {
			aliasElem := strings.TrimPrefix(elem.TypeAlias, "[]")
			cdtGoType := cdtStorageGoType(IDL.MetaFFIType(base))
			if normalizeGoAlias(aliasElem) != normalizeGoAlias(cdtGoType) {
				return false
			}
		}
		return true
	}

	// Regular (non-packed) array that wasn't promoted → fallback
	if strings.HasSuffix(typeStr, "_array") {
		return false
	}

	// Multi-dim arrays → fallback
	if elem.Dimensions > 1 {
		return false
	}

	// Scalar primitive (dims == 0 at this point since autoPackedType would have
	// promoted dims == 1 to packed if the base type supports it)
	return typeToGoFuncCase(typeStr) != ""
}

// isDirectlySerializableScalarOnly is like isDirectlySerializable but only
// returns true for scalar primitives, never for packed arrays.
// Use for PARAMETERS in the guest template: the caller (host) determines the
// wire format for arrays, which may be per-element CDT arrays instead of packed.
func isDirectlySerializableScalarOnly(elem *IDL.ArgDefinition) bool {
	effectiveType := autoPackedType(elem)
	typeStr := string(effectiveType)

	// Any array (packed or not) → fallback
	if strings.Contains(typeStr, "_array") {
		return false
	}
	if elem.Dimensions >= 1 {
		return false
	}

	return typeToGoFuncCase(typeStr) != ""
}

// normalizeGoAlias resolves Go's built-in type aliases for comparison.
func normalizeGoAlias(t string) string {
	switch t {
	case "byte":
		return "uint8"
	case "rune":
		return "int32"
	default:
		return t
	}
}

// typeToGoFuncCase maps a MetaFFI base type string to the Go-cased suffix
// used in Direct*CDT function names (e.g. "int32" → "Int32").
// Returns "" for unsupported types.
func typeToGoFuncCase(metaffiBase string) string {
	switch IDL.MetaFFIType(metaffiBase) {
	case IDL.INT8:
		return "Int8"
	case IDL.INT16:
		return "Int16"
	case IDL.INT32:
		return "Int32"
	case IDL.INT64:
		return "Int64"
	case IDL.UINT8:
		return "Uint8"
	case IDL.UINT16:
		return "Uint16"
	case IDL.UINT32:
		return "Uint32"
	case IDL.UINT64:
		return "Uint64"
	case IDL.FLOAT32:
		return "Float32"
	case IDL.FLOAT64:
		return "Float64"
	case IDL.BOOL:
		return "Bool"
	case IDL.STRING8:
		return "String8"
	default:
		return ""
	}
}

// directCDTFuncSuffix returns the suffix for Direct*CDT function names
// (e.g. "Int32" for scalars, "Uint8PackedSlice" for packed arrays).
func directCDTFuncSuffix(effectiveType IDL.MetaFFIType) string {
	typeStr := string(effectiveType)
	if strings.HasSuffix(typeStr, "_packed_array") {
		base := strings.TrimSuffix(typeStr, "_packed_array")
		return typeToGoFuncCase(base) + "PackedSlice"
	}
	return typeToGoFuncCase(typeStr)
}

// directCDTGoReturnType returns the Go type that DirectGetCDT* returns.
func directCDTGoReturnType(effectiveType IDL.MetaFFIType) string {
	typeStr := string(effectiveType)
	if strings.HasSuffix(typeStr, "_packed_array") {
		base := strings.TrimSuffix(typeStr, "_packed_array")
		return "[]" + cdtStorageGoType(IDL.MetaFFIType(base))
	}
	return cdtStorageGoType(effectiveType)
}

// directExtractExpr generates a Go expression that extracts a parameter from
// the CDT array using a Direct* function. Includes type conversion if needed.
// Example outputs:
//
//	DirectGetCDTUint8PackedSlice(unsafe.Pointer(parameters_CDTS), 0)
//	int(DirectGetCDTInt64(unsafe.Pointer(parameters_CDTS), 1))
func directExtractExpr(elem *IDL.ArgDefinition, mod *IDL.ModuleDefinition, index int, cdtsVarName string) string {
	effectiveType := autoPackedType(elem)
	suffix := directCDTFuncSuffix(effectiveType)
	expr := fmt.Sprintf("DirectGetCDT%s(unsafe.Pointer(%s), %d)", suffix, cdtsVarName, index)

	// Packed arrays return the correct slice type – no conversion needed.
	if strings.HasSuffix(string(effectiveType), "_packed_array") {
		return expr
	}

	// Scalar: check if the CDT return type differs from the target Go type.
	cdtRet := directCDTGoReturnType(effectiveType)
	target := safeTypeForAssertion(elem, mod)
	if target != "interface{}" && cdtRet != target {
		return fmt.Sprintf("%s(%s)", target, expr)
	}
	return expr
}

// directSetStmt generates a Go statement that writes a return value to the
// CDT array using a Direct* function. Includes type conversion if needed.
// Example outputs:
//
//	DirectSetCDTUint8PackedSlice(unsafe.Pointer(retvals_CDTS), 0, data)
//	DirectSetCDTInt64(unsafe.Pointer(retvals_CDTS), 0, int64(result))
func directSetStmt(elem *IDL.ArgDefinition, mod *IDL.ModuleDefinition, index int, cdtsVarName string, varName string) string {
	effectiveType := autoPackedType(elem)
	suffix := directCDTFuncSuffix(effectiveType)

	// Packed arrays – pass the slice directly.
	if strings.HasSuffix(string(effectiveType), "_packed_array") {
		return fmt.Sprintf("DirectSetCDT%s(unsafe.Pointer(%s), %d, %s)", suffix, cdtsVarName, index, varName)
	}

	// Scalar: check if the Go variable type differs from what DirectSet expects.
	cdtParam := directCDTGoReturnType(effectiveType)
	target := safeTypeForAssertion(elem, mod)
	if target != "interface{}" && cdtParam != target {
		return fmt.Sprintf("DirectSetCDT%s(unsafe.Pointer(%s), %d, %s(%s))", suffix, cdtsVarName, index, cdtParam, varName)
	}
	return fmt.Sprintf("DirectSetCDT%s(unsafe.Pointer(%s), %d, %s)", suffix, cdtsVarName, index, varName)
}

// convertGlobalSetterExpression returns a Go expression that converts _globalSetValAsInterface
// to the correct Go type for assignment to a global variable.
// This is separate from convertEmptyInterfaceFromCDTSToCorrectType because for global setters,
// the parameter name matches the global name, which would cause a shadowing issue with :=.
func convertGlobalSetterExpression(elem *IDL.ArgDefinition, mod *IDL.ModuleDefinition) string {
	varName := "_globalSetValAsInterface"

	if elem.IsAny() {
		return varName
	}

	if elem.Dimensions > 0 {
		typ := elem.TypeAlias
		if typ == "" {
			typ = convertToGoType(elem, mod)
		}
		if typ == "" {
			typ = "interface{}"
		}
		return fmt.Sprintf("%v.(%v)", varName, typ)
	}

	baseType := stripArraySuffix(elem.Type)

	if baseType == IDL.HANDLE {
		targetType := elem.TypeAlias
		if targetType == "" || isGoKeywordType(targetType) {
			targetType = "interface{}"
		}
		return fmt.Sprintf("%v.(%v)", varName, targetType)
	}

	if baseType == IDL.CALLABLE {
		targetType := elem.TypeAlias
		if targetType == "" {
			targetType = "interface{}"
		}
		return fmt.Sprintf("%v.(%v)", varName, targetType)
	}

	cdtType := cdtStorageGoType(baseType)
	targetType := safeTypeForAssertion(elem, mod)

	if cdtType == targetType {
		return fmt.Sprintf("%v.(%v)", varName, cdtType)
	}

	return fmt.Sprintf("%v(%v.(%v))", targetType, varName, cdtType)
}
