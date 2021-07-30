#pragma once

#include <boost/dll.hpp>
#include "../runtime/cdt_structs.h"

#define foreign_function_entrypoint_signature void(cdt*, uint64_t,    \
                                                   cdt*, uint64_t,    \
                                                   char**, uint64_t*)

typedef boost::dll::detail::library_function<foreign_function_entrypoint_signature> foreign_function_entrypoint;

#define function_path_entry_metaffi_guest_lib "metaffi_guest_lib"
#define function_path_class_entrypoint_function "entrypoint_class"
#define function_path_entry_entrypoint_function "entrypoint_function"
#define guest_package "metaffi_guest."