#pragma once

#include <string>

namespace metaffi::compiler::cpp {

/**
 * CppHostCompiler orchestrates host-side code generation.
 *
 * Steps:
 *   1. Parse the IDL JSON string into an IDLDefinition.
 *   2. Invoke CppCodeGenerator to produce header + source content strings.
 *   3. Write <output_filename_stem>_MetaFFIHost.hpp and .cpp to output_path.
 *
 * Throws std::runtime_error on invalid JSON or I/O failure.
 */
class CppHostCompiler {
public:
    // Parses IDL JSON, generates .hpp + .cpp, writes to output_path.
    // output_filename_stem: base name for the generated files (no extension).
    void compile(const std::string& idl_def_json,
                 const std::string& output_path,
                 const std::string& output_filename_stem);
};

} // namespace metaffi::compiler::cpp
