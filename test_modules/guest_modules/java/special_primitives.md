# Special Primitives (Java)

These tests exercise Java collections/types that are not directly mapped to MetaFFI types.
They should be invoked by host languages directly to ensure correct handling or clear failure modes.

## Map
- Return a `Map<String, Object>` with mixed values.

## List
- Return a `List<Object>` with mixed values.

## Set
- Return a `Set<Integer>` and validate membership.

## Optional
- Return `Optional<String>` for present/empty cases.

## BigInteger / BigDecimal
- Return `BigInteger` and `BigDecimal` values for precision handling.
