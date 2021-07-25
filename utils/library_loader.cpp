#include "library_loader.h"

namespace openffi { namespace utils
{
std::shared_ptr<boost::dll::shared_library> load_library(const std::string& library_name)
{
	// first try to load from OPENFFI_HOME
	std::string openffi_home = std::getenv("OPENFFI_HOME");
	if(openffi_home.empty()){
		throw std::runtime_error("OPENFFI_HOME environment variable is not set");
	}
	
	// prepend OPENFFI_HOME directory to the file name
	boost::filesystem::path lib_in_openffi_home(openffi_home);
	lib_in_openffi_home.append(library_name + boost::dll::shared_library::suffix().generic_string());
	
	if(boost::filesystem::exists(lib_in_openffi_home))
	{
		// load plugin
		std::shared_ptr<boost::dll::shared_library> plugin_dll = std::make_shared<boost::dll::shared_library>();
		plugin_dll->load(lib_in_openffi_home, boost::dll::load_mode::rtld_now | boost::dll::load_mode::rtld_global );
		
		return plugin_dll;
	}
	
	// load library from different locations
	std::string module_filename = library_name + boost::dll::shared_library::suffix().generic_string();
	
	std::shared_ptr <boost::dll::shared_library> mod = std::make_shared<boost::dll::shared_library>();
	
	// if plugin exists in the same path of the program, load it from there (mainly used for easier development)
	// otherwise, search system folders
	if (boost::filesystem::exists(boost::filesystem::current_path().append(module_filename)))
	{
		//std::cout << "Loading Go module from: " << boost::filesystem::current_path().append(plugin_filename) << std::endl;
		//std::cout << "Loading dynamic library from: " << boost::filesystem::current_path().append(plugin_filename) << std::endl;
		mod->load(boost::filesystem::current_path().append(module_filename));
	}
	else if (boost::filesystem::exists(boost::dll::program_location().append(module_filename)))
	{
		//std::cout << "Loading Go module from: " << boost::dll::program_location().append(module_filename) << std::endl;
		//std::cout << "Loading dynamic library from: " << boost::dll::program_location().append(module_filename) << std::endl;
		mod->load(boost::dll::program_location().append(module_filename));
	}
	else
	{
		//std::cout << "Loading Go module from: " << module_filename << std::endl;
		mod->load(module_filename, boost::dll::load_mode::search_system_folders);
	}
	
	return mod;
}
}}