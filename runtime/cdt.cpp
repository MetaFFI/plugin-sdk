#include "cdt.h"

#ifdef __cplusplus
cdts::cdts(metaffi_size length, metaffi_int64 fixed_dimensions) : arr(nullptr), length(length), fixed_dimensions(fixed_dimensions)
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

void cdts::set(metaffi_size index, cdt&& val) const
{
	arr[index] = std::move(val);
}

void cdts::free()
{
	delete[] arr;
	arr = nullptr;
}

cdts::~cdts()
{
	free();
}

#endif