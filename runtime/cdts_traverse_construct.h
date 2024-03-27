#pragma once
#include "cdt.h"


//--------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
struct traverse_cdts_callbacks
{
	void* context = nullptr;
	
	void (*on_float64)(const metaffi_size* index, metaffi_size index_size, metaffi_float64 val, void* context);
	void (*on_float32)(const metaffi_size* index, metaffi_size index_size, metaffi_float32 val, void* context);
	void (*on_int8)(const metaffi_size* index, metaffi_size index_size, metaffi_int8 val, void* context);
	void (*on_uint8)(const metaffi_size* index, metaffi_size index_size, metaffi_uint8 val, void* context);
	void (*on_int16)(const metaffi_size* index, metaffi_size index_size, metaffi_int16 val, void* context);
	void (*on_uint16)(const metaffi_size* index, metaffi_size index_size, metaffi_uint16 val, void* context);
	void (*on_int32)(const metaffi_size* index, metaffi_size index_size, metaffi_int32 val, void* context);
	void (*on_uint32)(const metaffi_size* index, metaffi_size index_size, metaffi_uint32 val, void* context);
	void (*on_int64)(const metaffi_size* index, metaffi_size index_size, metaffi_int64 val, void* context);
	void (*on_uint64)(const metaffi_size* index, metaffi_size index_size, metaffi_uint64 val, void* context);
	void (*on_bool)(const metaffi_size* index, metaffi_size index_size, metaffi_bool val, void* context);
	void (*on_char8)(const metaffi_size* index, metaffi_size index_size, metaffi_char8 val, void* context);
	void (*on_string8)(const metaffi_size* index, metaffi_size index_size, metaffi_string8 val, void* context);
	void (*on_char16)(const metaffi_size* index, metaffi_size index_size, metaffi_char16 val, void* context);
	void (*on_string16)(const metaffi_size* index, metaffi_size index_size, metaffi_string16 val, void* context);
	void (*on_char32)(const metaffi_size* index, metaffi_size index_size, metaffi_char32 val, void* context);
	void (*on_string32)(const metaffi_size* index, metaffi_size index_size, metaffi_string32 val, void* context);
	void (*on_handle)(const metaffi_size* index, metaffi_size index_size, const cdt_metaffi_handle& val, void* context);
	void (*on_callable)(const metaffi_size* index, metaffi_size index_size, const cdt_metaffi_callable& val, void* context);
	void (*on_null)(const metaffi_size* index, metaffi_size index_size, void* context);
	
	// if array dimensions are fixed, fixed_dimensions is the dimensions count of the array
	// if array has a common type to all of its elements, common_type is the type of the elements. Otherwise, 0.
	void (*on_array)(const metaffi_size* index, metaffi_size index_size, const cdts& val, metaffi_int64 fixed_dimensions, metaffi_type common_type, void* context);

#ifdef __cplusplus
	
	traverse_cdts_callbacks() = default;
	
	explicit traverse_cdts_callbacks(void* context) : context(context)
	{
		on_float64 = nullptr;
		on_float32 = nullptr;
		on_int8 = nullptr;
		on_uint8 = nullptr;
		on_int16 = nullptr;
		on_uint16 = nullptr;
		on_int32 = nullptr;
		on_uint32 = nullptr;
		on_int64 = nullptr;
		on_uint64 = nullptr;
		on_bool = nullptr;
		on_char8 = nullptr;
		on_string8 = nullptr;
		on_char16 = nullptr;
		on_string16 = nullptr;
		on_char32 = nullptr;
		on_string32 = nullptr;
		on_handle = nullptr;
		on_callable = nullptr;
		on_null = nullptr;
		on_array = nullptr;
	}
	
