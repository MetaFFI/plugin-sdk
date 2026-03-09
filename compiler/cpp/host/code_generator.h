#pragma once

#include <string>
#include <sstream>
#include <metaffi/idl/idl_definition.hpp>

namespace metaffi::compiler::cpp {

/**
 * CppCodeGenerator builds the .hpp and .cpp content strings from an IDLDefinition.
 *
 * Design:
 *   - One pass over all modules in the IDL.
 *   - Each module becomes its own <name>_host namespace block in both files.
 *   - Free functions, classes, and globals are emitted per module.
 *   - The generated source only contains text — it does not compile MetaFFI APIs.
 */
class CppCodeGenerator {
public:
    struct GeneratedFiles {
        std::string header_content;
        std::string source_content;
    };

    // Generate header + source strings from the IDL.
    // output_filename_stem: base name of the output files (e.g. "test_module_MetaFFIHost").
    GeneratedFiles generate(const metaffi::idl::IDLDefinition& idl,
                            const std::string& output_filename_stem) const;

private:
    // --- Per-module generation ---

    void generate_module_header(std::ostringstream& out,
                                const metaffi::idl::ModuleDefinition& module) const;

    void generate_module_source(std::ostringstream& out,
                                const metaffi::idl::ModuleDefinition& module,
                                const std::string& header_stem,
                                const metaffi::idl::IDLDefinition& idl) const;

    // --- Header sections ---

    void emit_free_function_decls(std::ostringstream& out,
                                  const metaffi::idl::ModuleDefinition& module) const;

    void emit_global_decls(std::ostringstream& out,
                           const metaffi::idl::ModuleDefinition& module) const;

    void emit_class_decl(std::ostringstream& out,
                         const metaffi::idl::ClassDefinition& cls) const;

    // --- Source sections ---

    void emit_entity_statics(std::ostringstream& out,
                             const metaffi::idl::ModuleDefinition& module) const;

    void emit_bind_function(std::ostringstream& out,
                            const metaffi::idl::ModuleDefinition& module,
                            const metaffi::idl::IDLDefinition& idl) const;

    void emit_free_function_impls(std::ostringstream& out,
                                  const metaffi::idl::ModuleDefinition& module) const;

    void emit_global_impls(std::ostringstream& out,
                           const metaffi::idl::ModuleDefinition& module) const;

    void emit_class_impls(std::ostringstream& out,
                          const metaffi::idl::ModuleDefinition& module,
                          const metaffi::idl::ClassDefinition& cls) const;

    // --- Utility helpers ---

    // Build comma-separated list of "type name" pairs from parameters.
    std::string params_decl(const std::vector<metaffi::idl::ArgDefinition>& params) const;

    // Build comma-separated list of argument names from parameters.
    std::string params_names(const std::vector<metaffi::idl::ArgDefinition>& params) const;

    // Build "{...}" initialiser list of MetaFFITypeInfo for a parameter list.
    std::string type_info_list(const std::vector<metaffi::idl::ArgDefinition>& args) const;

    // Build the call expression: "auto [r] = _entity.call<T>(args)" or "_entity.call(args)".
    // instance_first: prepend "_handle" before other arguments.
    std::string call_expression(const std::string& entity_var,
                                const std::vector<metaffi::idl::ArgDefinition>& params,
                                const std::vector<metaffi::idl::ArgDefinition>& returns,
                                bool instance_first) const;

    // Filter the instance-handle parameter from method parameter lists.
    // When instance_required is true, the first parameter in IDL params is the
    // implicit handle (e.g. "h": handle).  C++ methods hide it via _handle.
    static std::vector<metaffi::idl::ArgDefinition> filter_instance_params(
        const std::vector<metaffi::idl::ArgDefinition>& params,
        bool instance_required);

    // Build an entity variable name from class (optional) and entity label.
    static std::string entity_var(const std::string& class_name,
                                  const std::string& entity_label);

    // Sanitise a string to be a valid C++ identifier fragment.
    static std::string to_ident(const std::string& s);
};

} // namespace metaffi::compiler::cpp
