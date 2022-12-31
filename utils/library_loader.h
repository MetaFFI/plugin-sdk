#pragma once
#include <memory>
#include <boost/dll.hpp>

namespace metaffi { namespace utils
{
	std::shared_ptr <boost::dll::shared_library> load_library(const std::string &library_name, bool load_from_metaffi_home = true, bool append_extension = true);
}}