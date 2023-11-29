#pragma once
#include <boost/dll.hpp>

namespace metaffi{ namespace utils
{
//--------------------------------------------------------------------
class xllr_api_wrapper
{
private:
	std::unique_ptr<boost::dll::shared_library> xllr_mod;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, char**, uint32_t*)>::type> pload_runtime_plugin;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, char**, uint32_t*)>::type> pfree_runtime_plugin;
	
	std::shared_ptr<boost::dll::detail::import_type<void**(const char*, uint32_t, const char*, uint32_t, const char*, uint32_t, metaffi_types_ptr, metaffi_types_ptr, int8_t, int8_t, char**, uint32_t*)>::type> pload_function;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, void*, char**, uint32_t*)>::type> pfree_function;
	
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint64_t)>::type> pset_runtime_flag;
	std::shared_ptr<boost::dll::detail::import_type<int(const char*, uint64_t)>::type> pis_runtime_flag_set;

public:
	xllr_api_wrapper();
	~xllr_api_wrapper() = default;
	
	void set_runtime_flag(const char* flag, uint64_t flag_len);
	bool is_runtime_flag_set(const char* flag, uint64_t flag_len);
	
	void load_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);
	void free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);
	void** load_function(const char* runtime_plugin, uint32_t runtime_plugin_len, const char* module_path, uint32_t module_path_len, const char* function_path, uint32_t function_path_len, metaffi_types_ptr params_types, metaffi_types_ptr retvals_types, uint8_t params_count, uint8_t retval_count, char** err, uint32_t* err_len);
	void free_function(const char* runtime_plugin, uint32_t runtime_plugin_len, void*, char** err, uint32_t* err_len);
	
};
//--------------------------------------------------------------------
}}
