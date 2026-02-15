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
	handlesToObjects map[uint64]interface{}
	nextHandleID     uint64
	lock             sync.RWMutex
)

func init() {
	handlesToObjects = make(map[uint64]interface{})
}

func handleToKey(h Handle) uint64 {
	return uint64(uintptr(unsafe.Pointer(C.metaffi_handle(h))))
}

// sets the object and returns a handle
// if object already set, it returns the existing handle
func SetObject(obj interface{}) Handle {

	lock.Lock()
	defer lock.Unlock()

	nextHandleID++
	handleID := nextHandleID
	handlesToObjects[handleID] = obj

	return Handle(C.int_to_handle(C.ulonglong(handleID)))
}

func GetObject(h Handle) interface{} {

	lock.RLock()
	defer lock.RUnlock()

	if o, found := handlesToObjects[handleToKey(h)]; found {

		return o
	} else {
		return nil
	}

}

func ReleaseObject(h Handle) error {
	lock.Lock()
	defer lock.Unlock()

	key := handleToKey(h)
	_, found := handlesToObjects[key]
	if !found {
		return fmt.Errorf("Given handle (%v) is not found in MetaFFI Go's object table", h)
	}

	delete(handlesToObjects, key)

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
