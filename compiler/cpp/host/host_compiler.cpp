#include "host_compiler.h"
#include "code_generator.h"

#include <metaffi/idl/idl_definition.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace metaffi::compiler::cpp {

void CppHostCompiler::compile(const std::string& idl_def_json,
                               const std::string& output_path,
                               const std::string& output_filename_stem) {
    // --- Parse IDL ---
    metaffi::idl::IDLDefinition idl;
    try {
        idl = metaffi::idl::IDLDefinition::load_from_json(idl_def_json);
    } catch (const metaffi::idl::IDLException& e) {
        throw std::runtime_error(std::string("CppHostCompiler: IDL parse error: ") + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("CppHostCompiler: JSON parse error: ") + e.what());
    }

    // --- Generate code ---
    CppCodeGenerator generator;
    const std::string stem = output_filename_stem + "_MetaFFIHost";
    auto [header_content, source_content] = generator.generate(idl, stem);

    // --- Ensure output directory exists ---
    const std::filesystem::path out_dir(output_path);
    try {
        std::filesystem::create_directories(out_dir);
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error(std::string("CppHostCompiler: cannot create output directory '")
                                 + output_path + "': " + e.what());
    }

    // --- Write header file ---
    const std::filesystem::path header_path = out_dir / (stem + ".hpp");
    {
        std::ofstream header_file(header_path);
        if (!header_file) {
            throw std::runtime_error("CppHostCompiler: cannot write to '" + header_path.string() + "'");
        }
        header_file << header_content;
    }

    // --- Write source file ---
    const std::filesystem::path source_path = out_dir / (stem + ".cpp");
    {
        std::ofstream source_file(source_path);
        if (!source_file) {
            throw std::runtime_error("CppHostCompiler: cannot write to '" + source_path.string() + "'");
        }
        source_file << source_content;
    }
}

} // namespace metaffi::compiler::cpp
