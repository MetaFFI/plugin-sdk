# Special Primitives (C++)

These tests exercise C++ types that may not map directly to MetaFFI types.
They should be invoked by host languages directly to ensure correct handling or clear failure modes.

## std::any
- Return and accept `std::any` with multiple contained types.

## std::variant
- Return a `std::variant` holding different primitive/object types.

## std::optional
- Return present/empty optional values.

## std::tuple
- Return tuples with mixed values.

## std::function
- Return/accept `std::function` callbacks.

## std::unique_ptr / std::shared_ptr
- Return smart pointers and validate ownership behavior.
