#include "cdts_wrapper.h"
#include <mutex>


// NOLINT(bugprone-macro-parentheses)

struct capi_loader
{
	capi_loader()
	{
		const char* err = load_cdt_capi();
		if(err){
			throw std::runtime_error(err);
		}
	}
};
static capi_loader _l; // load statically the CDTS CAPI-API


namespace metaffi::runtime
{
//--------------------------------------------------------------------
cdts_wrapper::cdts_wrapper(cdt* cdts, metaffi_size cdts_length, bool is_free_cdts /*= false*/):cdts(cdts),cdts_length(cdts_length), is_free_cdts(is_free_cdts)
{
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
}