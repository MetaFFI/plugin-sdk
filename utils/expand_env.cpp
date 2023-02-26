#include "expand_env.h"
#include <boost/algorithm/string.hpp>
#include <regex>
#include <algorithm>

#ifdef _WIN32
	#include <Windows.h>
#endif

namespace metaffi::utils
{
//--------------------------------------------------------------------
// https://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c
bool iequals(const std::string& a, const std::string& b)
{
	return std::equal(a.begin(), a.end(),
	                  b.begin(), b.end(),
	                  [](char a, char b) {
		                  return tolower(a) == tolower(b);
	                  });
}
//--------------------------------------------------------------------
std::string expand_env(const std::string& str)
{
	static std::regex re(R"(%([A-Z0-9_\(\){}\[\]\$\*\+\\\/\"#',;\.!@\?-]+)%|\$ENV:([A-Z0-9_]+)|\$([A-Z_]{1}[A-Z0-9_]+)|\$\{([^}]+)\})", std::regex::icase);
	static std::string cur_dir_win("CD");
	static std::string cur_dir_nix("PWD");
	
	std::string working_copy_str = str;
	std::string res = str;
	for(std::smatch m; std::regex_search(working_copy_str, m, re); working_copy_str = m.suffix())
	{
		std::string env_var_name;
		if(m[1].matched){ env_var_name = m[1].str(); }
		else if(m[2].matched){ env_var_name = m[2].str(); }
		else if(m[3].matched){ env_var_name = m[3].str(); }
		else if(m[4].matched){ env_var_name = m[4].str(); }
		
		if(env_var_name.length() == 2 && iequals(env_var_name, cur_dir_win) ||
				env_var_name.length() == 3 && iequals(env_var_name, cur_dir_nix))
		{
			char cwd[1024] = {0};

#ifdef _WIN32
			GetCurrentDirectory(1024,cwd);
#else
			getcwd(cwd, 1024);
#endif
			boost::algorithm::replace_all(res, m.str(), cwd);
		}
		else
		{
			const char* tmp = std::getenv(env_var_name.c_str());
			if(tmp)
			{
				boost::algorithm::replace_all(res, m.str(), tmp);
			}
		}
	}
	
	return res;
}
//--------------------------------------------------------------------
}