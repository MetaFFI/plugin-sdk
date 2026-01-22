# Special Primitives (Python)

These tests exercise Python primitives and collections that are not directly mapped to MetaFFI types.
They should be invoked by host languages directly to ensure correct handling or clear failure modes.

## dict
- Create and return a dictionary with mixed value types.
- Example target:
  - `return {"a": 1, "b": "two", "c": [3, 4]}`
- Host should validate key access and value types.

## set
- Create and return a set of integers.
- Example target:
  - `return {1, 2, 3}`
- Host should validate membership and size.

## tuple
- Create and return a tuple with mixed values.
- Example target:
  - `return (1, "two", 3.0)`
- Host should validate order and types.

## frozenset
- Create and return a frozenset.
- Example target:
  - `return frozenset([1, 2, 3])`
- Host should validate membership and size.

## complex
- Return a complex number.
- Example target:
  - `return 3 + 4j`
- Host should validate real/imag parts or stringify if needed.
