#pragma once
#include <boost/dll.hpp>

namespace openffi{ namespace utils
{
//--------------------------------------------------------------------
class xllr_api_wrapper
{
private:
	std::unique_ptr<boost::dll::shared_library> xllr_mod;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, char**, uint32_t*)>::type> pload_runtime_plugin;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, char**, uint32_t*)>::type> pfree_runtime_plugin;
	
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, const char*, uint32_t, char**, uint32_t*)>::type> pload_module;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t, const char*, uint32_t, char**, uint32_t*)>::type> pfree_module;
	
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint32_t,
	                                                     const char*, uint32_t,
	                                                     const char*, uint32_t,
	                                                     unsigned char*, uint64_t,
	                                                     unsigned char**, uint64_t*,
	                                                     unsigned char**, uint64_t*,
	                                                     uint8_t*)>::type> pcall;

public:
	xllr_api_wrapper();
	~xllr_api_wrapper() = default;
	
	void load_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);
	void free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);
	void load_module(const char* runtime_plugin, uint32_t runtime_plugin_len, const char* module, uint32_t module_len, char** err, uint32_t* err_len);
	void free_module(const char* runtime_plugin, uint32_t runtime_plugin_len, const char* module, uint32_t module_len, char** err, uint32_t* err_len);
	void call(const char* runtime_plugin, uint32_t runtime_plugin_len,
	          const char* module, uint32_t module_len,
	          const char* func_name, uint32_t func_name_len,
	          unsigned char* in_params, uint64_t in_params_len,
	          unsigned char** out_params, uint64_t* out_params_len,
	          unsigned char** out_ret, uint64_t* out_ret_len,
	          uint8_t* out_is_error);
};
//--------------------------------------------------------------------
}}

// to make this class a headers-only
#include "xllr_api_wrapper.cpp"