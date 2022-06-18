#pragma once

#include <boost/dll.hpp>
#include <memory>
#include <string>

namespace metaffi::utils
{
//--------------------------------------------------------------------
inline std::shared_ptr<boost::dll::shared_library> load_plugin(const std::string& plugin_filename_without_extension)
{
	std::string metaffi_home = std::getenv("METAFFI_HOME");
	if(metaffi_home.empty()){
		throw std::runtime_error("METAFFI_HOME environment variable is not set");
	}
	
	// prepend METAFFI_HOME directory to the file name
	boost::filesystem::path plugin_full_path(metaffi_home);
	plugin_full_path.append(plugin_filename_without_extension+boost::dll::shared_library::suffix().generic_string());
	
	// load plugin
	std::shared_ptr<boost::dll::shared_library> plugin_dll = std::make_shared<boost::dll::shared_library>();
	try
	{
		plugin_dll->load( plugin_full_path, boost::dll::load_mode::rtld_now | boost::dll::load_mode::rtld_global );
	}
	catch(std::exception& ex)
	{
		std::stringstream ss;
		ss << ex.what() << " Failed to load " << plugin_full_path;
		throw std::runtime_error(ss.str());
	}
	
	return plugin_dll;
}
//--------------------------------------------------------------------
}