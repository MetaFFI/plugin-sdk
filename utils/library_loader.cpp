#include "library_loader.h"

namespace metaffi { namespace utils
{
std::shared_ptr<boost::dll::shared_library> load_library(const std::string& library_name, bool load_from_metaffi_home /*= true*/, bool append_extension /*= true*/)
{
	// first try to load from METAFFI_HOME
	if(load_from_metaffi_home)
	{
		std::string metaffi_home = std::getenv("METAFFI_HOME");
		if(metaffi_home.empty()){
			throw std::runtime_error("METAFFI_HOME environment variable is not set");
		}
		
		// prepend METAFFI_HOME directory to the file name
		boost::filesystem::path lib_in_metaffi_home(metaffi_home);
		lib_in_metaffi_home.append(append_extension ? library_name + boost::dll::shared_library::suffix().generic_string() : library_name);
		
		if(boost::filesystem::exists(lib_in_metaffi_home))
		{
			// load plugin
			std::shared_ptr<boost::dll::shared_library> plugin_dll = std::make_shared<boost::dll::shared_library>();
			plugin_dll->load(lib_in_metaffi_home, boost::dll::load_mode::rtld_now | boost::dll::load_mode::rtld_global );
			
			return plugin_dll;
		}
	}
	
	// load library from different locations
	std::string module_filename = append_extension ? library_name + boost::dll::shared_library::suffix().generic_string() : library_name;
	
	std::shared_ptr <boost::dll::shared_library> mod = std::make_shared<boost::dll::shared_library>();
	
	// if plugin exists in the same path of the program, load it from there (mainly used for easier development)
	// otherwise, search system folders
	if(boost::filesystem::exists(module_filename))
	{
		mod->load(module_filename);
	}
	else if (boost::filesystem::exists(boost::filesystem::current_path().append(module_filename)))
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