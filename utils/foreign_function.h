#pragma once

#include <boost/dll.hpp>

#define foreign_function_entrypoint_signature void(unsigned char*, uint64_t, unsigned char**, uint64_t*, unsigned char**, uint64_t*, uint8_t*)

typedef boost::dll::detail::library_function<foreign_function_entrypoint_signature> foreign_function_entrypoint;

#define function_path_entry_openffi_guest_lib "openffi_guest_lib"
#define function_path_class_entrypoint_function "entrypoint_class"
#define function_path_entry_entrypoint_function "entrypoint_function"
#define guest_package "openffi_guest."