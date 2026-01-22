# Guest Test Modules - Expectations

This document defines the expectations for future guest test modules. It is language-agnostic and is meant to guide developers/agents so guest modules are consistent, comparable, and comprehensive across languages.

The goal is to provide diverse modules that exercise MetaFFI features and every available language feature, while keeping a core set of scenarios aligned across languages.

## Structure
- Each supported language should have a guest module in `sdk/test_modules/guest_modules/<language>/`.
- The guest module can be a single file or multiple files, based on what is idiomatic for that language (e.g., Python modules vs. Java packages).
- The module must include both source code and generated executable artifacts (dynamic library / jar / py or pyc / etc.).
- A `CMakeLists.txt` must be provided per language module, and must be included by the parent `CMakeLists.txt`.
  - Use the CMake helper scripts under `cmake/` (already available in the repo).

## Mandatory coverage (all languages)
Each language module must cover ALL relevant language features that MetaFFI is expected to support for that language. If the language has a feature, it should be represented in the guest module. The API does not need exact names, but it must map cleanly to the same test logic.

### Functions
- No-arg, no-return function (side effects like printing/logging).
- Function returning a value (integer math).
- Function receiving an array/list and returning an aggregated/transformed value.
- Function that raises or returns an error (exception/panic/error result).
- Function that accepts a callback and validates the result.
- Function that returns a callback (if supported).

### Globals / Constants
- Read-only constant (e.g., a number).
- Mutable global or module state (if applicable).

### Classes / Objects
- Class/struct with constructor and fields.
- Instance methods (read/write state).
- Static/class methods if supported.
- Object handles passed across the boundary.
- Destructor/finalizer behavior if the language supports it.

### Primitive and non-primitive types
All primitive types of the language should be represented at least once:
- Integers (signed/unsigned, different widths).
- Floats/doubles.
- Boolean.
- Char/byte if applicable.
- String.

Non-primitives:
- Arrays/slices/lists.
- Maps/dictionaries.
- Bytes/buffers.
- Enums (if supported).
- Structs/records/tuples.

### Arrays and collections
- 1D array.
- 2D array.
- 3D array.
- Ragged (jagged) arrays.
- Collections of objects (array/list of handles).

### Error handling
- Throwing exceptions / panic.
- Returning error values (if idiomatic).
- Errors from callbacks (callback throws/returns error).

### Language-specific features (required when supported)
Include features that exist in the language, mapped to the same logical test goals:
- Overloading (functions/methods).
- Named or default arguments.
- Variadic arguments.
- Nested classes or types.
- Interfaces/traits and implementations.
- Generics/templates.
- Operator overloading (if supported).
- Modules/packages/namespaces (if supported).

## Dependencies
If a module needs external dependencies (packages, jars, modules), they must be declared in the language's standard way (e.g., go.mod, requirements, Maven/Gradle).
When dependencies are optional or not installed by default, document host-invoked tests in a `third_parties.md` file so hosts can load them directly.

## Special primitives
Some languages expose primitives/collections that do not map cleanly to MetaFFI types (e.g., Python `dict`, `set`, `tuple`, `frozenset`, `complex`).
Document host-invoked tests for these in a `special_primitives.md` file per language, so hosts can validate behavior or expected failure modes.

## Build and artifacts
- Each guest module must be buildable via the module's `CMakeLists.txt`.
- The build should produce the executable artifacts needed by tests.
- The output must remain in the repo (no CI).
- Built artifacts should be placed under `sdk/test_modules/guest_modules/<language>/test_bin`.
- For interpreted languages, place only compiled artifacts in `test_bin` (e.g., Python `.pyc`), while keeping source files in the module directory.

## Tests
- Provide a unit-test that validates the entities exposed by the module:
  - Functions, globals/constants, classes, methods, constructors/destructors, fields.
  - Types and signatures behave as expected.
  - Errors surface correctly.
- The test can be written in the host language (or the guest language if needed), but must validate the MetaFFI-visible surface.

## Alignment notes
- Keep the logical scenarios aligned across languages, even if names differ.
- Use each language's naming conventions and idioms.
- Prefer clarity and coverage over minimal code size.
