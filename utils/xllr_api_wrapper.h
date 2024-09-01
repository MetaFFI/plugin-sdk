#pragma once
#include <boost/dll.hpp>
#include <runtime/metaffi_primitives.h>
#include <runtime/xcall.h>

namespace metaffi{ namespace utils
{
//--------------------------------------------------------------------
class xllr_api_wrapper
{
private:
	std::unique_ptr<boost::dll::shared_library> xllr_mod;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, char**)>::type> pload_runtime_plugin;
	std::shared_ptr<boost::dll::detail::import_type<void(const char*, char**)>::type> pfree_runtime_plugin;
	
	std::shared_ptr<boost::dll::detail::import_type<xcall*(const char*, const char*, const char*, metaffi_type_info*, int8_t, metaffi_type_info*, int8_t, char**)>::type> pload_entity;
	std::shared_ptr<boost::dll::detail::import_type<xcall*(const char*, void*, metaffi_type_info*, int8_t, metaffi_type_info*, int8_t, char**)>::type> pmake_callable;

	std::shared_ptr<boost::dll::detail::import_type<void(const char*, xcall*, char**)>::type> pfree_xcall;
	
	std::shared_ptr<boost::dll::detail::import_type<void(const char*)>::type> pset_runtime_flag;
	std::shared_ptr<boost::dll::detail::import_type<int(const char*)>::type> pis_runtime_flag_set;

public:
	xllr_api_wrapper();
	~xllr_api_wrapper() = default;
	
	void set_runtime_flag(const char* flag);
	bool is_runtime_flag_set(const char* flag);
	
	void load_runtime_plugin(const char* runtime_plugin, char** err);
	void free_runtime_plugin(const char* runtime_plugin, char** err);
	struct xcall* load_entity(const char* runtime_plugin, const char* module_path, const char* entity_path, struct metaffi_type_info* params_types, int8_t params_count, struct metaffi_type_info* retvals_types, int8_t retval_count, char** err);
	struct xcall* make_callable(const char* runtime_plugin, void* make_callable_context, struct metaffi_type_info* params_types, int8_t params_count, struct metaffi_type_info* retvals_types, int8_t retval_count, char** err);
	void free_xcall(const char* runtime_plugin, xcall*, char** err);
	
};
//--------------------------------------------------------------------
}}
