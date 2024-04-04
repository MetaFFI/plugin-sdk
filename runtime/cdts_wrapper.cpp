#include "cdts_wrapper.h"

namespace metaffi::runtime
{
	//--------------------------------------------------------------------
	cdt* cdts_wrapper::operator[](int index) const
	{
		return pcdts->arr + index;
	}
	//--------------------------------------------------------------------
	metaffi_size cdts_wrapper::elements_count() const
	{
		return pcdts->length;
	}
	//--------------------------------------------------------------------
}
