package guest

type PrimitiveValues struct {
	BoolVal    bool
	ByteVal    byte
	Int8Val    int8
	Int16Val   int16
	Int32Val   int32
	Int64Val   int64
	UintVal    uint
	Uint8Val   uint8
	Uint16Val  uint16
	Uint32Val  uint32
	Uint64Val  uint64
	Float32Val float32
	Float64Val float64
	Complex64  complex64
	Complex128 complex128
	RuneVal    rune
	StringVal  string
}

func AcceptsPrimitives(
	boolVal bool,
	byteVal byte,
	int8Val int8,
	int16Val int16,
	int32Val int32,
	int64Val int64,
	uintVal uint,
	uint8Val uint8,
	uint16Val uint16,
	uint32Val uint32,
	uint64Val uint64,
	float32Val float32,
	float64Val float64,
	complex64Val complex64,
	complex128Val complex128,
	runeVal rune,
	stringVal string,
) PrimitiveValues {
	return PrimitiveValues{
		BoolVal:    boolVal,
		ByteVal:    byteVal,
		Int8Val:    int8Val,
		Int16Val:   int16Val,
		Int32Val:   int32Val,
		Int64Val:   int64Val,
		UintVal:    uintVal,
		Uint8Val:   uint8Val,
		Uint16Val:  uint16Val,
		Uint32Val:  uint32Val,
		Uint64Val:  uint64Val,
		Float32Val: float32Val,
		Float64Val: float64Val,
		Complex64:  complex64Val,
		Complex128: complex128Val,
		RuneVal:    runeVal,
		StringVal:  stringVal,
	}
}

func EchoBytes(data []byte) []byte {
	return data
}

func ToUpperRune(r rune) rune {
	if r >= 'a' && r <= 'z' {
		return r - 32
	}
	return r
}
