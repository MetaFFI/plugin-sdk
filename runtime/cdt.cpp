#include "cdt.h"

#ifdef __cplusplus
cdts::cdts(metaffi_size length, metaffi_int64 fixed_dimensions) : arr(nullptr), length(length), free_required(true), fixed_dimensions(fixed_dimensions)
{
	if(length > 0)
	{
		arr = new cdt[length]{};
	}
}

cdt& cdts::operator[](metaffi_size index) const
{
	return arr[index];
}

cdt& cdts::at(metaffi_size index) const
{
	return arr[index];
}

void cdts::set(metaffi_size index, cdt&& val)
{
	arr[index] = std::move(val);
}

void cdts::free() const
{
	if(free_required)
	{
		delete[] arr;
	}
}
#endif