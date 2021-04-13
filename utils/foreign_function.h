#pragma once

#include <boost/dll.hpp>

#define foreign_function_entrypoint_signature void(unsigned char*, uint64_t, unsigned char**, uint64_t*, unsigned char**, uint64_t*, uint8_t*)

typedef boost::dll::detail::library_function<foreign_function_entrypoint_signature> foreign_function_entrypoint;