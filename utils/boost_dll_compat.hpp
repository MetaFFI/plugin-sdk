#pragma once

#include <boost/dll.hpp>
#include <type_traits>

namespace metaffi::utils
{
template<typename T, typename = void>
struct boost_dll_import
{
	using type = boost::dll::detail::import_type<T>;
};

template<typename T>
struct boost_dll_import<T, std::void_t<typename boost::dll::detail::import_type<T>::type>>
{
	using type = typename boost::dll::detail::import_type<T>::type;
};

template<typename T>
using boost_dll_import_t = typename boost_dll_import<T>::type;
}