	traverse_cdts_callbacks(
			void* context,
			void (*on_float64)(const metaffi_size* index, metaffi_size index_size, metaffi_float64 val, void* context),
			void (*on_float32)(const metaffi_size* index, metaffi_size index_size, metaffi_float32 val, void* context),
			void (*on_int8)(const metaffi_size* index, metaffi_size index_size, metaffi_int8 val, void* context),
			void (*on_uint8)(const metaffi_size* index, metaffi_size index_size, metaffi_uint8 val, void* context),
			void (*on_int16)(const metaffi_size* index, metaffi_size index_size, metaffi_int16 val, void* context),
			void (*on_uint16)(const metaffi_size* index, metaffi_size index_size, metaffi_uint16 val, void* context),
			void (*on_int32)(const metaffi_size* index, metaffi_size index_size, metaffi_int32 val, void* context),
			void (*on_uint32)(const metaffi_size* index, metaffi_size index_size, metaffi_uint32 val, void* context),
			void (*on_int64)(const metaffi_size* index, metaffi_size index_size, metaffi_int64 val, void* context),
			void (*on_uint64)(const metaffi_size* index, metaffi_size index_size, metaffi_uint64 val, void* context),
			void (*on_bool)(const metaffi_size* index, metaffi_size index_size, metaffi_bool val, void* context),
			void (*on_char8)(const metaffi_size* index, metaffi_size index_size, metaffi_char8 val, void* context),
			void (*on_string8)(const metaffi_size* index, metaffi_size index_size, metaffi_string8 val, void* context),
			void (*on_char16)(const metaffi_size* index, metaffi_size index_size, metaffi_char16 val, void* context),
			void (*on_string16)(const metaffi_size* index, metaffi_size index_size, metaffi_string16 val, void* context),
			void (*on_char32)(const metaffi_size* index, metaffi_size index_size, metaffi_char32 val, void* context),
			void (*on_string32)(const metaffi_size* index, metaffi_size index_size, metaffi_string32 val, void* context),
			void (*on_handle)(const metaffi_size* index, metaffi_size index_size, const cdt_metaffi_handle& val, void* context),
			void (*on_callable)(const metaffi_size* index, metaffi_size index_size, const cdt_metaffi_callable& val, void* context),
			void (*on_null)(const metaffi_size* index, metaffi_size index_size, void* context),
			void (*on_array)(const metaffi_size* index, metaffi_size index_size, const cdts& val, metaffi_int64 fixed_dimensions, metaffi_type common_type, void* context)
	) : context(context),
	    on_float64(on_float64),
	    on_float32(on_float32),
	    on_int8(on_int8),
	    on_uint8(on_uint8),
	    on_int16(on_int16),
	    on_uint16(on_uint16),
	    on_int32(on_int32),
	    on_uint32(on_uint32),
	    on_int64(on_int64),
	    on_uint64(on_uint64),
	    on_bool(on_bool),
	    on_char8(on_char8),
	    on_string8(on_string8),
	    on_char16(on_char16),
	    on_string16(on_string16),
	    on_char32(on_char32),
	    on_string32(on_string32),
	    on_handle(on_handle),
	    on_callable(on_callable),
	    on_null(on_null),
	    on_array(on_array)
	{}
#endif // __cplusplus
};
#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
void traverse_cdts(const cdts& arr, const traverse_cdts_callbacks& callbacks);
void traverse_cdts(const cdts& arr, const traverse_cdts_callbacks& callbacks, const std::vector<metaffi_size>& starting_index);
void traverse_cdt(const cdt& item, const traverse_cdts_callbacks& callbacks);
void traverse_cdt(const cdt& item, const traverse_cdts_callbacks& callbacks, const std::vector<metaffi_size>& current_index);
#endif // __cplusplus
//--------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
struct construct_cdts_callbacks
{
	void* context;
	
	// if array dimensions is fixed, fill dimensions and return the size of the array
	// in_out_common_type is the type of the elements if all of them have the same type. Otherwise, 0.
	// in_out_common_type "in" value is extracted from metaffi_type_info. if it doesn't contain a type, it is 0.
	// in_out_common_type "out" value is the common type of the elements if they have the same type. Otherwise, 0.
	metaffi_size(*get_array)(const metaffi_size* index, metaffi_size index_length, metaffi_int64* out_fixed_dimensions, metaffi_type* in_out_common_type, void* context);
	
