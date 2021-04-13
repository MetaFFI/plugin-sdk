#pragma once

namespace openffi { namespace utils
{
	std::shared_ptr <boost::dll::shared_library> load_library(const std::string &library_name);
}}