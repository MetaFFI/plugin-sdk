# MetaFFI Runtime Manager Test Documentation

**Version:** 1.0  
**Purpose:** Comprehensive test plan for MetaFFI runtime_manager implementations

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Test Categories](#2-test-categories)
3. [Runtime Lifecycle Tests](#3-runtime-lifecycle-tests)
4. [Module Loading Tests](#4-module-loading-tests)
5. [Entity Loading Tests](#5-entity-loading-tests)
6. [Entity Execution Tests](#6-entity-execution-tests)
7. [Caching Tests](#7-caching-tests)
8. [Thread Safety Tests](#8-thread-safety-tests)
9. [Memory Management Tests](#9-memory-management-tests)
10. [Error Handling Tests](#10-error-handling-tests)
11. [Integration Tests](#11-integration-tests)
12. [Runtime-Specific Tests](#12-runtime-specific-tests)

---

## 1. Introduction

### 1.1 Purpose

This document defines comprehensive test requirements for all runtime_manager implementations. These tests ensure:

- Consistent behavior across all runtime implementations
- Full coverage of functionality
- Thread safety verification
- Memory leak detection
- Proper error handling
- Correct caching behavior

### 1.2 Test Implementation Requirements

**All tests must be implemented** for each runtime_manager implementation. Tests should:

- Be automated and repeatable
- Provide clear pass/fail results
- Include meaningful error messages
- Cover both success and failure cases
- Test edge cases and boundary conditions

**Note**: runtime_manager APIs use runtime-native types and throw exceptions (no `char** out_err` and no CDTS serialization inside runtime_manager). Update any steps that mention CDTS/out_err to use native-type calls and exception checks.

### 1.3 Test Organization

Tests should be organized by category and clearly named. Each test should:

- Have a descriptive name indicating what it tests
- Be independent (can run in isolation)
- Clean up after itself
- Not depend on test execution order

---

## 2. Test Categories

The test suite is organized into the following categories:

1. **Runtime Lifecycle Tests** - Runtime initialization and shutdown
2. **Module Loading Tests** - Module loading and management
3. **Entity Loading Tests** - Entity discovery and loading
4. **Entity Execution Tests** - Calling entities and accessing variables
5. **Caching Tests** - Cache behavior and invalidation
6. **Thread Safety Tests** - Concurrent access and operations
7. **Memory Management Tests** - Resource cleanup and leak detection
8. **Error Handling Tests** - Error conditions and reporting
9. **Integration Tests** - End-to-end scenarios
10. **Runtime-Specific Tests** - Language-specific functionality

---

## 3. Runtime Lifecycle Tests

### 3.1 Load Runtime - Success

**Purpose**: Verify runtime loads successfully.

**Steps**:
1. Create RuntimeManager instance
2. Call `load_runtime(&err)`
3. Verify `err == nullptr`
4. Verify `is_runtime_loaded() == true`

**Expected**: Runtime loads without error.

### 3.2 Load Runtime - Idempotency

**Purpose**: Verify multiple load calls are idempotent.

**Steps**:
1. Create RuntimeManager instance
2. Call `load_runtime(&err)` multiple times (e.g., 5 times)
3. Verify `err == nullptr` for all calls
4. Verify `is_runtime_loaded() == true`

**Expected**: All calls succeed, runtime remains loaded.

### 3.3 Release Runtime - Success

**Purpose**: Verify runtime releases successfully.

**Steps**:
1. Create RuntimeManager instance
2. Call `load_runtime(&err)`
3. Call `release_runtime(&err)`
4. Verify `err == nullptr`
5. Verify `is_runtime_loaded() == false`

**Expected**: Runtime releases without error.

### 3.4 Release Runtime - Idempotency

**Purpose**: Verify multiple release calls are idempotent.

**Steps**:
1. Create RuntimeManager instance
2. Call `load_runtime(&err)`
3. Call `release_runtime(&err)` multiple times (e.g., 5 times)
4. Verify `err == nullptr` for all calls
5. Verify `is_runtime_loaded() == false`

**Expected**: All calls succeed, runtime remains released.

### 3.5 Release Runtime Without Load

**Purpose**: Verify releasing without loading doesn't cause errors.

**Steps**:
1. Create RuntimeManager instance (don't load)
2. Call `release_runtime(&err)`
3. Verify `err == nullptr` (or appropriate error handling)

**Expected**: No crash, graceful handling.

### 3.6 Load After Release

**Purpose**: Verify runtime can be loaded again after release.

**Steps**:
1. Create RuntimeManager instance
2. Call `load_runtime(&err)`
3. Call `release_runtime(&err)`
4. Call `load_runtime(&err)` again
5. Verify `err == nullptr`
6. Verify `is_runtime_loaded() == true`

**Expected**: Runtime can be reloaded after release.

### 3.7 Destructor Cleanup

**Purpose**: Verify destructor releases runtime.

**Steps**:
1. Create RuntimeManager instance in scope
2. Call `load_runtime(&err)`
3. Let RuntimeManager go out of scope (destructor called)
4. Verify no resources leaked

**Expected**: Destructor properly releases runtime.

---

## 4. Module Loading Tests

### 4.1 Load Valid Module

**Purpose**: Verify loading a valid module succeeds.

**Steps**:
1. Create RuntimeManager and load runtime
2. Call `load_module(valid_module_path, &err)`
3. Verify `err == nullptr`
4. Verify returned Module is not `nullptr`
5. Verify `module->get_module_path() == valid_module_path`

**Expected**: Module loads successfully.

### 4.2 Load Invalid Module Path

**Purpose**: Verify loading invalid module path fails gracefully.

**Steps**:
1. Create RuntimeManager and load runtime
2. Call `load_module("/invalid/path/module", &err)`
3. Verify `err != nullptr`
4. Verify returned Module is `nullptr`
5. Verify error message is descriptive

**Expected**: Error reported, no crash.

### 4.3 Load Module Without Runtime

**Purpose**: Verify loading module without loaded runtime fails.

**Steps**:
1. Create RuntimeManager (don't load runtime)
2. Call `load_module(module_path, &err)`
3. Verify `err != nullptr` or runtime is auto-loaded (implementation-specific)
4. Verify appropriate error handling

**Expected**: Error or auto-load (document behavior).

### 4.4 Load Same Module Multiple Times

**Purpose**: Verify module caching works.

**Steps**:
1. Create RuntimeManager and load runtime
2. Call `load_module(module_path, &err)` first time
3. Store first Module pointer
4. Call `load_module(module_path, &err)` second time
5. Verify both calls return same Module instance (shared_ptr comparison)
6. Verify `err == nullptr` for both calls

**Expected**: Same Module instance returned (cached).

### 4.5 Load Different Modules

**Purpose**: Verify multiple different modules can be loaded.

**Steps**:
1. Create RuntimeManager and load runtime
2. Load module1: `load_module(path1, &err)`
3. Load module2: `load_module(path2, &err)`
4. Verify both modules load successfully
5. Verify modules are different instances
6. Verify both modules remain valid

**Expected**: Multiple modules can coexist.

### 4.6 Module Path Variations

**Purpose**: Verify different module path formats work.

**Test Cases**:
- Absolute file path
- Relative file path (if supported)
- Module/package name (if supported)
- Path with spaces/special characters
- Path normalization (if applicable)

**Expected**: All supported path formats work correctly.

### 4.7 Module Unload

**Purpose**: Verify module can be unloaded.

**Steps**:
1. Create RuntimeManager and load runtime
2. Load module and load an entity from it
3. Call `module->unload(&err)`
4. Verify `err == nullptr`
5. Verify entities are released

**Expected**: Module unloads successfully, entities released.

### 4.8 Module Unload Without Entities

**Purpose**: Verify unloading module without entities works.

**Steps**:
1. Create RuntimeManager and load runtime
2. Load module (don't load any entities)
3. Call `module->unload(&err)`
4. Verify `err == nullptr`

**Expected**: Unload succeeds even without entities.

---

## 5. Entity Loading Tests

### 5.1 Load Function Entity

**Purpose**: Verify loading a function entity succeeds.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Call `module->load_entity("callable=my_function", params_types, retval_types, &err)`
3. Verify `err == nullptr`
4. Verify returned Entity is not `nullptr`
5. Verify entity is callable (has `call()` method)

**Expected**: Function entity loads successfully.

### 5.2 Load Method Entity

**Purpose**: Verify loading a method entity succeeds.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Call `module->load_entity("callable=MyClass.myMethod,instance_required", params_types, retval_types, &err)`
3. Verify `err == nullptr`
4. Verify returned Entity is not `nullptr`
5. Verify entity is callable

**Expected**: Method entity loads successfully.

### 5.3 Load Constructor Entity

**Purpose**: Verify loading a constructor entity succeeds.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Call `module->load_entity("callable=MyClass.__init__", params_types, retval_types, &err)` (Python) or equivalent
3. Verify `err == nullptr`
4. Verify returned Entity is not `nullptr`
5. Verify entity is callable

**Expected**: Constructor entity loads successfully.

### 5.4 Load Global Getter Entity

**Purpose**: Verify loading a global variable getter succeeds.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Call `module->load_entity("global=myGlobal,getter", params_types, retval_types, &err)` (Go) or equivalent
3. Verify `err == nullptr`
4. Verify returned Entity is not `nullptr`
5. Verify entity has `get()` method

**Expected**: Global getter entity loads successfully.

### 5.5 Load Global Setter Entity

**Purpose**: Verify loading a global variable setter succeeds.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Call `module->load_entity("global=myGlobal,setter", params_types, retval_types, &err)` (Go) or equivalent
3. Verify `err == nullptr`
4. Verify returned Entity is not `nullptr`
5. Verify entity has `set()` method

**Expected**: Global setter entity loads successfully.

### 5.6 Load Field Getter Entity

**Purpose**: Verify loading a field getter succeeds.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Call `module->load_entity("class=MyClass,field=myField,getter,instance_required", params_types, retval_types, &err)` (Java) or equivalent
3. Verify `err == nullptr`
4. Verify returned Entity is not `nullptr`
5. Verify entity has `get()` method

**Expected**: Field getter entity loads successfully.

### 5.7 Load Field Setter Entity

**Purpose**: Verify loading a field setter succeeds.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Call `module->load_entity("class=MyClass,field=myField,setter,instance_required", params_types, retval_types, &err)` (Java) or equivalent
3. Verify `err == nullptr`
4. Verify returned Entity is not `nullptr`
5. Verify entity has `set()` method

**Expected**: Field setter entity loads successfully.

### 5.8 Load Entity with Correct Types

**Purpose**: Verify loading entity with matching type signature succeeds.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Determine correct parameter and return types for entity
3. Call `module->load_entity(entity_path, correct_params, correct_retvals, &err)`
4. Verify `err == nullptr`
5. Verify entity loads successfully

**Expected**: Entity loads with correct types.

### 5.9 Load Entity with Incorrect Types

**Purpose**: Verify loading entity with mismatched types fails.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Determine correct types for entity
3. Use incorrect types (wrong count, wrong types)
4. Call `module->load_entity(entity_path, incorrect_params, incorrect_retvals, &err)`
5. Verify `err != nullptr`
6. Verify returned Entity is `nullptr`
7. Verify error message indicates type mismatch

**Expected**: Error reported for type mismatch.

### 5.10 Load Non-Existent Entity

**Purpose**: Verify loading non-existent entity fails.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Call `module->load_entity("callable=nonexistent_function", params_types, retval_types, &err)`
3. Verify `err != nullptr`
4. Verify returned Entity is `nullptr`
5. Verify error message indicates entity not found

**Expected**: Error reported for non-existent entity.

### 5.11 Load Same Entity Multiple Times

**Purpose**: Verify entity caching works.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Load entity first time: `load_entity(entity_path, types1, types2, &err)`
3. Store first Entity pointer
4. Load entity second time with same types: `load_entity(entity_path, types1, types2, &err)`
5. Verify both calls return same Entity instance (shared_ptr comparison)
6. Verify `err == nullptr` for both calls

**Expected**: Same Entity instance returned (cached).

### 5.12 Load Same Entity with Different Types

**Purpose**: Verify same entity with different types creates separate instances.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Load entity with types1: `load_entity(entity_path, types1, retvals1, &err)`
3. Load entity with types2: `load_entity(entity_path, types2, retvals2, &err)`
4. Verify both entities load successfully
5. Verify entities are different instances (overloading support)

**Expected**: Different Entity instances for different type signatures.

### 5.13 Load Entity with Invalid Entity Path

**Purpose**: Verify invalid entity path format fails.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Call `module->load_entity("invalid_format", params_types, retval_types, &err)`
3. Verify `err != nullptr`
4. Verify returned Entity is `nullptr`
5. Verify error message indicates invalid format

**Expected**: Error reported for invalid entity path.

### 5.14 Load Entity from Unloaded Module

**Purpose**: Verify loading entity after module unload fails appropriately.

**Steps**:
1. Create RuntimeManager, load runtime, load module
2. Call `module->unload(&err)`
3. Try to load entity: `module->load_entity(entity_path, params_types, retval_types, &err)`
4. Verify appropriate error handling (error or module reload)

**Expected**: Error or module reload (document behavior).

---

## 6. Entity Execution Tests

### 6.1 Call Function with Parameters and Return Value

**Purpose**: Verify calling a function with parameters and return value works.

**Steps**:
1. Load function entity
2. Prepare parameters in CDTS format
3. Prepare return value CDTS structure
4. Call `entity->call(params_cdts, retvals_cdts, &err)`
5. Verify `err == nullptr`
6. Verify return value is correct

**Expected**: Function executes correctly, return value is correct.

### 6.2 Call Function with No Parameters

**Purpose**: Verify calling function without parameters works.

**Steps**:
1. Load function entity (no parameters)
2. Call `entity->call(nullptr, retvals_cdts, &err)`
3. Verify `err == nullptr`
4. Verify return value is correct

**Expected**: Function executes correctly.

### 6.3 Call Function with No Return Value

**Purpose**: Verify calling function without return value works.

**Steps**:
1. Load function entity (void return)
2. Prepare parameters in CDTS format
3. Call `entity->call(params_cdts, nullptr, &err)`
4. Verify `err == nullptr`

**Expected**: Function executes correctly.

### 6.4 Call Function with No Parameters and No Return Value

**Purpose**: Verify calling void function works.

**Steps**:
1. Load function entity (void void)
2. Call `entity->call(nullptr, nullptr, &err)`
3. Verify `err == nullptr`

**Expected**: Function executes correctly.

### 6.5 Call Method - Instance Method

**Purpose**: Verify calling instance method works.

**Steps**:
1. Load method entity (instance_required)
2. Prepare parameters including instance handle as first parameter
3. Call `entity->call(params_cdts, retvals_cdts, &err)`
4. Verify `err == nullptr`
5. Verify return value is correct

**Expected**: Instance method executes correctly.

### 6.6 Call Method - Static Method

**Purpose**: Verify calling static method works.

**Steps**:
1. Load method entity (not instance_required)
2. Prepare parameters (no instance handle)
3. Call `entity->call(params_cdts, retvals_cdts, &err)`
4. Verify `err == nullptr`
5. Verify return value is correct

**Expected**: Static method executes correctly.

### 6.7 Call Constructor

**Purpose**: Verify calling constructor creates instance.

**Steps**:
1. Load constructor entity
2. Prepare constructor parameters
3. Call `entity->call(params_cdts, retvals_cdts, &err)`
4. Verify `err == nullptr`
5. Verify return value is instance handle

**Expected**: Constructor creates instance, returns handle.

### 6.8 Call with Various Parameter Types

**Purpose**: Verify calling with different parameter types works.

**Test Cases**:
- Primitive types (int8, int16, int32, int64, uint8, uint16, uint32, uint64)
- Floating point types (float32, float64)
- String types (string8, string16, string32)
- Boolean type
- Handle type
- Array types (1D, 2D, etc.)
- Mixed parameter types

**Expected**: All parameter types handled correctly.

### 6.9 Call with Various Return Types

**Purpose**: Verify different return value types work.

**Test Cases**:
- Primitive return types
- Floating point return types
- String return types
- Boolean return type
- Handle return type
- Array return types
- Multiple return values (if supported)

**Expected**: All return types handled correctly.

### 6.10 Call with Wrong Parameter Count

**Purpose**: Verify calling with wrong parameter count fails.

**Steps**:
1. Load function entity (e.g., expects 2 parameters)
2. Prepare CDTS with wrong count (e.g., 3 parameters)
3. Call `entity->call(params_cdts, retvals_cdts, &err)`
4. Verify `err != nullptr`
5. Verify error message indicates parameter count mismatch

**Expected**: Error reported for parameter count mismatch.

### 6.11 Call with Wrong Parameter Types

**Purpose**: Verify calling with wrong parameter types fails.

**Steps**:
1. Load function entity (e.g., expects int32)
2. Prepare CDTS with wrong type (e.g., string8)
3. Call `entity->call(params_cdts, retvals_cdts, &err)`
4. Verify `err != nullptr`
5. Verify error message indicates type mismatch

**Expected**: Error reported for type mismatch.

### 6.12 Get Global Variable

**Purpose**: Verify reading global variable works.

**Steps**:
1. Load global getter entity
2. Prepare return value CDTS structure
3. Call `entity->get(retval_cdts, &err)`
4. Verify `err == nullptr`
5. Verify value is correct

**Expected**: Global variable value retrieved correctly.

### 6.13 Set Global Variable

**Purpose**: Verify writing global variable works.

**Steps**:
1. Load global setter entity
2. Prepare value in CDTS format
3. Call `entity->set(value_cdts, &err)`
4. Verify `err == nullptr`
5. Verify value was set (read back and verify)

**Expected**: Global variable value set correctly.

### 6.14 Get Field

**Purpose**: Verify reading field works.

**Steps**:
1. Load field getter entity
2. Prepare parameters (instance handle if instance_required)
3. Prepare return value CDTS structure
4. Call `entity->get(retval_cdts, &err)` (or call with instance if required)
5. Verify `err == nullptr`
6. Verify value is correct

**Expected**: Field value retrieved correctly.

### 6.15 Set Field

**Purpose**: Verify writing field works.

**Steps**:
1. Load field setter entity
2. Prepare parameters (instance handle if instance_required, value)
3. Prepare value in CDTS format
4. Call `entity->set(value_cdts, &err)` (or call with instance if required)
5. Verify `err == nullptr`
6. Verify value was set (read back and verify)

**Expected**: Field value set correctly.

### 6.16 Call with Null Pointers

**Purpose**: Verify calling with null pointers fails gracefully.

**Test Cases**:
- `call(nullptr, retvals, &err)` when parameters expected
- `call(params, nullptr, &err)` when return value expected
- `get(nullptr, &err)`
- `set(nullptr, &err)`

**Expected**: Errors reported for null pointers.

---

## 7. Caching Tests

### 7.1 Module Cache Hit

**Purpose**: Verify module cache returns cached instance.

**Steps**:
1. Create RuntimeManager and load runtime
2. Load module first time: `load_module(path, &err)`
3. Store Module pointer
4. Load module second time: `load_module(path, &err)`
5. Verify both pointers point to same instance
6. Verify module was not reloaded (check side effects if any)

**Expected**: Same Module instance returned from cache.

### 7.2 Module Cache Miss

**Purpose**: Verify different module paths create different instances.

**Steps**:
1. Create RuntimeManager and load runtime
2. Load module1: `load_module(path1, &err)`
3. Load module2: `load_module(path2, &err)`
4. Verify modules are different instances

**Expected**: Different Module instances for different paths.

### 7.3 Entity Cache Hit

**Purpose**: Verify entity cache returns cached instance.

**Steps**:
1. Load module
2. Load entity first time: `load_entity(path, types1, types2, &err)`
3. Store Entity pointer
4. Load entity second time with same types: `load_entity(path, types1, types2, &err)`
5. Verify both pointers point to same instance

**Expected**: Same Entity instance returned from cache.

### 7.4 Entity Cache Miss - Different Types

**Purpose**: Verify different type signatures create different instances.

**Steps**:
1. Load module
2. Load entity with types1: `load_entity(path, types1, retvals1, &err)`
3. Load entity with types2: `load_entity(path, types2, retvals2, &err)`
4. Verify entities are different instances

**Expected**: Different Entity instances for different types.

### 7.5 Cache Invalidation - Release Runtime

**Purpose**: Verify cache is cleared on runtime release.

**Steps**:
1. Create RuntimeManager and load runtime
2. Load module and entity
3. Call `release_runtime(&err)`
4. Load module again (should reload, not use cache)
5. Verify new Module instance

**Expected**: Cache cleared, module reloaded.

### 7.6 Cache Invalidation - Module Unload

**Purpose**: Verify entity cache is cleared on module unload.

**Steps**:
1. Load module and entity
2. Store Entity pointer
3. Call `module->unload(&err)`
4. Load entity again (should reload, not use cache)
5. Verify new Entity instance

**Expected**: Entity cache cleared, entity reloaded.

### 7.7 Cache Persistence Across Calls

**Purpose**: Verify cache persists across multiple calls.

**Steps**:
1. Load module
2. Load entity multiple times (e.g., 10 times)
3. Verify all calls return same Entity instance
4. Verify entity still works correctly

**Expected**: Cache persists, same instance returned.

---

## 8. Thread Safety Tests

### 8.1 Concurrent Runtime Load

**Purpose**: Verify concurrent runtime loads are safe.

**Steps**:
1. Create RuntimeManager instance
2. Spawn multiple threads (e.g., 10 threads)
3. Each thread calls `load_runtime(&err)`
4. Wait for all threads
5. Verify all calls succeed
6. Verify `is_runtime_loaded() == true`
7. Verify no crashes or deadlocks

**Expected**: All threads succeed, runtime loaded once.

### 8.2 Concurrent Runtime Release

**Purpose**: Verify concurrent runtime releases are safe.

**Steps**:
1. Create RuntimeManager and load runtime
2. Spawn multiple threads (e.g., 10 threads)
3. Each thread calls `release_runtime(&err)`
4. Wait for all threads
5. Verify all calls succeed
6. Verify `is_runtime_loaded() == false`
7. Verify no crashes or deadlocks

**Expected**: All threads succeed, runtime released.

### 8.3 Concurrent Module Load

**Purpose**: Verify concurrent module loads are safe.

**Steps**:
1. Create RuntimeManager and load runtime
2. Spawn multiple threads (e.g., 10 threads)
3. Each thread calls `load_module(same_path, &err)`
4. Wait for all threads
5. Verify all calls succeed
6. Verify all threads get same Module instance (cache works)
7. Verify no crashes or deadlocks

**Expected**: All threads succeed, same Module instance.

### 8.4 Concurrent Entity Load

**Purpose**: Verify concurrent entity loads are safe.

**Steps**:
1. Load module
2. Spawn multiple threads (e.g., 10 threads)
3. Each thread calls `load_entity(same_path, same_types, &err)`
4. Wait for all threads
5. Verify all calls succeed
6. Verify all threads get same Entity instance (cache works)
7. Verify no crashes or deadlocks

**Expected**: All threads succeed, same Entity instance.

### 8.5 Concurrent Entity Calls

**Purpose**: Verify concurrent entity calls are safe.

**Steps**:
1. Load callable entity
2. Spawn multiple threads (e.g., 10 threads)
3. Each thread calls `entity->call(params, retvals, &err)` with different parameters
4. Wait for all threads
5. Verify all calls succeed
6. Verify return values are correct
7. Verify no crashes or deadlocks

**Expected**: All threads succeed, correct return values.

### 8.6 Concurrent Get/Set Operations

**Purpose**: Verify concurrent get/set operations are safe.

**Steps**:
1. Load variable entity (global or field)
2. Spawn multiple threads (e.g., 5 getter threads, 5 setter threads)
3. Getter threads call `entity->get(retval, &err)`
4. Setter threads call `entity->set(value, &err)`
5. Wait for all threads
6. Verify operations complete without crashes
7. Verify final value is consistent

**Expected**: All operations complete, no crashes, consistent state.

### 8.7 Mixed Concurrent Operations

**Purpose**: Verify mixed concurrent operations are safe.

**Steps**:
1. Create RuntimeManager and load runtime
2. Spawn threads performing various operations:
   - Load modules
   - Load entities
   - Call entities
   - Get/set variables
   - Release runtime
3. Run for extended period (e.g., 1 second)
4. Verify no crashes, deadlocks, or data corruption

**Expected**: All operations complete safely.

### 8.8 Race Condition Detection

**Purpose**: Verify no race conditions in cache operations.

**Steps**:
1. Use thread sanitizer or similar tool
2. Run concurrent tests (module load, entity load, entity call)
3. Verify no race conditions reported
4. Verify cache consistency

**Expected**: No race conditions detected.

---

## 9. Memory Management Tests

### 9.1 Destructor Cleanup

**Purpose**: Verify destructors properly cleanup resources.

**Steps**:
1. Create RuntimeManager in scope
2. Load runtime, load modules, load entities
3. Let RuntimeManager go out of scope
4. Verify all resources released (use memory leak detector)

**Expected**: No memory leaks, all resources released.

### 9.2 Module Cleanup

**Purpose**: Verify module cleanup releases entities.

**Steps**:
1. Load module and multiple entities
2. Call `module->unload(&err)`
3. Verify entities are released
4. Verify no memory leaks

**Expected**: Entities released, no leaks.

### 9.3 Entity Cleanup

**Purpose**: Verify entity cleanup releases runtime resources.

**Steps**:
1. Load entity
2. Let Entity go out of scope (shared_ptr released)
3. Verify runtime resources released (runtime-specific)
4. Verify no memory leaks

**Expected**: Runtime resources released, no leaks.

### 9.4 Error Path Cleanup

**Purpose**: Verify cleanup on error paths.

**Steps**:
1. Load runtime and module
2. Trigger error (e.g., load invalid entity)
3. Verify resources are still properly managed
4. Verify no memory leaks

**Expected**: Resources cleaned up even on error.

### 9.5 Memory Leak Detection

**Purpose**: Verify no memory leaks in normal operation.

**Steps**:
1. Use memory leak detection tool (Valgrind, AddressSanitizer, etc.)
2. Run comprehensive test suite
3. Verify no memory leaks reported
4. Verify no use-after-free errors

**Expected**: No memory leaks detected.

### 9.6 Resource Limit Tests

**Purpose**: Verify behavior under resource constraints.

**Steps**:
1. Load many modules (e.g., 100 modules)
2. Load many entities (e.g., 1000 entities)
3. Verify all load successfully
4. Verify cleanup works correctly
5. Verify no resource exhaustion

**Expected**: Handles many resources correctly.

---

## 10. Error Handling Tests

### 10.1 Error Message Format

**Purpose**: Verify error messages are descriptive.

**Steps**:
1. Trigger various errors (invalid module, invalid entity, type mismatch, etc.)
2. Verify error messages are not null
3. Verify error messages contain relevant information
4. Verify error messages are human-readable

**Expected**: Descriptive error messages.

### 10.2 Error Message Memory

**Purpose**: Verify error messages are properly allocated.

**Steps**:
1. Trigger error
2. Verify error message is allocated
3. Free error message
4. Verify no double-free errors
5. Verify memory is properly managed

**Expected**: Error messages properly allocated and freed.

### 10.3 Error Propagation

**Purpose**: Verify errors propagate correctly through call stack.

**Steps**:
1. Load module (succeeds)
2. Load entity (fails)
3. Verify error is reported correctly
4. Verify module is still valid
5. Verify can retry with correct parameters

**Expected**: Errors don't corrupt state.

### 10.4 Null Pointer Handling

**Purpose**: Verify null pointer parameters are handled.

**Test Cases**:
- `load_module(nullptr, &err)`
- `load_entity(nullptr, types, types, &err)`
- `entity->call(nullptr, retvals, &err)` when params expected
- `entity->get(nullptr, &err)`
- `entity->set(nullptr, &err)`

**Expected**: Appropriate errors reported, no crashes.

### 10.5 Invalid Type Handling

**Purpose**: Verify invalid type information is handled.

**Steps**:
1. Load entity with invalid type enum
2. Load entity with null type info
3. Verify appropriate errors reported

**Expected**: Errors reported for invalid types.

---

## 11. Integration Tests

### 11.1 End-to-End Function Call

**Purpose**: Verify complete flow from runtime load to function call.

**Steps**:
1. Create RuntimeManager
2. Load runtime
3. Load module
4. Load function entity
5. Call function with parameters
6. Verify return value
7. Release runtime

**Expected**: Complete flow works correctly.

### 11.2 Multiple Modules and Entities

**Purpose**: Verify multiple modules and entities work together.

**Steps**:
1. Load runtime
2. Load multiple modules (e.g., 5 modules)
3. Load multiple entities from each module (e.g., 10 entities per module)
4. Call entities from different modules
5. Verify all work correctly
6. Release runtime

**Expected**: Multiple modules and entities coexist correctly.

### 11.3 Complex Entity Paths

**Purpose**: Verify complex entity paths work.

**Test Cases**:
- Nested class methods
- Deep attribute paths
- Overloaded functions
- Complex type signatures

**Expected**: Complex entity paths handled correctly.

### 11.4 Runtime Reload Scenario

**Purpose**: Verify runtime can be reloaded after release.

**Steps**:
1. Load runtime, load modules, load entities
2. Call entities
3. Release runtime
4. Load runtime again
5. Load modules and entities again
6. Call entities
7. Verify all work correctly

**Expected**: Runtime can be reloaded successfully.

### 11.5 Module Reload Scenario

**Purpose**: Verify modules can be reloaded after unload.

**Steps**:
1. Load module and entities
2. Call entities
3. Unload module
4. Load module again
5. Load entities again
6. Call entities
7. Verify all work correctly

**Expected**: Modules can be reloaded successfully.

---

## 12. Runtime-Specific Tests

### 12.1 Python-Specific Tests

**Test Cases**:
- Python varargs support (`*args`)
- Python named args support (`**kwargs`)
- Python module import variations
- Python class method vs instance method
- Python static method
- Python property access

### 12.2 Java-Specific Tests

**Test Cases**:
- Java class loading from JAR files
- Java class loading from classpath
- Java static vs instance methods
- Java constructor overloading
- Java field access (public, private with getters/setters)
- Java nested classes

### 12.3 Go-Specific Tests

**Test Cases**:
- Go package loading
- Go exported vs unexported functions
- Go struct method receivers (value vs pointer)
- Go package-level functions
- Go global variables

### 12.4 Runtime-Specific Error Cases

**Test Cases**:
- Runtime-specific error conditions
- Runtime-specific type limitations
- Runtime-specific path formats
- Runtime-specific entity types

---

## Test Implementation Checklist

For each runtime_manager implementation, verify:

- [ ] All runtime lifecycle tests pass
- [ ] All module loading tests pass
- [ ] All entity loading tests pass (for all entity types in sdk/idl_entities/entity_path_specs.json)
- [ ] All entity execution tests pass
- [ ] All caching tests pass
- [ ] All thread safety tests pass
- [ ] All memory management tests pass (no leaks)
- [ ] All error handling tests pass
- [ ] All integration tests pass
- [ ] All runtime-specific tests pass
- [ ] Tests run in CI/CD pipeline
- [ ] Tests have clear pass/fail reporting
- [ ] Tests include meaningful error messages

---

## Summary

This test plan provides comprehensive coverage of runtime_manager functionality. All tests should be implemented for each runtime_manager implementation to ensure:

1. **Correctness**: All functionality works as specified
2. **Reliability**: No crashes, deadlocks, or data corruption
3. **Performance**: Caching works correctly
4. **Safety**: Thread-safe operations, proper memory management
5. **Usability**: Clear error messages, graceful error handling

For the main specification, see [runtime_manager_doc.md](runtime_manager_doc.md).
