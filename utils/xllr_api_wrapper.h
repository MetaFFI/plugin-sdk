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
	
	std::shared_ptr<boost::dll::detail::import_type<int64_t(const char*, uint32_t, const char*, uint32_t, int64_t, char**, uint32_t*)>::type> pload_function;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, int64_t, char**, uint32_t*)>::type> pfree_function;
	
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t,
	                                                     int64_t,
	                                                     unsigned char*, uint64_t,
	                                                     unsigned char**, uint64_t*,
	                                                     unsigned char**, uint64_t*,
	                                                     uint8_t*)>::type> pcall;
	
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint64_t)>::type> pset_runtime_flag;
	std::shared_ptr<boost::dll::detail::import_type<int(const char*, uint64_t)>::type> pis_runtime_flag_set;

public:
	xllr_api_wrapper();
	~xllr_api_wrapper() = default;
	
	void set_runtime_flag(const char* flag, uint64_t flag_len);
	bool is_runtime_flag_set(const char* flag, uint64_t flag_len);
	
	void load_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);
	void free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);
	int64_t load_function(const char* runtime_plugin, uint32_t runtime_plugin_len, const char* function_path, uint32_t function_path_len, int64_t function_id, char** err, uint32_t* err_len);
	void free_function(const char* runtime_plugin, uint32_t runtime_plugin_len, int64_t, char** err, uint32_t* err_len);
	void call(const char* runtime_plugin, uint32_t runtime_plugin_len,
	          int64_t function_id,
	          unsigned char* in_params, uint64_t in_params_len,
	          unsigned char** out_params, uint64_t* out_params_len,
	          unsigned char** out_ret, uint64_t* out_ret_len,
	          uint8_t* out_is_error);
};
//--------------------------------------------------------------------
}}
