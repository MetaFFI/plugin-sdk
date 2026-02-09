#include "go_compiler_wrapper.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <ctime>
#include <atomic>

#ifdef _WIN32
	#include <process.h>
	#define getpid _getpid
#else
	#include <unistd.h>
	#include <sys/wait.h>
#endif

#include <boost/filesystem.hpp>

namespace metaffi {
namespace compiler {
namespace go {

GoCompilerWrapper::GoCompilerWrapper(const std::string& executable_path)
	: executable_path_(executable_path)
{
}

static std::atomic<unsigned> s_temp_counter{0};

std::string GoCompilerWrapper::write_temp_idl(const std::string& idl_json)
{
	boost::filesystem::path tmp = boost::filesystem::temp_directory_path();
	tmp /= "metaffi_go_idl_" + std::to_string(static_cast<unsigned long>(std::time(nullptr))) + "_" + std::to_string(s_temp_counter++) + "_" + std::to_string(static_cast<long>(getpid())) + ".json";
	std::string path = tmp.string();
	std::ofstream f(path);
	if (!f)
		throw std::runtime_error("Failed to create temp file for IDL JSON");
	f << idl_json;
	f.close();
	return path;
}

void GoCompilerWrapper::compile_to_guest(const std::string& idl_def_json,
	const std::string& output_path,
	const std::string& guest_options)
{
	std::string idl_file = write_temp_idl(idl_def_json);
	try
	{
		std::string command = executable_path_ + " guest " + quote_string(idl_file) + " " + quote_string(output_path);
		if (!guest_options.empty())
			command += " " + quote_string(guest_options);

		int exit_code = run_command(command);
		boost::filesystem::remove(idl_file);

		if (exit_code != 0)
		{
			std::ostringstream err;
			err << "go_compiler guest failed (exit " << exit_code << ")";
			throw std::runtime_error(err.str());
		}
	}
	catch (...)
	{
		boost::filesystem::remove(idl_file);
		throw;
	}
}

void GoCompilerWrapper::compile_from_host(const std::string& idl_def_json,
	const std::string& output_path,
	const std::string& host_options)
{
	std::string idl_file = write_temp_idl(idl_def_json);
	try
	{
		std::string command = executable_path_ + " host " + quote_string(idl_file) + " " + quote_string(output_path);
		if (!host_options.empty())
			command += " " + quote_string(host_options);

		int exit_code = run_command(command);
		boost::filesystem::remove(idl_file);

		if (exit_code != 0)
		{
			std::ostringstream err;
			err << "go_compiler host failed (exit " << exit_code << ")";
			throw std::runtime_error(err.str());
		}
	}
	catch (...)
	{
		boost::filesystem::remove(idl_file);
		throw;
	}
}

int GoCompilerWrapper::run_command(const std::string& command)
{
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

std::string GoCompilerWrapper::quote_string(const std::string& str)
{
#ifdef _WIN32
	if (str.find(' ') != std::string::npos || str.find('"') != std::string::npos)
	{
		std::string quoted = "\"";
		for (char c : str)
		{
			if (c == '"')
				quoted += "\\\"";
			else
				quoted += c;
		}
		quoted += "\"";
		return quoted;
	}
	return str;
#else
	if (str.find(' ') != std::string::npos || str.find('\'') != std::string::npos)
	{
		std::string quoted = "'";
		for (char c : str)
		{
			if (c == '\'')
				quoted += "'\\''";
			else
				quoted += c;
		}
		quoted += "'";
		return quoted;
	}
	return str;
#endif
}

} // namespace go
} // namespace compiler
} // namespace metaffi
