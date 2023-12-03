#include "cdts_wrapper.h"
#include <mutex>


// NOLINT(bugprone-macro-parentheses)
bool capi_loaded = false;
const char* capi_loader_err = nullptr;
struct capi_loader
{
	capi_loader()
	{
		capi_loader_err = load_cdt_capi();
		if(capi_loader_err)
		{
			printf("FATAL ERROR! Failed to load CDT C-API. Error: %s\n", capi_loader_err);
		}
		
		capi_loaded = true;
	}
};
static capi_loader _l; // load statically the CDTS CAPI-API


namespace metaffi::runtime
{
//--------------------------------------------------------------------
cdts_wrapper::cdts_wrapper(cdt* cdts, metaffi_size cdts_length, bool is_free_cdts /*= false*/):cdts(cdts),cdts_length(cdts_length), is_free_cdts(is_free_cdts)
{
	if(!capi_loaded)
	{
		throw std::runtime_error("Failed to load CDT C-API");
	}
}
//--------------------------------------------------------------------
cdts_wrapper::~cdts_wrapper()
{
	//if(is_free_cdts){ free(this->cdts); }
}
//--------------------------------------------------------------------
cdt* cdts_wrapper::operator[](int index) const
{
	if(index >= this->cdts_length)
	{
		throw std::runtime_error("operator[] requested index is out of bounds");
	}
	
	return &this->cdts[index];
}
//--------------------------------------------------------------------
cdt* cdts_wrapper::get_cdts() const
{
	return this->cdts;
}
//--------------------------------------------------------------------
metaffi_size cdts_wrapper::get_cdts_length() const
{
	return this->cdts_length;
}
//--------------------------------------------------------------------
	
	
	
metaffi_type_with_alias make_type_with_alias(metaffi_type type, const std::string& alias)
{
	metaffi_type_with_alias inst = {0};
	inst.type = type;
	
	if(!alias.empty())
	{
		inst.alias = (char*)malloc(alias.size());
		std::copy(alias.begin(), alias.end(), inst.alias);
		inst.alias_length = (int64_t)alias.length();
	}
	
	return inst;
}
//--------------------------------------------------------------------
}
