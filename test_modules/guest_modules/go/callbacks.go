package guest

import "fmt"

type StringTransformer func(string) string

func CallTransformer(transformer StringTransformer, value string) string {
	return transformer(value)
}

func ReturnTransformer(suffix string) StringTransformer {
	return func(value string) string {
		return value + suffix
	}
}

func CallFunction(function func(string) int, value string) int {
	return function(value)
}

func CallTransformerWithError(cb func(string) (string, error), value string) (string, error) {
	return cb(value)
}

func CallTransformerRecover(cb func(string) string, value string) (res string, err error) {
	defer func() {
		if r := recover(); r != nil {
			err = fmt.Errorf("callback panic: %v", r)
		}
	}()
	return cb(value), nil
}
