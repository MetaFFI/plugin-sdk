package cdts_go_serializer

import (
	"strings"
	"testing"
	"unsafe"
)

// ===== Primitive Types - Round-Trip Tests =====

func TestSerializeDeserializeInt8(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := int8(-42)
	_, err = ser.AddInt8(original)
	if err != nil {
		t.Fatalf("AddInt8 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractInt8()
	if err != nil {
		t.Fatalf("ExtractInt8 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %d, got %d", original, val)
	}
}

func TestSerializeDeserializeInt16(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := int16(-12345)
	_, err = ser.AddInt16(original)
	if err != nil {
		t.Fatalf("AddInt16 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractInt16()
	if err != nil {
		t.Fatalf("ExtractInt16 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %d, got %d", original, val)
	}
}

func TestSerializeDeserializeInt32(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := int32(-123456789)
	_, err = ser.AddInt32(original)
	if err != nil {
		t.Fatalf("AddInt32 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractInt32()
	if err != nil {
		t.Fatalf("ExtractInt32 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %d, got %d", original, val)
	}
}

func TestSerializeDeserializeInt64(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := int64(-1234567890123456)
	_, err = ser.AddInt64(original)
	if err != nil {
		t.Fatalf("AddInt64 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractInt64()
	if err != nil {
		t.Fatalf("ExtractInt64 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %d, got %d", original, val)
	}
}

func TestSerializeDeserializeUint8(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := uint8(255)
	_, err = ser.AddUint8(original)
	if err != nil {
		t.Fatalf("AddUint8 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractUint8()
	if err != nil {
		t.Fatalf("ExtractUint8 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %d, got %d", original, val)
	}
}

func TestSerializeDeserializeUint16(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := uint16(65535)
	_, err = ser.AddUint16(original)
	if err != nil {
		t.Fatalf("AddUint16 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractUint16()
	if err != nil {
		t.Fatalf("ExtractUint16 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %d, got %d", original, val)
	}
}

func TestSerializeDeserializeUint32(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := uint32(4294967295)
	_, err = ser.AddUint32(original)
	if err != nil {
		t.Fatalf("AddUint32 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractUint32()
	if err != nil {
		t.Fatalf("ExtractUint32 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %d, got %d", original, val)
	}
}

func TestSerializeDeserializeUint64(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := uint64(18446744073709551615)
	_, err = ser.AddUint64(original)
	if err != nil {
		t.Fatalf("AddUint64 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractUint64()
	if err != nil {
		t.Fatalf("ExtractUint64 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %d, got %d", original, val)
	}
}

func TestSerializeDeserializeFloat32(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := float32(3.14159)
	_, err = ser.AddFloat32(original)
	if err != nil {
		t.Fatalf("AddFloat32 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractFloat32()
	if err != nil {
		t.Fatalf("ExtractFloat32 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %f, got %f", original, val)
	}
}

func TestSerializeDeserializeFloat64(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := 2.71828182845904523536
	_, err = ser.AddFloat64(original)
	if err != nil {
		t.Fatalf("AddFloat64 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractFloat64()
	if err != nil {
		t.Fatalf("ExtractFloat64 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %f, got %f", original, val)
	}
}

func TestSerializeDeserializeBool(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(2)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	_, err = ser.AddBool(true)
	if err != nil {
		t.Fatalf("AddBool(true) failed: %v", err)
	}

	_, err = ser.AddBool(false)
	if err != nil {
		t.Fatalf("AddBool(false) failed: %v", err)
	}

	ser.Reset()
	val1, err := ser.ExtractBool()
	if err != nil {
		t.Fatalf("ExtractBool failed: %v", err)
	}
	if !val1 {
		t.Error("Expected true, got false")
	}

	val2, err := ser.ExtractBool()
	if err != nil {
		t.Fatalf("ExtractBool failed: %v", err)
	}
	if val2 {
		t.Error("Expected false, got true")
	}
}

// ===== Platform-Dependent Int Tests =====

func TestSerializeDeserializeInt(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := 42
	_, err = ser.AddInt(original, MetaffiInt32Type())
	if err != nil {
		t.Fatalf("AddInt failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractInt(MetaffiInt32Type())
	if err != nil {
		t.Fatalf("ExtractInt failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %d, got %d", original, val)
	}
}

func TestAddIntRangeValidation(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	// Test value out of range for int8
	_, err = ser.AddInt(300, MetaffiInt8Type())
	if err == nil {
		t.Error("Expected error for value out of range, got nil")
	}
	if !strings.Contains(err.Error(), "out of range") {
		t.Errorf("Expected 'out of range' error, got: %v", err)
	}
}

// ===== String Tests =====

func TestSerializeDeserializeString8(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := "Hello, MetaFFI!"
	_, err = ser.AddString(original)
	if err != nil {
		t.Fatalf("AddString failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractString()
	if err != nil {
		t.Fatalf("ExtractString failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %q, got %q", original, val)
	}
}

func TestSerializeDeserializeString16(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := "Hello UTF-16 מה שלומך"
	_, err = ser.AddString16(original)
	if err != nil {
		t.Fatalf("AddString16 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractString16()
	if err != nil {
		t.Fatalf("ExtractString16 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %q, got %q", original, val)
	}
}

func TestSerializeDeserializeString32(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := "Hello UTF-32 🚀"
	_, err = ser.AddString32(original)
	if err != nil {
		t.Fatalf("AddString32 failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractString32()
	if err != nil {
		t.Fatalf("ExtractString32 failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %q, got %q", original, val)
	}
}

func TestSerializeDeserializeEmptyString(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := ""
	_, err = ser.AddString(original)
	if err != nil {
		t.Fatalf("AddString failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractString()
	if err != nil {
		t.Fatalf("ExtractString failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %q, got %q", original, val)
	}
}

// ===== Multiple Values Tests =====

func TestSerializeDeserializeMultiplePrimitives(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(5)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	_, err = ser.AddInt32(42)
	if err != nil {
		t.Fatalf("AddInt32 failed: %v", err)
	}
	_, err = ser.AddFloat32(3.14)
	if err != nil {
		t.Fatalf("AddFloat32 failed: %v", err)
	}
	_, err = ser.AddFloat64(2.71828)
	if err != nil {
		t.Fatalf("AddFloat64 failed: %v", err)
	}
	_, err = ser.AddBool(true)
	if err != nil {
		t.Fatalf("AddBool failed: %v", err)
	}
	_, err = ser.AddUint64(999)
	if err != nil {
		t.Fatalf("AddUint64 failed: %v", err)
	}

	ser.Reset()
	val1, err := ser.ExtractInt32()
	if err != nil {
		t.Fatalf("ExtractInt32 failed: %v", err)
	}
	if val1 != 42 {
		t.Errorf("Expected 42, got %d", val1)
	}

	val2, err := ser.ExtractFloat32()
	if err != nil {
		t.Fatalf("ExtractFloat32 failed: %v", err)
	}
	if val2 != 3.14 {
		t.Errorf("Expected 3.14, got %f", val2)
	}

	val3, err := ser.ExtractFloat64()
	if err != nil {
		t.Fatalf("ExtractFloat64 failed: %v", err)
	}
	if val3 != 2.71828 {
		t.Errorf("Expected 2.71828, got %f", val3)
	}

	val4, err := ser.ExtractBool()
	if err != nil {
		t.Fatalf("ExtractBool failed: %v", err)
	}
	if !val4 {
		t.Error("Expected true, got false")
	}

	val5, err := ser.ExtractUint64()
	if err != nil {
		t.Fatalf("ExtractUint64 failed: %v", err)
	}
	if val5 != 999 {
		t.Errorf("Expected 999, got %d", val5)
	}
}

// ===== Array Tests =====

func TestSerializeDeserializeInt32Slice(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := []int32{1, 2, 3, 4, 5}
	_, err = ser.AddInt32Slice(original)
	if err != nil {
		t.Fatalf("AddInt32Slice failed: %v", err)
	}

	// Note: Array extraction not yet implemented in Go serializer
	// This test verifies serialization works
	ser.Reset()
	peekType, err := ser.PeekType()
	if err != nil {
		t.Fatalf("PeekType failed: %v", err)
	}
	expectedType := CombineArrayType(MetaffiInt32Type())
	if peekType != expectedType {
		t.Errorf("Expected array type, got %d", peekType)
	}
}

func TestSerializeDeserializeFloat64Slice(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := []float64{1.1, 2.2, 3.3}
	_, err = ser.AddFloat64Slice(original)
	if err != nil {
		t.Fatalf("AddFloat64Slice failed: %v", err)
	}

	ser.Reset()
	peekType, err := ser.PeekType()
	if err != nil {
		t.Fatalf("PeekType failed: %v", err)
	}
	expectedType := CombineArrayType(MetaffiFloat64Type())
	if peekType != expectedType {
		t.Errorf("Expected array type, got %d", peekType)
	}
}

func TestSerializeDeserializeEmptySlice(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := []int32{}
	_, err = ser.AddInt32Slice(original)
	if err != nil {
		t.Fatalf("AddInt32Slice failed: %v", err)
	}

	ser.Reset()
	peekType, err := ser.PeekType()
	if err != nil {
		t.Fatalf("PeekType failed: %v", err)
	}
	expectedType := CombineArrayType(MetaffiInt32Type())
	if peekType != expectedType {
		t.Errorf("Expected array type, got %d", peekType)
	}
}

// ===== Special Values Tests =====

func TestSerializeDeserializeNull(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	_, err = ser.AddNull()
	if err != nil {
		t.Fatalf("AddNull failed: %v", err)
	}

	ser.Reset()
	isNull, err := ser.IsNull()
	if err != nil {
		t.Fatalf("IsNull failed: %v", err)
	}
	if !isNull {
		t.Error("Expected null, got non-null")
	}

	peekType, err := ser.PeekType()
	if err != nil {
		t.Fatalf("PeekType failed: %v", err)
	}
	if peekType != MetaffiNullType() {
		t.Errorf("Expected null type, got %d", peekType)
	}
}

// ===== Type Preservation Tests =====

func TestTypePreservationIntTypes(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(8)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	_, err = ser.AddInt8(-10)
	if err != nil {
		t.Fatalf("AddInt8 failed: %v", err)
	}
	_, err = ser.AddInt16(-1000)
	if err != nil {
		t.Fatalf("AddInt16 failed: %v", err)
	}
	_, err = ser.AddInt32(-100000)
	if err != nil {
		t.Fatalf("AddInt32 failed: %v", err)
	}
	_, err = ser.AddInt64(-10000000)
	if err != nil {
		t.Fatalf("AddInt64 failed: %v", err)
	}
	_, err = ser.AddUint8(10)
	if err != nil {
		t.Fatalf("AddUint8 failed: %v", err)
	}
	_, err = ser.AddUint16(1000)
	if err != nil {
		t.Fatalf("AddUint16 failed: %v", err)
	}
	_, err = ser.AddUint32(100000)
	if err != nil {
		t.Fatalf("AddUint32 failed: %v", err)
	}
	_, err = ser.AddUint64(10000000)
	if err != nil {
		t.Fatalf("AddUint64 failed: %v", err)
	}

	// Verify types are preserved
	ser.Reset()
	// Test each type in order - call helper functions directly without storing C types
	extractFuncs := []func() error{
		func() error { _, err := ser.ExtractInt8(); return err },
		func() error { _, err := ser.ExtractInt16(); return err },
		func() error { _, err := ser.ExtractInt32(); return err },
		func() error { _, err := ser.ExtractInt64(); return err },
		func() error { _, err := ser.ExtractUint8(); return err },
		func() error { _, err := ser.ExtractUint16(); return err },
		func() error { _, err := ser.ExtractUint32(); return err },
		func() error { _, err := ser.ExtractUint64(); return err },
	}
	// Call helper functions directly - can't store C types in test file
	// Just verify types match by calling helper functions and comparing
	for i := 0; i < 8; i++ {
		actualType, err := ser.PeekType()
		if err != nil {
			t.Fatalf("PeekType failed at index %d: %v", i, err)
		}
		// Get expected type by calling helper function - all C type usage is in helper file
		expectedType := GetExpectedTypeForIndex(i)
		if actualType != expectedType {
			t.Errorf("Index %d: Expected type %d, got %d", i, expectedType, actualType)
		}
		// Advance index
		if err := extractFuncs[i](); err != nil {
			t.Fatalf("Extract failed at index %d: %v", i, err)
		}
	}
}

// ===== Utility Methods Tests =====

func TestUtilityMethods(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(5)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	// Initial state
	if ser.GetIndex() != 0 {
		t.Errorf("Expected index 0, got %d", ser.GetIndex())
	}
	if ser.Size() != 5 {
		t.Errorf("Expected size 5, got %d", ser.Size())
	}
	if !ser.HasMore() {
		t.Error("Expected hasMore=true, got false")
	}

	// Add values
	_, err = ser.AddInt32(1)
	if err != nil {
		t.Fatalf("AddInt32 failed: %v", err)
	}
	_, err = ser.AddInt32(2)
	if err != nil {
		t.Fatalf("AddInt32 failed: %v", err)
	}
	_, err = ser.AddInt32(3)
	if err != nil {
		t.Fatalf("AddInt32 failed: %v", err)
	}

	if ser.GetIndex() != 3 {
		t.Errorf("Expected index 3, got %d", ser.GetIndex())
	}
	if !ser.HasMore() {
		t.Error("Expected hasMore=true, got false")
	}

	// Reset
	ser.Reset()
	if ser.GetIndex() != 0 {
		t.Errorf("Expected index 0 after reset, got %d", ser.GetIndex())
	}

	// Set index
	err = ser.SetIndex(2)
	if err != nil {
		t.Fatalf("SetIndex failed: %v", err)
	}
	if ser.GetIndex() != 2 {
		t.Errorf("Expected index 2, got %d", ser.GetIndex())
	}
}

// ===== Error Handling Tests =====

func TestErrorTypeMismatch(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	_, err = ser.AddInt32(42)
	if err != nil {
		t.Fatalf("AddInt32 failed: %v", err)
	}

	ser.Reset()
	_, err = ser.ExtractString()
	if err == nil {
		t.Error("Expected error for type mismatch, got nil")
	}
	if !strings.Contains(err.Error(), "type mismatch") {
		t.Errorf("Expected 'type mismatch' error, got: %v", err)
	}
}

func TestErrorBoundsViolationSerialization(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(2)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	_, err = ser.AddInt32(1)
	if err != nil {
		t.Fatalf("First AddInt32 failed: %v", err)
	}
	_, err = ser.AddInt32(2)
	if err != nil {
		t.Fatalf("Second AddInt32 failed: %v", err)
	}

	// Third add should fail - bounds violation
	_, err = ser.AddInt32(3)
	if err == nil {
		t.Error("Expected error for bounds violation, got nil")
	}
	if !strings.Contains(err.Error(), "out of bounds") {
		t.Errorf("Expected 'out of bounds' error, got: %v", err)
	}
}

func TestErrorBoundsViolationDeserialization(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	_, err = ser.AddInt32(42)
	if err != nil {
		t.Fatalf("AddInt32 failed: %v", err)
	}

	ser.Reset()
	_, err = ser.ExtractInt32()
	if err != nil {
		t.Fatalf("First ExtractInt32 failed: %v", err)
	}

	// Second extract should fail - bounds violation
	_, err = ser.ExtractInt32()
	if err == nil {
		t.Error("Expected error for bounds violation, got nil")
	}
	if !strings.Contains(err.Error(), "out of bounds") {
		t.Errorf("Expected 'out of bounds' error, got: %v", err)
	}
}

func TestErrorPeekTypeBeyondBounds(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	_, err = ser.AddInt32(42)
	if err != nil {
		t.Fatalf("AddInt32 failed: %v", err)
	}

	// After adding one value, index is at 1
	// peek_type at index 1 should fail
	_, err = ser.PeekType()
	if err == nil {
		t.Error("Expected error for peek_type beyond bounds, got nil")
	}
	if !strings.Contains(err.Error(), "out of bounds") {
		t.Errorf("Expected 'out of bounds' error, got: %v", err)
	}
}

// ===== Deserialization-Only Tests (Pre-filled CDTS) =====

func TestDeserializePrefilledCDTSPrimitives(t *testing.T) {
	// Manually create CDTS (simulating MetaFFI call)
	cdts, err := NewCDTSGoSerializerFromSize(3)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	// Manually set values using test helper functions
	cdt0 := GetCDTAtIndex(cdts.cdts, 0)
	SetInt32Direct(cdt0, 42)

	cdt1 := GetCDTAtIndex(cdts.cdts, 1)
	SetString8Direct(cdt1, "test")

	cdt2 := GetCDTAtIndex(cdts.cdts, 2)
	SetFloat64Direct(cdt2, 3.14)

	// Deserialize only
	cdts.Reset()

	val1, err := cdts.ExtractInt32()
	if err != nil {
		t.Fatalf("ExtractInt32 failed: %v", err)
	}
	if val1 != 42 {
		t.Errorf("Expected 42, got %d", val1)
	}

	val2, err := cdts.ExtractString()
	if err != nil {
		t.Fatalf("ExtractString failed: %v", err)
	}
	if val2 != "test" {
		t.Errorf("Expected 'test', got %q", val2)
	}

	val3, err := cdts.ExtractFloat64()
	if err != nil {
		t.Fatalf("ExtractFloat64 failed: %v", err)
	}
	if val3 != 3.14 {
		t.Errorf("Expected 3.14, got %f", val3)
	}
}

func TestDeserializePrefilledCDTSNull(t *testing.T) {
	cdts, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	// Manually set null
	cdt := GetCDTAtIndex(cdts.cdts, 0)
	SetNullDirect(cdt)

	// Deserialize
	isNull, err := cdts.IsNull()
	if err != nil {
		t.Fatalf("IsNull failed: %v", err)
	}
	if !isNull {
		t.Error("Expected null, got non-null")
	}

	peekType, err := cdts.PeekType()
	if err != nil {
		t.Fatalf("PeekType failed: %v", err)
	}
	if peekType != MetaffiNullType() {
		t.Errorf("Expected null type, got %d", peekType)
	}
}

// ===== Rune Tests =====

func TestSerializeDeserializeRune(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	original := rune('A')
	_, err = ser.AddRune(original, MetaffiChar32Type())
	if err != nil {
		t.Fatalf("AddRune failed: %v", err)
	}

	ser.Reset()
	val, err := ser.ExtractRune()
	if err != nil {
		t.Fatalf("ExtractRune failed: %v", err)
	}
	if val != original {
		t.Errorf("Expected %c, got %c", original, val)
	}
}

func TestAddRuneRangeValidation(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	// Test rune out of range for char8
	_, err = ser.AddRune(300, MetaffiChar8Type())
	if err == nil {
		t.Error("Expected error for rune out of range, got nil")
	}
	if !strings.Contains(err.Error(), "out of range") {
		t.Errorf("Expected 'out of range' error, got: %v", err)
	}
}

// ===== Handle Tests =====

func TestSerializeDeserializeHandle(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	// Create a test value to point to
	testValue := 42
	originalHandle := uintptr(unsafe.Pointer(&testValue))
	originalRuntimeID := uint64(100)

	_, err = ser.AddHandle(originalHandle, originalRuntimeID)
	if err != nil {
		t.Fatalf("AddHandle failed: %v", err)
	}

	ser.Reset()
	info, err := ser.ExtractHandle()
	if err != nil {
		t.Fatalf("ExtractHandle failed: %v", err)
	}

	if info.Handle != originalHandle {
		t.Errorf("Expected handle %v, got %v", originalHandle, info.Handle)
	}
	if info.RuntimeID != originalRuntimeID {
		t.Errorf("Expected runtime ID %d, got %d", originalRuntimeID, info.RuntimeID)
	}

	// Verify we can dereference the handle
	dereferenced := *(*int)(unsafe.Pointer(info.Handle))
	if dereferenced != testValue {
		t.Errorf("Expected dereferenced value %d, got %d", testValue, dereferenced)
	}
}

// ===== Callable Tests =====

func TestSerializeDeserializeCallable(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	// Create a dummy function pointer
	dummyFunc := func() {}
	originalCallable := uintptr(unsafe.Pointer(&dummyFunc))
	// Use helper function to create type slices (avoids direct C type references)
	originalParamTypes := CreateParamTypesSlice(MetaffiInt32Type(), MetaffiString8Type())
	originalRetvalTypes := CreateRetvalTypesSlice(MetaffiFloat64Type())

	_, err = ser.AddCallable(originalCallable, originalParamTypes, originalRetvalTypes)
	if err != nil {
		t.Fatalf("AddCallable failed: %v", err)
	}

	ser.Reset()
	info, err := ser.ExtractCallable()
	if err != nil {
		t.Fatalf("ExtractCallable failed: %v", err)
	}

	if info.Callable != originalCallable {
		t.Errorf("Expected callable %v, got %v", originalCallable, info.Callable)
	}

	if len(info.ParamTypes) != len(originalParamTypes) {
		t.Fatalf("Expected %d parameter types, got %d", len(originalParamTypes), len(info.ParamTypes))
	}
	for i, pt := range originalParamTypes {
		if info.ParamTypes[i] != pt {
			t.Errorf("Parameter type %d: expected %d, got %d", i, pt, info.ParamTypes[i])
		}
	}

	if len(info.RetvalTypes) != len(originalRetvalTypes) {
		t.Fatalf("Expected %d return value types, got %d", len(originalRetvalTypes), len(info.RetvalTypes))
	}
	for i, rt := range originalRetvalTypes {
		if info.RetvalTypes[i] != rt {
			t.Errorf("Return value type %d: expected %d, got %d", i, rt, info.RetvalTypes[i])
		}
	}
}

func TestSerializeDeserializeCallableEmptyTypes(t *testing.T) {
	ser, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	dummyFunc := func() {}
	originalCallable := uintptr(unsafe.Pointer(&dummyFunc))
	// Use helper function to create empty slices (avoids direct C type references)
	originalParamTypes := CreateParamTypesSlice()
	originalRetvalTypes := CreateRetvalTypesSlice()

	_, err = ser.AddCallable(originalCallable, originalParamTypes, originalRetvalTypes)
	if err != nil {
		t.Fatalf("AddCallable failed: %v", err)
	}

	ser.Reset()
	info, err := ser.ExtractCallable()
	if err != nil {
		t.Fatalf("ExtractCallable failed: %v", err)
	}

	if len(info.ParamTypes) != 0 {
		t.Errorf("Expected 0 parameter types, got %d", len(info.ParamTypes))
	}
	if len(info.RetvalTypes) != 0 {
		t.Errorf("Expected 0 return value types, got %d", len(info.RetvalTypes))
	}
}

// ===== Comprehensive Deserialization-Only Tests =====

func TestDeserializePrefilledCDTSAllPrimitives(t *testing.T) {
	cdts, err := NewCDTSGoSerializerFromSize(11)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	// Manually set all primitive types
	SetInt8Direct(GetCDTAtIndex(cdts.cdts, 0), -10)
	SetInt16Direct(GetCDTAtIndex(cdts.cdts, 1), -1000)
	SetInt32Direct(GetCDTAtIndex(cdts.cdts, 2), -100000)
	SetInt64Direct(GetCDTAtIndex(cdts.cdts, 3), -10000000)
	SetUint8Direct(GetCDTAtIndex(cdts.cdts, 4), 10)
	SetUint16Direct(GetCDTAtIndex(cdts.cdts, 5), 1000)
	SetUint32Direct(GetCDTAtIndex(cdts.cdts, 6), 100000)
	SetUint64Direct(GetCDTAtIndex(cdts.cdts, 7), 10000000)
	SetFloat32Direct(GetCDTAtIndex(cdts.cdts, 8), 3.14)
	SetFloat64Direct(GetCDTAtIndex(cdts.cdts, 9), 2.71828)
	SetBoolDirect(GetCDTAtIndex(cdts.cdts, 10), true)

	cdts.Reset()

	val1, err := cdts.ExtractInt8()
	if err != nil {
		t.Fatalf("ExtractInt8 failed: %v", err)
	}
	if val1 != -10 {
		t.Errorf("Expected -10, got %d", val1)
	}

	val2, err := cdts.ExtractInt16()
	if err != nil {
		t.Fatalf("ExtractInt16 failed: %v", err)
	}
	if val2 != -1000 {
		t.Errorf("Expected -1000, got %d", val2)
	}

	val3, err := cdts.ExtractInt32()
	if err != nil {
		t.Fatalf("ExtractInt32 failed: %v", err)
	}
	if val3 != -100000 {
		t.Errorf("Expected -100000, got %d", val3)
	}

	val4, err := cdts.ExtractInt64()
	if err != nil {
		t.Fatalf("ExtractInt64 failed: %v", err)
	}
	if val4 != -10000000 {
		t.Errorf("Expected -10000000, got %d", val4)
	}

	val5, err := cdts.ExtractUint8()
	if err != nil {
		t.Fatalf("ExtractUint8 failed: %v", err)
	}
	if val5 != 10 {
		t.Errorf("Expected 10, got %d", val5)
	}

	val6, err := cdts.ExtractUint16()
	if err != nil {
		t.Fatalf("ExtractUint16 failed: %v", err)
	}
	if val6 != 1000 {
		t.Errorf("Expected 1000, got %d", val6)
	}

	val7, err := cdts.ExtractUint32()
	if err != nil {
		t.Fatalf("ExtractUint32 failed: %v", err)
	}
	if val7 != 100000 {
		t.Errorf("Expected 100000, got %d", val7)
	}

	val8, err := cdts.ExtractUint64()
	if err != nil {
		t.Fatalf("ExtractUint64 failed: %v", err)
	}
	if val8 != 10000000 {
		t.Errorf("Expected 10000000, got %d", val8)
	}

	val9, err := cdts.ExtractFloat32()
	if err != nil {
		t.Fatalf("ExtractFloat32 failed: %v", err)
	}
	if val9 != 3.14 {
		t.Errorf("Expected 3.14, got %f", val9)
	}

	val10, err := cdts.ExtractFloat64()
	if err != nil {
		t.Fatalf("ExtractFloat64 failed: %v", err)
	}
	if val10 != 2.71828 {
		t.Errorf("Expected 2.71828, got %f", val10)
	}

	val11, err := cdts.ExtractBool()
	if err != nil {
		t.Fatalf("ExtractBool failed: %v", err)
	}
	if !val11 {
		t.Error("Expected true, got false")
	}
}

func TestDeserializePrefilledCDTSAllStrings(t *testing.T) {
	cdts, err := NewCDTSGoSerializerFromSize(3)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	SetString8Direct(GetCDTAtIndex(cdts.cdts, 0), "Hello UTF-8")
	SetString16Direct(GetCDTAtIndex(cdts.cdts, 1), "Hello UTF-16 מה שלומך")
	SetString32Direct(GetCDTAtIndex(cdts.cdts, 2), "Hello UTF-32 🚀")

	cdts.Reset()

	val1, err := cdts.ExtractString()
	if err != nil {
		t.Fatalf("ExtractString failed: %v", err)
	}
	if val1 != "Hello UTF-8" {
		t.Errorf("Expected 'Hello UTF-8', got %q", val1)
	}

	val2, err := cdts.ExtractString16()
	if err != nil {
		t.Fatalf("ExtractString16 failed: %v", err)
	}
	if val2 != "Hello UTF-16 מה שלומך" {
		t.Errorf("Expected 'Hello UTF-16 מה שלומך', got %q", val2)
	}

	val3, err := cdts.ExtractString32()
	if err != nil {
		t.Fatalf("ExtractString32 failed: %v", err)
	}
	if val3 != "Hello UTF-32 🚀" {
		t.Errorf("Expected 'Hello UTF-32 🚀', got %q", val3)
	}
}

func TestDeserializePrefilledCDTSHandle(t *testing.T) {
	cdts, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	testValue := 99
	handlePtr := uintptr(unsafe.Pointer(&testValue))
	runtimeID := uint64(123)

	SetHandleDirect(GetCDTAtIndex(cdts.cdts, 0), handlePtr, runtimeID)

	info, err := cdts.ExtractHandle()
	if err != nil {
		t.Fatalf("ExtractHandle failed: %v", err)
	}

	if info.Handle != handlePtr {
		t.Errorf("Expected handle %v, got %v", handlePtr, info.Handle)
	}
	if info.RuntimeID != runtimeID {
		t.Errorf("Expected runtime ID %d, got %d", runtimeID, info.RuntimeID)
	}

	// Verify dereferenced value
	dereferenced := *(*int)(unsafe.Pointer(info.Handle))
	if dereferenced != testValue {
		t.Errorf("Expected dereferenced value %d, got %d", testValue, dereferenced)
	}
}

func TestDeserializePrefilledCDTSCallable(t *testing.T) {
	cdts, err := NewCDTSGoSerializerFromSize(1)
	if err != nil {
		t.Fatalf("NewCDTSGoSerializerFromSize failed: %v", err)
	}

	dummyFunc := func() {}
	callablePtr := uintptr(unsafe.Pointer(&dummyFunc))
	// Use helper function to create type slices (avoids direct C type references)
	paramTypes := CreateParamTypesSlice(MetaffiInt32Type(), MetaffiBoolType())
	retvalTypes := CreateRetvalTypesSlice(MetaffiString8Type())

	SetCallableDirect(GetCDTAtIndex(cdts.cdts, 0), callablePtr, paramTypes, retvalTypes)

	info, err := cdts.ExtractCallable()
	if err != nil {
		t.Fatalf("ExtractCallable failed: %v", err)
	}

	if info.Callable != callablePtr {
		t.Errorf("Expected callable %v, got %v", callablePtr, info.Callable)
	}

	if len(info.ParamTypes) != len(paramTypes) {
		t.Fatalf("Expected %d parameter types, got %d", len(paramTypes), len(info.ParamTypes))
	}
	for i, pt := range paramTypes {
		if info.ParamTypes[i] != pt {
			t.Errorf("Parameter type %d: expected %d, got %d", i, pt, info.ParamTypes[i])
		}
	}

	if len(info.RetvalTypes) != len(retvalTypes) {
		t.Fatalf("Expected %d return value types, got %d", len(retvalTypes), len(info.RetvalTypes))
	}
	for i, rt := range retvalTypes {
		if info.RetvalTypes[i] != rt {
			t.Errorf("Return value type %d: expected %d, got %d", i, rt, info.RetvalTypes[i])
		}
	}
}
