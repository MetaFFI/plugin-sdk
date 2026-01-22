# Special Primitives (C)

These tests exercise C types that may not map directly to MetaFFI types.
They should be invoked by host languages directly to ensure correct handling or clear failure modes.

## void*
- Return and accept `void*` handles to opaque structs.

## function pointers
- Return/accept function pointers (if supported).

## bitfields
- Structs with bitfields for packing behavior.

## flexible array members
- Structs with flexible array members (if supported).
