#pragma once

#include <boost/dll.hpp>
#include <memory>
#include <string>

namespace openffi::utils
{
//--------------------------------------------------------------------
inline std::shared_ptr<boost::dll::shared_library> load_plugin(const std::string& plugin_filename_without_extension)
{
	std::string openffi_home = std::getenv("OPENFFI_HOME");
	if(openffi_home.empty()){
		throw std::runtime_error("OPENFFI_HOME environment variable is not set");
	}
	
	// prepend OPENFFI_HOME directory to the file name
	boost::filesystem::path plugin_full_path(openffi_home);
	plugin_full_path.append(plugin_filename_without_extension+boost::dll::shared_library::suffix().generic_string());
	
	// load plugin
	std::shared_ptr<boost::dll::shared_library> plugin_dll = std::make_shared<boost::dll::shared_library>();
	plugin_dll->load( plugin_full_path, boost::dll::load_mode::rtld_now | boost::dll::load_mode::rtld_global );
	
	return plugin_dll;
}
//--------------------------------------------------------------------
}