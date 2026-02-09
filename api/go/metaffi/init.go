package metaffi

// #include <include/xllr_capi_loader.h>
import "C"

func init() {
	err := C.load_xllr()
	if err != nil {
		panic("Failed to load MetaFFI XLLR functions: " + C.GoString(err))
	}
}
