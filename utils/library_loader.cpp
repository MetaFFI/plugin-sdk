#include "library_loader.h"

namespace openffi { namespace utils
{
std::shared_ptr <boost::dll::shared_library> load_library(const std::string &library_name)
{
	// load library and add to "modules"
	std::string module_filename = library_name + boost::dll::shared_library::suffix().generic_string();
	
	std::shared_ptr <boost::dll::shared_library> mod = std::make_shared<boost::dll::shared_library>();
	
	// if plugin exists in the same path of the program, load it from there (mainly used for easier development)
	// otherwise, search system folders
	if (boost::filesystem::exists(boost::filesystem::current_path().append(module_filename)))
	{
		//std::cout << "Loading Go module from: " << boost::filesystem::current_path().append(plugin_filename) << std::endl;
		mod->load(boost::filesystem::current_path().append(module_filename));
	}
	else if (boost::filesystem::exists(boost::dll::program_location().append(module_filename)))
	{
		//std::cout << "Loading Go module from: " << boost::dll::program_location().append(module_filename) << std::endl;
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