package guest

import "errors"

func ReturnErrorTuple(ok bool) (bool, error) {
	if ok {
		return true, nil
	}
	return false, errors.New("error")
}

func Panics() {
	panic("panic")
}
