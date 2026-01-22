#include "go_idl_compiler_wrapper.h"
#include <cstdio>
#include <array>
#include <sstream>
#include <memory>

#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
#endif

namespace metaffi {
namespace idl_compiler {

GoIDLCompilerWrapper::GoIDLCompilerWrapper(const std::string& executable_path)
    : executable_path_(executable_path) {
}

std::string GoIDLCompilerWrapper::compile(const std::string& source_path) {
    // Build command: go_idl_compiler <source_path>
    std::string command = executable_path_ + " " + quote_string(source_path);

    std::string stdout_output;
    std::string stderr_output;

    int exit_code = execute_command(command, stdout_output, stderr_output);

    if (exit_code != 0) {
        std::ostringstream err;
        err << "Go IDL compiler failed with exit code " << exit_code;
        if (!stderr_output.empty()) {
            err << ": " << stderr_output;
        }
        throw std::runtime_error(err.str());
    }

    return stdout_output;
}

void GoIDLCompilerWrapper::compile_to_file(const std::string& source_path,
                                           const std::string& output_path) {
    // Build command: go_idl_compiler <source_path> <output_path>
    std::string command = executable_path_ + " " +
                         quote_string(source_path) + " " +
                         quote_string(output_path);

    std::string stdout_output;
    std::string stderr_output;

    int exit_code = execute_command(command, stdout_output, stderr_output);

    if (exit_code != 0) {
        std::ostringstream err;
        err << "Go IDL compiler failed with exit code " << exit_code;
        if (!stderr_output.empty()) {
            err << ": " << stderr_output;
        }
        throw std::runtime_error(err.str());
    }
}

int GoIDLCompilerWrapper::execute_command(const std::string& command,
                                         std::string& stdout_output,
                                         std::string& stderr_output) {
    // Redirect stderr to stdout so we can capture both
    std::string full_command = command + " 2>&1";

    // Open pipe to command
    FILE* pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to open pipe to go_idl_compiler");
    }

    // Read output
    std::array<char, 128> buffer;
    std::ostringstream output_stream;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output_stream << buffer.data();
    }

    // Close pipe and get exit code
    int exit_code = pclose(pipe);

#ifdef _WIN32
    // Windows pclose returns the exit code directly
    // No need to use WEXITSTATUS
#else
    // On Unix, extract exit code from status
    if (WIFEXITED(exit_code)) {
        exit_code = WEXITSTATUS(exit_code);
    }
#endif

    // The output contains both stdout and stderr mixed
    // For simplicity, treat all as stdout for now
    // A more sophisticated implementation could separate them
    stdout_output = output_stream.str();

    return exit_code;
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
                quoted += "'\\''";  // Escape single quote
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
