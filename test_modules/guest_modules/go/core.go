package guest

import (
	"errors"
	"strings"
)

func HelloWorld() string {
	return "Hello World, from Go"
}

func ReturnsAnError() error {
	return errors.New("Error")
}

func DivIntegers(x int64, y int64) float64 {
	return float64(x) / float64(y)
}

func JoinStrings(arr []string) string {
	return strings.Join(arr, ",")
}

func WaitABit(ms int64) {
	_ = ms
}

func NoOp() {}

func ReturnNull() any {
	return nil
}

func CallCallbackAdd(add func(int64, int64) int64) (int64, error) {
	res := add(1, 2)
	if res != 3 {
		return res, errors.New("expected 3")
	}
	return res, nil
}

func ReturnCallbackAdd() func(int64, int64) int64 {
	return func(a int64, b int64) int64 {
		return a + b
	}
}

func ReturnMultipleReturnValues() (int64, string, float64, any, []byte, *SomeClass) {
	return 1, "string", 3.0, nil, []byte{1, 2, 3}, &SomeClass{Name: "some"}
}

func ReturnsArrayWithDifferentDimensions() []any {
	return []any{[]int{1, 2, 3}, 4, [][]int{{5, 6}, {7, 8}}}
}

func ReturnsArrayOfDifferentObjects() []any {
	return []any{int64(1), "string", 3.0, nil, []byte{1, 2, 3}, &SomeClass{Name: "some"}}
}

func ReturnAny(which int) any {
	switch which {
	case 0:
		return int64(1)
	case 1:
		return "string"
	case 2:
		return 3.0
	case 3:
		return []string{"list", "of", "strings"}
	case 4:
		return &SomeClass{Name: "some"}
	default:
		return nil
	}
}

func AcceptsAny(which int, val any) error {
	switch which {
	case 0:
		if _, ok := val.(int64); !ok {
			return errors.New("expected int64")
		}
	case 1:
		if _, ok := val.(string); !ok {
			return errors.New("expected string")
		}
	case 2:
		if _, ok := val.(float64); !ok {
			return errors.New("expected float64")
		}
	case 3:
		if val != nil {
			return errors.New("expected nil")
		}
	case 4:
		if _, ok := val.([]byte); !ok {
			return errors.New("expected []byte")
		}
	case 5:
		if _, ok := val.(*SomeClass); !ok {
			return errors.New("expected *SomeClass")
		}
	default:
		return errors.New("unsupported type")
	}
	return nil
}
