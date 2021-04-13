#include "function_path_parser.h"

namespace openffi { namespace utils
{
//--------------------------------------------------------------------
function_path_parser::function_path_parser(const std::string &function_path)
{
	std::vector<std::string> items;
	boost::split(items, function_path, boost::is_any_of(","));
	
	for(const std::string& item : items)
	{
		std::vector<std::string> keyval;
		boost::split(keyval, item, boost::is_any_of("="));
		
		if(keyval.size() != 2)
		{
			throw std::runtime_error("function path is invalid, cannot split by '=' to key and value");
		}
		
		function_path_items[keyval[0]] = keyval[1];
	}
}
//--------------------------------------------------------------------
std::string function_path_parser::operator[](const std::string &key) const
{
	auto i = function_path_items.find(key);
	if(i == function_path_items.end()){
		return "";
	}
	
	return i->second;
}
//--------------------------------------------------------------------
}}