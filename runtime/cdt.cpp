#include "cdt.h"

#ifdef __cplusplus
cdts::cdts(metaffi_size length, metaffi_int64 fixed_dimensions) : arr(nullptr), length(length), fixed_dimensions(fixed_dimensions), allocated_on_cache(0)
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
	arr[index].type = val.type;
	arr[index].free_required = val.free_required;
	arr[index].cdt_val = val.cdt_val;
	
	val.type = metaffi_null_type;
	val.free_required = 0;
	std::memset(&val.cdt_val, 0, sizeof(val.cdt_val));
}

void cdts::free()
{
	if(!arr)
	{
		return;
	}
	
	// if NOT allocated on cache (i.e. array)
	if(!this->allocated_on_cache){
		delete[] arr; // delete[] calls elements destructors
	}
	else // arr must not be freed - just free the elements.
	{
		for(metaffi_size i = 0; i < length; i++)
		{
			arr[i].~cdt(); // call destructor of each element
		}
	}
	
	arr = nullptr;
}

cdts::~cdts()
{
	free();
}

#endif