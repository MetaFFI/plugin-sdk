package api

import (
	"fmt"
	"runtime"
	"unsafe"

	goruntime "github.com/MetaFFI/sdk/api/go/metaffi"
	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

type MetaFFIModule struct {
	runtime    *MetaFFIRuntime
	modulePath string
}

func (this *MetaFFIModule) Load(entityPath string, paramsMetaFFITypes []IDL.MetaFFIType, retvalMetaFFITypes []IDL.MetaFFIType) (ff interface{}, err error) {
	var params []IDL.MetaFFITypeInfo
	if paramsMetaFFITypes != nil {
		params = make([]IDL.MetaFFITypeInfo, 0)
		for _, p := range paramsMetaFFITypes {
			params = append(params, IDL.MetaFFITypeInfo{StringType: p})
		}
	}

	var retvals []IDL.MetaFFITypeInfo
	if retvalMetaFFITypes != nil {
		retvals = make([]IDL.MetaFFITypeInfo, 0)
		for _, r := range retvalMetaFFITypes {
			retvals = append(retvals, IDL.MetaFFITypeInfo{StringType: r})
		}
	}

	return this.LoadWithInfo(entityPath, params, retvals)
}

// directSetCDTParam tries to set a parameter using Direct* typed functions.
// Returns true if handled, false to fall back to FromGoToCDT.
func directSetCDTParam(cdtsArr unsafe.Pointer, index int, val interface{}, typeInfo IDL.MetaFFITypeInfo) bool {
	switch typeInfo.StringType {
	// --- Packed arrays (main performance target) ---
	case IDL.INT8_PACKED_ARRAY:
		goruntime.DirectSetCDTInt8PackedSlice(cdtsArr, index, val.([]int8))
	case IDL.INT16_PACKED_ARRAY:
		goruntime.DirectSetCDTInt16PackedSlice(cdtsArr, index, val.([]int16))
	case IDL.INT32_PACKED_ARRAY:
		goruntime.DirectSetCDTInt32PackedSlice(cdtsArr, index, val.([]int32))
	case IDL.INT64_PACKED_ARRAY:
		goruntime.DirectSetCDTInt64PackedSlice(cdtsArr, index, val.([]int64))
	case IDL.UINT8_PACKED_ARRAY:
		goruntime.DirectSetCDTUint8PackedSlice(cdtsArr, index, val.([]uint8))
	case IDL.UINT16_PACKED_ARRAY:
		goruntime.DirectSetCDTUint16PackedSlice(cdtsArr, index, val.([]uint16))
	case IDL.UINT32_PACKED_ARRAY:
		goruntime.DirectSetCDTUint32PackedSlice(cdtsArr, index, val.([]uint32))
	case IDL.UINT64_PACKED_ARRAY:
		goruntime.DirectSetCDTUint64PackedSlice(cdtsArr, index, val.([]uint64))
	case IDL.FLOAT32_PACKED_ARRAY:
		goruntime.DirectSetCDTFloat32PackedSlice(cdtsArr, index, val.([]float32))
	case IDL.FLOAT64_PACKED_ARRAY:
		goruntime.DirectSetCDTFloat64PackedSlice(cdtsArr, index, val.([]float64))
	case IDL.BOOL_PACKED_ARRAY:
		goruntime.DirectSetCDTBoolPackedSlice(cdtsArr, index, val.([]bool))
	case IDL.STRING8_PACKED_ARRAY:
		goruntime.DirectSetCDTString8PackedSlice(cdtsArr, index, val.([]string))
	// --- Scalars (non-ambiguous types) ---
	case IDL.FLOAT32:
		goruntime.DirectSetCDTFloat32(cdtsArr, index, val.(float32))
	case IDL.FLOAT64:
		goruntime.DirectSetCDTFloat64(cdtsArr, index, val.(float64))
	case IDL.BOOL:
		goruntime.DirectSetCDTBool(cdtsArr, index, val.(bool))
	case IDL.STRING8:
		goruntime.DirectSetCDTString8(cdtsArr, index, val.(string))
	// --- Integer scalars (handle int alias) ---
	case IDL.INT8:
		goruntime.DirectSetCDTInt8(cdtsArr, index, val.(int8))
	case IDL.INT16:
		goruntime.DirectSetCDTInt16(cdtsArr, index, val.(int16))
	case IDL.INT32:
		if v, ok := val.(int32); ok {
			goruntime.DirectSetCDTInt32(cdtsArr, index, v)
		} else {
			return false
		}
	case IDL.INT64:
		if v, ok := val.(int64); ok {
			goruntime.DirectSetCDTInt64(cdtsArr, index, v)
		} else if v, ok := val.(int); ok {
			goruntime.DirectSetCDTInt64(cdtsArr, index, int64(v))
		} else {
			return false
		}
	case IDL.UINT8:
		goruntime.DirectSetCDTUint8(cdtsArr, index, val.(uint8))
	case IDL.UINT16:
		goruntime.DirectSetCDTUint16(cdtsArr, index, val.(uint16))
	case IDL.UINT32:
		goruntime.DirectSetCDTUint32(cdtsArr, index, val.(uint32))
	case IDL.UINT64:
		goruntime.DirectSetCDTUint64(cdtsArr, index, val.(uint64))
	default:
		return false
	}
	return true
}

// directGetCDTRetval tries to extract a return value using Direct* typed functions.
// Returns (value, true) if handled, (nil, false) to fall back to FromCDTToGo.
func directGetCDTRetval(cdtsArr unsafe.Pointer, index int, typeInfo IDL.MetaFFITypeInfo) (interface{}, bool) {
	switch typeInfo.StringType {
	// --- Packed arrays ---
	case IDL.INT8_PACKED_ARRAY:
		return goruntime.DirectGetCDTInt8PackedSlice(cdtsArr, index), true
	case IDL.INT16_PACKED_ARRAY:
		return goruntime.DirectGetCDTInt16PackedSlice(cdtsArr, index), true
	case IDL.INT32_PACKED_ARRAY:
		return goruntime.DirectGetCDTInt32PackedSlice(cdtsArr, index), true
	case IDL.INT64_PACKED_ARRAY:
		return goruntime.DirectGetCDTInt64PackedSlice(cdtsArr, index), true
	case IDL.UINT8_PACKED_ARRAY:
		return goruntime.DirectGetCDTUint8PackedSlice(cdtsArr, index), true
	case IDL.UINT16_PACKED_ARRAY:
		return goruntime.DirectGetCDTUint16PackedSlice(cdtsArr, index), true
	case IDL.UINT32_PACKED_ARRAY:
		return goruntime.DirectGetCDTUint32PackedSlice(cdtsArr, index), true
	case IDL.UINT64_PACKED_ARRAY:
		return goruntime.DirectGetCDTUint64PackedSlice(cdtsArr, index), true
	case IDL.FLOAT32_PACKED_ARRAY:
		return goruntime.DirectGetCDTFloat32PackedSlice(cdtsArr, index), true
	case IDL.FLOAT64_PACKED_ARRAY:
		return goruntime.DirectGetCDTFloat64PackedSlice(cdtsArr, index), true
	case IDL.BOOL_PACKED_ARRAY:
		return goruntime.DirectGetCDTBoolPackedSlice(cdtsArr, index), true
	case IDL.STRING8_PACKED_ARRAY:
		return goruntime.DirectGetCDTString8PackedSlice(cdtsArr, index), true
	// --- Scalars ---
	case IDL.INT8:
		return goruntime.DirectGetCDTInt8(cdtsArr, index), true
	case IDL.INT16:
		return goruntime.DirectGetCDTInt16(cdtsArr, index), true
	case IDL.INT32:
		return goruntime.DirectGetCDTInt32(cdtsArr, index), true
	case IDL.INT64:
		return goruntime.DirectGetCDTInt64(cdtsArr, index), true
	case IDL.UINT8:
		return goruntime.DirectGetCDTUint8(cdtsArr, index), true
	case IDL.UINT16:
		return goruntime.DirectGetCDTUint16(cdtsArr, index), true
	case IDL.UINT32:
		return goruntime.DirectGetCDTUint32(cdtsArr, index), true
	case IDL.UINT64:
		return goruntime.DirectGetCDTUint64(cdtsArr, index), true
	case IDL.FLOAT32:
		return goruntime.DirectGetCDTFloat32(cdtsArr, index), true
	case IDL.FLOAT64:
		return goruntime.DirectGetCDTFloat64(cdtsArr, index), true
	case IDL.BOOL:
		return goruntime.DirectGetCDTBool(cdtsArr, index), true
	case IDL.STRING8:
		return goruntime.DirectGetCDTString8(cdtsArr, index), true
	default:
		return nil, false
	}
}

// LoadWithInfo loads a foreign entity and returns a callable function.
// The returned interface{} is one of 4 specialized function types based on the signature:
//   - func() error                                        (no params, no retvals)
//   - func() ([]interface{}, error)                       (no params, with retvals)
//   - func(...interface{}) error                           (with params, no retvals)
//   - func(...interface{}) ([]interface{}, error)          (with params, with retvals)
func (this *MetaFFIModule) LoadWithInfo(entityPath string, paramsMetaFFITypes []IDL.MetaFFITypeInfo, retvalMetaFFITypes []IDL.MetaFFITypeInfo) (ff interface{}, err error) {

	// convert Go's String metaffi types to INT metaffi types
	if paramsMetaFFITypes != nil {
		for i := 0; i < len(paramsMetaFFITypes); i++ {
			item := paramsMetaFFITypes[i]
			item.FillMetaFFITypeFromStringMetaFFIType()
			paramsMetaFFITypes[i] = item
		}
	}

	if retvalMetaFFITypes != nil {
		for i := 0; i < len(retvalMetaFFITypes); i++ {
			item := retvalMetaFFITypes[i]
			item.FillMetaFFITypeFromStringMetaFFIType()
			retvalMetaFFITypes[i] = item
		}
	}

	var pff unsafe.Pointer
	pff, err = goruntime.XLLRLoadEntityWithAliases(this.runtime.runtimePlugin, this.modulePath, entityPath, paramsMetaFFITypes, retvalMetaFFITypes)

	if err != nil { // failed
		return
	}

	paramsCount := len(paramsMetaFFITypes)
	retvalCount := len(retvalMetaFFITypes)

	if paramsCount == 0 && retvalCount == 0 {
		// Fast path: no params, no return values — skip CDT buffer allocation entirely.
		ff = func() error {
			return goruntime.XLLRXCallNoParamsNoRet(pff)
		}
	} else if paramsCount == 0 && retvalCount > 0 {
		// No params, with return values.
		ff = func() ([]interface{}, error) {
			runtime.LockOSThread()
			defer runtime.UnlockOSThread()

			xcall_params, _, return_valuesCDTS := goruntime.XLLRAllocCDTSBuffer(0, uint64(retvalCount))

			err := goruntime.XLLRXCallNoParamsRet(pff, xcall_params)
			if err != nil {
				goruntime.XLLRClearAndFreeCDTSBuffer(xcall_params, 0)
				return nil, err
			}

			retvals := make([]interface{}, retvalCount)
			for i := 0; i < retvalCount; i++ {
				if v, ok := directGetCDTRetval(return_valuesCDTS, i, retvalMetaFFITypes[i]); ok {
					retvals[i] = v
				} else {
					retvals[i] = goruntime.FromCDTToGo(return_valuesCDTS, i, nil)
				}
			}

			goruntime.XLLRClearAndFreeCDTSBuffer(xcall_params, 0)

			return retvals, nil
		}
	} else if paramsCount > 0 && retvalCount == 0 {
		// With params, no return values.
		ff = func(params ...interface{}) error {
			runtime.LockOSThread()
			defer runtime.UnlockOSThread()

			if len(params) != paramsCount {
				return fmt.Errorf("Expecting %v parameters, received %v parameters", paramsCount, len(params))
			}

			xcall_params, parametersCDTS, _ := goruntime.XLLRAllocCDTSBuffer(uint64(paramsCount), 0)

			for i, p := range params {
				if !directSetCDTParam(parametersCDTS, i, p, paramsMetaFFITypes[i]) {
					goruntime.FromGoToCDT(p, parametersCDTS, paramsMetaFFITypes[i], i)
				}
			}

			err := goruntime.XLLRXCallParamsNoRet(pff, xcall_params)

			goruntime.XLLRClearAndFreeCDTSBuffer(xcall_params, paramsCount)

			return err
		}
	} else {
		// With params, with return values (general case).
		ff = func(params ...interface{}) ([]interface{}, error) {
			runtime.LockOSThread()
			defer runtime.UnlockOSThread()

			if len(params) != paramsCount {
				return nil, fmt.Errorf("Expecting %v parameters, received %v parameters", paramsCount, len(params))
			}

			xcall_params, parametersCDTS, return_valuesCDTS := goruntime.XLLRAllocCDTSBuffer(uint64(paramsCount), uint64(retvalCount))

			for i, p := range params {
				if !directSetCDTParam(parametersCDTS, i, p, paramsMetaFFITypes[i]) {
					goruntime.FromGoToCDT(p, parametersCDTS, paramsMetaFFITypes[i], i)
				}
			}

			err := goruntime.XLLRXCallParamsRet(pff, xcall_params)
			if err != nil {
				goruntime.XLLRClearAndFreeCDTSBuffer(xcall_params, paramsCount)
				return nil, err
			}

			retvals := make([]interface{}, retvalCount)
			for i := 0; i < retvalCount; i++ {
				if v, ok := directGetCDTRetval(return_valuesCDTS, i, retvalMetaFFITypes[i]); ok {
					retvals[i] = v
				} else {
					retvals[i] = goruntime.FromCDTToGo(return_valuesCDTS, i, nil)
				}
			}

			goruntime.XLLRClearAndFreeCDTSBuffer(xcall_params, paramsCount)

			return retvals, nil
		}
	}

	return
}
