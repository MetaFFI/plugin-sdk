#pragma once
#include <boost/dll.hpp>

namespace metaffi::utils
{
//--------------------------------------------------------------------
template<typename T>
std::shared_ptr<typename boost::dll::detail::import_type<T>::type> load_func(const std::string& fullpath, const std::string& funcname, boost::dll::load_mode::type mode )
{

	return std::make_shared<typename boost::dll::detail::import_type<T>::type>(
#if BOOST_VERSION >= 107100
		boost::dll::import_symbol<T>(fullpath, funcname, mode)
#else
		boost::dll::import<T>(fullpath, funcname, mode);
#endif
	);
}
//--------------------------------------------------------------------
template<typename T>
std::shared_ptr<typename boost::dll::detail::import_type<T>::type> load_func(const boost::dll::shared_library& so, const std::string& funcname)
{
	try
	{
		return std::make_shared<typename boost::dll::detail::import_type<T>::type>(
#if BOOST_VERSION >= 107100
			    boost::dll::import_symbol<T>(so, funcname)
#else
			    boost::dll::import<T>(so, funcname)
#endif
		);
	}
	catch(const std::exception& e)
	{
		// catch exception, rethrow the exception and add function name and library name
		std::string msg = "Failed to load function " + funcname + " from " + so.location().string() + ". Error: " + e.what();
		throw std::runtime_error(msg);
	}
}
//--------------------------------------------------------------------
}