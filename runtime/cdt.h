#ifndef CDT_H
#define CDT_H


#pragma once
#include "metaffi_primitives.h"
#ifdef __cplusplus
#include <vector>
#include <string_view>
#include <sstream>

#endif

// It is important that the structs will hold a pointer to the primitives and NOT the primitives themselves.
// this is to avoid needless copy and the ability to pass by reference.

#define cdt_cache_size 100
#define cdts_cache_size 50

struct cdt;

struct cdts
{
	struct cdt*     arr;
	metaffi_size    length;
	
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
	
	// if the array is allocated on the cache, this flag is set to true.
	// It means the cleanup must not delete the array itself, but only the elements.
	metaffi_bool allocated_on_cache;

#ifdef __cplusplus
	cdts() : arr(nullptr), length(0), allocated_on_cache(0), fixed_dimensions(1) {}
	cdts(cdt* pre_allocated_cdts, metaffi_size length, metaffi_int64 fixed_dimensions = MIXED_OR_UNKNOWN_DIMENSIONS, metaffi_bool allocated_on_cache = 0) : arr(pre_allocated_cdts), length(length), fixed_dimensions(fixed_dimensions), allocated_on_cache(0) {}
	explicit cdts(metaffi_size length, metaffi_int64 fixed_dimensions = MIXED_OR_UNKNOWN_DIMENSIONS);
	cdts(cdts&& other) noexcept : arr(other.arr), length(other.length), fixed_dimensions(other.fixed_dimensions), allocated_on_cache(other.allocated_on_cache)
	{
		other.arr = nullptr;
	}
	
	~cdts();
	
	cdts& operator=(cdts&& other) noexcept
	{
		if(this != &other)
		{
			arr = other.arr;
			length = other.length;
			fixed_dimensions = other.fixed_dimensions;
			allocated_on_cache = other.allocated_on_cache;
			
			other.arr = nullptr;
			other.length = 0;
			other.fixed_dimensions = 0;
			other.allocated_on_cache = 0;
		}
		return *this;
	}
	
	[[nodiscard]] cdt& operator[](metaffi_size index) const;
	[[nodiscard]] cdt& at(metaffi_size index) const;
	void set(metaffi_size index, cdt&& val) const;
	void free();
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
	struct metaffi_char32 char32_val;
	metaffi_string32 string32_val;
	struct cdt_metaffi_handle handle_val;
	struct cdt_metaffi_callable* callable_val;
	struct cdts* array_val;

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
	cdt() : type(metaffi_null_type), free_required(false), cdt_val(){}
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
	
