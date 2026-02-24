#pragma once

#include <boost/dll.hpp>
#include <memory>
#include <string>
#include <sstream>
#include <filesystem>
#include <vector>
#include <utils/env_utils.h>
#include <utils/logger.hpp>
#if !defined(_WIN32)
#  include <dlfcn.h>
#endif

namespace fs = std::filesystem;

namespace metaffi::utils
{
inline std::vector<std::string> find_files_recursively(const fs::path& root, const std::string& filename, const std::string& extension)
{
    std::vector<std::string> matching_files;

    if (!fs::exists(root) || !fs::is_directory(root)) {
        auto logger = metaffi::get_logger("sdk.utils");
        METAFFI_ERROR(logger, "Path is not a directory or does not exist.");
        return matching_files;
    }

    for (const auto& entry : fs::recursive_directory_iterator(root)) 
    {
        if (entry.is_regular_file()) 
        {
            auto path = entry.path();
            if (path.stem() == filename && path.extension() == extension) 
            {
                matching_files.push_back(fs::absolute(path).string());
            }
        }
    }

    return matching_files;
}
//--------------------------------------------------------------------
inline std::shared_ptr<boost::dll::shared_library> load_plugin(const std::string& plugin_filename_without_extension)
{
	auto logger = metaffi::get_logger("sdk.utils");
	METAFFI_INFO(logger, "Loading plugin: {}", plugin_filename_without_extension);

	std::string metaffi_home = get_env_var("METAFFI_HOME");
	if(metaffi_home.empty()){
		throw std::runtime_error("METAFFI_HOME environment variable is not set");
	}

	auto plugin = find_files_recursively(metaffi_home, plugin_filename_without_extension, boost::dll::shared_library::suffix().generic_string());
	if(plugin.empty())
	{
		std::stringstream ss;
		ss << "Failed to find " << plugin_filename_without_extension << boost::dll::shared_library::suffix().generic_string() << " in " << metaffi_home;
		throw std::runtime_error(ss.str());
	}
	
	if(plugin.size() > 1)
	{
		std::stringstream ss;
		ss << "Found more than one " << plugin_filename_without_extension << boost::dll::shared_library::suffix().generic_string() << " in " << metaffi_home << std::endl;
		ss << "Specify the plugin name you want to use";
		throw std::runtime_error(ss.str());
	}

	// prepend METAFFI_HOME directory to the file name
	boost::filesystem::path plugin_full_path = plugin[0];

	// load plugin
	std::shared_ptr<boost::dll::shared_library> plugin_dll = std::make_shared<boost::dll::shared_library>();
	try
	{
		plugin_dll->load( plugin_full_path, boost::dll::load_mode::rtld_now | boost::dll::load_mode::rtld_global );

#if !defined(_WIN32) && defined(RTLD_NODELETE) && defined(RTLD_NOLOAD)
		// Apply RTLD_NODELETE so dlclose() never unmaps this plugin.
		// Language plugins (Go, JVM, Python) all have issues with premature unloading:
		// Go can't be safely unloaded, JVM hangs on DestroyJavaVM, Python corrupts
		// glibc's heap via Py_Finalize. RTLD_NOLOAD doesn't re-load the library —
		// it only upgrades the flags on the already-resident handle. The OS reclaims
		// memory cleanly at process exit.
		// boost::dll::load_mode::rtld_nodelete was added in Boost 1.76; using dlopen
		// directly avoids a compile error on older Boost versions.
		dlopen(plugin[0].c_str(), RTLD_LAZY | RTLD_GLOBAL | RTLD_NODELETE | RTLD_NOLOAD);
#endif
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
