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

