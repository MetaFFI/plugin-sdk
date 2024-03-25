#pragma once
#include "metaffi_primitives.h"
#ifdef __cplusplus
#include <vector>
#include <string_view>

#endif

// It is important that the structs will hold a pointer to the primitives and NOT the primitives themselves.
// this is to avoid needless copy and the ability to pass by reference.

#define cdts_cache_size 50


struct cdt;

struct cdts
{
	struct cdt*     arr;
	metaffi_size    length;
	metaffi_bool    free_required;
	
	// if the dimensions are fixed, the fixed_dimensions holds the count of dimensions.
	// NOTICE, fixed_dimensions is NOT regarding the *length of the arrays*, but the count of the dimensions!
	//         i.e. ragged arrays can have fixed dimensions!
	// Examples:
	// [] -> fixed_dimensions = 1
	// [][] -> fixed_dimensions = 2
	// [][][] -> fixed_dimensions = 3
	// [] ... [] -> fixed_dimensions = N
	// { {1}, {2,3,4}, {5,6,7,8} } -> fixed_dimensions = 2
	
	// [int, [], int] -> fixed_dimensions = -1
	// [ [], [][] , [] ] -> fixed_dimensions = -1
	metaffi_int64   fixed_dimensions;

#ifdef __cplusplus
	cdts() : arr(nullptr), length(0), free_required(true), fixed_dimensions(1) {}
	cdts(metaffi_size length, metaffi_int64 fixed_dimensions);
	cdts(cdts&& other) noexcept
	{
		*this = std::move(other);
	}
	
	cdts& operator=(cdts&& other) noexcept
	{
		if(this != &other)
		{
			arr = other.arr;
			length = other.length;
			free_required = other.free_required;
			fixed_dimensions = other.fixed_dimensions;
			other.arr = nullptr;
		}
		return *this;
	}
	
	cdt* operator[](metaffi_size index) const;
	void free() const;
#endif
};

union cdt_types
{
	metaffi_float32 float32_val;
	metaffi_float64 float64_val;
	metaffi_int8 int8_val;
	metaffi_uint8 uint8_val;
	metaffi_int16 int16_val;
	metaffi_uint16 uint16_val;
	metaffi_int32 int32_val;
	metaffi_uint32 uint32_val;
	metaffi_int64 int64_val;
	metaffi_uint64 uint64_val;
	metaffi_bool bool_val;
	struct metaffi_char8 char8_val;
	metaffi_string8 string8_val;
	struct metaffi_char16 char16_val;
	metaffi_string16 string16_val;
	metaffi_char32 char32_val;
	metaffi_string32 string32_val;
	struct cdt_metaffi_handle handle_val;
	struct cdt_metaffi_callable callable_val;
	struct cdts array_val;

#ifdef __cplusplus
	cdt_types() : int64_val(0) {}
#endif
};

struct cdt
{
	metaffi_type type;
	union cdt_types cdt_val;
	metaffi_bool free_required;
	
#ifdef __cplusplus
	cdt() : type(metaffi_null_type), free_required(false) {}
	explicit cdt(metaffi_float32 val): type(metaffi_float32_type), free_required(false) { cdt_val.float32_val = val; }
	explicit cdt(metaffi_float64 val): type(metaffi_float64_type), free_required(false) { cdt_val.float64_val = val; }
	explicit cdt(metaffi_int8 val): type(metaffi_int8_type), free_required(false) { cdt_val.int8_val = val; }
	explicit cdt(metaffi_uint8 val): type(metaffi_uint8_type), free_required(false) { cdt_val.uint8_val = val; }
	explicit cdt(metaffi_int16 val): type(metaffi_int16_type), free_required(false) { cdt_val.int16_val = val; }
	explicit cdt(metaffi_uint16 val): type(metaffi_uint16_type), free_required(false) { cdt_val.uint16_val = val; }
	explicit cdt(metaffi_int32 val): type(metaffi_int32_type), free_required(false) { cdt_val.int32_val = val; }
	explicit cdt(metaffi_uint32 val): type(metaffi_uint32_type), free_required(false) { cdt_val.uint32_val = val; }
	explicit cdt(metaffi_int64 val): type(metaffi_int64_type), free_required(false) { cdt_val.int64_val = val; }
	explicit cdt(metaffi_uint64 val): type(metaffi_uint64_type), free_required(false) { cdt_val.uint64_val = val; }
	explicit cdt(bool val): type(metaffi_bool_type), free_required(false) { cdt_val.bool_val = val ? 1 : 0; }
	explicit cdt(metaffi_char8 val): type(metaffi_char8_type), free_required(false) { cdt_val.char8_val = val; }
	
