package guest

type Box[T any] struct {
	Value T
}

func NewBox[T any](value T) Box[T] {
	return Box[T]{Value: value}
}

func (b *Box[T]) Get() T {
	return b.Value
}

func (b *Box[T]) Set(value T) {
	b.Value = value
}

func NewIntBox(value int) Box[int] {
	return NewBox(value)
}

func NewStringBox(value string) Box[string] {
	return NewBox(value)
}
