#pragma once

#include <string>

namespace metaffi::compiler::cpp {

/**
 * CppGuestCompiler orchestrates guest-side xcall code generation.
 *
 * Steps:
 *   1. Parse IDL JSON string into an IDLDefinition.
 *   2. Extract the original library path from the IDL (first external_resource
 *      of the first module, or empty string if none).
 *   3. Invoke CppXcallGenerator to produce the xcall wrapper .cpp source.
 *   4. Write <stem>.cpp to output_path.
 *   5. Print compile commands for Windows / Linux / macOS to STDOUT.
 *
 * Throws std::runtime_error on invalid JSON or I/O failure.
 */
class CppGuestCompiler {
public:
    /**
     * @param idl_def_json  IDL JSON produced by the IDL plugin.
     * @param output_path   Directory to write the generated .cpp file.
     * @param stem          Output filename stem (default "guest_xcall").
     */
    void compile(const std::string& idl_def_json,
                 const std::string& output_path,
                 const std::string& stem = "guest_xcall");
};

} // namespace metaffi::compiler::cpp
