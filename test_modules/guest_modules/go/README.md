# Go Guest Test Module

This guest module exercises Go language features and MetaFFI entity coverage.

Contents:
- `*.go`: Go sources in the module root.
- `sub/`: subpackage to validate multi-package modules.
- `third_parties.md`: optional third-party tests to be invoked by hosts.
- `special_primitives.md`: Go types that may not map directly to MetaFFI.

Build artifacts:
- This module does not produce an executable. MetaFFI will generate the guest artifacts later.
- `test_bin/` is created by CMake to keep artifact locations consistent.
