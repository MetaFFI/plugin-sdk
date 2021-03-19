#include "xllr_api_wrapper.h"
#include <boost/filesystem.hpp>
#include "function_loader.hpp"

namespace openffi{ namespace utils
{

//--------------------------------------------------------------------
xllr_api_wrapper::xllr_api_wrapper()
{
	try
	{
		std::string openffi_home = std::getenv("OPENFFI_HOME");
		if(openffi_home.empty()){
			throw std::runtime_error("OPENFFI_HOME environment variable is not set");
		}
		
		boost::filesystem::path xllr_fullname = boost::filesystem::path(openffi_home).append(std::string("xllr")+boost::dll::shared_library::suffix().generic_string());
		
		this->xllr_mod = std::make_unique<boost::dll::shared_library>();
		
		this->xllr_mod->load( xllr_fullname );
		
		this->pload_runtime_plugin = load_func<void(const char*, uint32_t, char**, uint32_t*)>(*this->xllr_mod, "load_runtime_plugin");
		
		this->pfree_runtime_plugin = load_func<void(const char*, uint32_t, char**, uint32_t*)>(*this->xllr_mod, "free_runtime_plugin");
		
		this->pload_module = load_func<void(const char*, uint32_t, const char*, uint32_t, char**, uint32_t*)>(*this->xllr_mod, "load_module");
		this->pfree_module = load_func<void(const char*, uint32_t, const char*, uint32_t, char**, uint32_t*)>(*this->xllr_mod, "free_module");
		
		this->pcall = load_func<void(const char*, uint32_t,
		const char*, uint32_t,
		const char*, uint32_t,
				unsigned char*, uint64_t,
				unsigned char**, uint64_t*,
				unsigned char**, uint64_t*,
				uint8_t*)>(*this->xllr_mod, "call");
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
void xllr_api_wrapper::load_module(const char* runtime_plugin, uint32_t runtime_plugin_len, const char* module, uint32_t module_len, char** err, uint32_t* err_len)
{
	*err = nullptr;
	*err_len = 0;
	(*this->pload_module)(runtime_plugin, runtime_plugin_len,
	                      module, module_len,
	                      err, err_len);
}
//--------------------------------------------------------------------
void xllr_api_wrapper::free_module(const char* runtime_plugin, uint32_t runtime_plugin_len, const char* module, uint32_t module_len, char** err, uint32_t* err_len)
{
	*err = nullptr;
	*err_len = 0;
	(*this->pfree_module)(runtime_plugin, runtime_plugin_len,
	                      module, module_len,
	                      err, err_len);
}
//--------------------------------------------------------------------
void xllr_api_wrapper::call(const char* runtime_plugin, uint32_t runtime_plugin_len,
                            const char* module, uint32_t module_len,
                            const char* func_name, uint32_t func_name_len,
                            unsigned char* in_params, uint64_t in_params_len,
                            unsigned char** out_params, uint64_t* out_params_len,
                            unsigned char** out_ret, uint64_t* out_ret_len,
                            uint8_t* out_is_error)
{
	*out_ret = nullptr;
	*out_ret_len = 0;
	*out_is_error = 0;
	
	(*this->pcall)(runtime_plugin, runtime_plugin_len,
	               module, module_len,
	               func_name, func_name_len,
	               in_params, in_params_len,
	               out_params, out_params_len,
	               out_ret, out_ret_len,
	               out_is_error);
}
//--------------------------------------------------------------------

}}