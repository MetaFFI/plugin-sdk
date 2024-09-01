#include "xllr_api_wrapper.h"
#include <filesystem>
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
		
		this->pload_runtime_plugin = load_func<void(const char*, char**)>(*this->xllr_mod, "load_runtime_plugin");
		
		this->pfree_runtime_plugin = load_func<void(const char*, char**)>(*this->xllr_mod, "free_runtime_plugin");
		
		this->pload_entity = load_func<xcall*(const char*, const char*, const char*, metaffi_type_info*, int8_t, metaffi_type_info*, int8_t, char**)>(*this->xllr_mod, "load_entity");
		this->pfree_xcall = load_func<void(const char*, xcall*, char**)>(*this->xllr_mod, "free_xcall");
		this->pmake_callable = load_func<xcall*(const char*, void*, metaffi_type_info*, int8_t, metaffi_type_info*, int8_t, char**)>(*this->xllr_mod, "make_callable");
		
		this->pset_runtime_flag = load_func<void(const char*)>(*this->xllr_mod, "set_runtime_flag");
		this->pis_runtime_flag_set = load_func<int(const char*)>(*this->xllr_mod, "is_runtime_flag_set");
	}
	catch(std::exception& e)
	{
		printf("Error loading XLLR API wrapper: %s\n", e.what());
		std::exit(1);
	}
}
//--------------------------------------------------------------------
void xllr_api_wrapper::load_runtime_plugin(const char* runtime_plugin, char** err)
{
	*err = nullptr;
	(*this->pload_runtime_plugin)(runtime_plugin, err);
}
//--------------------------------------------------------------------
void xllr_api_wrapper::free_runtime_plugin(const char* runtime_plugin, char** err)
{
	*err = nullptr;
	(*this->pfree_runtime_plugin)(runtime_plugin, err);
}
//--------------------------------------------------------------------
xcall* xllr_api_wrapper::load_entity(const char* runtime_plugin, const char* module_path, const char* entity_path, metaffi_type_info* params_types, int8_t params_count, metaffi_type_info* retvals_types, int8_t retval_count, char** err)
{
	*err = nullptr;
	return (*this->pload_entity)(runtime_plugin,
	                                module_path,
	                                entity_path,
	                                params_types, params_count, retvals_types, retval_count,
			                        err);
}
//--------------------------------------------------------------------
xcall* xllr_api_wrapper::make_callable(const char* runtime_plugin, void* make_callable_context, metaffi_type_info* params_types, int8_t params_count, metaffi_type_info* retvals_types, int8_t retval_count, char** err)
{
	*err = nullptr;
	return (*this->pmake_callable)(runtime_plugin,
									make_callable_context,
									params_types, params_count, retvals_types, retval_count,
									err);
}
//--------------------------------------------------------------------
void xllr_api_wrapper::free_xcall(const char* runtime_plugin, xcall* pxcall, char** err)
{
	*err = nullptr;
	(*this->pfree_xcall)(runtime_plugin, pxcall, err);
}
//--------------------------------------------------------------------
void xllr_api_wrapper::set_runtime_flag(const char* flag)
{
	(*this->pset_runtime_flag)(flag);
}
//--------------------------------------------------------------------
bool xllr_api_wrapper::is_runtime_flag_set(const char* flag)
{
	return (*this->pis_runtime_flag_set)(flag) != 0;
}
//--------------------------------------------------------------------

}}