	cdt(metaffi_string8 val, bool is_copy): type(metaffi_string8_type), free_required(true)
	{
		if(is_copy)
		{
			// calculate the length of val - a char8_t*
			std::u8string_view val_view(val);
			cdt_val.string8_val = new char8_t[val_view.size() + 1];
			
			// copy val, using val_view to cdt_val.string8_val
			std::memcpy(cdt_val.string8_val, val_view.data(), val_view.size());
			cdt_val.string8_val[val_view.size()] = 0;
		}
		else
		{
			cdt_val.string8_val = val;
		}
	}
	
	explicit cdt(metaffi_char16 val): type(metaffi_char16_type), free_required(false) { cdt_val.char16_val = val; }
	cdt(metaffi_string16 val, bool is_copy): type(metaffi_string16_type), free_required(true)
	{
		if(is_copy)
		{
			// calculate the length of val - a char16_t*
			std::u16string_view val_view(val);
			cdt_val.string16_val = new char16_t[val_view.size() + 1];
			
			// copy val, using val_view to cdt_val.string16_val
			std::memcpy(cdt_val.string16_val, val_view.data(), val_view.size());
			cdt_val.string16_val[val_view.size()] = 0;
		}
		else
		{
			cdt_val.string16_val = val;
		}
	}
	
	explicit cdt(metaffi_char32 val): type(metaffi_char32_type), free_required(false) { cdt_val.char32_val = val; }
	cdt(metaffi_string32 val, bool is_copy): type(metaffi_string32_type), free_required(true)
	{
		if(is_copy)
		{
			// calculate the length of val - a char32_t*
			std::u32string_view val_view(val);
			cdt_val.string32_val = new char32_t[val_view.size() + 1];
			
			// copy val, using val_view to cdt_val.string32_val
			std::memcpy(cdt_val.string32_val, val_view.data(), val_view.size());
			cdt_val.string32_val[val_view.size()] = 0;
		}
		else
		{
			cdt_val.string32_val = val;
		}
	}
	
	explicit cdt(const cdt_metaffi_handle& val): type(metaffi_handle_type), free_required(true) { cdt_val.handle_val = val; }
	explicit cdt(cdt_metaffi_callable&& val): type(metaffi_callable_type), free_required(true) { cdt_val.callable_val = std::move(val); }
	
	explicit cdt(cdts&& val): type(metaffi_array_type), free_required(true) { cdt_val.array_val = std::move(val); }
	cdt(metaffi_size length, metaffi_int64 fixed_dimensions): type(metaffi_array_type), free_required(true)
	{
		cdt_val.array_val.length = length;
		cdt_val.array_val.fixed_dimensions = fixed_dimensions;
		cdt_val.array_val.arr = new cdt[length]{};
	}
	
	explicit operator metaffi_float32() const { return cdt_val.float32_val; }
	explicit operator metaffi_float64() const { return cdt_val.float64_val; }
	explicit operator metaffi_int8() const { return cdt_val.int8_val; }
	explicit operator metaffi_uint8() const { return cdt_val.uint8_val; }
	explicit operator metaffi_int16() const { return cdt_val.int16_val; }
	explicit operator metaffi_uint16() const { return cdt_val.uint16_val; }
	explicit operator metaffi_int32() const { return cdt_val.int32_val; }
	explicit operator metaffi_uint32() const { return cdt_val.uint32_val; }
	explicit operator metaffi_int64() const { return cdt_val.int64_val; }
	explicit operator metaffi_uint64() const { return cdt_val.uint64_val; }
	explicit operator bool() const { return cdt_val.bool_val != 0; }
	explicit operator const metaffi_char8&() const { return cdt_val.char8_val; }
	explicit operator metaffi_string8() const { return cdt_val.string8_val; }
	explicit operator const metaffi_char16&() const { return cdt_val.char16_val; }
	explicit operator metaffi_string16() const { return cdt_val.string16_val; }
	explicit operator const metaffi_char32&() const { return cdt_val.char32_val; }
	explicit operator metaffi_string32() const { return cdt_val.string32_val; }
	explicit operator const cdt_metaffi_handle&() const { return cdt_val.handle_val; }
	explicit operator const cdt_metaffi_callable&() const { return cdt_val.callable_val; }
	
	~cdt()
	{
		if(free_required)
		{
			switch(type)
			{
				case metaffi_string8_type:
				{
					delete[] cdt_val.string8_val;
				}break;
				
				case metaffi_string16_type:
				{
					delete[] cdt_val.string16_val;
				}break;
				
				case metaffi_string32_type:
				{
					delete[] cdt_val.string32_val;
				}break;
				
				case metaffi_handle_type:
				{
					if(cdt_val.handle_val.release)
					{
						reinterpret_cast<cdt_metaffi_handle::release_fptr>(cdt_val.handle_val.release)(&cdt_val.handle_val);
					}
				}break;
				
				case metaffi_callable_type:
				{
					cdt_val.callable_val.free();
				}break;
				
				case metaffi_array_type:
				{
					cdt_val.array_val.free();
				}break;
			}
			
		}
	}
	
#endif // __cplusplus
};

