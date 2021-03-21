#include <boost/dll.hpp>
#include <memory>
#include <string>

namespace openffi{ namespace utils
{
//--------------------------------------------------------------------
std::shared_ptr<boost::dll::shared_library> load_plugin(const std::string& plugin_filename_without_extension)
{
	std::string openffi_home = std::getenv("OPENFFI_HOME");
	if(openffi_home.empty()){
		throw std::runtime_error("OPENFFI_HOME environment variable is not set");
	}
	
	// prepend OPENFFI_HOME directory to the file name
	boost::filesystem::path openffi_home_path(openffi_home);
	std::string plugin_filename = plugin_filename_without_extension+boost::dll::shared_library::suffix().generic_string();
	openffi_home_path.append(plugin_filename);
	
	// load plugin
	std::shared_ptr<boost::dll::shared_library> plugin_dll = std::make_shared<boost::dll::shared_library>();
	plugin_dll->load( boost::filesystem::current_path().append(plugin_filename) );
	
	return plugin_dll;
}
//--------------------------------------------------------------------
}}