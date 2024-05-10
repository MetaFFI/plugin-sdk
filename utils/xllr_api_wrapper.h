#pragma once
#include <boost/dll.hpp>
#include <runtime/metaffi_primitives.h>

namespace metaffi{ namespace utils
{
//--------------------------------------------------------------------
class xllr_api_wrapper
{
private:
	std::unique_ptr<boost::dll::shared_library> xllr_mod;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, char**, uint32_t*)>::type> pload_runtime_plugin;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, char**, uint32_t*)>::type> pfree_runtime_plugin;
	
	std::shared_ptr<boost::dll::detail::import_type<void**(const char*, uint32_t, const char*, uint32_t, const char*, uint32_t, metaffi_type_info*, metaffi_type_info*, int8_t, int8_t, char**, uint32_t*)>::type> pload_function;
	std::shared_ptr<boost::dll::detail::import_type<void**(const char*, uint32_t, void*, metaffi_type_info*, metaffi_type_info*, int8_t, int8_t, char**, uint32_t*)>::type> pmake_callable;

	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, void*, char**, uint32_t*)>::type> pfree_function;
	
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint64_t)>::type> pset_runtime_flag;
	std::shared_ptr<boost::dll::detail::import_type<int(const char*, uint64_t)>::type> pis_runtime_flag_set;

public:
	xllr_api_wrapper();
	~xllr_api_wrapper() = default;
	
	void set_runtime_flag(const char* flag);
	bool is_runtime_flag_set(const char* flag);
	
	void load_runtime_plugin(const char* runtime_plugin, char** err);
	void free_runtime_plugin(const char* runtime_plugin, char** err);
	struct xcall* load_entity(const char* runtime_plugin, uint32_t runtime_plugin_len, const char* module_path, const char* function_path, struct metaffi_type_info* params_types, uint8_t params_count, struct metaffi_type_info* retvals_types, uint8_t retval_count, char** err);
	struct xcall* make_callable(const char* runtime_plugin, uint32_t runtime_plugin_len, void* make_callable_context, struct metaffi_type_info* params_types, uint8_t params_count, struct metaffi_type_info* retvals_types, uint8_t retval_count, char** err);
	void free_function(const char* runtime_plugin, uint32_t runtime_plugin_len, void*, char** err, uint32_t* err_len);
	
};
//--------------------------------------------------------------------
}}
