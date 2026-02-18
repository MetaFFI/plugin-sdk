package guest

import (
	"errors"
	"reflect"
	"strings"
	"testing"

	metaffisub "metaffi_guest_go/sub"
)

func TestCoreFunctions(t *testing.T) {
	if !strings.Contains(HelloWorld(), "Hello") {
		t.Fatalf("HelloWorld")
	}
	if err := ReturnsAnError(); err == nil {
		t.Fatalf("ReturnsAnError")
	}
	if DivIntegers(10, 2) != 5.0 {
		t.Fatalf("DivIntegers")
	}
	if JoinStrings([]string{"a", "b", "c"}) != "a,b,c" {
		t.Fatalf("JoinStrings")
	}
	WaitABit(1)
	if ReturnNull() != nil {
		t.Fatalf("ReturnNull")
	}
	if res, err := CallCallbackAdd(func(a int64, b int64) int64 { return a + b }); err != nil || res != 3 {
		t.Fatalf("CallCallbackAdd")
	}
	if ReturnCallbackAdd()(3, 4) != 7 {
		t.Fatalf("ReturnCallbackAdd")
	}
	_, s, f, anyVal, b, sc := ReturnMultipleReturnValues()
	if s != "string" || f != 3.0 || anyVal != nil || len(b) != 3 || sc == nil {
		t.Fatalf("ReturnMultipleReturnValues")
	}
	dims := ReturnsArrayWithDifferentDimensions()
	if _, ok := dims[0].([]int); !ok {
		t.Fatalf("ReturnsArrayWithDifferentDimensions 1d")
	}
	if _, ok := dims[2].([][]int); !ok {
		t.Fatalf("ReturnsArrayWithDifferentDimensions 2d")
	}
	diff := ReturnsArrayOfDifferentObjects()
	if len(diff) != 6 {
		t.Fatalf("ReturnsArrayOfDifferentObjects length")
	}
	if err := AcceptsAny(0, int64(1)); err != nil {
		t.Fatalf("AcceptsAny int64")
	}
	if err := AcceptsAny(1, "string"); err != nil {
		t.Fatalf("AcceptsAny string")
	}
	if err := AcceptsAny(2, 3.0); err != nil {
		t.Fatalf("AcceptsAny float64")
	}
	if err := AcceptsAny(3, nil); err != nil {
		t.Fatalf("AcceptsAny nil")
	}
	if err := AcceptsAny(4, []byte{1}); err != nil {
		t.Fatalf("AcceptsAny bytes")
	}
	if err := AcceptsAny(5, &SomeClass{Name: "x"}); err != nil {
		t.Fatalf("AcceptsAny SomeClass")
	}
	if ReturnAny(4) == nil {
		t.Fatalf("ReturnAny")
	}
}

func TestBuffersAndClasses(t *testing.T) {
	buffers := GetThreeBuffers()
	if len(buffers) != 3 {
		t.Fatalf("GetThreeBuffers")
	}
	if err := ExpectThreeBuffers(buffers); err != nil {
		t.Fatalf("ExpectThreeBuffers")
	}
	classes := GetSomeClasses()
	if len(classes) != 3 {
		t.Fatalf("GetSomeClasses")
	}
	if err := ExpectThreeSomeClasses(classes); err != nil {
		t.Fatalf("ExpectThreeSomeClasses")
	}
}

func TestArrays(t *testing.T) {
	if len(Make2DArray()) != 2 {
		t.Fatalf("Make2DArray")
	}
	if Sum3DArray(Make3DArray()) != 10 {
		t.Fatalf("Sum3DArray")
	}
	if SumRaggedArray(MakeRaggedArray()) != 21 {
		t.Fatalf("SumRaggedArray")
	}
}

func TestPackedArrays(t *testing.T) {
	if Sum1dInt64Array([]int64{1, 2, 3, 4, 5}) != 15 {
		t.Fatalf("Sum1dInt64Array")
	}
	arr := []int64{100, 200, 300}
	result := Echo1dInt64Array(arr)
	if !reflect.DeepEqual(result, arr) {
		t.Fatalf("Echo1dInt64Array")
	}
	farr := []float64{1.5, 2.5, 3.5}
	fresult := Echo1dFloat64Array(farr)
	if !reflect.DeepEqual(fresult, farr) {
		t.Fatalf("Echo1dFloat64Array")
	}
	made := Make1dInt64Array()
	if !reflect.DeepEqual(made, []int64{10, 20, 30, 40, 50}) {
		t.Fatalf("Make1dInt64Array")
	}
}