	metaffi_size (*get_root_elements_count)(void* context);
	metaffi_type_info (*get_type_info)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_float64 (*get_float64)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_float32 (*get_float32)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_int8 (*get_int8)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_uint8 (*get_uint8)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_int16 (*get_int16)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_uint16 (*get_uint16)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_int32 (*get_int32)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_uint32 (*get_uint32)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_int64 (*get_int64)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_uint64 (*get_uint64)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_bool (*get_bool)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_char8 (*get_char8)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_string8 (*get_string8)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_char16 (*get_char16)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_string16 (*get_string16)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_char32 (*get_char32)(const metaffi_size* index, metaffi_size index_size, void* context);
	metaffi_string32 (*get_string32)(const metaffi_size* index, metaffi_size index_size, void* context);
	cdt_metaffi_handle (*get_handle)(const metaffi_size* index, metaffi_size index_size, void* context);
	cdt_metaffi_callable (*get_callable)(const metaffi_size* index, metaffi_size index_size, void* context);

#ifdef __cplusplus
	
	construct_cdts_callbacks() = default;
	
	explicit construct_cdts_callbacks(void* context):context(context)
	{
		get_array = nullptr;
		get_root_elements_count = nullptr;
		get_type_info = nullptr;
		get_float64 = nullptr;
		get_float32 = nullptr;
		get_int8 = nullptr;
		get_uint8 = nullptr;
		get_int16 = nullptr;
		get_uint16 = nullptr;
		get_int32 = nullptr;
		get_uint32 = nullptr;
		get_int64 = nullptr;
		get_uint64 = nullptr;
		get_bool = nullptr;
		get_char8 = nullptr;
		get_string8 = nullptr;
		get_char16 = nullptr;
		get_string16 = nullptr;
		get_char32 = nullptr;
		get_string32 = nullptr;
		get_handle = nullptr;
		get_callable = nullptr;
	}
	
	construct_cdts_callbacks(
			void* context,
			metaffi_size(*get_array)(const metaffi_size* index, metaffi_size index_length, metaffi_int64* fixed_dimensions, metaffi_type* out_common_type, void* context),
			metaffi_size (*get_length)(void* context),
			metaffi_type_info (*get_type_info)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_float64 (*get_float64)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_float32 (*get_float32)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_int8 (*get_int8)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_uint8 (*get_uint8)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_int16 (*get_int16)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_uint16 (*get_uint16)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_int32 (*get_int32)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_uint32 (*get_uint32)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_int64 (*get_int64)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_uint64 (*get_uint64)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_bool (*get_bool)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_char8 (*get_char8)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_string8 (*get_string8)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_char16 (*get_char16)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_string16 (*get_string16)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_char32 (*get_char32)(const metaffi_size* index, metaffi_size index_size, void* context),
			metaffi_string32 (*get_string32)(const metaffi_size* index, metaffi_size index_size, void* context),
			cdt_metaffi_handle (*get_handle)(const metaffi_size* index, metaffi_size index_size, void* context),
			cdt_metaffi_callable (*get_callable)(const metaffi_size* index, metaffi_size index_size, void* context)
	) : context(context),
	    get_array(get_array),
	    get_root_elements_count(get_length),
	    get_type_info(get_type_info),
	    get_float64(get_float64),
	    get_float32(get_float32),
	    get_int8(get_int8),
	    get_uint8(get_uint8),
	    get_int16(get_int16),
	    get_uint16(get_uint16),
	    get_int32(get_int32),
	    get_uint32(get_uint32),
	    get_int64(get_int64),
	    get_uint64(get_uint64),
	    get_bool(get_bool),
	    get_char8(get_char8),
	    get_string8(get_string8),
	    get_char16(get_char16),
	    get_string16(get_string16),
	    get_char32(get_char32),
	    get_string32(get_string32),
	    get_handle(get_handle),
	    get_callable(get_callable)
	{}
	
#endif // __cplusplus
};
#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
void construct_cdts(cdts& arr, const construct_cdts_callbacks& callbacks);
void construct_cdts(cdts& arr, const construct_cdts_callbacks& callbacks, const std::vector<metaffi_size>& starting_index);
void construct_cdt(cdt& item, const construct_cdts_callbacks& callbacks);
void construct_cdt(cdt& item, const construct_cdts_callbacks& callbacks, const std::vector<metaffi_size>& current_index);
#endif // __cplusplus
//--------------------------------------------------------------------
