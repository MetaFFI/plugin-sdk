#pragma once
#include "cdt.h"
#include <sstream>
#include <functional>
#include <utility>
#include "xllr_capi_loader.h"
#include <queue>
#include <vector>

namespace metaffi::runtime
{
	
metaffi_type_info make_type_with_options(metaffi_type type, const std::string& alias = "", int dimensions = 0);

/************************************************
*   CDTS wrapper class
*************************************************/
	
class cdts_wrapper
{
private:
	cdts* pcdts = nullptr;

public:
	explicit cdts_wrapper(cdts* pcdts): pcdts(pcdts) {}
	cdt* operator [](int index) const;
	[[nodiscard]] metaffi_size elements_count() const;
};
	
//--------------------------------------------------------------------



}

