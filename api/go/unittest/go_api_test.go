package unittest

import (
	"fmt"
	"math"
	"os"
	"strings"
	"testing"

	api "github.com/MetaFFI/sdk/api/go"
	goruntime "github.com/MetaFFI/sdk/api/go/metaffi"
	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

// ---------------------------------------------------------------------------
// Global test fixtures
// ---------------------------------------------------------------------------

var (
	runtime    *api.MetaFFIRuntime
	testModule *api.MetaFFIModule
)

func TestMain(m *testing.M) {
	home := os.Getenv("METAFFI_HOME")
	if home == "" {
		fmt.Fprintln(os.Stderr, "METAFFI_HOME must be set")
		os.Exit(1)
	}

	runtime = api.NewMetaFFIRuntime("test")
	if err := runtime.LoadRuntimePlugin(); err != nil {
		fmt.Fprintf(os.Stderr, "Failed to load test runtime plugin: %v\n", err)
		os.Exit(1)
	}

	var err error
	testModule, err = runtime.LoadModule("")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Failed to load test module: %v\n", err)
		os.Exit(1)
	}

	code := m.Run()
	_ = runtime.ReleaseRuntimePlugin()
	os.Exit(code)
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// ti creates a MetaFFITypeInfo for a scalar type.
func ti(t IDL.MetaFFIType) IDL.MetaFFITypeInfo {
	return IDL.MetaFFITypeInfo{StringType: t}
}

// tiArray creates a MetaFFITypeInfo for an array type with the given dimensions.
func tiArray(t IDL.MetaFFIType, dims int) IDL.MetaFFITypeInfo {
	return IDL.MetaFFITypeInfo{StringType: t, Dimensions: dims}
}

// load loads an entity from the test module; fails immediately on error.
func load(t *testing.T, entityPath string, params []IDL.MetaFFITypeInfo, retvals []IDL.MetaFFITypeInfo) func(...interface{}) ([]interface{}, error) {
	t.Helper()
	ff, err := testModule.LoadWithInfo(entityPath, params, retvals)
	if err != nil {
		t.Fatalf("load %q: %v", entityPath, err)
	}
	if ff == nil {
		t.Fatalf("load %q: returned nil", entityPath)
	}
	return ff
}

// call invokes ff and fatals on error, returning the result slice.
func call(t *testing.T, name string, ff func(...interface{}) ([]interface{}, error), args ...interface{}) []interface{} {
	t.Helper()
	ret, err := ff(args...)
	if err != nil {
		t.Fatalf("%s: %v", name, err)
	}
	return ret
}

// callExpectError invokes ff and expects an error whose message contains substr.
func callExpectError(t *testing.T, name string, ff func(...interface{}) ([]interface{}, error), substr string, args ...interface{}) {
	t.Helper()
	_, err := ff(args...)
	if err == nil {
		t.Fatalf("%s: expected error containing %q but got nil", name, substr)
	}
	if !strings.Contains(strings.ToLower(err.Error()), strings.ToLower(substr)) {
		t.Fatalf("%s: error %q does not contain %q", name, err.Error(), substr)
	}
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

func TestPrimitivesNoParamsNoRet(t *testing.T) {
	noOp := load(t, "test::no_op", nil, nil)
	call(t, "no_op", noOp)

	printHello := load(t, "test::print_hello", nil, nil)
	call(t, "print_hello", printHello)
}

func TestPrimitivesReturnOnly(t *testing.T) {
	// int8
	ret := call(t, "return_int8", load(t, "test::return_int8", nil, []IDL.MetaFFITypeInfo{ti(IDL.INT8)}))
	if v, ok := ret[0].(int8); !ok || v != 42 {
		t.Fatalf("return_int8: got %v (%T), want int8(42)", ret[0], ret[0])
	}

	// int16
	ret = call(t, "return_int16", load(t, "test::return_int16", nil, []IDL.MetaFFITypeInfo{ti(IDL.INT16)}))
	if v, ok := ret[0].(int16); !ok || v != 1000 {
		t.Fatalf("return_int16: got %v (%T), want int16(1000)", ret[0], ret[0])
	}

	// int32
	ret = call(t, "return_int32", load(t, "test::return_int32", nil, []IDL.MetaFFITypeInfo{ti(IDL.INT32)}))
	if v, ok := ret[0].(int32); !ok || v != 100000 {
		t.Fatalf("return_int32: got %v (%T), want int32(100000)", ret[0], ret[0])
	}

	// int64
	ret = call(t, "return_int64", load(t, "test::return_int64", nil, []IDL.MetaFFITypeInfo{ti(IDL.INT64)}))
	if v, ok := ret[0].(int64); !ok || v != math.MaxInt64 {
		t.Fatalf("return_int64: got %v (%T), want int64(MaxInt64)", ret[0], ret[0])
	}

	// uint8
	ret = call(t, "return_uint8", load(t, "test::return_uint8", nil, []IDL.MetaFFITypeInfo{ti(IDL.UINT8)}))
	if v, ok := ret[0].(uint8); !ok || v != 255 {
		t.Fatalf("return_uint8: got %v (%T), want uint8(255)", ret[0], ret[0])
	}

	// uint16
	ret = call(t, "return_uint16", load(t, "test::return_uint16", nil, []IDL.MetaFFITypeInfo{ti(IDL.UINT16)}))
	if v, ok := ret[0].(uint16); !ok || v != 65535 {
		t.Fatalf("return_uint16: got %v (%T), want uint16(65535)", ret[0], ret[0])
	}

	// uint32
	ret = call(t, "return_uint32", load(t, "test::return_uint32", nil, []IDL.MetaFFITypeInfo{ti(IDL.UINT32)}))
	if v, ok := ret[0].(uint32); !ok || v != math.MaxUint32 {
		t.Fatalf("return_uint32: got %v (%T), want uint32(MaxUint32)", ret[0], ret[0])
	}

	// uint64
	ret = call(t, "return_uint64", load(t, "test::return_uint64", nil, []IDL.MetaFFITypeInfo{ti(IDL.UINT64)}))
	if v, ok := ret[0].(uint64); !ok || v != math.MaxUint64 {
		t.Fatalf("return_uint64: got %v (%T), want uint64(MaxUint64)", ret[0], ret[0])
	}

	// float32
	ret = call(t, "return_float32", load(t, "test::return_float32", nil, []IDL.MetaFFITypeInfo{ti(IDL.FLOAT32)}))
	if v, ok := ret[0].(float32); !ok || math.Abs(float64(v)-3.14159) > 0.001 {
		t.Fatalf("return_float32: got %v (%T), want ~3.14159", ret[0], ret[0])
	}

	// float64
	ret = call(t, "return_float64", load(t, "test::return_float64", nil, []IDL.MetaFFITypeInfo{ti(IDL.FLOAT64)}))
	if v, ok := ret[0].(float64); !ok || math.Abs(v-3.141592653589793) > 1e-12 {
		t.Fatalf("return_float64: got %v (%T), want ~3.141592653589793", ret[0], ret[0])
	}

	// bool true
	ret = call(t, "return_bool_true", load(t, "test::return_bool_true", nil, []IDL.MetaFFITypeInfo{ti(IDL.BOOL)}))
	if v, ok := ret[0].(bool); !ok || !v {
		t.Fatalf("return_bool_true: got %v (%T), want true", ret[0], ret[0])
	}

	// bool false
	ret = call(t, "return_bool_false", load(t, "test::return_bool_false", nil, []IDL.MetaFFITypeInfo{ti(IDL.BOOL)}))
	if v, ok := ret[0].(bool); !ok || v {
		t.Fatalf("return_bool_false: got %v (%T), want false", ret[0], ret[0])
	}

	// string8
	ret = call(t, "return_string8", load(t, "test::return_string8", nil, []IDL.MetaFFITypeInfo{ti(IDL.STRING8)}))
	if v, ok := ret[0].(string); !ok || v != "Hello from test plugin" {
		t.Fatalf("return_string8: got %v (%T), want \"Hello from test plugin\"", ret[0], ret[0])
	}

	// null
	ret = call(t, "return_null", load(t, "test::return_null", nil, []IDL.MetaFFITypeInfo{ti(IDL.NULL)}))
	if ret[0] != nil {
		t.Fatalf("return_null: got %v (%T), want nil", ret[0], ret[0])
	}
}

func TestPrimitivesAcceptOnly(t *testing.T) {
	call(t, "accept_int8", load(t, "test::accept_int8", []IDL.MetaFFITypeInfo{ti(IDL.INT8)}, nil), int8(42))
	call(t, "accept_int16", load(t, "test::accept_int16", []IDL.MetaFFITypeInfo{ti(IDL.INT16)}, nil), int16(1000))
	call(t, "accept_int32", load(t, "test::accept_int32", []IDL.MetaFFITypeInfo{ti(IDL.INT32)}, nil), int32(100000))
	call(t, "accept_int64", load(t, "test::accept_int64", []IDL.MetaFFITypeInfo{ti(IDL.INT64)}, nil), int64(math.MaxInt64))
	call(t, "accept_float32", load(t, "test::accept_float32", []IDL.MetaFFITypeInfo{ti(IDL.FLOAT32)}, nil), float32(3.14159))
	call(t, "accept_float64", load(t, "test::accept_float64", []IDL.MetaFFITypeInfo{ti(IDL.FLOAT64)}, nil), float64(3.141592653589793))
	call(t, "accept_bool", load(t, "test::accept_bool", []IDL.MetaFFITypeInfo{ti(IDL.BOOL)}, nil), true)
	call(t, "accept_string8", load(t, "test::accept_string8", []IDL.MetaFFITypeInfo{ti(IDL.STRING8)}, nil), "test string")
}

func TestEchoFunctions(t *testing.T) {
	// echo int64
	ret := call(t, "echo_int64",
		load(t, "test::echo_int64", []IDL.MetaFFITypeInfo{ti(IDL.INT64)}, []IDL.MetaFFITypeInfo{ti(IDL.INT64)}),
		int64(42))
	if v, ok := ret[0].(int64); !ok || v != 42 {
		t.Fatalf("echo_int64: got %v (%T), want 42", ret[0], ret[0])
	}

	// echo float64
	ret = call(t, "echo_float64",
		load(t, "test::echo_float64", []IDL.MetaFFITypeInfo{ti(IDL.FLOAT64)}, []IDL.MetaFFITypeInfo{ti(IDL.FLOAT64)}),
		float64(3.14))
	if v, ok := ret[0].(float64); !ok || math.Abs(v-3.14) > 1e-10 {
		t.Fatalf("echo_float64: got %v (%T), want 3.14", ret[0], ret[0])
	}

	// echo string8
	ret = call(t, "echo_string8",
		load(t, "test::echo_string8", []IDL.MetaFFITypeInfo{ti(IDL.STRING8)}, []IDL.MetaFFITypeInfo{ti(IDL.STRING8)}),
		"hello")
	if v, ok := ret[0].(string); !ok || v != "hello" {
		t.Fatalf("echo_string8: got %v (%T), want \"hello\"", ret[0], ret[0])
	}

	// echo bool
	ret = call(t, "echo_bool",
		load(t, "test::echo_bool", []IDL.MetaFFITypeInfo{ti(IDL.BOOL)}, []IDL.MetaFFITypeInfo{ti(IDL.BOOL)}),
		true)
	if v, ok := ret[0].(bool); !ok || !v {
		t.Fatalf("echo_bool: got %v (%T), want true", ret[0], ret[0])
	}
}

func TestArithmeticFunctions(t *testing.T) {
	// add_int64
	ret := call(t, "add_int64",
		load(t, "test::add_int64",
			[]IDL.MetaFFITypeInfo{ti(IDL.INT64), ti(IDL.INT64)},
			[]IDL.MetaFFITypeInfo{ti(IDL.INT64)}),
		int64(10), int64(20))
	if v, ok := ret[0].(int64); !ok || v != 30 {
		t.Fatalf("add_int64: got %v (%T), want 30", ret[0], ret[0])
	}

	// add_float64
	ret = call(t, "add_float64",
		load(t, "test::add_float64",
			[]IDL.MetaFFITypeInfo{ti(IDL.FLOAT64), ti(IDL.FLOAT64)},
			[]IDL.MetaFFITypeInfo{ti(IDL.FLOAT64)}),
		float64(1.5), float64(2.5))
	if v, ok := ret[0].(float64); !ok || math.Abs(v-4.0) > 1e-10 {
		t.Fatalf("add_float64: got %v (%T), want 4.0", ret[0], ret[0])
	}

	// concat_strings
	ret = call(t, "concat_strings",
		load(t, "test::concat_strings",
			[]IDL.MetaFFITypeInfo{ti(IDL.STRING8), ti(IDL.STRING8)},
			[]IDL.MetaFFITypeInfo{ti(IDL.STRING8)}),
		"hello", "world")
	if v, ok := ret[0].(string); !ok || v != "helloworld" {
		t.Fatalf("concat_strings: got %v (%T), want \"helloworld\"", ret[0], ret[0])
	}
}

func TestArrays(t *testing.T) {
	// return 1D int64 array → [1,2,3]
	ret := call(t, "return_int64_array_1d",
		load(t, "test::return_int64_array_1d", nil, []IDL.MetaFFITypeInfo{tiArray(IDL.INT64_ARRAY, 1)}))
	arr1d, ok := ret[0].([]int64)
	if !ok {
		t.Fatalf("return_int64_array_1d: unexpected type %T", ret[0])
	}
	want1d := []int64{1, 2, 3}
	if len(arr1d) != len(want1d) {
		t.Fatalf("return_int64_array_1d: len=%d, want %d", len(arr1d), len(want1d))
	}
	for i := range want1d {
		if arr1d[i] != want1d[i] {
			t.Fatalf("return_int64_array_1d[%d]: got %d, want %d", i, arr1d[i], want1d[i])
		}
	}

	// return 2D int64 array → [[1,2],[3,4]]
	ret = call(t, "return_int64_array_2d",
		load(t, "test::return_int64_array_2d", nil, []IDL.MetaFFITypeInfo{tiArray(IDL.INT64_ARRAY, 2)}))
	arr2d, ok := ret[0].([][]int64)
	if !ok {
		t.Fatalf("return_int64_array_2d: unexpected type %T", ret[0])
	}
	if len(arr2d) != 2 || len(arr2d[0]) != 2 || len(arr2d[1]) != 2 {
		t.Fatalf("return_int64_array_2d: shape mismatch %v", arr2d)
	}
	if arr2d[0][0] != 1 || arr2d[0][1] != 2 || arr2d[1][0] != 3 || arr2d[1][1] != 4 {
		t.Fatalf("return_int64_array_2d: values mismatch %v", arr2d)
	}

	// return 3D int64 array → [[[1,2],[3,4]],[[5,6],[7,8]]]
	ret = call(t, "return_int64_array_3d",
		load(t, "test::return_int64_array_3d", nil, []IDL.MetaFFITypeInfo{tiArray(IDL.INT64_ARRAY, 3)}))
	arr3d, ok := ret[0].([][][]int64)
	if !ok {
		t.Fatalf("return_int64_array_3d: unexpected type %T", ret[0])
	}
	if arr3d[0][0][0] != 1 || arr3d[0][0][1] != 2 || arr3d[0][1][0] != 3 || arr3d[0][1][1] != 4 ||
		arr3d[1][0][0] != 5 || arr3d[1][0][1] != 6 || arr3d[1][1][0] != 7 || arr3d[1][1][1] != 8 {
		t.Fatalf("return_int64_array_3d: values mismatch %v", arr3d)
	}

	// return ragged array → [[1,2,3],[4],[5,6]]
	ret = call(t, "return_ragged_array",
		load(t, "test::return_ragged_array", nil, []IDL.MetaFFITypeInfo{tiArray(IDL.INT64_ARRAY, 2)}))
	ragged, ok := ret[0].([][]int64)
	if !ok {
		t.Fatalf("return_ragged_array: unexpected type %T", ret[0])
	}
	if len(ragged) != 3 {
		t.Fatalf("return_ragged_array: len=%d, want 3", len(ragged))
	}
	if len(ragged[0]) != 3 || ragged[0][0] != 1 || ragged[0][1] != 2 || ragged[0][2] != 3 {
		t.Fatalf("return_ragged_array[0]: %v", ragged[0])
	}
	if len(ragged[1]) != 1 || ragged[1][0] != 4 {
		t.Fatalf("return_ragged_array[1]: %v", ragged[1])
	}
	if len(ragged[2]) != 2 || ragged[2][0] != 5 || ragged[2][1] != 6 {
		t.Fatalf("return_ragged_array[2]: %v", ragged[2])
	}

	// return string array → ["one","two","three"]
	ret = call(t, "return_string_array",
		load(t, "test::return_string_array", nil, []IDL.MetaFFITypeInfo{tiArray(IDL.STRING8_ARRAY, 1)}))
	strArr, ok := ret[0].([]string)
	if !ok {
		t.Fatalf("return_string_array: unexpected type %T", ret[0])
	}
	wantStr := []string{"one", "two", "three"}
	if len(strArr) != len(wantStr) {
		t.Fatalf("return_string_array: len=%d, want %d", len(strArr), len(wantStr))
	}
	for i := range wantStr {
		if strArr[i] != wantStr[i] {
			t.Fatalf("return_string_array[%d]: got %q, want %q", i, strArr[i], wantStr[i])
		}
	}

	// sum_int64_array([1,2,3,4,5]) → 15
	ret = call(t, "sum_int64_array",
		load(t, "test::sum_int64_array",
			[]IDL.MetaFFITypeInfo{tiArray(IDL.INT64_ARRAY, 1)},
			[]IDL.MetaFFITypeInfo{ti(IDL.INT64)}),
		[]int64{1, 2, 3, 4, 5})
	if v, ok := ret[0].(int64); !ok || v != 15 {
		t.Fatalf("sum_int64_array: got %v (%T), want 15", ret[0], ret[0])
	}

	// echo_int64_array([10,20,30]) → [10,20,30]
	ret = call(t, "echo_int64_array",
		load(t, "test::echo_int64_array",
			[]IDL.MetaFFITypeInfo{tiArray(IDL.INT64_ARRAY, 1)},
			[]IDL.MetaFFITypeInfo{tiArray(IDL.INT64_ARRAY, 1)}),
		[]int64{10, 20, 30})
	echoArr, ok := ret[0].([]int64)
	if !ok {
		t.Fatalf("echo_int64_array: unexpected type %T", ret[0])
	}
	wantEcho := []int64{10, 20, 30}
	for i := range wantEcho {
		if echoArr[i] != wantEcho[i] {
			t.Fatalf("echo_int64_array[%d]: got %d, want %d", i, echoArr[i], wantEcho[i])
		}
	}

	// join_strings(["one","two","three"]) → "one, two, three"
	ret = call(t, "join_strings",
		load(t, "test::join_strings",
			[]IDL.MetaFFITypeInfo{tiArray(IDL.STRING8_ARRAY, 1)},
			[]IDL.MetaFFITypeInfo{ti(IDL.STRING8)}),
		[]string{"one", "two", "three"})
	if v, ok := ret[0].(string); !ok || v != "one, two, three" {
		t.Fatalf("join_strings: got %v (%T), want \"one, two, three\"", ret[0], ret[0])
	}
}

func TestHandles(t *testing.T) {
	// Create handle
	ret := call(t, "create_handle",
		load(t, "test::create_handle", nil, []IDL.MetaFFITypeInfo{ti(IDL.HANDLE)}))
	handle := ret[0]
	if handle == nil {
		t.Fatal("create_handle: returned nil")
	}

	// Get handle data → "test_data"
	getData := load(t, "test::get_handle_data",
		[]IDL.MetaFFITypeInfo{ti(IDL.HANDLE)},
		[]IDL.MetaFFITypeInfo{ti(IDL.STRING8)})
	ret = call(t, "get_handle_data", getData, handle)
	if v, ok := ret[0].(string); !ok || v != "test_data" {
		t.Fatalf("get_handle_data: got %v (%T), want \"test_data\"", ret[0], ret[0])
	}

	// Set handle data → "new_data"
	setData := load(t, "test::set_handle_data",
		[]IDL.MetaFFITypeInfo{ti(IDL.HANDLE), ti(IDL.STRING8)},
		nil)
	call(t, "set_handle_data", setData, handle, "new_data")

	// Verify data was updated
	ret = call(t, "get_handle_data (after set)", getData, handle)
	if v, ok := ret[0].(string); !ok || v != "new_data" {
		t.Fatalf("get_handle_data after set: got %v (%T), want \"new_data\"", ret[0], ret[0])
	}

	// Release handle
	releaseHandle := load(t, "test::release_handle",
		[]IDL.MetaFFITypeInfo{ti(IDL.HANDLE)},
		nil)
	call(t, "release_handle", releaseHandle, handle)
}

func TestHandleClassMethods(t *testing.T) {
	// Create handle
	ret := call(t, "create_handle",
		load(t, "test::create_handle", nil, []IDL.MetaFFITypeInfo{ti(IDL.HANDLE)}))
	handle := ret[0]
	if handle == nil {
		t.Fatal("create_handle: returned nil")
	}

	// Get id (should be > 0)
	ret = call(t, "TestHandle.get_id",
		load(t, "test::TestHandle.get_id",
			[]IDL.MetaFFITypeInfo{ti(IDL.HANDLE)},
			[]IDL.MetaFFITypeInfo{ti(IDL.INT64)}),
		handle)
	id, ok := ret[0].(int64)
	if !ok || id <= 0 {
		t.Fatalf("TestHandle.get_id: got %v (%T), want > 0", ret[0], ret[0])
	}

	// Append to data
	call(t, "TestHandle.append_to_data",
		load(t, "test::TestHandle.append_to_data",
			[]IDL.MetaFFITypeInfo{ti(IDL.HANDLE), ti(IDL.STRING8)},
			nil),
		handle, "_suffix")

	// Verify data was appended → "test_data_suffix"
	getData := load(t, "test::get_handle_data",
		[]IDL.MetaFFITypeInfo{ti(IDL.HANDLE)},
		[]IDL.MetaFFITypeInfo{ti(IDL.STRING8)})
	ret = call(t, "get_handle_data (after append)", getData, handle)
	if v, ok := ret[0].(string); !ok || v != "test_data_suffix" {
		t.Fatalf("get_handle_data after append: got %v (%T), want \"test_data_suffix\"", ret[0], ret[0])
	}

	// Release handle
	call(t, "release_handle",
		load(t, "test::release_handle",
			[]IDL.MetaFFITypeInfo{ti(IDL.HANDLE)}, nil),
		handle)
}

func TestCallableCallbackAdd(t *testing.T) {
	// Define a native Go adder function to pass as callback.
	// The test plugin's call_callback_add calls it with (3, 4) and expects 7.
	adder := func(a, b int64) int64 { return a + b }

	callCallbackAdd := load(t, "test::call_callback_add",
		[]IDL.MetaFFITypeInfo{ti(IDL.CALLABLE)},
		[]IDL.MetaFFITypeInfo{ti(IDL.INT64)})

	ret := call(t, "call_callback_add", callCallbackAdd, adder)
	if v, ok := ret[0].(int64); !ok || v != 7 {
		t.Fatalf("call_callback_add: got %v (%T), want 7", ret[0], ret[0])
	}
}

func TestCallableCallbackString(t *testing.T) {
	// The test plugin's call_callback_string calls the callback with "test"
	// and returns the callback's result.
	echo := func(s string) string { return s }

	callCallbackString := load(t, "test::call_callback_string",
		[]IDL.MetaFFITypeInfo{ti(IDL.CALLABLE)},
		[]IDL.MetaFFITypeInfo{ti(IDL.STRING8)})

	ret := call(t, "call_callback_string", callCallbackString, echo)
	if v, ok := ret[0].(string); !ok || v != "test" {
		t.Fatalf("call_callback_string: got %v (%T), want \"test\"", ret[0], ret[0])
	}
}

func TestCallableReturnAdder(t *testing.T) {
	// return_adder_callback returns a foreign callable that adds two int64 values.
	// FromCDTToGo returns it as *MetaFFICallable (no target type hint).
	// We use MetaFFICallable.Call() to invoke it.
	returnAdder := load(t, "test::return_adder_callback", nil,
		[]IDL.MetaFFITypeInfo{ti(IDL.CALLABLE)})

	ret := call(t, "return_adder_callback", returnAdder)
	if len(ret) != 1 || ret[0] == nil {
		t.Fatalf("return_adder_callback: expected 1 non-nil return, got %v", ret)
	}

	callable, ok := ret[0].(*goruntime.MetaFFICallable)
	if !ok {
		t.Fatalf("return_adder_callback: expected *MetaFFICallable, got %T", ret[0])
	}

	// Invoke the returned adder callable with (10, 20), expect 30
	result, err := callable.Call(int64(10), int64(20))
	if err != nil {
		t.Fatalf("callable.Call(10, 20): %v", err)
	}
	if len(result) != 1 {
		t.Fatalf("callable.Call: expected 1 return, got %d", len(result))
	}
	if v, ok := result[0].(int64); !ok || v != 30 {
		t.Fatalf("callable.Call(10, 20): got %v (%T), want 30", result[0], result[0])
	}
}

func TestErrorThrow(t *testing.T) {
	throwError := load(t, "test::throw_error", nil, nil)
	callExpectError(t, "throw_error", throwError, "Test error thrown intentionally")
}

func TestErrorThrowWithMessage(t *testing.T) {
	throwWithMessage := load(t, "test::throw_with_message",
		[]IDL.MetaFFITypeInfo{ti(IDL.STRING8)}, nil)
	callExpectError(t, "throw_with_message", throwWithMessage, "Custom error message", "Custom error message")
}

func TestErrorIfNegative(t *testing.T) {
	errorIfNegative := load(t, "test::error_if_negative",
		[]IDL.MetaFFITypeInfo{ti(IDL.INT64)}, nil)

	// Should succeed for positive
	call(t, "error_if_negative(42)", errorIfNegative, int64(42))

	// Should fail for negative
	callExpectError(t, "error_if_negative(-1)", errorIfNegative, "negative", int64(-1))
}

func TestAnyType(t *testing.T) {
	acceptAny := load(t, "test::accept_any",
		[]IDL.MetaFFITypeInfo{ti(IDL.ANY)},
		[]IDL.MetaFFITypeInfo{ti(IDL.ANY)})

	// int64: 42 → 142
	ret := call(t, "accept_any(int64)", acceptAny, int64(42))
	if v, ok := ret[0].(int64); !ok || v != 142 {
		t.Fatalf("accept_any(int64): got %v (%T), want 142", ret[0], ret[0])
	}

	// float64: 3.14 → 6.28
	ret = call(t, "accept_any(float64)", acceptAny, float64(3.14))
	if v, ok := ret[0].(float64); !ok || math.Abs(v-6.28) > 1e-6 {
		t.Fatalf("accept_any(float64): got %v (%T), want 6.28", ret[0], ret[0])
	}

	// string8: "hello" → "echoed: hello"
	ret = call(t, "accept_any(string8)", acceptAny, "hello")
	if v, ok := ret[0].(string); !ok || v != "echoed: hello" {
		t.Fatalf("accept_any(string8): got %v (%T), want \"echoed: hello\"", ret[0], ret[0])
	}

	// int64[]: [1,2,3] → [10,20,30]
	ret = call(t, "accept_any(int64[])", acceptAny, []int64{1, 2, 3})
	switch arr := ret[0].(type) {
	case []int64:
		want := []int64{10, 20, 30}
		if len(arr) != len(want) {
			t.Fatalf("accept_any(int64[]): len=%d, want %d", len(arr), len(want))
		}
		for i := range want {
			if arr[i] != want[i] {
				t.Fatalf("accept_any(int64[])[%d]: got %d, want %d", i, arr[i], want[i])
			}
		}
	default:
		t.Fatalf("accept_any(int64[]): unexpected type %T", ret[0])
	}
}

func TestMultipleReturnValues(t *testing.T) {
	// return_two_values → (42, "answer")
	ret := call(t, "return_two_values",
		load(t, "test::return_two_values", nil,
			[]IDL.MetaFFITypeInfo{ti(IDL.INT64), ti(IDL.STRING8)}))
	if len(ret) != 2 {
		t.Fatalf("return_two_values: got %d returns, want 2", len(ret))
	}
	if v, ok := ret[0].(int64); !ok || v != 42 {
		t.Fatalf("return_two_values[0]: got %v (%T), want 42", ret[0], ret[0])
	}
	if v, ok := ret[1].(string); !ok || v != "answer" {
		t.Fatalf("return_two_values[1]: got %v (%T), want \"answer\"", ret[1], ret[1])
	}

	// return_three_values → (1, 2.5, true)
	ret = call(t, "return_three_values",
		load(t, "test::return_three_values", nil,
			[]IDL.MetaFFITypeInfo{ti(IDL.INT64), ti(IDL.FLOAT64), ti(IDL.BOOL)}))
	if len(ret) != 3 {
		t.Fatalf("return_three_values: got %d returns, want 3", len(ret))
	}
	if v, ok := ret[0].(int64); !ok || v != 1 {
		t.Fatalf("return_three_values[0]: got %v (%T), want 1", ret[0], ret[0])
	}
	if v, ok := ret[1].(float64); !ok || math.Abs(v-2.5) > 1e-10 {
		t.Fatalf("return_three_values[1]: got %v (%T), want 2.5", ret[1], ret[1])
	}
	if v, ok := ret[2].(bool); !ok || !v {
		t.Fatalf("return_three_values[2]: got %v (%T), want true", ret[2], ret[2])
	}

	// swap_values(42, "hello") → ("hello", 42)
	ret = call(t, "swap_values",
		load(t, "test::swap_values",
			[]IDL.MetaFFITypeInfo{ti(IDL.INT64), ti(IDL.STRING8)},
			[]IDL.MetaFFITypeInfo{ti(IDL.STRING8), ti(IDL.INT64)}),
		int64(42), "hello")
	if len(ret) != 2 {
		t.Fatalf("swap_values: got %d returns, want 2", len(ret))
	}
	if v, ok := ret[0].(string); !ok || v != "hello" {
		t.Fatalf("swap_values[0]: got %v (%T), want \"hello\"", ret[0], ret[0])
	}
	if v, ok := ret[1].(int64); !ok || v != 42 {
		t.Fatalf("swap_values[1]: got %v (%T), want 42", ret[1], ret[1])
	}
}

func TestGlobalVariable(t *testing.T) {
	getGName := load(t, "test::get_g_name", nil, []IDL.MetaFFITypeInfo{ti(IDL.STRING8)})
	setGName := load(t, "test::set_g_name", []IDL.MetaFFITypeInfo{ti(IDL.STRING8)}, nil)

	// Read original value
	ret := call(t, "get_g_name (original)", getGName)
	oldValue, ok := ret[0].(string)
	if !ok {
		t.Fatalf("get_g_name: unexpected type %T", ret[0])
	}

	newValue := "go_api_test_value"
	// Set new value and restore in defer
	call(t, "set_g_name", setGName, newValue)
	defer func() {
		call(t, "set_g_name (restore)", setGName, oldValue)
	}()

	// Verify new value
	ret = call(t, "get_g_name (after set)", getGName)
	if v, ok := ret[0].(string); !ok || v != newValue {
		t.Fatalf("get_g_name after set: got %v (%T), want %q", ret[0], ret[0], newValue)
	}
}