func TestObjects(t *testing.T) {
	sc := &SomeClass{Name: "x"}
	if !strings.Contains(sc.Print(), "SomeClass") {
		t.Fatalf("SomeClass Print")
	}
	tm := NewTestMap()
	tm.Set("k", 1)
	if !tm.Contains("k") || tm.Get("k") != 1 {
		t.Fatalf("TestMap")
	}
	if tm.Name != "name1" {
		t.Fatalf("TestMap Name")
	}
	base := Base{BaseValue: 3}
	derived := Derived{Base: base, Extra: "extra"}
	if derived.BaseMethod() != 3 || derived.DerivedMethod() != "extra" {
		t.Fatalf("Derived")
	}
	outer := NewOuter(5)
	if outer.Inner.Value != 5 {
		t.Fatalf("Outer/Inner")
	}
}

func TestPrimitives(t *testing.T) {
	prims := AcceptsPrimitives(true, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1.5, 2.5, complex64(1+2i), complex128(3+4i), 'a', "s")
	if !prims.BoolVal || prims.Int64Val != 5 || prims.Float64Val != 2.5 || prims.RuneVal != 'a' {
		t.Fatalf("AcceptsPrimitives")
	}
	if ToUpperRune('a') != 'A' {
		t.Fatalf("ToUpperRune")
	}
}

func TestCollections(t *testing.T) {
	if len(MakeStringList()) != 3 {
		t.Fatalf("MakeStringList")
	}
	if len(MakeStringIntMap()) != 2 {
		t.Fatalf("MakeStringIntMap")
	}
	if len(MakeIntSet()) != 3 {
		t.Fatalf("MakeIntSet")
	}
	if !reflect.DeepEqual(MakeNestedMap()["nums"], []int{1, 2, 3}) {
		t.Fatalf("MakeNestedMap")
	}
	if len(MakeSomeClassList()) != 3 {
		t.Fatalf("MakeSomeClassList")
	}
	if _, ok := MakeMapAny()["b"].(string); !ok {
		t.Fatalf("MakeMapAny")
	}
}

func TestInterfacesAndCallbacks(t *testing.T) {
	if CallGreeter(SimpleGreeter{}, "Bob") != "Hello Bob" {
		t.Fatalf("CallGreeter")
	}
	if CallTransformer(func(value string) string { return value + "_x" }, "a") != "a_x" {
		t.Fatalf("CallTransformer")
	}
	if ReturnTransformer("_y")("b") != "b_y" {
		t.Fatalf("ReturnTransformer")
	}
	if CallFunction(func(value string) int { return len(value) }, "abc") != 3 {
		t.Fatalf("CallFunction")
	}
	if _, err := CallTransformerWithError(func(value string) (string, error) {
		return "", errors.New("callback error")
	}, "x"); err == nil {
		t.Fatalf("CallTransformerWithError")
	}
	if _, err := CallTransformerRecover(func(value string) string {
		panic("panic")
	}, "x"); err == nil {
		t.Fatalf("CallTransformerRecover")
	}
}

func TestGenericsAndVarargs(t *testing.T) {
	intBox := NewIntBox(2)
	if intBox.Get() != 2 {
		t.Fatalf("NewIntBox")
	}
	strBox := NewStringBox("x")
	if strBox.Get() != "x" {
		t.Fatalf("NewStringBox")
	}
	if Sum(1, 2, 3) != 6 {
		t.Fatalf("Sum")
	}
	if Join("p", "a", "b") != "p:a:b" {
		t.Fatalf("Join")
	}
}

func TestErrorsAndState(t *testing.T) {
	if _, err := ReturnErrorTuple(true); err != nil {
		t.Fatalf("ReturnErrorTuple ok")
	}
	if _, err := ReturnErrorTuple(false); err == nil {
		t.Fatalf("ReturnErrorTuple error")
	}
	defer func() {
		if recover() == nil {
			t.Fatalf("Panics")
		}
	}()
	if ConstFiveSeconds != 5 {
		t.Fatalf("ConstFiveSeconds")
	}
	FiveSeconds = 5
	SetCounter(0)
	if IncCounter(1) != 1 {
		t.Fatalf("IncCounter")
	}
	Panics()
}

func TestChannels(t *testing.T) {
	ch := MakeBufferedChannel(3)
	SendAndClose(ch, []int{1, 2, 3})
	res := ReceiveAll(ch)
	if !reflect.DeepEqual(res, []int{1, 2, 3}) {
		t.Fatalf("ReceiveAll")
	}
	if AddAsync(2, 3) != 5 {
		t.Fatalf("AddAsync")
	}
}

func TestEnumAndSubPackage(t *testing.T) {
	if ColorName(GetColor(0)) != "RED" {
		t.Fatalf("GetColor")
	}
	if metaffisub.Echo("v") != "v" {
		t.Fatalf("sub Echo")
	}
}
