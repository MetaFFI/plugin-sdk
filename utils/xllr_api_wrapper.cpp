#include "xllr_api_wrapper.h"
#include <boost/filesystem.hpp>
#include "function_loader.hpp"

namespace metaffi{ namespace utils
{

//--------------------------------------------------------------------
xllr_api_wrapper::xllr_api_wrapper()
{
	try
	{
		std::string metaffi_home = std::getenv("METAFFI_HOME");
		if(metaffi_home.empty()){
			throw std::runtime_error("METAFFI_HOME environment variable is not set");
		}
		
		boost::filesystem::path xllr_fullname = boost::filesystem::path(metaffi_home).append(std::string("xllr")+boost::dll::shared_library::suffix().generic_string());
		
		this->xllr_mod = std::make_unique<boost::dll::shared_library>();
		
		this->xllr_mod->load( xllr_fullname );
		
		this->pload_runtime_plugin = load_func<void(const char*, uint32_t, char**, uint32_t*)>(*this->xllr_mod, "load_runtime_plugin");
		
		this->pfree_runtime_plugin = load_func<void(const char*, uint32_t, char**, uint32_t*)>(*this->xllr_mod, "free_runtime_plugin");
		
		this->pload_function = load_func<int64_t(const char*, uint32_t, const char*, uint32_t, int64_t, char**, uint32_t*)>(*this->xllr_mod, "load_function");
		this->pfree_function = load_func<void(const char*, uint32_t, int64_t, char**, uint32_t*)>(*this->xllr_mod, "free_function");
		
		this->pcall = load_func<void(const char*, uint32_t,
									int64_t,
									unsigned char*, uint64_t,
									unsigned char**, uint64_t*,
									unsigned char**, uint64_t*,
									uint8_t*)>(*this->xllr_mod, "call");
		
		this->pset_runtime_flag = load_func<void(const char*, uint64_t)>(*this->xllr_mod, "set_runtime_flag");
		this->pis_runtime_flag_set = load_func<int(const char*, uint64_t)>(*this->xllr_mod, "is_runtime_flag_set");
	}
	catch(std::exception& e)
	{
		printf("Error loading XLLR API wrapper: %s\n", e.what());
		std::exit(1);
	}
}
//--------------------------------------------------------------------
void xllr_api_wrapper::load_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len)
{
	*err = nullptr;
	*err_len = 0;
	(*this->pload_runtime_plugin)(runtime_plugin, runtime_plugin_len, err, err_len);
}
//--------------------------------------------------------------------
void xllr_api_wrapper::free_runtime_plugin(const char* runtime_plugin, uint32_t runtime_plugin_len, char** err, uint32_t* err_len)
{
	*err = nullptr;
	*err_len = 0;
	(*this->pfree_runtime_plugin)(runtime_plugin, runtime_plugin_len, err, err_len);
}
//--------------------------------------------------------------------
int64_t xllr_api_wrapper::load_function(const char* runtime_plugin, uint32_t runtime_plugin_len, const char* function_path, uint32_t function_path_len, int64_t function_id, char** err, uint32_t* err_len)
{
	*err = nullptr;
	*err_len = 0;
	return (*this->pload_function)(runtime_plugin, runtime_plugin_len,
	                             function_path, function_path_len,
			                      function_id,
			                      err, err_len);
}
//--------------------------------------------------------------------
void xllr_api_wrapper::free_function(const char* runtime_plugin, uint32_t runtime_plugin_len, int64_t function_id, char** err, uint32_t* err_len)
{
	*err = nullptr;
	*err_len = 0;
	(*this->pfree_function)(runtime_plugin, runtime_plugin_len,
	                      function_id,
	                      err, err_len);
}
//--------------------------------------------------------------------
void xllr_api_wrapper::call(const char* runtime_plugin, uint32_t runtime_plugin_len,
                            int64_t function_id,
                            unsigned char* in_params, uint64_t in_params_len,
                            unsigned char** out_params, uint64_t* out_params_len,
                            unsigned char** out_ret, uint64_t* out_ret_len,
                            uint8_t* out_is_error)
{
	*out_ret = nullptr;
	*out_ret_len = 0;
	*out_is_error = 0;
	
	(*this->pcall)(runtime_plugin, runtime_plugin_len,
	               function_id,
	               in_params, in_params_len,
	               out_params, out_params_len,
	               out_ret, out_ret_len,
	               out_is_error);
}
//--------------------------------------------------------------------
void xllr_api_wrapper::set_runtime_flag(const char* flag, uint64_t flag_len)
{
	(*this->pset_runtime_flag)(flag, flag_len);
}
//--------------------------------------------------------------------
bool xllr_api_wrapper::is_runtime_flag_set(const char* flag, uint64_t flag_len)
{
	return (*this->pis_runtime_flag_set)(flag, flag_len) != 0;
}
//--------------------------------------------------------------------

}}