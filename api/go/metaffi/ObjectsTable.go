package metaffi

/*
typedef void* metaffi_handle;

metaffi_handle int_to_handle(unsigned long long i)
{
	return (metaffi_handle)i;
}

*/
import "C"
import (
	"fmt"
	"os"
	"sync"
	"unsafe"
)

const GO_RUNTIME_ID = 3958232544

type Handle C.metaffi_handle
type MetaFFIHandle struct {
	Val       Handle
	RuntimeID uint64
	Releaser  func() error
	CReleaser unsafe.Pointer
}

var (
	handlesToObjects map[C.metaffi_handle]interface{}
	lock             sync.RWMutex
)

func init() {
	handlesToObjects = make(map[C.metaffi_handle]interface{})
}

// sets the object and returns a handle
// if object already set, it returns the existing handle
func SetObject(obj interface{}) Handle {

	lock.Lock()
	defer lock.Unlock()

	handleID := C.int_to_handle(C.ulonglong(len(handlesToObjects) + 1))
	handlesToObjects[handleID] = obj

	return Handle(handleID)
}

func GetObject(h Handle) interface{} {

	lock.RLock()
	defer lock.RUnlock()

	if o, found := handlesToObjects[C.metaffi_handle(h)]; found {

		return o
	} else {
		return nil
	}

}

func ReleaseObject(h Handle) error {
	lock.Lock()
	defer lock.Unlock()

	_, found := handlesToObjects[C.metaffi_handle(h)]
	if !found {
		return fmt.Errorf("Given handle (%v) is not found in MetaFFI Go's object table", h)
	}

	delete(handlesToObjects, C.metaffi_handle(h))

	return nil
}

//export Releaser
func Releaser(h C.metaffi_handle) {
	err := ReleaseObject(Handle(h))

	// print error to stderr
	if err != nil {
		fmt.Fprintf(os.Stderr, "Failed to release Go object: %v\n", err)
	}
}
