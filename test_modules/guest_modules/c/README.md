# C Guest Test Module

This guest module exercises C language features and MetaFFI entity coverage.

Contents:
- `include/`: exported C API headers.
- `src/`: implementation for the shared library.
- `tests/`: doctest unit tests (C++ test runner).
- `third_parties.md`: optional third-party tests to be invoked by hosts.
- `special_primitives.md`: C types that may not map directly to MetaFFI.

Build artifacts:
- Shared library in `test_bin/`.
- Exported headers copied to `test_bin/include/`.
