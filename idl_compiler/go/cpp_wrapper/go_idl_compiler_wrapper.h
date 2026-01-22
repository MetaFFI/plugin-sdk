#pragma once

#include <string>
#include <stdexcept>

namespace metaffi {
namespace idl_compiler {

/**
 * @brief C++ wrapper class for executing the Go IDL compiler executable
 *
 * This class provides a convenient C++ interface for invoking the go_idl_compiler
 * executable and capturing its output. The executable must be in the system PATH
 * or in the same directory as the calling application.
 *
 * The Go compiler executable:
 * - Accepts source path as command line argument
 * - Outputs IDL JSON to STDOUT
 * - Outputs error messages to STDERR
 * - Returns exit code 0 on success, non-zero on error
 */
class GoIDLCompilerWrapper {
public:
    /**
     * @brief Construct a new GoIDLCompilerWrapper
     * @param executable_path Path to go_idl_compiler executable (default: "go_idl_compiler")
     */
    explicit GoIDLCompilerWrapper(const std::string& executable_path = "go_idl_compiler");

    /**
     * @brief Compile Go source to MetaFFI IDL JSON
     * @param source_path Path to .go file or directory
     * @return std::string IDL JSON output
     * @throws std::runtime_error if compilation fails
     */
    std::string compile(const std::string& source_path);

    /**
     * @brief Compile Go source to MetaFFI IDL JSON and write to file
     * @param source_path Path to .go file or directory
     * @param output_path Path to output JSON file
     * @throws std::runtime_error if compilation fails
     */
    void compile_to_file(const std::string& source_path, const std::string& output_path);

    /**
     * @brief Get the path to the executable
     * @return std::string Path to go_idl_compiler executable
     */
    std::string get_executable_path() const { return executable_path_; }

    /**
     * @brief Set the path to the executable
     * @param path Path to go_idl_compiler executable
     */
    void set_executable_path(const std::string& path) { executable_path_ = path; }

private:
    std::string executable_path_;

    /**
     * @brief Execute a command and capture output
     * @param command Command to execute
     * @param stdout_output Output string for STDOUT
     * @param stderr_output Output string for STDERR
     * @return int Exit code
     */
    int execute_command(const std::string& command,
                       std::string& stdout_output,
                       std::string& stderr_output);

    /**
     * @brief Quote a string for shell command
     * @param str String to quote
     * @return std::string Quoted string
     */
    std::string quote_string(const std::string& str);
};

} // namespace idl_compiler
} // namespace metaffi
