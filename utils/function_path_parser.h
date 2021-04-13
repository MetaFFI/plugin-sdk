#pragma once
#include <string>
#include <boost/algorithm/string.hpp>
#include <vector>

//-----------------------------------------------------
namespace openffi { namespace utils
{
class function_path_parser
{
private:
	std::map<std::string, std::string> function_path_items;
	
public:
	function_path_parser(const std::string &function_path);
	
	~function_path_parser() = default;
	
	[[nodiscard]] std::string operator[](const std::string &key) const;
};
}}
//--------------------------------------------------------------------