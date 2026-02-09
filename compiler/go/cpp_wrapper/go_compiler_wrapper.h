#pragma once

#include <string>
#include <stdexcept>

namespace metaffi {
namespace compiler {
namespace go {

/**
 * C++ wrapper for the go_compiler executable (same pattern as go_idl_compiler).
 *
 * go_compiler guest <idl_json_file> <output_path> [options]
 * go_compiler host  <idl_json_file> <output_path> [options]
 *
 * - Writes IDL JSON to a temp file and passes path to the executable
 * - Errors go to STDERR; exit 0 = success, non-zero = failure
 */
class GoCompilerWrapper
{
public:
	explicit GoCompilerWrapper(const std::string& executable_path = "go_compiler");

	void compile_to_guest(const std::string& idl_def_json,
		const std::string& output_path,
		const std::string& guest_options);

	void compile_from_host(const std::string& idl_def_json,
		const std::string& output_path,
		const std::string& host_options);

	std::string get_executable_path() const { return executable_path_; }
	void set_executable_path(const std::string& path) { executable_path_ = path; }

private:
	std::string executable_path_;

	int run_command(const std::string& command);
	std::string quote_string(const std::string& str);
	std::string write_temp_idl(const std::string& idl_json);
};

} // namespace go
} // namespace compiler
} // namespace metaffi
