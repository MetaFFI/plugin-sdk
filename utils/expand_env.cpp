#include "expand_env.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>

namespace openffi::utils
{
//--------------------------------------------------------------------
std::string expand_env(const std::string& str)
{
	bool in_var = false;
	std::string res;
	std::string curvar;
	for(char c : str)
	{
		if(!in_var)
		{
#ifndef _WIN32
			if( c == '$' ) // start of var
#else
			if( c == '%' ) // start of var
#endif
			{
				in_var = true;
				curvar += c;
			}
			else
			{
				res += c;
			}
		}
		else
		{
#ifndef _WIN32
			if(c == ' ' || c == '$' || c == '\\' || c == '/') // end of var
#else
			if(c == '%') // end of var
#endif
			{
				// get current directory
#ifndef _WIN32
				if(curvar == "$PWD")
#else
				if(boost::algorithm::to_upper_copy(curvar) == "%CD")
#endif
				{
					res += boost::filesystem::current_path().string();
				}
				else
				{
					// get environment variable
					const char* tmp = std::getenv(curvar.c_str());
					if(tmp){ res += tmp; };
				}

#ifndef _WIN32
				res += c; // append "c" to result if not windows end symbol
#endif
				
				curvar.clear();
				in_var = false;
			}
			else
			{
				curvar += c;
			}
		}
	}
	
	return res;
}
//--------------------------------------------------------------------
}