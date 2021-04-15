#pragma once
#include <memory>
#include <boost/dll.hpp>

namespace openffi { namespace utils
{
	std::shared_ptr <boost::dll::shared_library> load_library(const std::string &library_name);
}}