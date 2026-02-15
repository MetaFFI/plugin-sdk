package api

// #include <stdlib.h>
import "C"
import (
	"fmt"
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
			xcall_params, _, return_valuesCDTS := goruntime.XLLRAllocCDTSBuffer(0, uint64(retvalCount))

			err := goruntime.XLLRXCallNoParamsRet(pff, goruntime.XLLRGetCDTS(xcall_params, 1))
			if err != nil {
				return nil, err
			}

			retvals := make([]interface{}, retvalCount)
			for i := 0; i < retvalCount; i++ {
				retvals[i] = goruntime.FromCDTToGo(return_valuesCDTS, i, nil)
			}

			if goruntime.GetCacheSize() < retvalCount {
				C.free(xcall_params)
			}

			return retvals, nil
		}
	} else if paramsCount > 0 && retvalCount == 0 {
		// With params, no return values.
		ff = func(params ...interface{}) error {
			if len(params) != paramsCount {
				return fmt.Errorf("Expecting %v parameters, received %v parameters", paramsCount, len(params))
			}

			xcall_params, parametersCDTS, _ := goruntime.XLLRAllocCDTSBuffer(uint64(paramsCount), 0)

			for i, p := range params {
				goruntime.FromGoToCDT(p, parametersCDTS, paramsMetaFFITypes[i], i)
			}

			err := goruntime.XLLRXCallParamsNoRet(pff, goruntime.XLLRGetCDTS(xcall_params, 0))

			if goruntime.GetCacheSize() < paramsCount {
				C.free(xcall_params)
			}

			return err
		}
	} else {
		// With params, with return values (general case).
		ff = func(params ...interface{}) ([]interface{}, error) {
			if len(params) != paramsCount {
				return nil, fmt.Errorf("Expecting %v parameters, received %v parameters", paramsCount, len(params))
			}

			xcall_params, parametersCDTS, return_valuesCDTS := goruntime.XLLRAllocCDTSBuffer(uint64(paramsCount), uint64(retvalCount))

			for i, p := range params {
				goruntime.FromGoToCDT(p, parametersCDTS, paramsMetaFFITypes[i], i)
			}

			err := goruntime.XLLRXCallParamsRet(pff, xcall_params)
			if err != nil {
				return nil, err
			}

			retvals := make([]interface{}, retvalCount)
			for i := 0; i < retvalCount; i++ {
				retvals[i] = goruntime.FromCDTToGo(return_valuesCDTS, i, nil)
			}

			if goruntime.GetCacheSize() < paramsCount+retvalCount {
				C.free(xcall_params)
			}

			return retvals, nil
		}
	}

	return
}
