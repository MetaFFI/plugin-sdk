#include "go_idl_compiler_wrapper.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

#ifdef _WIN32
    #include <process.h>
    #define getpid _getpid
#else
    #include <unistd.h>
    #include <sys/wait.h>
#endif

#include <boost/filesystem.hpp>

namespace metaffi {
namespace idl_compiler {

GoIDLCompilerWrapper::GoIDLCompilerWrapper(const std::string& executable_path)
    : executable_path_(executable_path) {
}

std::string GoIDLCompilerWrapper::compile(const std::string& source_path) {
    // Write JSON to a temp file instead of capturing stdout.
    // This lets the child process print logs directly to the console.
    boost::filesystem::path tmp = boost::filesystem::temp_directory_path();
    tmp /= "metaffi_go_idl_" + std::to_string(getpid()) + ".json";
    std::string temp_file = tmp.string();

    // Build command: go_idl_compiler <source_path> <temp_file>
    std::string command = executable_path_ + " " +
                         quote_string(source_path) + " " +
                         quote_string(temp_file);

    int exit_code = run_command(command);

    if (exit_code != 0) {
        boost::filesystem::remove(temp_file);
        std::ostringstream err;
        err << "Go IDL compiler failed with exit code " << exit_code;
        throw std::runtime_error(err.str());
    }

    // Read the JSON from the temp file
    std::ifstream f(temp_file);
    if (!f) {
        throw std::runtime_error("Go IDL compiler did not produce output file: " + temp_file);
    }
    std::ostringstream buf;
    buf << f.rdbuf();
    f.close();
    boost::filesystem::remove(temp_file);

    return buf.str();
}

void GoIDLCompilerWrapper::compile_to_file(const std::string& source_path,
                                           const std::string& output_path) {
    // Build command: go_idl_compiler <source_path> <output_path>
    std::string command = executable_path_ + " " +
                         quote_string(source_path) + " " +
                         quote_string(output_path);

    int exit_code = run_command(command);

    if (exit_code != 0) {
        std::ostringstream err;
        err << "Go IDL compiler failed with exit code " << exit_code;
        throw std::runtime_error(err.str());
    }
}

int GoIDLCompilerWrapper::run_command(const std::string& command) {
    // Use system() so the child process inherits the parent's stdout/stderr.
    // The child prints directly to the console.
    int status = std::system(command.c_str());
#ifdef _WIN32
    return status;
#else
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return status;
#endif
}

std::string GoIDLCompilerWrapper::quote_string(const std::string& str) {
#ifdef _WIN32
    // Windows: use double quotes
    if (str.find(' ') != std::string::npos || str.find('"') != std::string::npos) {
        std::string quoted = "\"";
        for (char c : str) {
            if (c == '"') {
                quoted += "\\\"";
            } else {
                quoted += c;
            }
        }
        quoted += "\"";
        return quoted;
    }
    return str;
#else
    // Unix: use single quotes (simpler, no escaping needed except for single quotes)
    if (str.find(' ') != std::string::npos || str.find('\'') != std::string::npos) {
        std::string quoted = "'";
        for (char c : str) {
            if (c == '\'') {
                quoted += "'\\''";
            } else {
                quoted += c;
            }
        }
        quoted += "'";
        return quoted;
    }
    return str;
#endif
}

} // namespace idl_compiler
} // namespace metaffi
