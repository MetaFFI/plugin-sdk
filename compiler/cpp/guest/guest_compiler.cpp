#include "guest_compiler.h"
#include "xcall_generator.h"

#include <metaffi/idl/idl_definition.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace metaffi::compiler::cpp {

void CppGuestCompiler::compile(const std::string& idl_def_json,
                                const std::string& output_path,
                                const std::string& stem) {
    // --- Parse IDL ---
    metaffi::idl::IDLDefinition idl;
    try {
        idl = metaffi::idl::IDLDefinition::load_from_json(idl_def_json);
    } catch (const metaffi::idl::IDLException& e) {
        throw std::runtime_error(std::string("CppGuestCompiler: IDL parse error: ") + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("CppGuestCompiler: JSON parse error: ") + e.what());
    }

    // --- Extract original library path ---
    // By convention: first external_resource of the first module is the DLL path.
    std::string orig_lib;
    if (!idl.modules().empty() && !idl.modules()[0].external_resources().empty()) {
        orig_lib = idl.modules()[0].external_resources()[0];
    }

    // --- Generate xcall wrapper source ---
    CppXcallGenerator generator;
    const std::string source = generator.generate(idl, orig_lib);

    // --- Ensure output directory exists ---
    const std::filesystem::path out_dir(output_path);
    try {
        std::filesystem::create_directories(out_dir);
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error(
            std::string("CppGuestCompiler: cannot create output directory '")
            + output_path + "': " + e.what());
    }

    // --- Write generated .cpp file ---
    const std::filesystem::path cpp_path = out_dir / (stem + ".cpp");
    {
        std::ofstream cpp_file(cpp_path);
        if (!cpp_file) {
            throw std::runtime_error(
                "CppGuestCompiler: cannot write to '" + cpp_path.string() + "'");
        }
        cpp_file << source;
    }

    // --- Print compile commands to STDOUT ---
    CppXcallGenerator::print_compile_commands(stem);

    std::cout << "// Generated: " << std::filesystem::absolute(cpp_path).string() << "\n";
}

} // namespace metaffi::compiler::cpp
