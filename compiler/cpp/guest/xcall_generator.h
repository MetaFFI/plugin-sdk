#pragma once

#include <string>
#include <sstream>
#include <metaffi/idl/idl_definition.hpp>

namespace metaffi::compiler::cpp {

/**
 * CppXcallGenerator emits a single .cpp file containing xcall entrypoints
 * for every entity in an IDLDefinition.
 *
 * Generated code layout:
 *   - Platform-guarded includes (dlfcn.h / Windows.h)
 *   - Static function pointer per exported symbol
 *   - A lazy init_lib() that dlopen/LoadLibrary the original DLL on first call
 *   - For each free function: an extern "C" xcall entrypoint
 *   - For each class: xcall entrypoints for constructors, methods, destructor
 *   - For each global: getter/setter xcall entrypoints
 *
 * After generate() returns the source text, call print_compile_commands()
 * to emit the corresponding build commands to stdout.
 */
class CppXcallGenerator {
public:
    /**
     * Generate the complete xcall wrapper .cpp source text.
     *
     * @param idl         Parsed IDL definition (all modules).
     * @param orig_lib    Path embedded in the generated file for dlopen (first
     *                    external_resource of the first module, or empty).
     * @return            Generated .cpp source as a string.
     */
    [[nodiscard]] std::string generate(const metaffi::idl::IDLDefinition& idl,
                                       const std::string& orig_lib) const;

    /**
     * Print shell compile commands for Windows / Linux / macOS to stdout.
     *
     * @param stem  Output filename stem (e.g. "guest_xcall").
     */
    static void print_compile_commands(const std::string& stem);

private:
    // --- Header / init section ---
    void emit_prologue(std::ostringstream& out, const std::string& orig_lib) const;

    // --- Per-module sections ---
    void emit_module(std::ostringstream& out,
                     const metaffi::idl::ModuleDefinition& module) const;

    void emit_free_function(std::ostringstream& out,
                            const metaffi::idl::FunctionDefinition& func) const;

    void emit_class(std::ostringstream& out,
                    const metaffi::idl::ClassDefinition& cls) const;

    void emit_global(std::ostringstream& out,
                     const metaffi::idl::GlobalDefinition& global) const;

    // --- CDT marshaling helpers (code-generating, not runtime) ---

    // Emit param-extract statements for params_ret[0] into named locals.
    void emit_unpack_params(std::ostringstream& out,
                            const std::vector<metaffi::idl::ArgDefinition>& params) const;

    // Emit pack-result statements from a named local into params_ret[1].
    void emit_pack_returns(std::ostringstream& out,
                           const std::vector<metaffi::idl::ArgDefinition>& rets) const;

    // Build the C++ native type string for an IDL arg (using type_alias if set).
    static std::string native_type(const metaffi::idl::ArgDefinition& arg);

    // Build the cdt_val field name for a scalar IDL type ("int64_val", etc.).
    static std::string cdt_field(const std::string& idl_type);

    // Build the metaffi_*_type constant for a scalar IDL type.
    static std::string metaffi_type_const(const std::string& idl_type);

    // Sanitise string to valid C identifier fragment.
    static std::string to_ident(const std::string& s);

    // Build xcall entrypoint name from class (may be empty) and entity name.
    static std::string entrypoint_name(const std::string& class_name,
                                       const std::string& entity_name,
                                       int overload_index = 0);
};

} // namespace metaffi::compiler::cpp
