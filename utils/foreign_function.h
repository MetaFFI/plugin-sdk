#pragma once

#ifdef __cplusplus
#include <boost/dll.hpp>
#endif

#include "../runtime/cdt.h"

#define no_params_no_ret_types void*,char**,uint64_t*
#define no_params_params_or_ret_types void*,cdts[1],char**,uint64_t*
#define no_params_params_and_ret_types void*,cdts[2],char**,uint64_t*

#define foreign_function_entrypoint_signature_no_params_no_ret void(no_params_no_ret_types)
#define foreign_function_entrypoint_signature_params_no_ret void(no_params_params_or_ret_types)
#define foreign_function_entrypoint_signature_no_params_ret void(no_params_params_or_ret_types)
#define foreign_function_entrypoint_signature_params_ret void(no_params_params_and_ret_types)

#define pforeign_function_entrypoint_signature_no_params_no_ret void(*)(no_params_no_ret_types)
#define pforeign_function_entrypoint_signature_params_no_ret void(*)(no_params_params_or_ret_types)
#define pforeign_function_entrypoint_signature_no_params_ret void(*)(no_params_params_or_ret_types)
#define pforeign_function_entrypoint_signature_params_ret void(*)(no_params_params_and_ret_types)

typedef void(*pforeign_function_entrypoint_signature_no_params_no_ret_t)(no_params_no_ret_types);
typedef void(*pforeign_function_entrypoint_signature_params_no_ret_t)(no_params_params_or_ret_types);
typedef void(*pforeign_function_entrypoint_signature_no_params_ret_t)(no_params_params_or_ret_types);
typedef void(*pforeign_function_entrypoint_signature_params_ret_t)(no_params_params_and_ret_types);

#ifdef __cplusplus
typedef std::function<void(no_params_params_and_ret_types)> foreign_function_entrypoint_signature_params_ret_t;
typedef std::function<void(no_params_params_or_ret_types)> foreign_function_entrypoint_signature_no_params_ret_t;
typedef std::function<void(no_params_params_or_ret_types)> foreign_function_entrypoint_signature_params_no_ret_t;
typedef std::function<void(no_params_no_ret_types)> foreign_function_entrypoint_signature_no_params_no_ret_t;

typedef boost::dll::detail::library_function<foreign_function_entrypoint_signature_no_params_no_ret> foreign_function_no_params_no_ret_entrypoint;
typedef boost::dll::detail::library_function<foreign_function_entrypoint_signature_params_no_ret> foreign_function_params_no_ret_entrypoint;
typedef boost::dll::detail::library_function<foreign_function_entrypoint_signature_no_params_ret> foreign_function_no_params_ret_entrypoint;
typedef boost::dll::detail::library_function<foreign_function_entrypoint_signature_params_ret> foreign_function_params_ret_entrypoint;
#endif

#define function_path_entry_metaffi_guest_lib "metaffi_guest_lib"
#define function_path_entrypoint_class "entrypoint_class"
#define function_path_entry_entrypoint_function "entrypoint_function"
#define guest_package "metaffi_guest."
#define function_path_entry_parameters "params"
#define function_path_entry_rets "rets"