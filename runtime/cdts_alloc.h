#pragma once
#include <cstring>

namespace metaffi::runtime
{
//--------------------------------------------------------------------
inline cdt* alloc_cdts_buffer(metaffi_size cdt_count)
{
	if (cdt_count <= 0)
	{
		return nullptr;
	}
	
	return (cdt*) calloc(sizeof(cdt), cdt_count);
}
//--------------------------------------------------------------------
}