#include "function_path_parser.h"
#include "expand_env.h"

namespace metaffi { namespace utils
{
//--------------------------------------------------------------------
function_path_parser::function_path_parser(const std::string &function_path)
{
	std::vector<std::string> items;
	boost::split(items, function_path, boost::is_any_of(","));
	
	for(const std::string& item : items)
	{
		if(item.empty()){ continue; }
		
		std::vector<std::string> keyval;
		boost::split(keyval, item, boost::is_any_of("="));
		
		if(keyval.size() > 2)
		{
			throw std::runtime_error("function path is invalid, too many '='");
		}
		
		if(keyval.size() == 1)
		{
			function_path_items[keyval[0]] = "";
		}
		else
		{
			// expand environment variable in keyval[1]
			std::string exp = expand_env(keyval[1]);
			function_path_items[keyval[0]] = exp;
		}
	}
}
//--------------------------------------------------------------------
std::string function_path_parser::operator[](const std::string &key) const
{
	auto i = function_path_items.find(key);
	if(i == function_path_items.end()){
		return "";
	}
	
	// expand environment variables
	
	
	return i->second;
}
//--------------------------------------------------------------------
bool function_path_parser::contains(const std::string& key) const
{
	return function_path_items.find(key) != function_path_items.end();
}
//--------------------------------------------------------------------
}}