#pragma once

#ifdef __cplusplus
#include <boost/dll.hpp>
#endif

#include "../runtime/cdt_structs.h"

#define foreign_function_entrypoint_signature_no_params_no_ret void(char**, uint64_t*)
#define foreign_function_entrypoint_signature_params_no_ret void(cdts[1], char**, uint64_t*)
#define foreign_function_entrypoint_signature_no_params_ret void(cdts[1], char**, uint64_t*)
#define foreign_function_entrypoint_signature_params_ret void(cdts[2], char**, uint64_t*)

#define pforeign_function_entrypoint_signature_no_params_no_ret void(*)(char**, uint64_t*)
#define pforeign_function_entrypoint_signature_params_no_ret void(*)(cdts[1], char**, uint64_t*)
#define pforeign_function_entrypoint_signature_no_params_ret void(*)(cdts[1], char**, uint64_t*)
#define pforeign_function_entrypoint_signature_params_ret void(*)(cdts[2], char**, uint64_t*)

#define pforeign_function_entrypoint_signature_no_params_no_ret_cparam void(*)(void* params, char**, uint64_t*)
#define pforeign_function_entrypoint_signature_params_no_ret_cparam void(*)(cdts[1], void* params, char**, uint64_t*)
#define pforeign_function_entrypoint_signature_no_params_ret_cparam void(*)(cdts[1], void* params, char**, uint64_t*)
#define pforeign_function_entrypoint_signature_params_ret_cparam void(*)(cdts[2], void* params, char**, uint64_t*)



typedef void(*pforeign_function_entrypoint_signature_no_params_no_ret_t)(char**, uint64_t*);
typedef void(*pforeign_function_entrypoint_signature_params_no_ret_t)(cdts[1], char**, uint64_t*);
typedef void(*pforeign_function_entrypoint_signature_no_params_ret_t)(cdts[1], char**, uint64_t*);
typedef void(*pforeign_function_entrypoint_signature_params_ret_t)(cdts[2], char**, uint64_t*);

#define ppforeign_function_entrypoint_signature_no_params_no_ret void(**)(char**, uint64_t*)
#define ppforeign_function_entrypoint_signature_params_no_ret void(**)(cdts[1], char**, uint64_t*)
#define ppforeign_function_entrypoint_signature_no_params_ret void(**)(cdts[1], char**, uint64_t*)
#define ppforeign_function_entrypoint_signature_params_ret void(**)(cdts[2], char**, uint64_t*)

#ifdef __cplusplus
typedef std::function<void(cdts[2], char**, uint64_t*)> foreign_function_entrypoint_signature_params_ret_t;
typedef std::function<void(cdts[1], char**, uint64_t*)> foreign_function_entrypoint_signature_no_params_ret_t;
typedef std::function<void(cdts[1], char**, uint64_t*)> foreign_function_entrypoint_signature_params_no_ret_t;
typedef std::function<void(char**, uint64_t*)> foreign_function_entrypoint_signature_no_params_no_ret_t;

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