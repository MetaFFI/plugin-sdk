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
	
	std::shared_ptr<boost::dll::detail::import_type<void(int64_t, struct cdts[2], char**, uint64_t*)>::type> pxllr_xcall_params_ret;
	std::shared_ptr<boost::dll::detail::import_type<void(int64_t, struct cdts[1], char**, uint64_t*)>::type> pxllr_xcall_no_params_ret;
	std::shared_ptr<boost::dll::detail::import_type<void(int64_t, struct cdts[1], char**, uint64_t*)>::type> pxllr_xcall_params_no_ret;
	std::shared_ptr<boost::dll::detail::import_type<void(int64_t, char**, uint64_t*)>::type> pxllr_xcall_no_params_no_ret;
	
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, uint64_t)>::type> pset_runtime_flag;
	std::shared_ptr<boost::dll::detail::import_type<int(const char*, uint64_t)>::type> pis_runtime_flag_set;

public:
	xllr_api_wrapper();
	~xllr_api_wrapper() = default;
	
	void set_runtime_flag(const char* flag, uint64_t flag_len);
	bool is_runtime_flag_set(const char* flag, uint64_t flag_len);
	
	void load_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);
	void free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len);
	int64_t load_function(const char* runtime_plugin, uint32_t runtime_plugin_len, const char* function_path, uint32_t function_path_len, int64_t function_id, int8_t params_count, int8_t retval_count, char** err, uint32_t* err_len);
	void free_function(const char* runtime_plugin, uint32_t runtime_plugin_len, int64_t, char** err, uint32_t* err_len);
	
	void xllr_xcall_params_ret(int64_t function_id, struct cdts pcdts[2], char** out_err, uint64_t* out_err_len);
	void xllr_xcall_no_params_ret(int64_t function_id, struct cdts pcdts[1], char** out_err, uint64_t* out_err_len);
	void xllr_xcall_params_no_ret(int64_t function_id, struct cdts pcdts[1], char** out_err, uint64_t* out_err_len);
	void xllr_xcall_no_params_no_ret(int64_t function_id, char** out_err, uint64_t* out_err_len);
};
//--------------------------------------------------------------------
}}