	cdt(const char8_t* val, bool is_copy): type(metaffi_string8_type), free_required(true)
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
			cdt_val.string8_val = (metaffi_string8)val;
		}
	}
	
	explicit cdt(const std::u8string_view& val): type(metaffi_string8_type), free_required(true)
	{
		// calculate the length of val - a char8_t*
		cdt_val.string8_val = new char8_t[val.size() + 1];
		
		// copy val, using val_view to cdt_val.string8_val
		std::memcpy(cdt_val.string8_val, val.data(), val.size());
		cdt_val.string8_val[val.size()] = 0;
	}
	
	explicit cdt(metaffi_char16 val): type(metaffi_char16_type), free_required(false) { cdt_val.char16_val = val; }
	cdt(const char16_t* val, bool is_copy): type(metaffi_string16_type), free_required(true)
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
			cdt_val.string16_val = (metaffi_string16)val;
		}
	}
	
	explicit cdt(const std::u16string_view val): type(metaffi_string16_type), free_required(true)
	{
		// calculate the length of val - a char16_t*
		cdt_val.string16_val = new char16_t[val.size() + 1];
		
		// copy val, using val_view to cdt_val.string16_val
		std::memcpy(cdt_val.string16_val, val.data(), val.size());
		cdt_val.string16_val[val.size()] = 0;
	}
	
	explicit cdt(metaffi_char32 val): type(metaffi_char32_type), free_required(false) { cdt_val.char32_val = val; }
	cdt(const char32_t* val, bool is_copy): type(metaffi_string32_type), free_required(true)
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
			cdt_val.string32_val = (metaffi_string32)val;
		}
	}
	
	explicit cdt(const std::u32string_view val): type(metaffi_string32_type), free_required(true)
	{
		// calculate the length of val - a char32_t*
		cdt_val.string32_val = new char32_t[val.size() + 1];
		
		// copy val, using val_view to cdt_val.string32_val
		std::memcpy(cdt_val.string32_val, val.data(), val.size());
		cdt_val.string32_val[val.size()] = 0;
	}
	
	explicit cdt(const cdt_metaffi_handle& val): type(metaffi_handle_type), free_required(true) { cdt_val.handle_val = val; }
	explicit cdt(cdt_metaffi_callable&& val): type(metaffi_callable_type), free_required(true) { cdt_val.callable_val = &val; }
	
	explicit cdt(cdts&& val): type(metaffi_array_type), free_required(true) { cdt_val.array_val = &val; }
	cdt(metaffi_size length, metaffi_int64 fixed_dimensions, metaffi_type common_type = metaffi_any_type): type(metaffi_array_type | common_type), free_required(true)
	{
		cdt_val.array_val->length = length;
		cdt_val.array_val->fixed_dimensions = fixed_dimensions;
		cdt_val.array_val->arr = new cdt[length]{};
	}
	
	cdt& operator=(cdt&& other)
	{
		if(this != &other)
		{
			type = other.type;
			free_required = other.free_required;
			
			// based on the type, move the cdt_val to this
			metaffi_type type_to_check = type & metaffi_array_type ? metaffi_array_type : type;
			
			switch(type_to_check)
			{
				case metaffi_float32_type:
				{
					cdt_val.float32_val = other.cdt_val.float32_val;
				}break;
				
				case metaffi_float64_type:
				{
					cdt_val.float64_val = other.cdt_val.float64_val;
				}break;
				
				case metaffi_int8_type:
				{
					cdt_val.int8_val = other.cdt_val.int8_val;
				}break;
				
				case metaffi_uint8_type:
				{
					cdt_val.uint8_val = other.cdt_val.uint8_val;
				}break;
				
				case metaffi_int16_type:
				{
					cdt_val.int16_val = other.cdt_val.int16_val;
				}break;
				
				case metaffi_uint16_type:
				{
					cdt_val.uint16_val = other.cdt_val.uint16_val;
				}break;
				
				case metaffi_int32_type:
				{
					cdt_val.int32_val = other.cdt_val.int32_val;
				}break;
				
				case metaffi_uint32_type:
				{
					cdt_val.uint32_val = other.cdt_val.uint32_val;
				}break;
				
				case metaffi_int64_type:
				{
					cdt_val.int64_val = other.cdt_val.int64_val;
				}break;
				
				case metaffi_uint64_type:
				{
					cdt_val.uint64_val = other.cdt_val.uint64_val;
				}break;
				
				case metaffi_bool_type:
				{
					cdt_val.bool_val = other.cdt_val.bool_val;
				}break;
				
				case metaffi_char8_type:
				{
					cdt_val.char8_val = other.cdt_val.char8_val;
				}break;
				
				case metaffi_string8_type:
				{
					cdt_val.string8_val = other.cdt_val.string8_val;
					other.cdt_val.string8_val = nullptr;
				}break;
				
				case metaffi_char16_type:
				{
					cdt_val.char16_val = other.cdt_val.char16_val;
				}break;
				
				case metaffi_string16_type:
				{
					cdt_val.string16_val = other.cdt_val.string16_val;
					other.cdt_val.string16_val = nullptr;
				}break;
				
				case metaffi_char32_type:
				{
					cdt_val.char32_val = other.cdt_val.char32_val;
				}break;
				
				case metaffi_string32_type:
				{
					cdt_val.string32_val = other.cdt_val.string32_val;
					other.cdt_val.string32_val = nullptr;
				}break;
				
				case metaffi_handle_type:
				{
					cdt_val.handle_val = other.cdt_val.handle_val;
				}break;
				
				case metaffi_callable_type:
				{
					cdt_val.callable_val = std::move(other.cdt_val.callable_val);
				}break;
				
				case metaffi_array_type:
				{
					cdt_val.array_val = std::move(other.cdt_val.array_val);
				}break;
				
				default:
				{
					std::stringstream ss;
					ss << "Invalid type: " << type;
					throw std::runtime_error(ss.str());
				}
			}
			
			other.type = metaffi_null_type;
			other.free_required = false;
		}
		return *this;
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
	
	explicit operator const cdts&() const
	{
		if(cdt_val.array_val == nullptr)
		{
			throw std::runtime_error("Array is null");
		}
		
		return *cdt_val.array_val;
	}
	
	explicit operator const cdt_metaffi_callable&() const
	{
		if(cdt_val.array_val == nullptr)
		{
			throw std::runtime_error("Callable is null");
		}
		
		return *cdt_val.callable_val;
	}
	
	~cdt()
	{
		if(free_required)
		{
			if(type & metaffi_array_type)
			{
				delete cdt_val.array_val;
			}
			else
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
						delete cdt_val.callable_val;
					}break;
				
				}
			}
		}
	}
	
#endif // __cplusplus
};

#endif // CDT_H