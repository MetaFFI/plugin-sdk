# Special Primitives (Go)

These tests exercise Go types that are not directly mapped to MetaFFI types.
They should be invoked by host languages directly to ensure correct handling or clear failure modes.

## map
- Return a `map[string]any` with mixed values.

## set
- Represent a set as `map[int]struct{}` and validate membership.

## chan
- Return a `chan int` and validate send/receive behavior.

## func
- Return a function value and invoke it from host (if supported).

## complex
- Return `complex64` and `complex128` values.

## rune
- Return and validate `rune` (alias for int32).

## unsafe.Pointer
- Only if MetaFFI supports it; otherwise validate expected failure.
