#pragma once
#include <string>
#include <boost/algorithm/string.hpp>
#include <map>

//-----------------------------------------------------
namespace metaffi { namespace utils
{
class function_path_parser
{
private:
	std::map<std::string, std::string> function_path_items;
	
public:
	explicit function_path_parser(const std::string& function_path);
	
	~function_path_parser() = default;
	
	[[nodiscard]] std::string operator[](const std::string &key) const;
	[[nodiscard]] bool contains(const std::string& key) const;
};
}}
//--------------------------------------------------------------------